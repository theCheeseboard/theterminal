#include "terminalwidget.h"
#include "ui_terminalwidget.h"

#include "graphicalParts/lscommand.h"
#include "graphicalParts/ttedcommand.h"

extern int lookbehindSpace(QString str, int from);
extern int lookaheadSpace(QString str, int from);
extern QStringList splitSpaces(QString str);

TerminalWidget::TerminalWidget(QString workDir, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TerminalWidget)
{
    ui->setupUi(this);
    ui->autocompletePages->setFixedHeight(0);
    ui->commandLine->installEventFilter(this);

    if (settings.value("terminal/type", "legacy").toString() == "legacy") {
        //Create a legacy terminal part
        TerminalPartConstruct termPartArgs;
        termPartArgs.workDir = workDir;

        initializeAsLegacy(new TerminalPart(termPartArgs));
    } else {
        ui->terminalTypeStack->setCurrentWidget(ui->newTerminalPage);
        //Set up built in functions
        builtinFunctions.insert("cd", [=](QString line) {
            QStringList args = splitSpaces(line);
            args.takeFirst();

            if (args.length() > 1) {
                currentCommandPart->appendOutput(tr("theterminal: cd: too many arguments"));
                return 1;
            }

            QString dirString;
            if (args.count() == 1) dirString = args.takeFirst();
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
            connect(pane, SIGNAL(scrollToBottom()), this, SLOT(scrollToBottom()));

            currentCommandPart->executeWidget(pane);
            return 0;
        });
        builtinFunctions.insert("tted", [=](QString line) {
            QStringList args = splitSpaces(line);
            args.takeFirst();

            if (args.contains("--help") || args.contains("-")) {
                currentCommandPart->appendOutput(tr("Usage: tted [OPTIONS] [FILE]\n"
                                                    "-h, --help                   Show this help output\n"));
                return 0;
            }

            if (args.length() < 1) {
                currentCommandPart->appendOutput(tr("theterminal: tted: not enough arguments"));
                return 1;
            }

            for (QString file : args) {
                QDir dir = workingDirectory;
                int lastDir = file.lastIndexOf("/") + 1;
                if (lastDir != 0) {
                    if (!dir.cd(file.left(lastDir))) {
                        currentCommandPart->appendOutput(tr("theterminal: tted: cannot access '%1' No such file or directory").arg(file));
                        return 1;
                    }
                    file = file.mid(lastDir);
                }

                ttedCommand* pane = new ttedCommand();
                pane->loadFile(dir.path() + "/" + file);
                currentCommandPart->executeWidget(pane);
            }
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
        builtinFunctions.insert("exit", [=](QString line) {
            QTimer::singleShot(0, this, SIGNAL(finished()));
            return 0;
        });
        builtinFunctions.insert("clear", [=](QString line) {
            for (CommandPart* part : commandParts) {
                part->close();
            }
            return 0;
        });

        //Create a new terminal
        ui->commandLine->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        workingDirectory = QDir(workDir);

        autocompleteAnimation = new tVariantAnimation();
        autocompleteAnimation->setEasingCurve(QEasingCurve::OutCubic);
        autocompleteAnimation->setDuration(250);
        connect(autocompleteAnimation, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->autocompletePages->setFixedHeight(value.toInt());
        });
        connect(this, SIGNAL(destroyed(QObject*)), autocompleteAnimation, SLOT(stop()));

        commandsLayout = (QBoxLayout*) ui->commands->layout();

#ifdef Q_OS_MAC
        uid_t currentUid = geteuid();
#else
        __uid_t currentUid = geteuid();
#endif
        passwd* pw = getpwuid(currentUid);
        if (pw == nullptr) {
            currentUser = "unknown user";
        } else {
            currentUser = pw->pw_name;
        }

        currentEnvironment = QProcessEnvironment::systemEnvironment();
        currentEnvironment.insert("TERM", "xterm");
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

TerminalWidget::TerminalWidget(TerminalPart* terminal, QWidget* parent) : QWidget(parent), ui(new Ui::TerminalWidget) {
    ui->setupUi(this);

    initializeAsLegacy(terminal);
}

TerminalWidget::~TerminalWidget()
{
    delete ui;
}

void TerminalWidget::initializeAsLegacy(TerminalPart *terminal) {
    legacyTerminalPart = terminal;

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->addWidget(legacyTerminalPart);
    layout->setMargin(0);
    ui->legacyTerminalPage->setLayout(layout);

    ui->terminalTypeStack->setCurrentWidget(ui->legacyTerminalPage);
    connect(legacyTerminalPart, SIGNAL(closeTerminal()), this, SIGNAL(finished()));
    connect(legacyTerminalPart, SIGNAL(bell(QString)), this, SIGNAL(bell(QString)));
    connect(legacyTerminalPart, &TerminalPart::openNewTerminal, [=](TerminalPart* terminal) {
        emit openNewTerminal(new TerminalWidget(terminal));
    });
}

void TerminalWidget::prepareForNextCommand() {
    currentCommandPart = nullptr;
    currentCommand = "";

    if (awaitingCommands.count() == 0) {
        QString prompt = currentEnvironment.value("PS1", "\\u \\$");
        QDateTime currentTime = QDateTime::currentDateTime();

        QString shellPrompt;
        if (geteuid() == 0) {
            shellPrompt = "#";
        } else {
            shellPrompt = "$";
        }

        prompt.replace("\\n", "");
        prompt.replace("\\$", shellPrompt);
        prompt.replace("\\w", workingDirectory.absolutePath());
        prompt.replace("\\d", currentTime.toString("ddd MMM dd"));
        prompt.replace("\\t", currentTime.toString("HH:mm:ss"));
        prompt.replace("\\T", currentTime.toString("hh:mm:ss"));
        prompt.replace("\\A", currentTime.toString("HH:mm"));
        prompt.replace("\\u", currentUser);

        ui->commandLine->setPlaceholderText(prompt);

        ui->statusWidget->setDir(workingDirectory);

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

void TerminalWidget::zoomIn() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        legacyTerminalPart->zoomIn();
    }
}

void TerminalWidget::zoomOut() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        legacyTerminalPart->zoomOut();
    }
}

void TerminalWidget::zoom100() {
    if (ui->terminalTypeStack->currentWidget() == ui->legacyTerminalPage) {
        legacyTerminalPart->zoom100();
    }
}

void TerminalWidget::runCommand(QString command) {
    if (command.trimmed() == "") {
        history.clearState();
        return;
    }
    history.appendToHistory(command);
    ui->commandLine->setVisible(false);

    currentCommandPart = new CommandPart(this);
    currentCommandPart->setEnvironment(currentEnvironment);
    commandsLayout->addWidget(currentCommandPart);
    commandParts.append(currentCommandPart);
    currentCommandPart->setCommandText(command);

    CommandPart* part = currentCommandPart;
    connect(currentCommandPart, &CommandPart::dismiss, [=] {
        commandParts.removeOne(part);
        commandsLayout->removeWidget(part);
        part->deleteLater();
    });

    QStringList pipeline = command.split("|");
    QProcess* previousProcess = nullptr;

    for (int i = 0; i < pipeline.count(); i++) {
        command = pipeline.at(i).trimmed();
        currentCommand = command;
        bool isLast = (i == pipeline.count() - 1);
        QString executable = command.split(" ").first();

        //Check if a builtin function exists
        if (builtinFunctions.contains(executable)) {
            //Run function
            int retval = builtinFunctions.value(executable)(command);
            currentCommandPart->setReturnValue(retval);
            prepareForNextCommand();
            return;
        }

        QString cmd = executableSearch(executable, command);

        if (cmd != "") {
            if (isLast) {
                connect(currentCommandPart, &CommandPart::finished, [=](int exitCode) {
                    prepareForNextCommand();
                });
                adjustCurrentTerminal();

                QTimer::singleShot(0, [=] {
                    ui->terminalArea->verticalScrollBar()->setValue(ui->terminalArea->verticalScrollBar()->maximum());
                    currentCommandPart->executeCommand(ui->terminalArea->height() - 18, previousProcess, cmd);
                });
            } else {
                QProcess* proc = new QProcess;
                QStringList args = splitSpaces(cmd);
                QString executable = args.takeFirst();
                proc->setArguments(args);
                connect(proc, SIGNAL(finished(int)), proc, SLOT(deleteLater()));

                if (previousProcess != nullptr) {
                    previousProcess->setStandardOutputProcess(proc);
                }

                proc->start(executable);
                previousProcess = proc;
            }
        } else {
            //Conclusion: command not found :(
            currentCommandPart->appendOutput(tr("theterminal: %1: command not found").arg(executable));
            currentCommandPart->setReturnValue(1);
            prepareForNextCommand();
        }
    }
}

QString TerminalWidget::executableSearch(QString executable, QString command) {

    //Check if executable is itself an executable file
    if (executable.startsWith("/")) {
        QFileInfo file(executable);
        if (file.isExecutable()) {
            return file.path() + "/" + command;
        }
    }
    if (executable.startsWith("./")) {
        QFileInfo file(workingDirectory.path() + "/" + executable);
        if (file.isExecutable()) {
            return file.path() + "/" + command;
        }
    }

    //Check if executable exists in the PATH
    if (!executable.startsWith("/")) {
        QStringList dirs = currentEnvironment.value("PATH").split(":");
        for (QString dirString : dirs) {
            QDir dir(dirString);
            for (QFileInfo info : dir.entryInfoList(QDir::Files | QDir::Executable)) {
                if (info.fileName() == executable) {
                    return dir.path() + "/" + command;
                }
            }
        }
    }

    return "";
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
        //currentCommandPart->setFixedHeight(ui->terminalArea->height() + ui->autocompletePages->height() - 18);
    }
}

