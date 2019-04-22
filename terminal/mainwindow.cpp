#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QShortcut>
#include <ttoast.h>

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
#else
    ui->menuBar->setVisible(false);

    QMenu* menu = new QMenu();
    menu->addAction(ui->actionNew_Window);
    menu->addAction(ui->actionNew_Tab);
    menu->addSeparator();
    menu->addAction(ui->actionCopy);
    menu->addAction(ui->actionPaste);
    menu->addSeparator();
    menu->addAction(ui->actionZoomIn);
    menu->addAction(ui->actionZoomOut);
    menu->addAction(ui->actionResetZoom);
    menu->addAction(ui->actionGo_Full_Screen);
    menu->addSeparator();
    menu->addAction(ui->actionSettings);
    menu->addMenu(ui->menuHelp);
    menu->addSeparator();
    menu->addAction(ui->actionClose_Tab);
    menu->addAction(ui->actionExit);
    ui->menuButton->setMenu(menu);
#endif

    this->addTerminal(workDir);
    this->resize(this->size() * theLibsGlobal::getDPIScaling());
    ui->termStack->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    QShortcut* fullscreenShortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F11), this);
    connect(fullscreenShortcut, &QShortcut::activated, this, &MainWindow::on_actionGo_Full_Screen_triggered);
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
    button->setAutoExclusive(true);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
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
        ui->termStack->removeWidget(widget);
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

    //ui->centralWidget->layout()->addWidget(widget);
    ui->termStack->addWidget(widget);

    changeToTerminal(widget);
}

void MainWindow::changeToTerminal(TerminalWidget *widget) {
    ui->termStack->setCurrentWidget(widget);
#ifdef Q_OS_MAC
    tabBar->setCurrentIndex(allTerminals.indexOf(widget));
#else
    terminalButtons.value(widget)->setChecked(true);
#endif

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
        ui->topFrame->setVisible(true);
#endif
    } else {
        this->showFullScreen();
#ifdef Q_OS_MAC
        tabBar->setVisible(true);
#else
        ui->topFrame->setVisible(false);
#endif
        tToast* toast = new tToast();
        toast->setTitle(tr("Full Screen"));
        toast->setText(tr("You're in full screen. You can exit full screen with SHIFT+F11."));
        toast->show(this);
        connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
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

void MainWindow::on_actionZoomIn_triggered()
{
    currentTerminal->zoomIn();
}

void MainWindow::on_actionZoomOut_triggered()
{
    currentTerminal->zoomOut();
}

void MainWindow::on_actionResetZoom_triggered()
{
    currentTerminal->zoom100();
}
