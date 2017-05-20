#include "settingswindow.h"
#include "ui_settingswindow.h"

extern bool capturingKeyPress;
extern NativeEventFilter* filter;

SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    ui->settingsLists->addItem(new QListWidgetItem(QIcon::fromTheme("configure"), "General"));
    ui->settingsLists->addItem(new QListWidgetItem(QIcon::fromTheme("go-down"), "Drop Down"));

    on_keybindingButton_toggled(false);

    connect(filter, SIGNAL(keypressCaptureComplete()), this, SLOT(keypressCaptureComplete()));
}

SettingsWindow::~SettingsWindow()
{
    disconnect(filter, SIGNAL(keypressCaptureComplete()), this, SLOT(keypressCaptureComplete()));
    delete ui;
}

void SettingsWindow::on_settingsLists_currentRowChanged(int currentRow)
{
    ui->pages->setCurrentIndex(currentRow);
}

void SettingsWindow::on_DoneButton_clicked()
{
    this->close();
}

void SettingsWindow::on_keybindingButton_toggled(bool checked)
{
    if (checked) {
        //Capture keyboard
        XGrabKeyboard(QX11Info::display(), RootWindow(QX11Info::display(), 0), True, GrabModeAsync, GrabModeAsync, CurrentTime);
        filter->captureKeyPresses(true);

        ui->keybindingButton->setText("Strike a key!");
    } else {
        XUngrabKeyboard(QX11Info::display(), CurrentTime);
        filter->captureKeyPresses(false);

        ui->keybindingButton->setText(settings.value("dropdown/keyString", "F12").toString());
    }
}

void SettingsWindow::keypressCaptureComplete() {
    ui->keybindingButton->setChecked(false);
}
