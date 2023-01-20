#include "dropdown.h"
#include "ui_dropdown.h"

#include "mainwindow.h"
#include "nativeeventfilter.h"
#include <tx11info.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

extern NativeEventFilter* filter;

struct DropdownPrivate {
        bool isExpanded = false;
        QScreen* currentScreen = nullptr;
        QTimer* tabTitleUpdater;
        QSettings settings;
};

Dropdown::Dropdown(QString workdir, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::Dropdown) {
    ui->setupUi(this);
    d = new DropdownPrivate();
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    ui->stackedTabs->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    QMenu* menu = new QMenu();
    menu->addSection(tr("For current tab"));
    menu->addAction(ui->actionFind);
    menu->addSection(tr("For theTerminal"));
    menu->addAction(QIcon::fromTheme("configure"), tr("Settings"), [=] {
        KeyCode kc = XKeysymToKeycode(tX11Info::display(), d->settings.value("dropdown/key", XK_F12).toLongLong());
        XUngrabKey(tX11Info::display(), kc, AnyModifier, DefaultRootWindow(tX11Info::display()));

        MainWindow::openSettingsWindow(this);

        kc = XKeysymToKeycode(tX11Info::display(), d->settings.value("dropdown/key", XK_F12).toLongLong());
        XGrabKey(tX11Info::display(), kc, AnyModifier, DefaultRootWindow(tX11Info::display()), true, GrabModeAsync, GrabModeAsync);
    });
    menu->addAction(QIcon::fromTheme("application-exit"), tr("Exit"), [=] {
        this->close();
    });
    ui->menuButton->setPopupMode(QToolButton::InstantPopup);
    ui->menuButton->setMenu(menu);

    connect(filter, &NativeEventFilter::toggleTerminal, [=]() {
        if (this->isVisible()) {
            this->hide();
        } else {
            this->show();
        }
    });

    // Make the background translucent in case the user wants the terminal to be translucent
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->setAttribute(Qt::WA_TranslucentBackground);

    ui->tabber->setShowNewTabButton(true);
    connect(ui->tabber, &tWindowTabber::newTabRequested, this, [=] {
        newTab(QDir::homePath());
    });

    KeyCode keycode = XKeysymToKeycode(tX11Info::display(), d->settings.value("dropdown/key", XK_F12).toLongLong());
    XGrabKey(tX11Info::display(), keycode, AnyModifier, DefaultRootWindow(tX11Info::display()), true, GrabModeAsync, GrabModeAsync);

    d->tabTitleUpdater = new QTimer(this);
    d->tabTitleUpdater->setInterval(1000);
    d->tabTitleUpdater->start();

    newTab(workdir);
}

Dropdown::~Dropdown() {
    delete ui;
    delete d;
}

void Dropdown::newTab(QString workDir) {
    newTab(new TerminalWidget(workDir));
}

void Dropdown::newTab(TerminalWidget* widget) {
    ui->stackedTabs->addWidget(widget);

    connect(widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(widget, &TerminalWidget::finished, [=]() {
        closeTab(widget);
    });
    connect(widget, &TerminalWidget::openNewTerminal, this, QOverload<TerminalWidget*>::of(&Dropdown::newTab));

    ui->stackedTabs->setCurrentWidget(widget);

    tWindowTabberButton* button = new tWindowTabberButton();
    button->setText(tr("Terminal %1").arg(ui->stackedTabs->indexOf(widget) + 1));
    button->syncWithStackedWidget(ui->stackedTabs, widget);
    ui->tabber->addButton(button);
    connect(widget, &TerminalWidget::titleChanged, this, [this, button, widget] {
        updateTabTitle(button, widget);
    });
    connect(d->tabTitleUpdater, &QTimer::timeout, widget, [this, button, widget] {
        updateTabTitle(button, widget);
    });
}

void Dropdown::closeTab(TerminalWidget* widget) {
    //    delete terminalButtons.value(widget);
    //    terminalButtons.remove(widget);

    ui->stackedTabs->removeWidget(widget);
    if (ui->stackedTabs->count() == 0) {
        newTab(QDir::homePath());
        this->hide();
    }
}

void Dropdown::show() {
    for (QScreen* screen : QApplication::screens()) {
        if (screen->geometry().contains(QCursor::pos())) {
            d->currentScreen = screen;
        }
    }

    if (d->currentScreen != nullptr) {
        QRect screenGeometry = d->currentScreen->geometry();
        QRect endGeometry;
        endGeometry.setRect(screenGeometry.left() /*+ screenGeometry.width() / 20*/, screenGeometry.top(), screenGeometry.width() /* * 0.9*/, screenGeometry.height() / 2);

        tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
        QRect startGeometry = endGeometry;
        startGeometry.moveBottom(screenGeometry.top() - 1);
        animation->setStartValue(startGeometry);
        animation->setEndValue(endGeometry);
        animation->setDuration(250);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
        animation->start();

        QDialog::show();
        this->activateWindow();

        ui->stackedTabs->currentWidget()->setFocus();
    }
}

void Dropdown::hide() {
    tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
    QRect endGeometry = this->geometry();
    endGeometry.moveBottom(this->y() - 1);
    animation->setStartValue(this->geometry());
    animation->setEndValue(endGeometry);
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InCubic);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    connect(animation, &tPropertyAnimation::finished, [=]() {
        QDialog::hide();
        d->isExpanded = false;
        ui->expand->setIcon(QIcon::fromTheme("go-down"));
    });
    animation->start();
}

