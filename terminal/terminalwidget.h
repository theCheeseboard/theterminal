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
#include <QListWidget>
#include <QTextBoundaryFinder>
#include <QMimeDatabase>
#include <QMessageBox>
#include "terminalpart.h"
#include "commandpart.h"
#include "terminalstatus.h"
#include "history.h"

class CommandPart;

namespace Ui {
    class TerminalWidget;
}

class TerminalWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit TerminalWidget(QString workDir = "", QString cmd = "", QWidget *parent = nullptr);
        ~TerminalWidget();

    public slots:
        void copyClipboard();
        void pasteClipboard();
        void toggleShowSearchBar();
        void close();
        void zoomIn();
        void zoomOut();
        void zoom100();

        QDir getWorkingDir();
        QProcessEnvironment getEnv();

    private slots:
        void prepareForNextCommand();
        void runCommand(QString command);
        void adjustCurrentTerminal();
        void scrollToBottom();

        QString executableSearch(QString executable, QString command);

        void openAutocomplete();
        void closeAutocomplete();

        void on_commandLine_returnPressed();

        void on_commandLine_cursorPositionChanged(int arg1, int arg2);

        void on_fileAutocompleteWidget_currentRowChanged(int currentRow);

    signals:
        void finished();
        void bell(QString message);
        void switchToThis();
        void openNewTerminal(TerminalWidget* terminalWidget);

    private:
        explicit TerminalWidget(TerminalPart* terminal, QWidget* parent = nullptr);
        void initializeAsLegacy(TerminalPart* terminal);

        Ui::TerminalWidget *ui;
        QSettings settings;
        TerminalPart* legacyTerminalPart = nullptr;

        QProcessEnvironment currentEnvironment;
        QDir workingDirectory;
        QString currentUser;
        QBoxLayout* commandsLayout;
        QMap<QString, std::function<int(QString)>> builtinFunctions;
        CommandPart* currentCommandPart = nullptr;
        QList<CommandPart*> commandParts;
        bool currentlyAtBottom = true;
        QStringList awaitingCommands;
        bool autocompleteOpen = false;
        QString currentCommand = "";

        QString autocompleteInitialWord;
        int autocompleteInitialStart;

        QMimeDatabase mimedb;
        tVariantAnimation* autocompleteAnimation;
        History history;

        void resizeEvent(QResizeEvent* event);
        bool eventFilter(QObject *watched, QEvent *event);
};

#endif // TERMINALWIDGET_H
