#include "terminalstatemachine.h"

#include <QMap>

struct TerminalStateMachinePrivate {
        quint64 maxState = 0;
        QMap<quint64, std::function<void()>> finalStates;
        QMultiMap<quint64, QPair<TerminalStateMachine::StateTransitionFunction, quint64>> transitions;

        quint64 currentState = 0;
        quint64 lastFinalState = 0;
        QString replayBuffer;
        QString escapeBuffer;
};

TerminalStateMachine::TerminalStateMachine(QObject* parent) :
    QObject{parent}, d{new TerminalStateMachinePrivate()} {
}

TerminalStateMachine::~TerminalStateMachine() {
    delete d;
}

TerminalStateMachine::Result TerminalStateMachine::pushCharacter(QChar c) {
    d->replayBuffer.append(c);
    auto transitions = d->transitions.values(d->currentState);
    for (const auto& transition : transitions) {
        if (transition.first(c)) {
            d->currentState = transition.second;
            if (d->finalStates.contains(d->currentState)) {
                d->lastFinalState = d->currentState;
                d->escapeBuffer.append(d->replayBuffer);
                d->replayBuffer.clear();
            }
            return Result::Pending;
        }
    }

    if (d->lastFinalState == 0) {
        return Result::Rejected;
    } else {
        d->finalStates.value(d->lastFinalState)();
        return Result::Accepted;
    }
}

QString TerminalStateMachine::replayBuffer() {
    return d->replayBuffer;
}

QString TerminalStateMachine::escapeBuffer() {
    return d->escapeBuffer;
}

void TerminalStateMachine::reset() {
    d->currentState = 0;
    d->lastFinalState = 0;
    d->replayBuffer.clear();
    d->escapeBuffer.clear();
}

quint64 TerminalStateMachine::addState() {
    return d->maxState++;
}

quint64 TerminalStateMachine::addFinalState(std::function<void()> function) {
    auto stateNum = addState();
    d->finalStates.insert(stateNum, function);
    return stateNum;
}

void TerminalStateMachine::addTransition(quint64 state1, QChar c, quint64 state2) {
    addTransition(state1, [c](QChar nextChar) {
        return nextChar == c;
    }, state2);
}

void TerminalStateMachine::addTransition(quint64 state1, StateTransitionFunction function, quint64 state2) {
    d->transitions.insert(state1, {function, state2});
}
