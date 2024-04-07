#include "vt100emulation.h"

#include "../StateMachine/terminalstatemachine.h"
#include "../terminalscreen.h"
#include <QIODevice>
#include <QRegularExpression>
#include <tlogger.h>

struct VT100EmulationPrivate {
        QIODevice* device;
        TerminalScreen* screen;

        bool echo = false;
        bool escapeMode = false;
        TerminalStateMachine escapeStateMachine;
        TerminalStateMachine csiStateMachine;

        bool autowrap = false;
        bool crlfMode = false;

        int savedCaretRow = 0;
        int savedCaretCol = 0;
        bool savedCaretAutowrap = false;
        TerminalScreen::CharacterSpace::CharacterFormat savedCaretFormat;
        QString** savedCharacterSet = &characterSetG0;

        QSet<int> tabStops;

        QString* characterSetG0 = nullptr;
        QString* characterSetG1 = nullptr;
        QString** currentCharacterSet = &characterSetG0;
        static QString decSpecialCharacterSet;
};
QString VT100EmulationPrivate::decSpecialCharacterSet = u" ◆▒␉␌␍␊°±␤␋┘┐┌└┼⎺⎻─⎼⎽├┤┴┬│≤≥π≠£·"_qs;

#include <QTimer>

VT100Emulation::VT100Emulation(QIODevice* device, TerminalScreen* screen, QObject* parent) :
    QObject{parent}, d{new VT100EmulationPrivate} {
    d->device = device;
    d->screen = screen;

    setupStateMachine();
    setupCsiStateMachine();

    connect(device, &QIODevice::readyRead, this, [this] {
        auto buf = d->device->readAll();
        for (auto character : buf) {
            processCharacter(character);
        }
    });
}

VT100Emulation::~VT100Emulation() {
    delete d;
}

void VT100Emulation::pressKey(Qt::KeyboardModifiers modifiers, Qt::Key key, QString keyChar) {
#ifdef Q_OS_MAC
    modifiers.setFlag(Qt::ControlModifier, modifiers & Qt::MetaModifier);
#endif

    if (key == Qt::Key_Return) {
        this->write("\r");
        if (d->crlfMode) {
            this->write("\n");
        }
        return;
    } else if (key == Qt::Key_Left) {
        this->write("\x1B[D");
        return;
    } else if (key == Qt::Key_Right) {
        this->write("\x1B[C");
        return;
    } else if (key == Qt::Key_Down) {
        this->write("\x1B[B");
        return;
    } else if (key == Qt::Key_Up) {
        this->write("\x1B[A");
        return;
    }

    this->write(keyChar);
}

