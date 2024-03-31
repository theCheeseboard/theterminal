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

    auto moveCursorRelative = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::escapeMoveCursorRelative, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN}, 'A', moveCursorRelative);
    d->csiStateMachine.addTransition({csr, csrN}, 'B', moveCursorRelative);
    d->csiStateMachine.addTransition({csr, csrN}, 'C', moveCursorRelative);
    d->csiStateMachine.addTransition({csr, csrN}, 'D', moveCursorRelative);

    auto eraseInDisplay = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::escapeEraseInDisplay, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN}, 'J', eraseInDisplay);

    auto eraseInLine = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::escapeEraseInLine, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN}, 'K', eraseInLine);

    auto csrBeforeM = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition({csr, csrN}, ';', csrBeforeM);

    auto csrM = d->csiStateMachine.addState();
    d->csiStateMachine.addTransition(csrBeforeM, transitionDigits, csrM);
    d->csiStateMachine.addTransition(csrM, transitionDigits, csrM);

    auto cursorPosition = d->csiStateMachine.addFinalState(std::bind(&VT100Emulation::escapeCursorPosition, this, std::placeholders::_1));
    d->csiStateMachine.addTransition({csr, csrN, csrM}, 'H', cursorPosition);
    d->csiStateMachine.addTransition({csr, csrN, csrM}, 'f', cursorPosition);
}

void VT100Emulation::processCharacter(QChar c) {
    // Ensure that we are not currently reading escape characters
    if (!d->escapeMode) {
        if (c == '\e') {
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
            tWarn("VT100Emulation") << "Unknown escape sequence: " << QString(d->csiStateMachine.replayBuffer().toUtf8().toHex());
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
        d->screen->setCaretCol(0);
        if (d->screen->caretRow() == d->screen->rows() - 1) {
            d->screen->pushToHistory();
            d->screen->setCaretRow(d->screen->rows() - 1);
        } else {
            d->screen->setCaretRow(d->screen->caretRow() + 1);
        }
    } else if (c == '\b') {
        d->screen->setCharacter(d->screen->caretCol(), d->screen->caretRow(), {' '});
        d->screen->setCaretCol(d->screen->caretCol() - 1);
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

void VT100Emulation::escapeMoveCursorRelative(QString escapeSequence) {
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

void VT100Emulation::escapeEraseInLine(QString escapeSequence) {
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
            for (auto i = 0; i < d->screen->caretCol(); i++) {
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

void VT100Emulation::escapeEraseInDisplay(QString escapeSequence) {
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
                    if (j == d->screen->caretCol() && i == d->screen->caretRow()) return;
                    d->screen->setCharacter(j, i, {' '});
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

void VT100Emulation::escapeCursorPosition(QString escapeSequence) {
    static QRegularExpression cursorPositionRegex("\\[(?<row>\\d+)?(?:;(?<col>\\d+))?(?:H|f)");
    auto matches = cursorPositionRegex.match(escapeSequence);
    auto rowStr = matches.captured("row");
    auto colStr = matches.captured("col");

    if (rowStr.isEmpty()) rowStr = "1";
    if (colStr.isEmpty()) colStr = "1";

    d->screen->setCaretCol(colStr.toInt() - 1);
    d->screen->setCaretRow(rowStr.toInt() - 1);
}
