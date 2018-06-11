#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QStackedWidget>
#include <QLineEdit>
#include <QFontDatabase>
#include <QProcessEnvironment>
#include <QDir>
#include <QDateTime>
#include <QLabel>
#include <unistd.h>
#include <pwd.h>
#include <functional>
#include <QBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include "graphicalParts/lscommand.h"
#include "terminalpart.h"
#include "commandpart.h"

class CommandPart;

namespace Ui {
    class TerminalWidget;
}

class TerminalWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit TerminalWidget(QString workDir = "", QWidget *parent = 0);
        ~TerminalWidget();

    public slots:
        void copyClipboard();
        void pasteClipboard();
        void toggleShowSearchBar();

        QDir getWorkingDir();
        QProcessEnvironment getEnv();

    private slots:
        void prepareForNextCommand();
        void runCommand(QString command);
        void adjustCurrentTerminal();

        void on_commandLine_returnPressed();

    signals:
        void finished();
        void bell(QString message);

    private:
        Ui::TerminalWidget *ui;
        QSettings settings;
        TerminalPart* legacyTerminalPart;

        QProcessEnvironment currentEnvironment;
        QDir workingDirectory;
        QString currentUser;
        QBoxLayout* commandsLayout;
        QMap<QString, std::function<int(QString)>> builtinFunctions;
        CommandPart* currentCommandPart = nullptr;
        bool currentlyAtBottom = true;
        QStringList awaitingCommands;
        QStringList commandHistory;

        void resizeEvent(QResizeEvent* event);
};

#endif // TERMINALWIDGET_H