void VT100Emulation::setupStateMachine() {
    auto initialState = d->escapeStateMachine.addState();

    auto csi = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(initialState, '[', csi);
    d->escapeStateMachine.addTransition(
        csi, [](QChar c) {
        return !(c.toLatin1() >= 0x40 && c.toLatin1() <= 0x7E);
    }, csi);

    auto csiEnd = d->escapeStateMachine.addFinalState(std::bind(&VT100Emulation::invokeCsi, this, std::placeholders::_1));
    d->escapeStateMachine.addTransition(
        csi, [](QChar c) {
        return c.toLatin1() >= 0x40 && c.toLatin1() <= 0x7E;
    }, csiEnd);

    auto osc = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(initialState, ']', osc);
    d->escapeStateMachine.addTransition(
        osc, [](QChar c) {
        return c.toLatin1() != '\x1B' && c.toLatin1() != '\x07';
    }, osc);

    auto oscEscEnd = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(osc, '\x1B', oscEscEnd);
    d->escapeStateMachine.addTransition(
        oscEscEnd, [](QChar c) {
        return c.toLatin1() != '\\';
    }, osc);

    auto oscEnd = d->escapeStateMachine.addFinalState(std::bind(&VT100Emulation::invokeOsc, this, std::placeholders::_1));
    d->escapeStateMachine.addTransition(oscEscEnd, '\\', oscEnd);
    d->escapeStateMachine.addTransition(osc, '\x07' /* BEL */, oscEnd);

    auto nextLine = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        echo('\n');
    });
    d->escapeStateMachine.addTransition(initialState, 'D', nextLine);

    auto nextLineCarriageReturn = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        echo('\r');
        echo('\n');
    });
    d->escapeStateMachine.addTransition(initialState, 'E', nextLineCarriageReturn);

    auto setTab = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->tabStops.insert(d->screen->caretCol());
    });
    d->escapeStateMachine.addTransition(initialState, 'H', setTab);

    auto previousLine = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setCaretRow(d->screen->caretRow() - 1);
    });
    d->escapeStateMachine.addTransition(initialState, 'M', previousLine);

    auto octothorpe = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(initialState, '#', octothorpe);

    auto doubleHeightUpper = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setRowScaleMode(d->screen->caretRow(), TerminalScreen::RowScaleMode::DoubleHeightUpper);
    });
    d->escapeStateMachine.addTransition(octothorpe, '3', doubleHeightUpper);

    auto doubleHeightLower = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setRowScaleMode(d->screen->caretRow(), TerminalScreen::RowScaleMode::DoubleHeightLower);
    });
    d->escapeStateMachine.addTransition(octothorpe, '4', doubleHeightLower);

    auto normalWidth = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setRowScaleMode(d->screen->caretRow(), TerminalScreen::RowScaleMode::Normal);
    });
    d->escapeStateMachine.addTransition(octothorpe, '5', normalWidth);

    auto doubleWidth = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setRowScaleMode(d->screen->caretRow(), TerminalScreen::RowScaleMode::DoubleWidth);
    });
    d->escapeStateMachine.addTransition(octothorpe, '6', doubleWidth);

    auto alignmentPattern = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        // Fill the screen with Es
        for (auto i = 0; i < d->screen->rows(); i++) {
            for (auto j = 0; j < d->screen->cols(); j++) {
                d->screen->setCharacter(j, i, 'E');
            }
        }
    });
    d->escapeStateMachine.addTransition(octothorpe, '8', alignmentPattern);

    auto pushCaret = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->savedCaretCol = d->screen->caretCol();
        d->savedCaretRow = d->screen->caretRow();
        d->savedCaretFormat = d->screen->currentCharacterFormat();
    });
    d->escapeStateMachine.addTransition(initialState, '7', pushCaret);

    auto popCaret = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setCaretCol(d->savedCaretCol);
        d->screen->setCaretRow(d->savedCaretRow);
        d->screen->setCurrentCharacterFormat(d->savedCaretFormat);
    });
    d->escapeStateMachine.addTransition(initialState, '8', popCaret);

    auto charsetChange = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(initialState, '(', charsetChange);
    d->escapeStateMachine.addTransition(initialState, ')', charsetChange);

    auto decCharset = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        QString** characterBank = escapeSequence.at(0) == '(' ? &d->characterSetG0 : &d->characterSetG1;
        *characterBank = &VT100EmulationPrivate::decSpecialCharacterSet;
    });
    d->escapeStateMachine.addTransition(charsetChange, '0', decCharset);

    auto asciiCharset = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        QString** characterBank = escapeSequence.at(0) == '(' ? &d->characterSetG0 : &d->characterSetG1;
        *characterBank = nullptr;
    });
    d->escapeStateMachine.addTransition(charsetChange, 'B', asciiCharset);
}

