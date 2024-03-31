#include "vt100emulation.h"

#include "../StateMachine/terminalstatemachine.h"
#include "../terminalscreen.h"
#include <QIODevice>
#include <tlogger.h>

struct VT100EmulationPrivate {
        QIODevice* device;
        TerminalScreen* screen;

        bool echo = false;
        bool escapeMode = false;
        TerminalStateMachine escapeStateMachine;
};

#include <QTimer>

VT100Emulation::VT100Emulation(QIODevice* device, TerminalScreen* screen, QObject* parent) :
    QObject{parent}, d{new VT100EmulationPrivate} {
    d->device = device;
    d->screen = screen;

    setupStateMachine();

    connect(device, &QIODevice::readyRead, this, [this] {
        for (auto character : d->device->readAll()) {
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

    auto csr = d->escapeStateMachine.addState();
    d->escapeStateMachine.addTransition(initialState, '[', csr);

    auto csrN = d->escapeStateMachine.addState();
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
        case TerminalStateMachine::Result::Accepted:
        case TerminalStateMachine::Result::Rejected:
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

void VT100Emulation::failEscapeSequence() {
    tWarn("VT100Emulation") << "Unknown escape sequence: " << QString(d->currentEscapeSequence.toHex());
    d->escapeMode = false;
}
