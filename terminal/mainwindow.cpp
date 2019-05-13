#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSplitter>
#include <QToolButton>
#include <QShortcut>
#include <QDesktopServices>
#include <tcsdtools.h>
#include <ttoast.h>

struct MainWindowPrivate {
    tCsdTools csd;
    QList<TerminalTabber*> tabbers;
    TerminalTabber* currentTabber;

    QToolButton *menuButton;
    QWidget* csdButtons;
    QSplitter* mainSplitter;
};

MainWindow::MainWindow(QString workDir, QString cmd, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    d = new MainWindowPrivate();

    d->csd.installResizeAction(this);
    d->csdButtons = d->csd.csdBoxForWidget(this);

#ifndef Q_OS_MAC
    ui->menuBar->setVisible(false);

    d->menuButton = new QToolButton();
    d->menuButton->setIcon(QIcon::fromTheme("utilities-terminal"));
    d->menuButton->setIconSize(QSize(24, 24) * theLibsGlobal::getDPIScaling());
    d->menuButton->setPopupMode(QToolButton::InstantPopup);
    d->menuButton->setAutoRaise(true);

    QMenu* menu = new QMenu();
    menu->addAction(ui->actionNew_Window);
    menu->addAction(ui->actionNew_Tab);
    menu->addSeparator();
    menu->addAction(ui->actionCopy);
    menu->addAction(ui->actionPaste);
    menu->addAction(ui->actionFind);
    menu->addSeparator();
    menu->addAction(ui->actionZoomIn);
    menu->addAction(ui->actionZoomOut);
    menu->addAction(ui->actionResetZoom);
    menu->addAction(ui->actionGo_Full_Screen);
    menu->addMenu(ui->menuSplit);
    menu->addSeparator();
    menu->addAction(ui->actionSettings);
    menu->addMenu(ui->menuHelp);
    menu->addSeparator();
    menu->addAction(ui->actionClose_Tab);
    menu->addAction(ui->actionExit);
    d->menuButton->setMenu(menu);
#endif

    this->resize(this->size() * theLibsGlobal::getDPIScaling());

    //Make the background translucent in case the user wants the terminal to be translucent
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->setAttribute(Qt::WA_TranslucentBackground);

    QShortcut* fullscreenShortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F11), this);
    connect(fullscreenShortcut, &QShortcut::activated, this, &MainWindow::on_actionGo_Full_Screen_triggered);

    //Create a new tabber
    TerminalTabber* tabber = newTabber();
    tabber->setMenuButton(d->menuButton);
    tabber->setCsdButtons(d->csdButtons);
    d->currentTabber = tabber;

    this->addTerminal(workDir, cmd);

    d->mainSplitter = new QSplitter(Qt::Horizontal);
    d->mainSplitter->addWidget(tabber);
    this->centralWidget()->layout()->addWidget(d->mainSplitter);
}

MainWindow::~MainWindow()
{
    delete d;
    delete ui;
}

TerminalTabber* MainWindow::currentTabber() {
    return d->currentTabber;
}

TerminalWidget* MainWindow::currentTerminal() {
    return currentTabber()->currentTerminal();
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
    currentTerminal()->copyClipboard();
}

void MainWindow::on_actionPaste_triggered()
{
    currentTerminal()->pasteClipboard();
}

void MainWindow::addTerminal(QString workDir, QString cmd) {
    addTerminal(new TerminalWidget(workDir, cmd));
}

void MainWindow::addTerminal(TerminalWidget* widget) {
    currentTabber()->addTab(widget);
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
    closeTerminal(currentTerminal());
}

void MainWindow::on_actionFind_triggered()
{
    currentTerminal()->toggleShowSearchBar();
}

