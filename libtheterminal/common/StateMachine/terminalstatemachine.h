#ifndef TERMINALSTATEMACHINE_H
#define TERMINALSTATEMACHINE_H

#include <QObject>

struct TerminalStateMachinePrivate;
class TerminalStateMachine : public QObject {
        Q_OBJECT
    public:
        explicit TerminalStateMachine(QObject* parent = nullptr);
        ~TerminalStateMachine();

        using StateTransitionFunction = std::function<bool(QChar)>;

        enum class Result {
            Accepted,
            Pending,
            Rejected
        };

        Result pushCharacter(QChar c);
        QString replayBuffer();
        QString escapeBuffer();
        void reset();

        quint64 addState();
        quint64 addFinalState(std::function<void()> function);
        void addTransition(quint64 state1, QChar c, quint64 state2);
        void addTransition(quint64 state1, StateTransitionFunction function, quint64 state2);

    signals:

    private:
        TerminalStateMachinePrivate* d;
};

#endif // TERMINALSTATEMACHINE_H
