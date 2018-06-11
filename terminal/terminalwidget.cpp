#include "terminalwidget.h"
#include "ui_terminalwidget.h"

TerminalWidget::TerminalWidget(QString workDir, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TerminalWidget)
{
    ui->setupUi(this);

    if (settings.value("terminal/type", "legacy").toString() == "legacy") {
        //Create a legacy terminal part
        legacyTerminalPart = new TerminalPart(workDir);

        QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
        layout->addWidget(legacyTerminalPart);
        layout->setMargin(0);
        ui->legacyTerminalPage->setLayout(layout);

        ui->terminalTypeStack->setCurrentWidget(ui->legacyTerminalPage);
        connect(legacyTerminalPart, SIGNAL(finished()), this, SIGNAL(finished()));
        connect(legacyTerminalPart, SIGNAL(bell(QString)), this, SIGNAL(bell(QString)));
    } else {
        //Set up built in functions
        builtinFunctions.insert("cd", [=](QString line) {
            QString dirString = line.remove(0, 2).trimmed(); //Remove the cd
            QDir dir = workingDirectory;

            if (dirString == "") {
                dir = QDir::home();
            } else if (!dir.cd(dirString)) {
                currentCommandPart->appendOutput(tr("theterminal: cd: %1: No such file or directory").arg(dirString));
                return 1;
            }

            workingDirectory = dir;
            return 0;
        });
        builtinFunctions.insert("ls", [=](QString line) {
            QString dirString = line.remove(0, 2).trimmed(); //Remove the ls
            QStringList args = dirString.split(" ");

            dirString.remove("-a");
            dirString.remove("-l");

            QDir dir = workingDirectory;

            if (dirString != "") {
                if (!dir.cd(dirString)) {
                    currentCommandPart->appendOutput(tr("theterminal: cannot access '%1' No such file or directory").arg(dirString));
                    return 1;
                }
            }

            lsCommand* pane = new lsCommand();
            pane->setDir(dir, args);
            connect(pane, &lsCommand::executeCommands, [=](QStringList commands) {
                awaitingCommands.append(commands);

                if (currentCommandPart == nullptr) prepareForNextCommand();
            });

            currentCommandPart->executeWidget(pane);
            return 0;
        });
        builtinFunctions.insert("export", [=](QString line) {
            QString envString = line.remove(0, 7).trimmed(); //Remove the export
            QString name, value = "";

            if (envString.contains("=")) {
                name = envString.left(envString.indexOf("="));
                value = envString.mid(envString.indexOf("=") + 1);
            } else {
                name = envString;
            }

            currentEnvironment.insert(name, value);
            return 0;
        });

        //Create a new terminal
        ui->commandLine->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        workingDirectory = QDir(workDir);

        commandsLayout = (QBoxLayout*) ui->commands->layout();

        __uid_t currentUid = geteuid();
        passwd* pw = getpwuid(currentUid);
        if (pw == nullptr) {
            currentUser = "unknown user";
        } else {
            currentUser = pw->pw_name;
        }

        currentEnvironment = QProcessEnvironment::systemEnvironment();
        prepareForNextCommand();

        connect(ui->terminalArea->verticalScrollBar(), &QScrollBar::valueChanged, [=](int value) {
            if (value == ui->terminalArea->verticalScrollBar()->maximum()) {
                currentlyAtBottom = true;
            } else {
                currentlyAtBottom = false;
            }
        });
        connect(ui->terminalArea->verticalScrollBar(), &QScrollBar::rangeChanged, [=](int min, int max) {
            if (currentlyAtBottom) ui->terminalArea->verticalScrollBar()->setValue(max);
        });
    }
}

TerminalWidget::~TerminalWidget()
{
    delete ui;
}

void TerminalWidget::prepareForNextCommand() {
    currentCommandPart = nullptr;

    if (awaitingCommands.count() == 0) {
        QString prompt = currentEnvironment.value("PS1", "\\u \\$");
        QDateTime currentTime = QDateTime::currentDateTime();

        prompt.replace("\\n", "");
        prompt.replace("\\$", "$");
        prompt.replace("\\w", workingDirectory.absolutePath());
        prompt.replace("\\d", currentTime.toString("ddd MMM dd"));
        prompt.replace("\\t", currentTime.toString("HH:mm:ss"));
        prompt.replace("\\T", currentTime.toString("hh:mm:ss"));
        prompt.replace("\\A", currentTime.toString("HH:mm"));
        prompt.replace("\\u", currentUser);

        ui->commandLine->setPlaceholderText(prompt);

        ui->statusLabel->setText(workingDirectory.absolutePath());

        ui->commandLine->setVisible(true);
        ui->commandLine->setFocus();
    } else { //Execute the next command
        runCommand(awaitingCommands.takeFirst());
    }
}

