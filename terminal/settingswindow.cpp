#include "settingswindow.h"
#include "ui_settingswindow.h"

#include <the-libs_global.h>
#include <QDir>

extern bool capturingKeyPress;
extern NativeEventFilter* filter;

SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    this->resize(this->size() * theLibsGlobal::getDPIScaling());
    ui->leftPane->setMaximumWidth(ui->leftPane->maximumWidth() * theLibsGlobal::getDPIScaling());

#ifdef Q_OS_MAC
    ui->keybindingButton->setVisible(false);
    ui->keybindLabel->setVisible(false);
#else
    ui->dropdownMacSupport->setVisible(false);
#endif
    on_keybindingButton_toggled(false);

    ui->scrollbackSpin->setValue(settings.value("term/scrollback", -1).toInt());
    ui->shellLineEdit->setText(settings.value("term/program", qgetenv("SHELL")).toString());

    if (settings.value("terminal/type", "legacy").toString() == "legacy") {
        ui->termTypeComboBox->setCurrentIndex(0);
    } else {
        ui->termTypeComboBox->setCurrentIndex(1);
    }

    ui->coloursComboBox->blockSignals(true);
    QDir systemColors("/usr/share/tttermwidget/color-schemes");
    for (QFileInfo col : systemColors.entryInfoList()) {
        if (col.suffix() == "colorscheme") {
            ui->coloursComboBox->addItem(col.baseName());
            if (col.baseName() == settings.value("theme/scheme", "Linux").toString()) {
                ui->coloursComboBox->setCurrentIndex(ui->coloursComboBox->count() - 1);
            }
        }
    }
    ui->coloursComboBox->blockSignals(false);

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
#ifndef Q_OS_MAC
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
#endif
}

void SettingsWindow::keypressCaptureComplete() {
    ui->keybindingButton->setChecked(false);
}

void SettingsWindow::on_scrollbackSpin_valueChanged(int arg1)
{
    settings.setValue("term/scrollback", arg1);
}


void SettingsWindow::on_termTypeComboBox_currentIndexChanged(int index)
{
    switch (index) {
        case 0: //Legacy
            settings.setValue("terminal/type", "legacy");
            break;
        case 1: //Contemporary
            settings.setValue("terminal/type", "contemporary");
            break;
    }
}

void SettingsWindow::on_coloursComboBox_currentIndexChanged(const QString &arg1)
{
    settings.setValue("theme/scheme", arg1);
}

void SettingsWindow::on_shellLineEdit_editingFinished()
{
    settings.setValue("term/program", ui->shellLineEdit->text());
}
