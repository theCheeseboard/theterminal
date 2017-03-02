#include "dropdown.h"
#include "ui_dropdown.h"

Dropdown::Dropdown(QString workdir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dropdown)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    NativeEventFilter* filter = new NativeEventFilter;
    connect(filter, &NativeEventFilter::toggleTerminal, [=]() {
        if (this->isVisible()) {
            this->hide();
        } else {
            this->show();
        }
    });

    KeyCode keycode = XKeysymToKeycode(QX11Info::display(), XF86XK_LaunchB);
    XGrabKey(QX11Info::display(), keycode, AnyModifier, DefaultRootWindow(QX11Info::display()), true, GrabModeAsync, GrabModeAsync);
    QApplication::instance()->installNativeEventFilter(filter);

    newTab(workdir);
}

Dropdown::~Dropdown()
{
    delete ui;
}


void Dropdown::newTab(QString workDir) {
    terminalWidget* widget = new terminalWidget(workDir);
    QPushButton* button = new QPushButton();
    widget->setContextMenuPolicy(Qt::CustomContextMenu);
    button->setCheckable(true);
    ui->stackedTabs->addWidget(widget);
    terminalButtons.insert(widget, button);

    //connect(widget, SIGNAL(copyAvailable(bool)), this, SLOT(on_terminal_copyAvailable(bool)));
    //connect(widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_terminal_customContextMenuRequested(QPoint)));
    connect(widget, &QTermWidget::finished, [=]() {
        closeTab(widget);
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

void Dropdown::closeTab(terminalWidget *widget) {
    delete terminalButtons.value(widget);
    terminalButtons.remove(widget);

    ui->stackedTabs->removeWidget(widget);
    if (ui->stackedTabs->count() == 0) {
        newTab(QDir::homePath());
    }
}

void Dropdown::show() {
    QScreen* currentScreen = NULL;
    for (QScreen* screen : QApplication::screens()) {
        if (screen->geometry().contains(QCursor::pos())) {
            currentScreen = screen;
        }
    }

    if (currentScreen != NULL) {
        QRect screenGeometry = currentScreen->availableGeometry();
        QRect endGeometry;
        endGeometry.setRect(screenGeometry.left() /*+ screenGeometry.width() / 20*/, screenGeometry.top(), screenGeometry.width() /* * 0.9*/, screenGeometry.height() / 2);

        QDialog::show();
        this->activateWindow();
        ui->stackedTabs->currentWidget()->setFocus();

        tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
        QRect startGeometry = endGeometry;
        startGeometry.moveBottom(screenGeometry.top());
        animation->setStartValue(startGeometry);
        animation->setEndValue(endGeometry);
        animation->setDuration(500);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
        animation->start();
    }
}

void Dropdown::on_AddTab_clicked()
{
    newTab(QDir::homePath());
}

void Dropdown::on_CloseTab_clicked()
{
    closeTab((terminalWidget*) ui->stackedTabs->currentWidget());
}

void Dropdown::on_stackedTabs_currentChanged(int arg1)
{
    if (terminalButtons.count() > 0) {
        terminalWidget* widget = (terminalWidget*) ui->stackedTabs->widget(arg1);

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
