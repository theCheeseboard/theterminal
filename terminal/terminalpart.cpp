#include "terminalpart.h"

#include "terminalcontroller.h"
#include <QFontDatabase>
#include <QMenu>
#include <QStack>
#include <tapplication.h>
#include <tnotification.h>
#include <tpopover.h>
#include <tsystemsound.h>

#include "dialogs/busydialog.h"

class TerminalPartPrivate {
    public:
        QSettings settings;

        enum QuitType {
            UseSettings = -1,
            AlwaysQuit = 0,
            NeverQuit,
            QuitOnCleanExit
        };

        QFont defaultFont;
        int zoomLevel = 0;
        bool copyOk = false;

        QuitType quitType = UseSettings;
        bool quitNeeded = false;
};

/*TerminalPart::TerminalPart(bool connectPty, QWidget* parent) : TTTermWidget(0, connectPty, parent) {
    d = new TerminalPartPrivate();
    setup();
}

TerminalPart::TerminalPart(QString workDir, QWidget *parent) : TTTermWidget(0, true, parent)
{
    d = new TerminalPartPrivate();
    setup();

    QStringList environment;
    environment.append("TERM=xterm");

    this->setEnvironment(environment);

    if (workDir != "") {
        this->setWorkingDirectory(workDir);
    }

    this->update();
    this->startShellProgram();
}*/

TerminalPart::TerminalPart(TerminalPartConstruct args, QWidget* parent) :
    TTTermWidget(0, args.connectPty, parent) {
    d = new TerminalPartPrivate();
    setup();

    // Set environment variables
    QProcessEnvironment currentEnvironment = QProcessEnvironment::systemEnvironment();
    currentEnvironment.insert("TERM", "xterm-256color");
    this->setEnvironment(currentEnvironment.toStringList());

    if (args.workDir != "") {
        this->setWorkingDirectory(args.workDir);
    }

    this->update();

    if (args.manPage != "") {
        auto manExecutable = libContemporaryCommon::searchInPath("man");
        if (manExecutable.isEmpty()) {
            this->setShellProgram("echo");
            this->setArgs({tr("theterminal: %1: command not found").arg("man")});
            d->quitType = TerminalPartPrivate::NeverQuit;
        } else {
            this->setShellProgram(manExecutable.first());
            this->setArgs({args.manPage});
            d->quitType = TerminalPartPrivate::QuitOnCleanExit;
        }
    } else if (args.shell != "") {
        this->setArgs(args.shellArgs);
        QString shellProgram = args.shell;
        if (!shellProgram.startsWith("/")) {
            QStringList availableApps = libContemporaryCommon::searchInPath(shellProgram);
            if (availableApps.count() > 0) {
                shellProgram = availableApps.first();
            } else {
                this->setArgs({tr("theterminal: %1: command not found").arg(shellProgram)});
                shellProgram = "echo";
                d->quitType = TerminalPartPrivate::NeverQuit;
            }
        }
        this->setShellProgram(shellProgram);
    }

    if (args.startShell) {
        this->startShellProgram();
    }
}

TerminalPart::~TerminalPart() {
    emit destroying();
    disconnect(this, nullptr, this, nullptr);
    delete d;
}