void VT100Emulation::setupCsiStateMachine() {
    auto transitionDigits = [](QChar c) {
        return c >= '0' && c <= '9';
    };
    auto transitionDigitsSemicolon = [](QChar c) {
        return (c >= '0' && c <= '9') || c == ';';
    };

    auto initialState = d->csiStateMachine.addState();

    auto csi = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(initialState, '[', csi);

    auto csrN = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition({csi, csrN}, transitionDigits, csrN);

    auto moveCursorRelative = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiMoveCursorRelative, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csi, csrN}, 'A', moveCursorRelative);
    d->csiStateMachine.addTransition({csi, csrN}, 'B', moveCursorRelative);
    d->csiStateMachine.addTransition({csi, csrN}, 'C', moveCursorRelative);
    d->csiStateMachine.addTransition({csi, csrN}, 'D', moveCursorRelative);

    auto moveLineRelative = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiMoveLineRelative, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csi, csrN}, 'E', moveLineRelative);
    d->csiStateMachine.addTransition({csi, csrN}, 'F', moveLineRelative);

    auto moveColumnRelative = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiMoveColumnRelative, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csi, csrN}, 'G', moveColumnRelative);

    auto eraseInDisplay = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiEraseInDisplay, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csi, csrN}, 'J', eraseInDisplay);

    auto eraseInLine = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiEraseInLine, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csi, csrN}, 'K', eraseInLine);

    auto csrBeforeM = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition({csi, csrN}, ';', csrBeforeM);

    auto csrM = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(csrBeforeM, transitionDigits, csrM);
    d->csiStateMachine.addTransition(csrM, transitionDigits, csrM);

    auto cursorPosition = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiCursorPosition, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csi, csrN, csrM}, 'H', cursorPosition);
    d->csiStateMachine.addTransition({csi, csrN, csrM}, 'f', cursorPosition);

    auto multiValueData = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition({csi, multiValueData}, transitionDigitsSemicolon, multiValueData);

    auto sgr = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiSgr, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csi, multiValueData}, 'm', sgr);

    auto verticalScrollingMargins = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiVerticalScrollingMargins, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csi, multiValueData}, 'r', verticalScrollingMargins);

    auto pushCaret = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::pushCaret, this));
    d->csiStateMachine.addTransition(csi, 's', pushCaret);

    auto popCaret = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::popCaret, this));
    d->csiStateMachine.addTransition(csi, 'u', popCaret);

    auto whatAreYou = d->csiStateMachine.addFinalState([this](QString escapeCode) {
        // Respond with 2 (AVO)
        write(QStringLiteral("\x1B[?1;2c"));
    });
    d->csiStateMachine.addTransition(csi, 'c', whatAreYou);
    d->csiStateMachine.addTransition(csi, "0c", whatAreYou);

    auto deviceStatusReportStatus = d->csiStateMachine.addFinalState([this](QString escapeCode) {
        // Respond with OK
        write(QStringLiteral("\x1B[0n"));
    });
    d->csiStateMachine.addTransition(csi, "5n", deviceStatusReportStatus);

    auto deviceStatusReportCursorPos = d->csiStateMachine.addFinalState([this](QString escapeCode) {
        // Respond to the Device Status Report
        // Row and column are 1-indexed
        auto row = d->screen->caretRow() + 1;
        if (d->screen->marginsBound()) row -= d->screen->firstRow();
        write(QStringLiteral("\x1B[%1;%2R").arg(row).arg(d->screen->caretCol() + 1));
    });
    d->csiStateMachine.addTransition(csi, "6n", deviceStatusReportCursorPos);

    auto mode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(csi, '?', mode);

    auto cursorKeyApplicationMode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(mode, '1', cursorKeyApplicationMode);

    auto blinkMode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(cursorKeyApplicationMode, '2', blinkMode);

    auto blinkModeOn = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        // TODO
    });
    d->csiStateMachine.addTransition(blinkMode, 'h', blinkModeOn);

    auto blinkModeOff = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        // TODO
    });
    d->csiStateMachine.addTransition(blinkMode, 'l', blinkModeOff);

    auto originMode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(mode, "6", originMode);

    auto originModeOn = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setMarginsBound(true);
    });
    d->csiStateMachine.addTransition(originMode, 'h', originModeOn);

    auto originModeOff = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setMarginsBound(false);
    });
    d->csiStateMachine.addTransition(originMode, 'l', originModeOff);

    auto cursorMode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(mode, "25", cursorMode);

    auto cursorModeOn = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setCaretVisible(true);
    });
    d->csiStateMachine.addTransition(cursorMode, 'h', cursorModeOn);

    auto cursorModeOff = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setCaretVisible(false);
    });
    d->csiStateMachine.addTransition(cursorMode, 'l', cursorModeOff);

    auto columnMode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(mode, '3', columnMode);

    auto setColumnMode = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        // We don't support this escape code but we do need to erase the screen
        csiEraseInDisplay("[3J");
        d->screen->setCaretCol(0);
        d->screen->setCaretRow(0);
    });
    d->csiStateMachine.addTransition(columnMode, 'h', setColumnMode);
    d->csiStateMachine.addTransition(columnMode, 'l', setColumnMode);

    auto altScreen = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(mode, "1049", altScreen);

    auto altScreenOn = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setScreenBuffer(TerminalScreen::ScreenBuffer::AlternateScreen);
    });
    d->csiStateMachine.addTransition(altScreen, 'h', altScreenOn);

    auto altScreenOff = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setScreenBuffer(TerminalScreen::ScreenBuffer::StandardScreen);
    });
    d->csiStateMachine.addTransition(altScreen, 'l', altScreenOff);

    auto textCursorMode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(csi, '2', textCursorMode);

    auto crlfMode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(textCursorMode, '0', crlfMode);

    auto crlfModeOn = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->crlfMode = true;
    });
    d->csiStateMachine.addTransition(crlfMode, 'h', crlfModeOn);

    auto crlfModeOnOff = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->crlfMode = false;
    });
    d->csiStateMachine.addTransition(crlfMode, 'l', crlfModeOnOff);

    auto screenInversionMode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(mode, '5', screenInversionMode);

    auto screenInversionModeOn = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setInvertScreen(true);
    });
    d->csiStateMachine.addTransition(screenInversionMode, 'h', screenInversionModeOn);

    auto screenInversionModeOff = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setInvertScreen(false);
    });
    d->csiStateMachine.addTransition(screenInversionMode, 'l', screenInversionModeOff);

    auto autowrapMode = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(mode, '7', autowrapMode);

    auto autowrapModeOn = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->autowrap = true;
    });
    d->csiStateMachine.addTransition(autowrapMode, 'h', autowrapModeOn);

    auto autowrapModeOff = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->autowrap = false;
    });
    d->csiStateMachine.addTransition(autowrapMode, 'l', autowrapModeOff);

    auto tab8Mode = d->csiStateMachine.addFinalState([this](QString escapeSequence) {
        d->tabStops.clear();
    });
    d->csiStateMachine.addTransition(mode, "5W", tab8Mode);

    auto clearTab = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(csi, '0', clearTab);

    auto clearTabDone = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiClearTab, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csi, clearTab}, 'g', clearTabDone);

    auto resetTab = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(csi, '3', resetTab);

    auto resetTabDone = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiResetTab, this, std::placeholders::_1));
    d->csiStateMachine.addTransition(resetTab, 'g', resetTabDone);
}

