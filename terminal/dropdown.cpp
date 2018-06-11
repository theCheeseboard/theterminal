#include "dropdown.h"
#include "ui_dropdown.h"

extern NativeEventFilter* filter;

Dropdown::Dropdown(QString workdir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dropdown)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    QMenu* menu = new QMenu();
    menu->addAction(QIcon::fromTheme("configure"), "Settings", [=] {
        KeyCode kc = XKeysymToKeycode(QX11Info::display(), settings.value("dropdown/key", XK_F12).toLongLong());
        XUngrabKey(QX11Info::display(), kc, AnyModifier, DefaultRootWindow(QX11Info::display()));

        SettingsWindow* settingsWin = new SettingsWindow();
        settingsWin->exec();
        settingsWin->deleteLater();

        kc = XKeysymToKeycode(QX11Info::display(), settings.value("dropdown/key", XK_F12).toLongLong());
        XGrabKey(QX11Info::display(), kc, AnyModifier, DefaultRootWindow(QX11Info::display()), true, GrabModeAsync, GrabModeAsync);
    });
    menu->addAction(QIcon::fromTheme("application-exit"), "Exit", [=] {
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

    KeyCode keycode = XKeysymToKeycode(QX11Info::display(), settings.value("dropdown/key", XK_F12).toLongLong());
    XGrabKey(QX11Info::display(), keycode, AnyModifier, DefaultRootWindow(QX11Info::display()), true, GrabModeAsync, GrabModeAsync);

    newTab(workdir);
}

Dropdown::~Dropdown()
{
    delete ui;
}


void Dropdown::newTab(QString workDir) {
    TerminalWidget* widget = new TerminalWidget(workDir);
    QPushButton* button = new QPushButton();
    widget->setContextMenuPolicy(Qt::CustomContextMenu);
    button->setCheckable(true);
    ui->stackedTabs->addWidget(widget);
    terminalButtons.insert(widget, button);

    //connect(widget, SIGNAL(copyAvailable(bool)), this, SLOT(on_terminal_copyAvailable(bool)));
    connect(widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(widget, &TerminalWidget::finished, [=]() {
        closeTab(widget);
    });
    connect(widget, &TerminalWidget::bell, [=](QString message) {
        /*tToast* toast = new tToast(widget);
        toast->setTitle("Bell");
        toast->setText(message);
        toast->setTimeout(3000);
        toast->show(widget);*/
    });

    button->setText("Terminal " + QString::number(ui->stackedTabs->indexOf(widget) + 1));
    ui->tabFrame->layout()->addWidget(button);
    ui->tabFrame->layout()->removeWidget(ui->endWidgets);
    ui->tabFrame->layout()->addWidget(ui->endWidgets);
    connect(button, &QPushButton::clicked, [=]() {
        ui->stackedTabs->setCurrentWidget(widget);
    });

    ui->stackedTabs->setCurrentWidget(widget);
}

void Dropdown::closeTab(TerminalWidget *widget) {
    delete terminalButtons.value(widget);
    terminalButtons.remove(widget);

    ui->stackedTabs->removeWidget(widget);
    if (ui->stackedTabs->count() == 0) {
        newTab(QDir::homePath());
        this->hide();
    }
}

void Dropdown::show() {
    for (QScreen* screen : QApplication::screens()) {
        if (screen->geometry().contains(QCursor::pos())) {
            currentScreen = screen;
        }
    }

    if (currentScreen != NULL) {
        QRect screenGeometry = currentScreen->geometry();
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
        isExpanded = false;
        ui->expand->setIcon(QIcon::fromTheme("go-down"));
    });
    animation->start();
}

void Dropdown::on_AddTab_clicked()
{
    newTab(QDir::homePath());
}

void Dropdown::on_CloseTab_clicked()
{
    closeTab((TerminalWidget*) ui->stackedTabs->currentWidget());
}

void Dropdown::on_stackedTabs_currentChanged(int arg1)
{
    if (terminalButtons.count() > 0) {
        TerminalWidget* widget = (TerminalWidget*) ui->stackedTabs->widget(arg1);

        for (QPushButton* button : terminalButtons.values()) {
            button->setChecked(false);
        }
        terminalButtons.value(widget)->setChecked(true);
    }
}

void Dropdown::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    //painter.drawLine(0, 0, 0, this->height() - 1);
    painter.drawLine(0, this->height() - 1, this->width() - 1, this->height() - 1);
    //painter.drawLine(this->width() - 1, this->height() - 1, this->width() - 1, 0);
}

void Dropdown::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
    this->setFixedSize(w, h);}

void Dropdown::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void Dropdown::on_expand_clicked()
{
    QRect screenGeometry = currentScreen->geometry();
    if (isExpanded) {
        isExpanded = false;
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
        isExpanded = true;
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


void Dropdown::showContextMenu(const QPoint &pos)
{
    QMenu* menu = new QMenu();

    menu->addAction(ui->actionCopy);
    menu->addAction(ui->actionPaste);

    menu->exec(ui->stackedTabs->currentWidget()->mapToGlobal(pos));
}

TerminalWidget* Dropdown::currentTerminal() {
    return (TerminalWidget*) ui->stackedTabs->currentWidget();
}

void Dropdown::on_actionCopy_triggered()
{
    currentTerminal()->copyClipboard();
}

void Dropdown::on_actionPaste_triggered()
{
    currentTerminal()->pasteClipboard();
}
