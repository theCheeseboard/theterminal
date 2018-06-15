#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QString workDir, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    ui->tabFrame->setVisible(false);
    tabBar = new QTabBar();
    tabBar->setDocumentMode(true);
    tabBar->setTabsClosable(true);
    ui->centralWidget->layout()->addWidget(tabBar);

    connect(tabBar, &QTabBar::currentChanged, [=](int index) {
        if (index != -1) {
            changeToTerminal(allTerminals.value(index));
        }
    });
    connect(tabBar, &QTabBar::tabCloseRequested, [=](int index) {
        allTerminals.value(index)->close();
    });
#endif

    this->addTerminal(workDir);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_Window_triggered()
{
    MainWindow* win = new MainWindow();
    win->show();
}

void MainWindow::on_actionExit_triggered()
{
    for (QWidget* w : QApplication::allWidgets()) {
        if (strcmp(w->metaObject()->className(), "MainWindow") == 0) {
            MainWindow* win = (MainWindow*) w;
            win->close();
        }
    }
}

void MainWindow::on_actionCopy_triggered()
{
    currentTerminal->copyClipboard();
}

void MainWindow::on_actionPaste_triggered()
{
    currentTerminal->pasteClipboard();
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QMenu* menu = new QMenu();

    menu->addAction(ui->actionCopy);
    menu->addAction(ui->actionPaste);

    menu->exec(currentTerminal->mapToGlobal(pos));
}

void MainWindow::addTerminal(QString workDir) {
    TerminalWidget* widget = new TerminalWidget(workDir);
    widget->setContextMenuPolicy(Qt::CustomContextMenu);
    allTerminals.append(widget);

#ifndef Q_OS_MAC
    QPushButton* button = new QPushButton();
    button->setCheckable(true);
    terminalButtons.insert(widget, button);
#endif

    connect(widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(widget, &TerminalWidget::finished, [=]() {
        int index = allTerminals.indexOf(widget);
#ifdef Q_OS_MAC
        tabBar->removeTab(index);
#else
        delete terminalButtons.value(widget);
        terminalButtons.remove(widget);
#endif
        allTerminals.removeOne(widget);
        delete widget;

        if (allTerminals.count() == 0) {
            this->close();
        } else {
            if (index == 0) {
                changeToTerminal(0);
            } else {
                changeToTerminal(index - 1);
            }
        }
    });
    connect(widget, &TerminalWidget::switchToThis, [=] {
        changeToTerminal(widget);
    });
    /*connect(widget, &TerminalWidget::copyAvailable, [=](bool canCopy) {
        if (currentTerminal == widget) {
            ui->actionCopy->setEnabled(canCopy);
        }
    });*/

#ifdef Q_OS_MAC
    tabBar->addTab("Terminal " + QString::number(allTerminals.indexOf(widget) + 1));
#else
    button->setText("Terminal " + QString::number(allTerminals.indexOf(widget) + 1));
    ui->tabFrame->layout()->addWidget(button);
    ui->tabFrame->layout()->removeItem(ui->horizontalSpacer);
    ui->tabFrame->layout()->addItem(ui->horizontalSpacer);
    connect(button, &QPushButton::clicked, [=]() {
        changeToTerminal(widget);
    });
#endif

    ui->centralWidget->layout()->addWidget(widget);

    changeToTerminal(widget);
}

void MainWindow::changeToTerminal(TerminalWidget *widget) {
    for (TerminalWidget* terminal : allTerminals) {
        terminal->setVisible(false);
    }

#ifdef Q_OS_MAC
    tabBar->setCurrentIndex(allTerminals.indexOf(widget));
#else
    for (QPushButton* button : terminalButtons.values()) {
        button->setChecked(false);
    }
    terminalButtons.value(widget)->setChecked(true);
#endif

    widget->setVisible(true);
    widget->setFocus();
    currentTerminal = widget;

    //ui->actionCopy->setEnabled(widget->canCopy());
}

void MainWindow::changeToTerminal(int index) {
    changeToTerminal(allTerminals.at(index));
}

void MainWindow::on_actionNew_Tab_triggered()
{
    addTerminal();
}

void MainWindow::closeTerminal(TerminalWidget *widget) {
    widget->close();
}

void MainWindow::on_actionClose_Tab_triggered()
{
    closeTerminal(currentTerminal);
}

void MainWindow::on_actionFind_triggered()
{
    currentTerminal->toggleShowSearchBar();
}

void MainWindow::on_actionGo_Full_Screen_triggered()
{
    if (this->isFullScreen()) {
        this->showNormal();

#ifdef Q_OS_MAC
        tabBar->setVisible(true);
#else
        ui->tabFrame->setVisible(true);
#endif
    } else {
        this->showFullScreen();
#ifdef Q_OS_MAC
        tabBar->setVisible(true);
#else
        ui->tabFrame->setVisible(false);
#endif
    }
}

void MainWindow::on_actionAbout_triggered()
{
    About* window = new About(this);
    window->exec();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsWindow* settings = new SettingsWindow();
    settings->exec();
    settings->deleteLater();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (allTerminals.count() != 0) {
        for (TerminalWidget* widget : allTerminals) {
            widget->close();
        }

        if (allTerminals.count() != 0) {
            event->ignore();
        }
    }
}
