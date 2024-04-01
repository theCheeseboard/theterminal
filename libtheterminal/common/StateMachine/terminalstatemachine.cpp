#include "terminalstatemachine.h"

#include <QMap>

struct TerminalStateMachinePrivate {
        quint64 maxState = 0;
        QMap<quint64, TerminalStateMachine::AcceptFunction> finalStates;
        QMultiMap<quint64, QPair<TerminalStateMachine::StateTransitionFunction, quint64>> transitions;

        struct CurrentState {
                quint64 stateNumber;
                QString escapeBuffer;
        };

        QList<CurrentState> currentState;
        QList<CurrentState> currentFinalStates;
        // quint64 lastFinalState = 0;
        QString replayBuffer;
        QString escapeBuffer;
        QString processBuffer;
};

TerminalStateMachine::TerminalStateMachine(QObject* parent) :
    QObject{parent}, d{new TerminalStateMachinePrivate()} {
}

TerminalStateMachine::~TerminalStateMachine() {
    delete d;
}

TerminalStateMachine::Result TerminalStateMachine::pushCharacter(QChar c) {
    d->processBuffer.append(c);

    // Create a list of new states. Traverse all current states at the same time.
    // At the end of this block, the newCurrentState variable will contain all
    // valid transitions from all current states
    QList<TerminalStateMachinePrivate::CurrentState> newCurrentState;
    for (const auto& constState : d->currentState) {
        auto state = constState;
        // Get all transitions for this state
        auto transitions = d->transitions.values(state.stateNumber);
        for (const auto& transition : transitions) {
            // If we can take this transition, add the new state to the newCurrentState variable.
            if (transition.first(c)) {
                // Preserve all existing data for now
                TerminalStateMachinePrivate::CurrentState newState;
                newState.stateNumber = transition.second;

                // If we have transitioned into a final state, push it to the final states list.
                if (d->finalStates.contains(newState.stateNumber)) {
                    newState.escapeBuffer = d->processBuffer;
                    d->currentFinalStates.append(newState);
                }
                newCurrentState.append(newState);
            }
        }
    }

    // At this point, all state transitions should be contained in newCurrentState.
    // If there are no items in there, that means that there were no valid transitions.
    if (newCurrentState.empty()) {
        if (d->currentFinalStates.isEmpty()) {
            // We crossed no final states and there are no more valid transitions, so reject.
            d->replayBuffer = d->processBuffer;
            return Result::Rejected;
        }

        // Find the final state with the longest escape buffer
        int maxLength = -1;
        TerminalStateMachinePrivate::CurrentState finalState;

        for (const auto& state : d->currentFinalStates) {
            if (state.escapeBuffer.length() > maxLength) {
                maxLength = state.escapeBuffer.length();
                finalState = state;
            }
        }

        d->escapeBuffer = finalState.escapeBuffer;
        d->replayBuffer = d->processBuffer.mid(finalState.escapeBuffer.length());
        d->finalStates.value(finalState.stateNumber)(d->escapeBuffer);
        return Result::Accepted;
    }

    // Update the current state and wait
    d->currentState = newCurrentState;
    return Result::Pending;
}

QString TerminalStateMachine::replayBuffer() {
    return d->replayBuffer;
}

QString TerminalStateMachine::escapeBuffer() {
    return d->escapeBuffer;
}

void TerminalStateMachine::reset() {
    // d->currentState = 0;
    d->currentState = {
        {0, 0}
    };
    d->replayBuffer.clear();
    d->escapeBuffer.clear();
    d->processBuffer.clear();
    d->currentFinalStates.clear();
}

quint64 TerminalStateMachine::addState() {
    return d->maxState++;
}

quint64 TerminalStateMachine::addFinalState(AcceptFunction function) {
    auto stateNum = addState();
    d->finalStates.insert(stateNum, function);
    return stateNum;
}

void TerminalStateMachine::addTransition(quint64 state1, QChar c, quint64 state2) {
    addTransition(state1, [c](QChar nextChar) {
        return nextChar == c;
    }, state2);
}

void TerminalStateMachine::addTransition(QList<quint64> state1s, QChar c, quint64 state2) {
    for (auto state : state1s) {
        addTransition(state, c, state2);
    }
}

void TerminalStateMachine::addTransition(quint64 state1, StateTransitionFunction function, quint64 state2) {
    d->transitions.insert(state1, {function, state2});
}

void TerminalStateMachine::addTransition(QList<quint64> state1s, StateTransitionFunction function, quint64 state2) {
    for (auto state : state1s) {
        addTransition(state, function, state2);
    }
}