void TerminalWidget::on_commandLine_cursorPositionChanged(int arg1, int arg2)
{
    ui->fileAutocompleteWidget->blockSignals(true);
    ui->fileAutocompleteWidget->clear();
    ui->fileAutocompleteWidget->blockSignals(false);

    QTextBoundaryFinder boundaries(QTextBoundaryFinder::Word, ui->commandLine->text());
    boundaries.setPosition(arg2);

    int startPos = ui->commandLine->text().lastIndexOf(" ", arg2 - 1) + 1;
    int endPos = ui->commandLine->text().indexOf(" ", arg2);

    if (startPos == -1) startPos = 0;
    if (endPos == -1) endPos = ui->commandLine->text().length();
    QString word = ui->commandLine->text().mid(startPos, endPos - startPos);

    autocompleteInitialStart = startPos;
    autocompleteInitialWord = word;

    QDir::Filters commandFilters = QDir::Dirs | QDir::Files;
    QString command = ui->commandLine->text().left(ui->commandLine->text().indexOf(" "));
    if (command == "cd") {
        commandFilters = QDir::Dirs;
    } else if (command.startsWith("/")) {
        commandFilters = QDir::Dirs | QDir::Files | QDir::Executable;
    }

    QList<QListWidgetItem*> items;
    if (word.startsWith("$")) { //Search environment variables
        for (QString key : currentEnvironment.keys()) {
            if (key.startsWith(word.mid(1))) {
                QString value = currentEnvironment.value(key);

                QListWidgetItem* item = new QListWidgetItem();
                item->setText("$" + key);
                items.append(item);
            }
        }
    } else if (startPos == 0 && !command.startsWith("/") && command.length() != 0) { //Search commands
        QStringList dirs = currentEnvironment.value("PATH").split(":");
        QStringList knownExecutables;

        auto addExecutable = [=, &items, &knownExecutables](QString name) {
            if (name.startsWith(word) && !knownExecutables.contains(name)) {
                knownExecutables.append(name);
                QListWidgetItem* item = new QListWidgetItem();
                item->setText(name);
                items.append(item);
            }
        };

        for (QString command : builtinFunctions.keys()) {
            addExecutable(command);
        }

        for (QString dirString : dirs) {
            QDir dir(dirString);
            for (QFileInfo info : dir.entryInfoList(QDir::Files | QDir::Executable)) {
                addExecutable(info.fileName());
            }
        }
    } else { //Search files
        bool performSearch = false;

        QDir dir = workingDirectory;
        int lastDir = word.lastIndexOf("/") + 1;
        if (lastDir != 0) {
            performSearch = true; //Always perform search on files when finding /
            autocompleteInitialStart = startPos + lastDir;
            dir.cd(word.left(lastDir));
            word = word.mid(lastDir);
        }

        if (word.length() != 0) performSearch = true; //Perform search when there is something to autocomplete

        if (performSearch) {
            QFileInfoList files = dir.entryInfoList(commandFilters);
            for (QFileInfo info : files) {
                if (info.fileName().startsWith(word)) {
                    QListWidgetItem* item = new QListWidgetItem();
                    item->setText(info.fileName());
                    item->setIcon(QIcon::fromTheme(mimedb.mimeTypeForFile(info).iconName()));
                    items.append(item);
                }
            }
        }
    }

    if (items.count() > 0) {
        for (QListWidgetItem* item : items) {
            ui->fileAutocompleteWidget->addItem(item);
        }
        openAutocomplete();
    } else {
        closeAutocomplete();
    }
}

