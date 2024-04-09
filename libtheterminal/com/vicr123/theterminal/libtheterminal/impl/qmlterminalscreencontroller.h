#ifndef QMLTERMINALSCREENCONTROLLER_H
#define QMLTERMINALSCREENCONTROLLER_H

#include <QObject>
#include <QPoint>
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
        Q_PROPERTY(bool invertScreen READ invertScreen NOTIFY invertScreenChanged FINAL)
        Q_PROPERTY(bool caretVisible READ caretVisible NOTIFY caretVisibleChanged FINAL)
        Q_PROPERTY(bool reportMouseEvents READ reportMouseEvents NOTIFY reportMouseEventsChanged FINAL)
        Q_PROPERTY(QPoint selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged FINAL)
        Q_PROPERTY(QPoint selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged FINAL)
        Q_PROPERTY(bool haveSelection READ haveSelection NOTIFY haveSelectionChanged FINAL)

        Q_PROPERTY(QPoint normalisedSelectionStart READ normalisedSelectionStart NOTIFY normalisedSelectionChanged FINAL)
        Q_PROPERTY(QPoint normalisedSelectionEnd READ normalisedSelectionEnd NOTIFY normalisedSelectionChanged FINAL)
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

        QPoint selectionStart();
        void setSelectionStart(QPoint selectionStart);

        QPoint selectionEnd();
        void setSelectionEnd(QPoint selectionEnd);

        QPoint normalisedSelectionStart() const;
        QPoint normalisedSelectionEnd() const;
        bool haveSelection() const;

        bool invertScreen();
        bool caretVisible();
        bool reportMouseEvents();
        quint64 scrollbackLines();

        Q_SCRIPTABLE void start(QString process);
        Q_SCRIPTABLE void pressKey(Qt::KeyboardModifiers modifiers, Qt::Key key, QString keyChar);
        Q_SCRIPTABLE QVariantList runs(int row, int start = 0);
        Q_SCRIPTABLE QVariantList scrollbackRuns(quint64 line, int start = 0);
        Q_SCRIPTABLE TerminalScreen::RowScaleMode rowScaleMode(int row);
        Q_SCRIPTABLE QStringList runningProcesses();

        Q_SCRIPTABLE void copy();
        Q_SCRIPTABLE void paste();

    signals:
        void colsChanged();
        void rowsChanged();
        void caretColChanged();
        void caretRowChanged();
        void scrollbackLinesChanged();
        void invertScreenChanged();
        void caretVisibleChanged();
        void reportMouseEventsChanged();
        void selectionStartChanged();
        void selectionEndChanged();
        void normalisedSelectionChanged();
        void haveSelectionChanged();

        Q_SCRIPTABLE void rowContentChanged(int row);

    private:
        QmlTerminalScreenControllerPrivate* d;

        QVariantList calculateRuns(int start, int cols, std::function<TerminalScreen::CharacterSpace(int)> getCharacter);
        QVariantMap initFormat(TerminalScreen::CharacterSpace::CharacterFormat format);
        void queueRowUpdate(int row);
        QString selectedText();
};

#endif // QMLTERMINALSCREENCONTROLLER_H
