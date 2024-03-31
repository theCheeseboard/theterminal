#include "terminalstatemachine.h"

#include <QMap>

struct TerminalStateMachinePrivate {
        quint64 maxState = 0;
        QMap<quint64, TerminalStateMachine::AcceptFunction> finalStates;
        QMultiMap<quint64, QPair<TerminalStateMachine::StateTransitionFunction, quint64>> transitions;

        struct CurrentState {
                quint64 stateNumber;
                quint64 lastFinalState;
                QString replayBuffer;
                QString escapeBuffer;
                bool brakes = false;
        };

        QList<CurrentState> currentState;
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
    // NOTE: This logic does not pick up the longest accepting string.
    // In the event that there is a graph like so:
    //
    //            C---2--[E]
    //           /
    //          1
    //         /
    //        A
    //         \
    //          1
    //           \
    //           [B]
    //
    // The string "12" will land on the accepting state B, not E.

    d->replayBuffer.append(c);

    // Work around
    bool forceEndStateMachine = false;

    // Create a list of new states. Traverse all current states at the same time.
    // At the end of this block, the newCurrentState variable will contain all
    // valid transitions from all current states
    QList<TerminalStateMachinePrivate::CurrentState> newCurrentState;
    for (auto state : d->currentState) {
        bool madeTransition = false;
        // Get all transitions for this state
        auto transitions = d->transitions.values(state.stateNumber);
        for (const auto& transition : transitions) {
            // If we can take this transition, add the new state to the newCurrentState variable.
            if (transition.first(c)) {
                // Preserve all existing data for now
                TerminalStateMachinePrivate::CurrentState newState;
                newState.stateNumber = transition.second;
                newState.lastFinalState = state.lastFinalState;
                newState.escapeBuffer = state.escapeBuffer;
                newState.replayBuffer = state.replayBuffer.append(c);

                madeTransition = true;

                // If we have transitioned into a final state, update the data accordingly.
                if (d->finalStates.contains(newState.stateNumber)) {
                    newState.lastFinalState = newState.stateNumber;
                    newState.escapeBuffer = state.escapeBuffer.append(state.replayBuffer);
                    newState.replayBuffer.clear();
                }
                newCurrentState.append(newState);
            }
        }

        if (!madeTransition && state.lastFinalState != 0) {
            forceEndStateMachine = true;
        }
    }

    // At this point, all state transitions should be contained in newCurrentState.
    // If there are no items in there, that means that there were no valid transitions.
    if (newCurrentState.empty() || forceEndStateMachine) {
        // Before we decide to reject, read the current state (before newCurrentState)
        // and if any of them landed on a final state, call the state machine accepted.
        for (const auto& state : d->currentState) {
            if (state.lastFinalState != 0) {
                // We got to a final state!
                d->escapeBuffer = state.escapeBuffer;
                d->replayBuffer = QString(state.replayBuffer).append(c);
                d->finalStates.value(state.lastFinalState)(state.escapeBuffer);
                return Result::Accepted;
            }
        }

        // None of the current states passed a final state, therefore we reject.
        return Result::Rejected;
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
    d->lastFinalState = 0;
    d->replayBuffer.clear();
    d->escapeBuffer.clear();
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
