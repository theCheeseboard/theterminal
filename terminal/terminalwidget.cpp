#include "terminalwidget.h"
#include "ui_terminalwidget.h"

#include "history.h"
#include "print/printcontroller.h"
#include "terminalpart.h"
#include "terminalstatus.h"
#include <QBoxLayout>
#include <QDateTime>
#include <QDir>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QProcessEnvironment>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QStackedWidget>
#include <QTextBoundaryFinder>
#include <QTimer>
#include <functional>
#include <tlogger.h>

extern int lookbehindSpace(QString str, int from);
extern int lookaheadSpace(QString str, int from);

struct TerminalWidgetPrivate {
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
        History history;
};

TerminalWidget::TerminalWidget(QString workDir, QString cmd, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::TerminalWidget) {
    d = new TerminalWidgetPrivate();

    ui->setupUi(this);
    ui->autocompletePages->setFixedHeight(0);
    ui->commandLine->installEventFilter(this);

    // Create a legacy terminal part
    TerminalPartConstruct termPartArgs;
    termPartArgs.workDir = workDir;
    termPartArgs.shell = cmd;

    initializeAsLegacy(new TerminalPart(termPartArgs));
}

TerminalWidget::TerminalWidget(TerminalPart* terminal, QWidget* parent) :
    QWidget(parent), ui(new Ui::TerminalWidget) {
    ui->setupUi(this);

    d = new TerminalWidgetPrivate();
    initializeAsLegacy(terminal);
}

TerminalWidget::~TerminalWidget() {
    delete d;
    delete ui;
}

QString TerminalWidget::title() {
    return d->legacyTerminalPart->title();
}

void TerminalWidget::print() {
    PrintController* controller = new PrintController(this);
    controller->confirmAndPerformPrint();
}

void TerminalWidget::initializeAsLegacy(TerminalPart* terminal) {
    d->legacyTerminalPart = terminal;
    this->setFocusProxy(terminal);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->addWidget(d->legacyTerminalPart);
    layout->setContentsMargins(0, 0, 0, 0);
    ui->legacyTerminalPage->setLayout(layout);

    ui->terminalTypeStack->setCurrentWidget(ui->legacyTerminalPage);
    connect(d->legacyTerminalPart, &TerminalPart::closeTerminal, this, &TerminalWidget::finished);
    connect(d->legacyTerminalPart, &TTTermWidget::bell, this, &TerminalWidget::bell);
    connect(d->legacyTerminalPart, &TerminalPart::openNewTerminal, [=](TerminalPart* terminal) {
        emit openNewTerminal(new TerminalWidget(terminal));
    });
    connect(d->legacyTerminalPart, &TTTermWidget::titleChanged, this, &TerminalWidget::titleChanged);
}

void TerminalWidget::copyClipboard() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        d->legacyTerminalPart->copyClipboard();
    }
}

void TerminalWidget::pasteClipboard() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        d->legacyTerminalPart->pasteClipboard();
    }
}

void TerminalWidget::toggleShowSearchBar() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        d->legacyTerminalPart->toggleShowSearchBar();
    }
}

void TerminalWidget::zoomIn() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        d->legacyTerminalPart->zoomIn();
    }
}

void TerminalWidget::zoomOut() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        d->legacyTerminalPart->zoomOut();
    }
}

void TerminalWidget::zoom100() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        d->legacyTerminalPart->zoom100();
    }
}

QDir TerminalWidget::getWorkingDir() {
    return d->workingDirectory;
}

QProcessEnvironment TerminalWidget::getEnv() {
    return d->currentEnvironment;
}

void TerminalWidget::resizeEvent(QResizeEvent* event) {
    adjustCurrentTerminal();
}

void TerminalWidget::adjustCurrentTerminal() {
    if (d->currentCommandPart != nullptr) {
        // currentCommandPart->setFixedHeight(ui->terminalArea->height() + ui->autocompletePages->height() - 18);
    }
}

void TerminalWidget::close() {
    if (d->legacyTerminalPart == nullptr) {
        // Using Contemporary terminal
        if (d->currentCommand != "") {
            emit switchToThis();
            QMessageBox* box = new QMessageBox();
            box->setParent(this->window());

#ifdef Q_OS_MAC
            box->setText(tr("Close this tab?"));
            box->setInformativeText(tr("Closing this tab will also close %1.").arg(d->currentCommand));
            box->setIconPixmap(QIcon(":/icons/utilities-terminal.svg").pixmap(48, 48));
            box->setWindowFlags(Qt::Sheet);
#else
            box->setWindowTitle(tr("Close tab?"));
            box->setText(tr("Closing this tab will also close %1.").arg(d->currentCommand));
#endif
            box->setStandardButtons(QMessageBox::Close | QMessageBox::Cancel);
            if (box->exec() == QMessageBox::Close) {
                emit finished();
            }
        } else {
            emit finished();
        }
    } else {
        // Using theterminal terminal
        d->legacyTerminalPart->tryClose();
        // emit finished();
    }
}