void MainWindow::on_actionGo_Full_Screen_triggered()
{
    if (this->isFullScreen()) {
        this->showNormal();
    } else {
        this->showFullScreen();

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
    if (d->tabbers.count() != 0) {
        event->ignore();
        for (TerminalTabber* tabber : d->tabbers) {
            tabber->closeAllTabs();
        }
    }
}

void MainWindow::on_actionZoomIn_triggered()
{
    currentTerminal()->zoomIn();
}

void MainWindow::on_actionZoomOut_triggered()
{
    currentTerminal()->zoomOut();
}

void MainWindow::on_actionResetZoom_triggered()
{
    currentTerminal()->zoom100();
}

TerminalTabber* MainWindow::splitVertically() {
    //Create a new tabber
    TerminalWidget* terminal = new TerminalWidget();
    TerminalTabber* tabber = newTabber();
    tabber->addTab(terminal);
    tabber->setFixedHeight(0);

    int newHeight;

    QSplitter* actingSplitter;
    QSplitter* splitter = qobject_cast<QSplitter*>(currentTabber()->parentWidget());
    int index = splitter->indexOf(currentTabber());
    if (splitter->orientation() == Qt::Vertical) {
        //Add the tabber to this splitter
        splitter->insertWidget(index + 1, tabber);
        newHeight = splitter->height() / splitter->count();
        actingSplitter = splitter;
    } else {
        //Create a vertical splitter and replace the widget
        newHeight = splitter->widget(index)->height() / 2;
        actingSplitter = new QSplitter(Qt::Vertical);
        QWidget* oldWidget = splitter->replaceWidget(index, actingSplitter);

        actingSplitter->addWidget(oldWidget);
        actingSplitter->addWidget(tabber);
    }

    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(0);
    anim->setEndValue(newHeight);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, tabber, [=](QVariant value) {
        tabber->setFixedHeight(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, tabber, [=] {
        actingSplitter->setSizes(actingSplitter->sizes());
        tabber->setFixedHeight(QWIDGETSIZE_MAX);
        anim->deleteLater();
    });
    anim->start();

    terminal->setFocus();

    moveTabberButtons();

    return tabber;
}

TerminalTabber* MainWindow::splitHorizontally() {
    //Create a new tabber
    TerminalWidget* terminal = new TerminalWidget();
    TerminalTabber* tabber = newTabber();
    tabber->addTab(terminal);
    tabber->setFixedWidth(0);

    int newWidth;

    QSplitter* actingSplitter;
    QSplitter* splitter = qobject_cast<QSplitter*>(currentTabber()->parentWidget());
    int index = splitter->indexOf(currentTabber());
    if (splitter->orientation() == Qt::Horizontal) {
        //Add the tabber to this splitter
        splitter->insertWidget(index + 1, tabber);
        newWidth = splitter->width() / splitter->count();
        actingSplitter = splitter;
    } else {
        //Create a horizontal splitter and replace the widget
        newWidth = splitter->widget(index)->width() / 2;
        actingSplitter = new QSplitter(Qt::Horizontal);
        QWidget* oldWidget = splitter->replaceWidget(index, actingSplitter);

        actingSplitter->addWidget(oldWidget);
        actingSplitter->addWidget(tabber);
    }

    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(0);
    anim->setEndValue(newWidth);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, tabber, [=](QVariant value) {
        tabber->setFixedWidth(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, tabber, [=] {
        actingSplitter->setSizes(actingSplitter->sizes());
        tabber->setFixedWidth(QWIDGETSIZE_MAX);
        anim->deleteLater();
    });
    anim->start();

    terminal->setFocus();

    moveTabberButtons();

    return tabber;
}

TerminalTabber* MainWindow::newTabber() {
    //Create a new tabber
    TerminalTabber* tabber = new TerminalTabber();
    connect(tabber, &TerminalTabber::done, this, [=] {
        //Clean up splitters
        QSplitter* parentSplitter = qobject_cast<QSplitter*>(tabber->parentWidget());
        tabber->setParent(nullptr); //Remove the tabber from its parent

        //Find the next available tabber to make the current tabber
        {
            QWidget* nextSplitter = parentSplitter;
            while (qobject_cast<TerminalTabber*>(nextSplitter) == nullptr) {
                QSplitter* splitter = qobject_cast<QSplitter*>(nextSplitter);
                if (splitter->count() == 0) {
                    //Bail out!
                    nextSplitter = nullptr;
                    break;
                } else {
                    nextSplitter = splitter->widget(0);
                }
            }

            if (nextSplitter != nullptr) {
                d->currentTabber = qobject_cast<TerminalTabber*>(nextSplitter);
                d->currentTabber->setFocus();
            }
        }

        while (parentSplitter->count() == 1 && parentSplitter != d->mainSplitter) {
            //We can clean up this splitter
            QWidget* cleanupWidget = parentSplitter->widget(0);
            QSplitter* parentParentSplitter = qobject_cast<QSplitter*>(parentSplitter->parentWidget());
            parentParentSplitter->replaceWidget(parentParentSplitter->indexOf(parentSplitter), cleanupWidget);

            parentSplitter->deleteLater();
            parentSplitter = parentParentSplitter;
        }

        moveTabberButtons();

        d->tabbers.removeAll(tabber);
        if (d->tabbers.count() == 0) {
            QTimer::singleShot(0, this, &MainWindow::close);
        }
    });
    connect(tabber, &TerminalTabber::gotFocus, this, [=] {
        d->currentTabber = tabber;
    });
    d->tabbers.append(tabber);
    return tabber;
}

void MainWindow::on_actionSplitVertically_triggered()
{
    this->splitVertically();
}

void MainWindow::on_actionSplitHorizontally_triggered()
{
    this->splitHorizontally();
}

void MainWindow::on_actionFileBug_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/vicr123/theslate/issues"));
}

void MainWindow::on_actionSources_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/vicr123/theslate"));
}

void MainWindow::moveTabberButtons() {
    //Ensure the top left splitter has the menu button
    {
        QWidget* nextSplitter = d->mainSplitter;
        while (qobject_cast<TerminalTabber*>(nextSplitter) == nullptr) {
            QSplitter* splitter = qobject_cast<QSplitter*>(nextSplitter);
            if (splitter->count() == 0) {
                //Bail out!
                nextSplitter = nullptr;
                break;
            } else {
                nextSplitter = splitter->widget(0);
            }
        }

        if (nextSplitter != nullptr) {
            qobject_cast<TerminalTabber*>(nextSplitter)->setMenuButton(d->menuButton);
            //If CSDs are on the left, make this the CSD splitter too
            if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left) qobject_cast<TerminalTabber*>(nextSplitter)->setCsdButtons(d->csdButtons);
        }
    }

    //Ensure the top right splitter has the CSD buttons if CSDs are on the right
    if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Right) {
        QWidget* nextSplitter = d->mainSplitter;
        while (qobject_cast<TerminalTabber*>(nextSplitter) == nullptr) {
            QSplitter* splitter = qobject_cast<QSplitter*>(nextSplitter);
            if (splitter->count() == 0) {
                //Bail out!
                nextSplitter = nullptr;
                break;
            } else {
                if (splitter->orientation() == Qt::Horizontal) {
                    nextSplitter = splitter->widget(splitter->count() - 1);
                } else {
                    nextSplitter = splitter->widget(0);
                }
            }
        }

        if (nextSplitter != nullptr) {
            qobject_cast<TerminalTabber*>(nextSplitter)->setCsdButtons(d->csdButtons);
        }
    }
}
