#include "settingswindow.h"
#include "ui_settingswindow.h"

#include "terminalcontroller.h"
#include <QDir>
#include <QScroller>
#include <libcontemporary_global.h>
#include <tapplication.h>
#include <tcsdtools.h>
#include <tx11info.h>

#include "nativeeventfilter.h"
#include <QComboBox>
#include <QDialog>
#include <QKeySequenceEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QStackedWidget>
#include <lib/tttermwidget.h>

#include "models/colorschemeselectiondelegate.h"

#ifndef Q_OS_MAC
#include <X11/XF86keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#endif

extern bool capturingKeyPress;
extern NativeEventFilter* filter;

SettingsWindow::SettingsWindow(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow) {
    ui->setupUi(this);

    ui->leftPane->setMaximumWidth(SC_DPI_W(ui->leftPane->maximumWidth(), this));

#ifdef Q_OS_MAC
    ui->keybindingButton->setVisible(false);
    ui->keybindLabel->setVisible(false);
#else
    ui->dropdownMacSupport->setVisible(false);
#endif
    on_keybindingButton_toggled(false);

    QString termProgram = settings.value("term/program").toString();
    if (termProgram.isEmpty()) termProgram = qEnvironmentVariable("SHELL");
    ui->shellLineEdit->setText(termProgram);

    QString fontFamily = settings.value("theme/fontFamily").toString();
    if (fontFamily.isEmpty()) fontFamily = QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
    ui->fontComboBox->setCurrentFont(QFont(fontFamily));

    int fontSize = settings.value("theme/fontSize").toInt();
    if (fontSize == -1) fontSize = QFontDatabase::systemFont(QFontDatabase::FixedFont).pointSize();
    ui->currentFontSize->setValue(fontSize);

    ui->blinkCursorSwitch->setChecked(settings.value("theme/blinkCursor").toBool());
    ui->bellActiveSoundSwitch->setChecked(settings.value("bell/bellActiveSound").toBool());
    ui->bellInactiveSoundSwitch->setChecked(settings.value("bell/bellInactiveSound").toBool());
    ui->bellInactiveNotificationSwitch->setChecked(settings.value("bell/bellInactiveNotification").toBool());
    ui->opacitySlider->setValue(settings.value("theme/opacity").toInt());
    ui->scrollKeystrokeSwitch->setChecked(settings.value("scrolling/scrollOnKeystroke").toBool());
    ui->systemTitlebarsCheckbox->setChecked(settings.value("appearance/useSsds").toBool());

    int scrollback = settings.value("term/scrollback").toInt();
    if (scrollback == -1) {
        ui->infiniteScrollbackRadioButton->setChecked(true);
    } else if (scrollback == 0) {
        ui->noScrollbackRadioButton->setChecked(true);
    } else {
        ui->limitScrollbackRadioButton->setChecked(true);
        ui->scrollbackSpin->setValue(scrollback);
    }

    switch (settings.value("theme/cursorType").toInt()) {
        case 0: // Block
            ui->blockCursor->setChecked(true);
            break;
        case 1: // Underline
            ui->underlineCursor->setChecked(true);
            break;
        case 2: // I-beam
            ui->ibeamCursor->setChecked(true);
            break;
    }

    ui->coloursComboBox->setItemDelegate(new ColorSchemeSelectionDelegate());
    ui->coloursComboBox->blockSignals(true);
    for (QString systemColorsDir : TTTermWidget::colorSchemeDirs()) {
        QDir systemColors(systemColorsDir);
        for (QFileInfo col : systemColors.entryInfoList()) {
            if (col.suffix() == "colorscheme") {
                ui->coloursComboBox->addItem(col.baseName(), col.filePath());
                if (col.baseName() == settings.value("theme/scheme").toString()) {
                    ui->coloursComboBox->setCurrentIndex(ui->coloursComboBox->count() - 1);
                }
            }
        }
    }
    ui->coloursComboBox->blockSignals(false);

    QScroller::grabGesture(ui->themingScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->scrollingScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->bellsScrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    connect(filter, SIGNAL(keypressCaptureComplete()), this, SLOT(keypressCaptureComplete()));
}

SettingsWindow::~SettingsWindow() {
    disconnect(filter, SIGNAL(keypressCaptureComplete()), this, SLOT(keypressCaptureComplete()));
    delete ui;
}

void SettingsWindow::on_settingsLists_currentRowChanged(int currentRow) {
    ui->pages->setCurrentIndex(currentRow);
}

void SettingsWindow::on_DoneButton_clicked() {
    this->close();
}

void SettingsWindow::on_keybindingButton_toggled(bool checked) {
#ifndef Q_OS_MAC
    if (checked) {
        // Capture keyboard
        XGrabKeyboard(tX11Info::display(), RootWindow(tX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync, CurrentTime);
        filter->captureKeyPresses(true);

        ui->keybindingButton->setText("Strike a key!");
    } else {
        XUngrabKeyboard(tX11Info::display(), CurrentTime);
        filter->captureKeyPresses(false);

        ui->keybindingButton->setText(settings.value("dropdown/keyString").toString());
    }
#endif
}

void SettingsWindow::keypressCaptureComplete() {
    ui->keybindingButton->setChecked(false);
}

void SettingsWindow::on_scrollbackSpin_valueChanged(int arg1) {
    settings.setValue("term/scrollback", arg1);
}

void SettingsWindow::on_termTypeComboBox_currentIndexChanged(int index) {
    switch (index) {
        case 0: // Legacy
            settings.setValue("terminal/type", "legacy");
            break;
        case 1: // Contemporary
            settings.setValue("terminal/type", "contemporary");
            break;
    }
}

void SettingsWindow::on_coloursComboBox_currentIndexChanged(const QString& arg1) {
    settings.setValue("theme/scheme", arg1);
    TerminalController::updateTerminalStyles();
}

void SettingsWindow::on_shellLineEdit_editingFinished() {
    settings.setValue("term/program", ui->shellLineEdit->text());
}

void SettingsWindow::on_fontComboBox_currentFontChanged(const QFont& f) {
    settings.setValue("theme/fontFamily", f.family());
    TerminalController::updateTerminalStyles();
}

void SettingsWindow::on_currentFontSize_valueChanged(int arg1) {
    settings.setValue("theme/fontSize", arg1);
    TerminalController::updateTerminalStyles();
}

void SettingsWindow::on_blockCursor_toggled(bool checked) {
    if (checked) {
        settings.setValue("theme/cursorType", 0);
        TerminalController::updateTerminalStyles();
    }
}

void SettingsWindow::on_underlineCursor_toggled(bool checked) {
    if (checked) {
        settings.setValue("theme/cursorType", 1);
        TerminalController::updateTerminalStyles();
    }
}

void SettingsWindow::on_ibeamCursor_toggled(bool checked) {
    if (checked) {
        settings.setValue("theme/cursorType", 2);
        TerminalController::updateTerminalStyles();
    }
}

void SettingsWindow::on_blinkCursorSwitch_toggled(bool checked) {
    settings.setValue("theme/blinkCursor", checked);
}

void SettingsWindow::on_bellActiveSoundSwitch_toggled(bool checked) {
    settings.setValue("bell/bellActiveSound", checked);
}

void SettingsWindow::on_bellInactiveSoundSwitch_toggled(bool checked) {
    settings.setValue("bell/bellInactiveSound", checked);
}

void SettingsWindow::on_bellInactiveNotificationSwitch_toggled(bool checked) {
    settings.setValue("bell/bellInactiveNotification", checked);
}

void SettingsWindow::on_opacitySlider_valueChanged(int value) {
    settings.setValue("theme/opacity", value);
    TerminalController::updateTerminalStyles();
}

void SettingsWindow::on_noScrollbackRadioButton_toggled(bool checked) {
    if (checked) {
        settings.setValue("term/scrollback", 0);
    }
}

void SettingsWindow::on_limitScrollbackRadioButton_toggled(bool checked) {
    if (checked) {
        settings.setValue("term/scrollback", ui->scrollbackSpin->value());
        ui->scrollbackSpin->setEnabled(true);
    } else {
        ui->scrollbackSpin->setEnabled(false);
    }
}

void SettingsWindow::on_infiniteScrollbackRadioButton_toggled(bool checked) {
    if (checked) {
        settings.setValue("term/scrollback", -1);
    }
}

void SettingsWindow::on_scrollKeystrokeSwitch_toggled(bool checked) {
    settings.setValue("scrolling/scrollOnKeystroke", checked);
    TerminalController::updateTerminalStyles();
}

void SettingsWindow::on_systemTitlebarsCheckbox_toggled(bool checked) {
    settings.setValue("appearance/useSsds", checked);
    tCsdGlobal::setCsdsEnabled(!checked);
}
