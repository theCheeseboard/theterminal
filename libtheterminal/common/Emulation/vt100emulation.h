#ifndef VT100EMULATION_H
#define VT100EMULATION_H

#include <QObject>

class TerminalScreen;
struct VT100EmulationPrivate;
class VT100Emulation : public QObject {
        Q_OBJECT
    public:
        explicit VT100Emulation(QIODevice* device, TerminalScreen* screen, QObject* parent = nullptr);
        ~VT100Emulation();

        void pressKey(Qt::KeyboardModifiers modifiers, Qt::Key key, QString keyChar);

    signals:

    private:
        VT100EmulationPrivate* d;

        void setupStateMachine();

        void processCharacter(QChar c);
        void write(QString characters);
        void echo(QChar c);

        void escapeEraseInDisplay(QString escapeSequence);
        void escapeCursorPosition(QString escapeSequence);
};

#endif // VT100EMULATION_H
