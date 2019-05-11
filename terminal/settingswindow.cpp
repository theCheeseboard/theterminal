#include "settingswindow.h"
#include "ui_settingswindow.h"

#include <the-libs_global.h>
#include <QDir>
#include "terminalcontroller.h"
#include <QScroller>

#include "models/colorschemeselectiondelegate.h"

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

    ui->shellLineEdit->setText(settings.value("term/program", qgetenv("SHELL")).toString());
    ui->fontComboBox->setCurrentFont(QFont(settings.value("theme/fontFamily", QFontDatabase::systemFont(QFontDatabase::FixedFont).family()).toString()));
    ui->currentFontSize->setValue(settings.value("theme/fontSize", QFontDatabase::systemFont(QFontDatabase::FixedFont).pointSize()).toInt());
    ui->blinkCursorSwitch->setChecked(settings.value("theme/blinkCursor", true).toBool());
    ui->bellActiveSoundSwitch->setChecked(settings.value("bell/bellActiveSound", true).toBool());
    ui->bellInactiveSoundSwitch->setChecked(settings.value("bell/bellInactiveSound", true).toBool());
    ui->bellInactiveNotificationSwitch->setChecked(settings.value("bell/bellInactiveNotification", true).toBool());
    ui->opacitySlider->setValue(settings.value("theme/opacity", 100).toInt());
    ui->scrollKeystrokeSwitch->setChecked(settings.value("scrolling/scrollOnKeystroke", true).toBool());

    int scrollback = settings.value("term/scrollback", -1).toInt();
    if (scrollback == -1) {
        ui->infiniteScrollbackRadioButton->setChecked(true);
    } else if (scrollback == 0) {
        ui->noScrollbackRadioButton->setChecked(true);
    } else {
        ui->limitScrollbackRadioButton->setChecked(true);
        ui->scrollbackSpin->setValue(scrollback);
    }

    if (settings.value("terminal/type", "legacy").toString() == "legacy") {
        ui->termTypeComboBox->setCurrentIndex(0);
    } else {
        ui->termTypeComboBox->setCurrentIndex(1);
    }

    switch (settings.value("theme/cursorType", 0).toInt()) {
        case 0: //Block
            ui->blockCursor->setChecked(true);
            break;
        case 1: //Underline
            ui->underlineCursor->setChecked(true);
            break;
        case 2: //I-beam
            ui->ibeamCursor->setChecked(true);
            break;
    }

    ui->coloursComboBox->setItemDelegate(new ColorSchemeSelectionDelegate());
    ui->coloursComboBox->blockSignals(true);
    QDir systemColors("/usr/share/tttermwidget/color-schemes");
    for (QFileInfo col : systemColors.entryInfoList()) {
        if (col.suffix() == "colorscheme") {
            ui->coloursComboBox->addItem(col.baseName(), col.filePath());
            if (col.baseName() == settings.value("theme/scheme", "Linux").toString()) {
                ui->coloursComboBox->setCurrentIndex(ui->coloursComboBox->count() - 1);
            }
        }
    }
    ui->coloursComboBox->blockSignals(false);

    QScroller::grabGesture(ui->themingScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->scrollingScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->bellsScrollArea->viewport(), QScroller::LeftMouseButtonGesture);

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
    TerminalController::updateTerminalStyles();
}

void SettingsWindow::on_shellLineEdit_editingFinished()
{
    settings.setValue("term/program", ui->shellLineEdit->text());
}

void SettingsWindow::on_fontComboBox_currentFontChanged(const QFont &f)
{
    settings.setValue("theme/fontFamily", f.family());
    TerminalController::updateTerminalStyles();
}

void SettingsWindow::on_currentFontSize_valueChanged(int arg1)
{
    settings.setValue("theme/fontSize", arg1);
    TerminalController::updateTerminalStyles();
}

void SettingsWindow::on_blockCursor_toggled(bool checked)
{
    if (checked) {
        settings.setValue("theme/cursorType", 0);
        TerminalController::updateTerminalStyles();
    }
}

void SettingsWindow::on_underlineCursor_toggled(bool checked)
{
    if (checked) {
        settings.setValue("theme/cursorType", 1);
        TerminalController::updateTerminalStyles();
    }
}

void SettingsWindow::on_ibeamCursor_toggled(bool checked)
{
    if (checked) {
        settings.setValue("theme/cursorType", 2);
        TerminalController::updateTerminalStyles();
    }
}

void SettingsWindow::on_blinkCursorSwitch_toggled(bool checked)
{
    settings.setValue("theme/blinkCursor", checked);
}

void SettingsWindow::on_bellActiveSoundSwitch_toggled(bool checked)
{
    settings.setValue("bell/bellActiveSound", checked);
}

void SettingsWindow::on_bellInactiveSoundSwitch_toggled(bool checked)
{
    settings.setValue("bell/bellInactiveSound", checked);
}

void SettingsWindow::on_bellInactiveNotificationSwitch_toggled(bool checked)
{
    settings.setValue("bell/bellInactiveNotification", checked);
}

void SettingsWindow::on_opacitySlider_valueChanged(int value)
{
    settings.setValue("theme/opacity", value);
    TerminalController::updateTerminalStyles();
}

void SettingsWindow::on_noScrollbackRadioButton_toggled(bool checked)
{
    if (checked) {
        settings.setValue("term/scrollback", 0);
    }
}

void SettingsWindow::on_limitScrollbackRadioButton_toggled(bool checked)
{
    if (checked) {
        settings.setValue("term/scrollback", ui->scrollbackSpin->value());
        ui->scrollbackSpin->setEnabled(true);
    } else {
        ui->scrollbackSpin->setEnabled(false);
    }
}

void SettingsWindow::on_infiniteScrollbackRadioButton_toggled(bool checked)
{
    if (checked) {
        settings.setValue("term/scrollback", -1);
    }
}

void SettingsWindow::on_scrollKeystrokeSwitch_toggled(bool checked)
{
    settings.setValue("scrolling/scrollOnKeystroke", checked);
    TerminalController::updateTerminalStyles();
}