void VT100Emulation::processCharacter(QChar c) {
    // Process any immediate control characters
    switch (c.unicode()) {
            //     case 0x0: // Null
            //         return;
        // case 0x7: // Bell
        //     echo('\x7');
        //     return;
        case 0x8: // Backspace
            echo('\b');
            return;
        // case 0x9: // Tab
        //     echo('\t');
        //     return;
        case 0xA: // LF
        case 0xB:
        case 0xC:
            echo('\n');
            return;
        case 0xD: // CR
            echo('\r');
            return;
        case 0xE: // Switch into G1 mode
            d->currentCharacterSet = &d->characterSetG1;
            return;
        case 0xF: // Switch into G0 mode
            d->currentCharacterSet = &d->characterSetG0;
            return;
        case 0x18: // Cancel escape sequence
        case 0x1A:
            d->escapeMode = false;
            echo('?');
            return;
    }

    // Ensure that we are not currently reading escape characters
    if (!d->escapeMode) {
        if (c == '\x1B') {
            // This is the ESC character. Enter escape mode!
            d->escapeMode = true;
            d->escapeStateMachine.reset();
            return;
        }

        echo(c);
        return;
    }

    switch (d->escapeStateMachine.pushCharacter(c)) {
        case TerminalStateMachine::Result::Rejected:
            tWarn("VT100Emulation") << "Unknown escape sequence: " << QString(d->escapeStateMachine.replayBuffer().toUtf8().toHex()) << " | " << d->escapeStateMachine.replayBuffer();
            // Fall through
        case TerminalStateMachine::Result::Accepted:
            d->escapeMode = false;
            for (auto replayCharacter : d->escapeStateMachine.replayBuffer()) {
                processCharacter(replayCharacter);
            }
        case TerminalStateMachine::Result::Pending:
            break;
    }
}

void VT100Emulation::write(QString characters) {
    d->device->write(characters.toUtf8());
    if (d->echo) {
        for (auto character : characters) {
            echo(character);
        }
    }
}

