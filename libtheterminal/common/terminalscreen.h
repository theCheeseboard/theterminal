#ifndef TERMINALSCREEN_H
#define TERMINALSCREEN_H

#include <QObject>

struct TerminalScreenPrivate;
class TerminalScreen : public QObject {
        Q_OBJECT
        Q_PROPERTY(int cols READ cols WRITE setCols NOTIFY colsChanged FINAL)
        Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY rowsChanged FINAL)
        Q_PROPERTY(int caretCol READ caretCol WRITE setCaretCol NOTIFY caretColChanged FINAL)
        Q_PROPERTY(int caretRow READ caretRow WRITE setCaretRow NOTIFY caretRowChanged FINAL)
    public:
        explicit TerminalScreen(QObject* parent = nullptr);
        ~TerminalScreen();

        struct CharacterSpace {
                QChar character = ' ';
        };

        int cols();
        void setCols(int cols);

        int rows();
        void setRows(int rows);

        int caretCol();
        void setCaretCol(int col);

        int caretRow();
        void setCaretRow(int row);

        void setCharacter(int col, int row, CharacterSpace character);
        CharacterSpace character(int col, int row);
        void pushToHistory();

    signals:
        void colsChanged();
        void rowsChanged();
        void caretColChanged();
        void caretRowChanged();
        void rowContentChanged(int row);
        void historyRolled();

    private:
        TerminalScreenPrivate* d;
};

#endif // TERMINALSCREEN_H