void TerminalPart::setup() {
    // Register this terminal part
    TerminalController::addTerminalPart(this);

    this->setScrollBarPosition(TTTermWidget::ScrollBarRight);
    this->setFlowControlEnabled(true);
    this->setHistorySize(d->settings.value("term/scrollback", -1).toInt());
    this->setShellProgram(d->settings.value("term/program", qgetenv("SHELL")).toString());

    reloadThemeSettings();

    connect(this, &TerminalPart::copyAvailable, this, [this](bool copyAvailable) {
        d->copyOk = copyAvailable;
    });

    connect(this, &TerminalPart::bell, this, [this] {
        if (this->hasFocus()) {
            // Active terminal
            if (d->settings.value("bell/bellActiveSound", true).toBool()) {
                // Play an audible bell
                tSystemSound::play("bell");
            }
        } else {
            // Inactive terminal
            bool bellSound = d->settings.value("bell/bellInactiveSound", true).toBool();
            bool notification = d->settings.value("bell/bellInactiveNotification", true).toBool();
            if (bellSound && !notification) {
                // Play an audible bell
                tSystemSound::play("bell");
            } else if (bellSound && notification) {
                // Post a system notification with the bell sound
                tNotification* notification = new tNotification();
                notification->setSummary(tr("Bell"));
                notification->setText(tr("A bell was sounded!"));
                notification->setAppName("theTerminal");
                notification->setAppIcon("utilities-terminal");
                notification->setSound(tSystemSound::soundLocation("bell"));
                notification->post(true);
            }
        }
    });
    connect(this, &TerminalPart::shellProgramFinished, this, [this](int exitCode) {
        if (d->quitType == TerminalPartPrivate::UseSettings) {
            d->quitType = static_cast<TerminalPartPrivate::QuitType>(d->settings.value("term/quitType", 0).toInt());
        }

        switch (d->quitType) {
            case TerminalPartPrivate::AlwaysQuit:
                d->quitNeeded = true;
                break;
            case TerminalPartPrivate::NeverQuit:
                d->quitNeeded = false;
                break;
            case TerminalPartPrivate::QuitOnCleanExit:
                if (exitCode == 0) {
                    d->quitNeeded = true;
                } else {
                    d->quitNeeded = false;
                }
        }
    });
    connect(this, &TerminalPart::finished, this, [=] {
        if (d->quitNeeded) emit closeTerminal();
    });

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &TerminalPart::customContextMenuRequested, [=](QPoint pos) {
        QMenu* menu = new QMenu();

        if (this->selectedText(false) != "") {
            menu->addSection(tr("For text \"%1\"").arg(menu->fontMetrics().elidedText(this->selectedText(false).trimmed(), Qt::ElideMiddle, SC_DPI_W(200, menu))));
            QAction* copyAction = menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), [=] {
                this->copyClipboard();
            });
            copyAction->setEnabled(canCopy());

            menu->addAction(tr("Open man page"), [=] {
                TerminalPartConstruct terminalPartArgs;
                terminalPartArgs.manPage = this->selectedText(false);
                emit openNewTerminal(new TerminalPart(terminalPartArgs));
            });
        }

        menu->addSection(tr("For this terminal"));
        menu->addAction(QIcon::fromTheme("edit-paste"), tr("Paste"), [=] {
            this->pasteClipboard();
        });
        menu->addAction(QIcon::fromTheme("edit-find"), tr("Find"), [=] {
            this->toggleShowSearchBar();
        });
        menu->addAction(QIcon::fromTheme("dialog-close"), tr("Close Terminal"), [=] {
            this->tryClose();
        });

        menu->exec(this->mapToGlobal(pos));
    });

    this->update();
}

bool TerminalPart::canCopy() {
    return d->copyOk;
}

void TerminalPart::resizeEvent(QResizeEvent* event) {
}

void TerminalPart::zoomIn() {
    d->zoomLevel++;
    QFont f = d->defaultFont;
    f.setPointSize(f.pointSize() + d->zoomLevel);
    this->setTerminalFont(f);
}

void TerminalPart::zoomOut() {
    d->zoomLevel--;
    QFont f = d->defaultFont;
    f.setPointSize(f.pointSize() + d->zoomLevel);
    this->setTerminalFont(f);
}

void TerminalPart::zoom100() {
    d->zoomLevel = 0;
    this->setTerminalFont(d->defaultFont);
}

void TerminalPart::reloadThemeSettings() {
    QString colorThemePath;
#ifdef Q_OS_MAC
    colorThemePath = tApplication::macOSBundlePath() + "/Contents/Frameworks/tttermwidget.framework/Resources/color-schemes/";
#else
    colorThemePath = "/usr/share/tttermwidget/color-schemes/";
#endif
    this->setColorScheme(QString(colorThemePath + "%1.colorscheme").arg(d->settings.value("theme/scheme", "Linux").toString()));
    this->setBlinkingCursor(d->settings.value("theme/blinkCursor", true).toBool());

    d->defaultFont.setFamily(d->settings.value("theme/fontFamily", QFontDatabase::systemFont(QFontDatabase::FixedFont).family()).toString());
    d->defaultFont.setPointSize(d->settings.value("theme/fontSize", QFontDatabase::systemFont(QFontDatabase::FixedFont).pointSize()).toInt());

    switch (d->settings.value("theme/cursorType", 0).toInt()) {
        case 0:
            this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::BlockCursor);
            break;
        case 1:
            this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::UnderlineCursor);
            break;
        case 2:
            this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::IBeamCursor);
            break;
    }

    QFont f = d->defaultFont;
    f.setPointSize(f.pointSize() + d->zoomLevel);
    this->setTerminalFont(f);

    this->setTerminalOpacity(d->settings.value("theme/opacity", 100).toInt() / 100.0);
    // this->setScrollOnKeypress(d->settings.value("scrolling/scrollOnKeystroke", true).toBool());
}

void TerminalPart::tryClose() {
    if (this->isBusy()) {
        /*if (QMessageBox::warning(this, tr("Busy"), tr("This terminal is busy. Do you want to close the terminal and kill any apps running within it?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
            return;
        }*/

        BusyDialog* dialog = new BusyDialog(this->runningProcesses());
        tPopover* popover = new tPopover(dialog);
        popover->setPopoverWidth(SC_DPI(300));
        connect(popover, &tPopover::dismissed, dialog, &BusyDialog::deleteLater);
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        connect(dialog, &BusyDialog::dismiss, popover, &tPopover::dismiss);
        connect(dialog, &BusyDialog::accept, this, &TerminalPart::closeTerminal);
        popover->show(this);
    } else {
        emit closeTerminal();
    }
}