void Dropdown::on_CloseTab_clicked() {
    closeTab((TerminalWidget*) ui->stackedTabs->currentWidget());
}

void Dropdown::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    // painter.drawLine(0, 0, 0, this->height() - 1);
    painter.drawLine(0, this->height() - 1, this->width() - 1, this->height() - 1);
    // painter.drawLine(this->width() - 1, this->height() - 1, this->width() - 1, 0);
}

void Dropdown::updateTabTitle(tWindowTabberButton* button, TerminalWidget* tab) {
    button->setText(tab->title());

    auto runningProcesses = tab->runningProcesses();
    for (const auto& process : runningProcesses) {
        if (process == "sudo" || process == "su" || process == "pkexec") {
            button->setSupplementaryText(process);
            return;
        } else if (process == "ssh") {
            button->setSupplementaryText("SSH");
            return;
        } else if (process == "telnet") {
            button->setSupplementaryText("Telnet");
            return;
        }
    }
    QString workingDirectory = tab->workingDirectory();
    if (workingDirectory.contains(QDir::homePath())) workingDirectory.replace(QDir::homePath(), "~");
    button->setSupplementaryText(workingDirectory);
}

void Dropdown::setGeometry(int x, int y, int w, int h) { // Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
    this->setFixedSize(w, h);
}

void Dropdown::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void Dropdown::on_expand_clicked() {
    QRect screenGeometry = d->currentScreen->geometry();
    if (d->isExpanded) {
        d->isExpanded = false;
        ui->expand->setIcon(QIcon::fromTheme("go-down"));

        QRect endGeometry;
        endGeometry.setRect(screenGeometry.left(), screenGeometry.top(), screenGeometry.width(), screenGeometry.height() / 2);

        tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
        animation->setStartValue(this->geometry());
        animation->setEndValue(endGeometry);
        animation->setDuration(250);
        animation->setEasingCurve(QEasingCurve::InOutCubic);
        connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
        animation->start();
    } else {
        d->isExpanded = true;
        ui->expand->setIcon(QIcon::fromTheme("go-up"));

        QRect endGeometry;
        endGeometry.setRect(screenGeometry.left(), screenGeometry.top(), screenGeometry.width(), screenGeometry.height() + 2);

        tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
        animation->setStartValue(this->geometry());
        animation->setEndValue(endGeometry);
        animation->setDuration(250);
        animation->setEasingCurve(QEasingCurve::InOutCubic);
        connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
        animation->start();
    }

    ui->stackedTabs->currentWidget()->setFocus();
}

TerminalWidget* Dropdown::currentTerminal() {
    return (TerminalWidget*) ui->stackedTabs->currentWidget();
}

void Dropdown::on_actionCopy_triggered() {
    currentTerminal()->copyClipboard();
}

void Dropdown::on_actionPaste_triggered() {
    currentTerminal()->pasteClipboard();
}

void Dropdown::on_actionFind_triggered() {
    currentTerminal()->toggleShowSearchBar();
}
