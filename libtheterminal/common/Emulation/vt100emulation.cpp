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

        int savedCaretRow = 0;
        int savedCaretCol = 0;
};

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
        this->write("\n");
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
    d->escapeStateMachine.addTransition(csi, [](QChar c) {
        return !(c.toLatin1() >= 0x40 && c.toLatin1() <= 0x7E);
    }, csi);

    auto csiEnd = d->escapeStateMachine.addFinalState(std::bind(&VT100Emulation::invokeCsi, this, std::placeholders::_1));
    d->escapeStateMachine.addTransition(csi, [](QChar c) {
        return c.toLatin1() >= 0x40 && c.toLatin1() <= 0x7E;
    }, csiEnd);

    auto osc = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(initialState, ']', osc);
    d->escapeStateMachine.addTransition(osc, [](QChar c) {
        return c.toLatin1() != '\x1B' && c.toLatin1() != '\x07';
    }, osc);

    auto oscEscEnd = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(osc, '\x1B', oscEscEnd);
    d->escapeStateMachine.addTransition(oscEscEnd, [](QChar c) {
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

    auto previousLine = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setCaretRow(d->screen->caretRow() - 1);
    });
    d->escapeStateMachine.addTransition(initialState, 'M', previousLine);

    auto octothorpe = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(initialState, '#', octothorpe);

    auto alignmentPattern = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        // Fill the screen with Es
        for (auto i = 0; i < d->screen->rows(); i++) {
            for (auto j = 0; j < d->screen->cols(); j++) {
                d->screen->setCharacter(j, i, {'E'});
            }
        }
    });
    d->escapeStateMachine.addTransition(octothorpe, '8', alignmentPattern);

    auto pushCaret = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->savedCaretCol = d->screen->caretCol();
        d->savedCaretRow = d->screen->caretRow();
    });
    d->escapeStateMachine.addTransition(initialState, '7', pushCaret);

    auto popCaret = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setCaretCol(d->savedCaretCol);
        d->screen->setCaretRow(d->savedCaretRow);
    });
    d->escapeStateMachine.addTransition(initialState, '8', popCaret);

    auto charsetChange = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(initialState, '(', charsetChange);
    d->escapeStateMachine.addTransition(initialState, ')', charsetChange);

    auto decCharset = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {

    });
    d->escapeStateMachine.addTransition(charsetChange, '0', decCharset);

    auto asciiCharset = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {

    });
    d->escapeStateMachine.addTransition(charsetChange, 'B', asciiCharset);
}

void VT100Emulation::setupCsiStateMachine() {
    auto transitionDigits = [](QChar c) {
        return c >= '0' && c <= '9';
    };

    auto initialState = d->csiStateMachine.addState();

    auto csr = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(initialState, '[', csr);

    auto csrN = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition({csr, csrN}, transitionDigits, csrN);

    auto moveCursorRelative = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiMoveCursorRelative, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN}, 'A', moveCursorRelative);
    d->csiStateMachine.addTransition({csr, csrN}, 'B', moveCursorRelative);
    d->csiStateMachine.addTransition({csr, csrN}, 'C', moveCursorRelative);
    d->csiStateMachine.addTransition({csr, csrN}, 'D', moveCursorRelative);

    auto moveLineRelative = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiMoveLineRelative, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN}, 'E', moveLineRelative);
    d->csiStateMachine.addTransition({csr, csrN}, 'F', moveLineRelative);

    auto moveColumnRelative = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiMoveColumnRelative, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN}, 'G', moveColumnRelative);

    auto eraseInDisplay = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiEraseInDisplay, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN}, 'J', eraseInDisplay);

    auto eraseInLine = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiEraseInLine, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN}, 'K', eraseInLine);

    auto csrBeforeM = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition({csr, csrN}, ';', csrBeforeM);

    auto csrM = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(csrBeforeM, transitionDigits, csrM);
    d->csiStateMachine.addTransition(csrM, transitionDigits, csrM);

    auto cursorPosition = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiCursorPosition, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN, csrM}, 'H', cursorPosition);
    d->csiStateMachine.addTransition({csr, csrN, csrM}, 'f', cursorPosition);

    auto sgrData = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition({csr, sgrData}, [](QChar c) {
        return (c >= '0' && c <= '9') || c == ';';
    }, sgrData);

    auto sgr = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::csiSgr, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, sgrData}, 'm', sgr);

    auto pushCaret = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->savedCaretCol = d->screen->caretCol();
        d->savedCaretRow = d->screen->caretRow();
    });
    d->escapeStateMachine.addTransition(csr, 's', pushCaret);

    auto popCaret = d->escapeStateMachine.addFinalState([this](QString escapeSequence) {
        d->screen->setCaretCol(d->savedCaretCol);
        d->screen->setCaretRow(d->savedCaretRow);
    });
    d->escapeStateMachine.addTransition(csr, 'u', popCaret);
}

void VT100Emulation::processCharacter(QChar c) {
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
            tWarn("VT100Emulation") << "Unknown escape sequence: " << QString(d->csiStateMachine.replayBuffer().toUtf8().toHex()) << " | " << d->csiStateMachine.replayBuffer();
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
    } else if (c == '\b') {
        d->screen->setCaretCol(d->screen->caretCol() - 1);
    } else if (c == '\r') {
        d->screen->setCaretCol(0);
    } else if (c == '\x07') { // BEL
        // TODO
    } else if (c == '\t') {
        do {
            d->screen->setCharacter(d->screen->caretCol(), d->screen->caretRow(), {' '});
            d->screen->setCaretCol(d->screen->caretCol() + 1);
        } while (d->screen->caretCol() % 8 != 0 && d->screen->caretCol() != d->screen->cols() - 1);
    } else {
        d->screen->setCharacter(d->screen->caretCol(), d->screen->caretRow(), {c});
        d->screen->setCaretCol(d->screen->caretCol() + 1);
    }
}

void VT100Emulation::invokeCsi(QString csi) {
    d->csiStateMachine.reset();

    TerminalStateMachine::Result lastResult = TerminalStateMachine::Result::Pending;
    for (auto character : csi) {
        lastResult = d->csiStateMachine.pushCharacter(character);
    }

    // Push a final character to trigger the final result
    switch (d->csiStateMachine.pushCharacter(' ')) {
        case TerminalStateMachine::Result::Accepted:
            break;
        case TerminalStateMachine::Result::Pending:
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
                    d->screen->setCharacter(i, d->screen->caretRow(), {' '});
                }
                break;
            }
        case '1':
            // Clear from beginning of screen to caret
            for (auto i = 0; i <= d->screen->caretCol(); i++) {
                d->screen->setCharacter(i, d->screen->caretRow(), {' '});
            }
            break;
        case '2':
            // Clear entire line
            for (auto i = 0; i < d->screen->cols(); i++) {
                d->screen->setCharacter(i, d->screen->caretRow(), {' '});
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
                        d->screen->setCharacter(j, i, {' '});
                    }
                }
                break;
            }
        case '1':
            // Clear from beginning of screen to caret
            for (auto i = 0; i < d->screen->rows(); i++) {
                for (auto j = 0; j < d->screen->cols(); j++) {
                    d->screen->setCharacter(j, i, {' '});
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
                for (auto j = 0; j < d->screen->cols(); j++) {
                    d->screen->setCharacter(j, i, {' '});
                }
            }
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

void VT100Emulation::csiSgr(QString escapeSequence) {
}

void VT100Emulation::invokeOsc(QString osc) {
    tWarn("VT100Emulation") << "Unknown OSC sequence: " << osc; // d->csiStateMachine.replayBuffer();
}