void VT100Emulation::echo(QChar c) {
    if (c == '\n') {
        if (d->screen->caretRow() == d->screen->rows() - 1) {
            d->screen->pushToHistory();
            d->screen->setCaretRow(d->screen->rows() - 1);
        } else {
            d->screen->setCaretRow(d->screen->caretRow() + 1);
        }

        if (d->crlfMode) {
            d->screen->setCaretCol(0);
        }
    } else if (c == '\b') {
        d->screen->setCaretCol(d->screen->caretCol() - 1);
    } else if (c == '\r') {
        d->screen->setCaretCol(0);
    } else if (c == '\x07') { // BEL
        // TODO
    } else if (c == '\t') {
        do {
            d->screen->setCharacter(d->screen->caretCol(), d->screen->caretRow(), TerminalScreen::emptyChar());
            d->screen->setCaretCol(d->screen->caretCol() + 1);
        } while ((d->tabStops.isEmpty() ? d->screen->caretCol() % 8 == 0 : !d->tabStops.contains(d->screen->caretCol())) && d->screen->caretCol() != d->screen->cols() - 1);
    } else {
        QChar echoedCharacter = c;
        if (*d->currentCharacterSet) {
            if (c.unicode() >= 0x5F && c.unicode() <= 0x7E) {
                echoedCharacter = (*d->currentCharacterSet)->at(c.unicode() - 0x5F);
            }
        }

        if (d->screen->caretCol() == d->screen->cols()) {
            if (!d->autowrap) {
                d->screen->setCharacter(d->screen->caretCol(), d->screen->caretRow(), echoedCharacter);
                return;
            }

            // Wrap to the next line first
            echo('\n');
            echo('\r');
        }

        d->screen->setCharacter(d->screen->caretCol(), d->screen->caretRow(), echoedCharacter);
        d->screen->setCaretCol(d->screen->caretCol() + 1);
    }
}

void VT100Emulation::invokeCsi(QString csi) {
    d->csiStateMachine.reset();

    TerminalStateMachine::Result lastResult = TerminalStateMachine::Result::Pending;
    for (auto character : csi) {
        lastResult = d->csiStateMachine.pushCharacter(character);
    }

    if (lastResult != TerminalStateMachine::Result::Pending) {
        return;
    }

    // Push a final character to trigger the final result
    switch (d->csiStateMachine.pushCharacter(TerminalScreen::emptyChar())) {
        case TerminalStateMachine::Result::Accepted:
        case TerminalStateMachine::Result::Pending:
            break;
        case TerminalStateMachine::Result::Rejected:
            tWarn("VT100Emulation") << "Unknown CSI sequence: " << d->csiStateMachine.replayBuffer();
            break;
    };
}

void VT100Emulation::csiMoveCursorRelative(QString escapeSequence) {
    static QRegularExpression cursorPositionRelativeRegex("\\[(?<num>\\d+)?(?<dir>A|B|C|D)");
    auto matches = cursorPositionRelativeRegex.match(escapeSequence);
    auto numStr = matches.captured("num");
    auto dir = matches.captured("dir");

    if (numStr.isEmpty()) numStr = "1";
    auto num = numStr.toInt();
    if (dir == "A") {
        // Move up
        d->screen->setCaretRow(d->screen->caretRow() - num);
    } else if (dir == "B") {
        // Move down
        d->screen->setCaretRow(d->screen->caretRow() + num);
    } else if (dir == "C") {
        // Move right
        d->screen->setCaretCol(d->screen->caretCol() + num);
    } else {
        // Move left
        d->screen->setCaretCol(d->screen->caretCol() - num);
    }
}

void VT100Emulation::csiMoveLineRelative(QString escapeSequence) {
    static QRegularExpression cursorPositionRelativeRegex("\\[(?<num>\\d+)?(?<dir>E|F)");
    auto matches = cursorPositionRelativeRegex.match(escapeSequence);
    auto numStr = matches.captured("num");
    auto dir = matches.captured("dir");

    if (numStr.isEmpty()) numStr = "1";
    auto num = numStr.toInt();
    d->screen->setCaretCol(0);
    if (dir == "E") {
        // Move down
        d->screen->setCaretRow(d->screen->caretRow() + num);
    } else if (dir == "F") {
        // Move up
        d->screen->setCaretRow(d->screen->caretRow() - num);
    }
}

