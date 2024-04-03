#ifndef TERMINALSCREEN_H
#define TERMINALSCREEN_H

#include "libtheterminal-common-exports.h"
#include "screencolormanager.h"
#include <QObject>

struct TerminalScreenPrivate;
class LIBTHETERMINAL_COMMON_EXPORT TerminalScreen : public QObject {
        Q_OBJECT
        Q_PROPERTY(int cols READ cols WRITE setCols NOTIFY colsChanged FINAL)
        Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY rowsChanged FINAL)
        Q_PROPERTY(int caretCol READ caretCol WRITE setCaretCol NOTIFY caretColChanged FINAL)
        Q_PROPERTY(int caretRow READ caretRow WRITE setCaretRow NOTIFY caretRowChanged FINAL)
    public:
        explicit TerminalScreen(QObject* parent = nullptr);
        ~TerminalScreen();

        enum class RowScaleMode {
            Normal = 0,
            DoubleWidth = 1,
            DoubleHeightUpper = 2,
            DoubleHeightLower = 3
        };
        Q_ENUM(RowScaleMode)

        struct CharacterSpace {
                QChar character = ' ';

                struct CharacterFormat {
                        bool operator==(const CharacterSpace::CharacterFormat&) const = default;
                        bool operator!=(const CharacterSpace::CharacterFormat&) const = default;

                        bool underline = false;
                        bool blink = false;
                        bool invert = false;
                        ScreenColorManager::Color color = 0x00000102;
                        ScreenColorManager::Color backgroundColor = 0x00000002;
                } format;
        };

        static QChar emptyChar();

        int cols();
        void setCols(int cols);

        int rows();
        void setRows(int rows);

        int caretCol();
        void setCaretCol(int col);

        int caretRow();
        void setCaretRow(int row);

        void setCurrentCharacterFormat(CharacterSpace::CharacterFormat format);
        CharacterSpace::CharacterFormat currentCharacterFormat();

        void setCharacter(int col, int row, CharacterSpace character);
        void setCharacter(int col, int row, QChar character);
        CharacterSpace character(int col, int row);
        void pushToHistory();

        void setRowScaleMode(int row, RowScaleMode mode);
        RowScaleMode rowScaleMode(int row);

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