void TerminalWidget::openAutocomplete() {
    autocompleteAnimation->setStartValue(ui->autocompletePages->height());
    autocompleteAnimation->setEndValue(qMin(200, ui->fileAutocompleteWidget->count() * (ui->fileAutocompleteWidget->sizeHintForRow(0))));
    autocompleteAnimation->start();

    autocompleteOpen = true;
}

void TerminalWidget::closeAutocomplete() {
    if (autocompleteOpen) {
        autocompleteAnimation->setStartValue(ui->autocompletePages->height());
        autocompleteAnimation->setEndValue(0);
        autocompleteAnimation->start();

        autocompleteOpen = false;
    }
}

bool TerminalWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->commandLine) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* e = (QKeyEvent*) event;
            switch (e->key()) {
                case Qt::Key_Down:
                case Qt::Key_Tab:
                    if (autocompleteOpen) {
                        //Go down through autocomplete
                        int newIndex = ui->fileAutocompleteWidget->currentRow() + 1;
                        if (newIndex == ui->fileAutocompleteWidget->count()) {
                            ui->fileAutocompleteWidget->setCurrentRow(-1);
                            ui->fileAutocompleteWidget->clearSelection();
                        } else {
                            ui->fileAutocompleteWidget->setCurrentRow(newIndex);
                        }
                    } else {
                        //Go down through history
                        //Disable autocomplete
                        ui->commandLine->blockSignals(true);
                        ui->commandLine->setText(history.historyDown(ui->commandLine->text()));
                        ui->commandLine->blockSignals(false);
                    }
                    break;
                case Qt::Key_Up:
                    if (autocompleteOpen) {
                        //Go up through autocomplete
                        int newIndex = ui->fileAutocompleteWidget->currentRow() - 1;
                        if (newIndex == -1) {
                            ui->fileAutocompleteWidget->setCurrentRow(-1);
                            ui->fileAutocompleteWidget->clearSelection();
                        } else if (newIndex == -2) {
                            ui->fileAutocompleteWidget->setCurrentRow(ui->fileAutocompleteWidget->count() - 1);
                        } else {
                            ui->fileAutocompleteWidget->setCurrentRow(newIndex);
                        }
                    } else {
                        //Go up through history
                        //Disable autocomplete
                        ui->commandLine->blockSignals(true);
                        ui->commandLine->setText(history.historyUp(ui->commandLine->text()));
                        ui->commandLine->blockSignals(false);
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }
    }
    return false;
}