void VT100Emulation::csiMoveColumnRelative(QString escapeSequence) {
    static QRegularExpression cursorPositionRelativeRegex("\\[(?<num>\\d+)?G");
    auto matches = cursorPositionRelativeRegex.match(escapeSequence);
    auto numStr = matches.captured("num");

    if (numStr.isEmpty()) numStr = "1";
    auto num = numStr.toInt();
    d->screen->setCaretCol(num - 1);
}

void VT100Emulation::csiEraseInLine(QString escapeSequence) {
    auto type = escapeSequence.at(1);
    switch (type.unicode()) {
        case 'K':
        case '0':
            {
                // Clear from caret to end of line
                for (auto i = d->screen->caretCol(); i < d->screen->cols(); i++) {
                    d->screen->setCharacter(i, d->screen->caretRow(), TerminalScreen::emptyChar());
                }
                break;
            }
        case '1':
            // Clear from beginning of screen to caret
            for (auto i = 0; i <= d->screen->caretCol(); i++) {
                d->screen->setCharacter(i, d->screen->caretRow(), TerminalScreen::emptyChar());
            }
            break;
        case '2':
            // Clear entire line
            for (auto i = 0; i < d->screen->cols(); i++) {
                d->screen->setCharacter(i, d->screen->caretRow(), TerminalScreen::emptyChar());
            }
            break;
        default:
            tDebug("VT100Emulation") << "Erase In Line: unkown erase type: " << escapeSequence;
    }
}

void VT100Emulation::csiEraseInDisplay(QString escapeSequence) {
    auto type = escapeSequence.at(1);
    switch (type.unicode()) {
        case 'J':
        case '0':
            {
                // Clear from caret to end of screen
                for (auto i = d->screen->caretRow(); i < d->screen->rows(); i++) {
                    for (auto j = (i == d->screen->caretRow() ? d->screen->caretCol() : 0); j < d->screen->cols(); j++) {
                        d->screen->setCharacter(j, i, TerminalScreen::emptyChar());
                    }
                }
                break;
            }
        case '1':
            // Clear from beginning of screen to caret
            for (auto i = 0; i < d->screen->rows(); i++) {
                for (auto j = 0; j < d->screen->cols(); j++) {
                    d->screen->setCharacter(j, i, TerminalScreen::emptyChar());
                    if (j == d->screen->caretCol() && i == d->screen->caretRow()) return;
                }
            }
            break;
        case '3':
            // Clear entire screen and scrollback buffer
            // fall through
        case '2':
            // Clear entire screen
            for (auto i = 0; i < d->screen->rows(); i++) {
                d->screen->setRowScaleMode(i, TerminalScreen::RowScaleMode::Normal);
                for (auto j = 0; j < d->screen->cols(); j++) {
                    d->screen->setCharacter(j, i, TerminalScreen::emptyChar());
                }
            }
            d->screen->clearScrollback();
            break;
        default:
            tDebug("VT100Emulation") << "Erase In Display: unkown erase type: " << escapeSequence;
    }
}

void VT100Emulation::csiCursorPosition(QString escapeSequence) {
    static QRegularExpression cursorPositionRegex("\\[(?<row>\\d+)?(?:;(?<col>\\d+))?(?:H|f)");
    auto matches = cursorPositionRegex.match(escapeSequence);
    auto rowStr = matches.captured("row");
    auto colStr = matches.captured("col");

    if (rowStr.isEmpty()) rowStr = "1";
    if (colStr.isEmpty()) colStr = "1";

    d->screen->setCaretCol(colStr.toInt() - 1);
    d->screen->setCaretRow(rowStr.toInt() - 1);
}

void VT100Emulation::csiAutoWrap(QString escapeSequence) {
}

void VT100Emulation::csiClearTab(QString escapeSequence) {
    d->tabStops.remove(d->screen->caretCol());
}

void VT100Emulation::csiResetTab(QString escapeSequence) {
    d->tabStops.clear();
}

