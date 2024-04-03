#ifndef QMLTERMINALSCREENCONTROLLER_H
#define QMLTERMINALSCREENCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <terminalscreen.h>

struct QmlTerminalScreenControllerPrivate;
class QmlTerminalScreenController : public QObject {
        Q_OBJECT
        Q_PROPERTY(int cols READ cols WRITE setCols NOTIFY colsChanged FINAL)
        Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY rowsChanged FINAL)
        Q_PROPERTY(int caretCol READ caretCol NOTIFY caretColChanged FINAL)
        Q_PROPERTY(int caretRow READ caretRow NOTIFY caretRowChanged FINAL)
        Q_PROPERTY(quint64 scrollbackLines READ scrollbackLines NOTIFY scrollbackLinesChanged FINAL)
        QML_ELEMENT
    public:
        explicit QmlTerminalScreenController(QObject* parent = nullptr);
        ~QmlTerminalScreenController();

        int cols();
        void setCols(int cols);

        int rows();
        void setRows(int rows);

        int caretCol();
        int caretRow();

        quint64 scrollbackLines();

        Q_SCRIPTABLE void start(QString process);
        Q_SCRIPTABLE void pressKey(Qt::KeyboardModifiers modifiers, Qt::Key key, QString keyChar);
        Q_SCRIPTABLE QVariantList runs(int row);
        Q_SCRIPTABLE TerminalScreen::RowScaleMode rowScaleMode(int row);

    signals:
        void colsChanged();
        void rowsChanged();
        void caretColChanged();
        void caretRowChanged();
        void scrollbackLinesChanged();
        Q_SCRIPTABLE void rowContentChanged(int row);

    private:
        QmlTerminalScreenControllerPrivate* d;

        QVariantMap initFormat(TerminalScreen::CharacterSpace::CharacterFormat format);
        void queueRowUpdate(int row);
};

#endif // QMLTERMINALSCREENCONTROLLER_H