void TerminalWidget::on_fileAutocompleteWidget_currentRowChanged(int currentRow)
{
    ui->commandLine->blockSignals(true);
    QString wordToReplace;
    if (currentRow == -1) {
        wordToReplace = autocompleteInitialWord;
    } else {
        wordToReplace = ui->fileAutocompleteWidget->item(currentRow)->text();
        if (wordToReplace.contains(" ")) {
            wordToReplace.prepend("\"");
            wordToReplace.append("\"");
        }
    }

    QString currentText = ui->commandLine->text();
    int endPos = lookaheadSpace(currentText, autocompleteInitialStart);
    if (endPos == -1) endPos = currentText.length();

    currentText.replace(autocompleteInitialStart, endPos - autocompleteInitialStart, wordToReplace);
    ui->commandLine->setText(currentText);

    ui->commandLine->blockSignals(false);
}

void TerminalWidget::scrollToBottom() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(ui->terminalArea->verticalScrollBar()->value());
    anim->setEndValue(ui->terminalArea->verticalScrollBar()->maximum());
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(250);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        anim->setEndValue(ui->terminalArea->verticalScrollBar()->maximum());
        ui->terminalArea->verticalScrollBar()->setValue(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}

void TerminalWidget::close() {
    if (legacyTerminalPart == nullptr) {
        //Using Contemporary terminal
        if (currentCommand != "") {
            emit switchToThis();
            QMessageBox* box = new QMessageBox();
            box->setParent(this->window());

#ifdef Q_OS_MAC
            box->setText(tr("Close this tab?"));
            box->setInformativeText(tr("Closing this tab will also close %1.").arg(currentCommand));
            box->setIconPixmap(QIcon(":/icons/utilities-terminal.svg").pixmap(48, 48));
            box->setWindowFlags(Qt::Sheet);
#else
            box->setWindowTitle(tr("Close tab?"));
            box->setText(tr("Closing this tab will also close %1.").arg(currentCommand));
#endif
            box->setStandardButtons(QMessageBox::Close | QMessageBox::Cancel);
            if (box->exec() == QMessageBox::Close) {
                emit finished();
            }
        } else {
            emit finished();
        }
    } else {
        //Using legacy terminal
        legacyTerminalPart->tryClose();
        //emit finished();
    }
}