void VT100Emulation::csiVerticalScrollingMargins(QString escapeSequence) {
    static QRegularExpression verticalScrollingMarginsRegex("\\[(?<top>\\d+)?(?:;(?<bottom>\\d+))?(?:r)");
    auto matches = verticalScrollingMarginsRegex.match(escapeSequence);
    auto topStr = matches.captured("top");
    auto bottomStr = matches.captured("bottom");

    if (topStr.isEmpty()) topStr = "0";
    if (bottomStr.isEmpty()) bottomStr = "0";

    d->screen->setVerticalMargins(topStr.toInt() - 1, bottomStr.toInt() - 1);
}

void VT100Emulation::csiSgr(QString escapeSequence) {
    QStringList sgrCommands = escapeSequence.mid(1, escapeSequence.length() - 2).split(";");
    if (sgrCommands.isEmpty()) {
        // SGR 0 (reset)
        d->screen->setCurrentCharacterFormat({});
        return;
    }

    auto format = d->screen->currentCharacterFormat();
    do {
        auto command = sgrCommands.takeFirst().toInt();
        bool setBackground = true;
        quint32* colorToSet = &format.backgroundColor;
        switch (command) {
            case 0: // reset
                format = {};
                break;
            case 1: // bold
                format.bold = true;
                break;
            case 4: // underline
                format.underline = true;
                break;
            case 5: // blink
                format.blink = true;
                break;
            case 7: // invert
                format.invert = true;
                break;
            case 22: // bold off
                format.bold = false;
                break;
            case 24: // underline off
                format.underline = true;
                break;
            case 25: // blink off
                format.blink = false;
                break;
            case 27: // invert off
                format.invert = false;
                break;
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37: // Set foreground
                format.color = ScreenColorManager::color8bit(command - 30);
                break;
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47: // Set background
                format.backgroundColor = ScreenColorManager::color8bit(command - 40);
                break;
            case 90:
            case 91:
            case 92:
            case 93:
            case 94:
            case 95:
            case 96:
            case 97: // Set foreground intense
                format.color = ScreenColorManager::color8bit(command - 82);
                break;
            case 100:
            case 101:
            case 102:
            case 103:
            case 104:
            case 105:
            case 106:
            case 107: // Set background intense
                format.backgroundColor = ScreenColorManager::color8bit(command - 92);
                break;
            case 38: // Set foreground extended
                colorToSet = &format.color;
                // fall through
            case 48: // Set background extended
                {
                    auto mode = sgrCommands.takeFirst().toInt();
                    if (mode == 5) {
                        // 8 bit mode
                        auto colorIndex = sgrCommands.takeFirst().toInt();
                        *colorToSet = ScreenColorManager::color8bit(colorIndex);
                    } else if (mode == 2) {
                        // 24 bit mode
                        auto r = sgrCommands.takeFirst().toInt();
                        auto g = sgrCommands.takeFirst().toInt();
                        auto b = sgrCommands.takeFirst().toInt();
                        *colorToSet = ScreenColorManager::color24bit(r, g, b);
                    }
                    break;
                }
            case 39: // Set default foreground
                format.color = ScreenColorManager::colorDefault(false);
                break;
            case 49: // Set default background
                format.backgroundColor = ScreenColorManager::colorDefault(true);
                break;
            default:
                tWarn("VT100Emulation") << "Unknown SGR command: " << command;
        }
    } while (!sgrCommands.isEmpty());
    d->screen->setCurrentCharacterFormat(format);
}

void VT100Emulation::invokeOsc(QString osc) {
    tWarn("VT100Emulation") << "Unknown OSC sequence: " << osc;
}

void VT100Emulation::pushCaret() {
    d->savedCaretCol = d->screen->caretCol();
    d->savedCaretRow = d->screen->caretRow();
    d->savedCaretFormat = d->screen->currentCharacterFormat();
    d->savedCaretAutowrap = d->autowrap;
    d->savedCharacterSet = d->currentCharacterSet;
}

void VT100Emulation::popCaret() {
    d->screen->setCaretCol(d->savedCaretCol);
    d->screen->setCaretRow(d->savedCaretRow);
    d->screen->setCurrentCharacterFormat(d->savedCaretFormat);
    d->autowrap = d->savedCaretAutowrap;
    d->currentCharacterSet = d->savedCharacterSet;
}