void TerminalWidget::copyClipboard() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        legacyTerminalPart->copyClipboard();
    }
}

void TerminalWidget::pasteClipboard() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        legacyTerminalPart->pasteClipboard();
    }
}

void TerminalWidget::toggleShowSearchBar() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        legacyTerminalPart->toggleShowSearchBar();
    }
}

void TerminalWidget::runCommand(QString command) {
    commandHistory.append(command);
    ui->commandLine->setVisible(false);
    QString executable = command.split(" ").first();

    currentCommandPart = new CommandPart(this);
    currentCommandPart->setEnvironment(currentEnvironment);
    commandsLayout->addWidget(currentCommandPart);
    currentCommandPart->setCommandText(command);

    //Check if a builtin function exists
    if (builtinFunctions.contains(executable)) {
        //Run function
        int retval = builtinFunctions.value(executable)(command);
        currentCommandPart->setReturnValue(retval);
        prepareForNextCommand();
        return;
    }

    //Check if executable is itself an executable file
    if (executable.startsWith("/")) {
        QFileInfo file(executable);
        if (file.isExecutable()) {
            connect(currentCommandPart, &CommandPart::finished, [=](int exitCode) {
                prepareForNextCommand();
            });
            adjustCurrentTerminal();

            QTimer::singleShot(0, [=] {
                ui->terminalArea->verticalScrollBar()->setValue(ui->terminalArea->verticalScrollBar()->maximum());
                currentCommandPart->executeCommand(ui->terminalArea->height() - 18, file.path() + "/" + command);
            });
            return;
        }
    }
    if (executable.startsWith("./")) {
        QFileInfo file(workingDirectory.path() + "/" + executable);
        if (file.isExecutable()) {
            connect(currentCommandPart, &CommandPart::finished, [=](int exitCode) {
                prepareForNextCommand();
            });
            adjustCurrentTerminal();

            QTimer::singleShot(0, [=] {
                ui->terminalArea->verticalScrollBar()->setValue(ui->terminalArea->verticalScrollBar()->maximum());
                currentCommandPart->executeCommand(ui->terminalArea->height() - 18, file.path() + "/" + command);
            });
            return;
        }
    }

    //Check if executable exists in the PATH
    if (!executable.startsWith("/")) {
        QStringList dirs = currentEnvironment.value("PATH").split(":");
        for (QString dirString : dirs) {
            QDir dir(dirString);
            for (QFileInfo info : dir.entryInfoList()) {
                if (info.isExecutable()) {
                    if (info.fileName() == executable) {
                        connect(currentCommandPart, &CommandPart::finished, [=](int exitCode) {
                            prepareForNextCommand();
                        });

                        QTimer::singleShot(0, [=] {
                            ui->terminalArea->verticalScrollBar()->setValue(ui->terminalArea->verticalScrollBar()->maximum());
                            currentCommandPart->executeCommand(ui->terminalArea->height() - 18, dir.path() + "/" + command);
                        });
                        return;
                    }
                }
            }
        }
    }

    //Conclusion: command not found :(
    currentCommandPart->appendOutput(tr("theterminal: %1: command not found").arg(executable));
    currentCommandPart->setReturnValue(1);
    prepareForNextCommand();
}

void TerminalWidget::on_commandLine_returnPressed()
{
    runCommand(ui->commandLine->text());
    ui->commandLine->setText("");
}

QDir TerminalWidget::getWorkingDir() {
    return workingDirectory;
}

QProcessEnvironment TerminalWidget::getEnv() {
    return currentEnvironment;
}

void TerminalWidget::resizeEvent(QResizeEvent *event) {
    adjustCurrentTerminal();
}

void TerminalWidget::adjustCurrentTerminal() {
    if (currentCommandPart != nullptr) {
        currentCommandPart->setFixedHeight(ui->terminalArea->height() - 18);
    }
}
