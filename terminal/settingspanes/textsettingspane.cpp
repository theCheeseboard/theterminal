#include "textsettingspane.h"
#include "ui_textsettingspane.h"

#include "terminalcontroller.h"
#include <tsettings.h>

struct TextSettingsPanePrivate {
        tSettings settings;
};

TextSettingsPane::TextSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::TextSettingsPane) {
    ui->setupUi(this);

    d = new TextSettingsPanePrivate();
    QString fontFamily = d->settings.value("theme/fontFamily").toString();
    if (fontFamily.isEmpty()) fontFamily = QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
    ui->fontComboBox->setCurrentFont(QFont(fontFamily));

    int fontSize = d->settings.value("theme/fontSize").toInt();
    if (fontSize == -1) fontSize = QFontDatabase::systemFont(QFontDatabase::FixedFont).pointSize();
    ui->currentFontSize->setValue(fontSize);

    switch (d->settings.value("theme/cursorType").toInt()) {
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

    ui->blinkCursorSwitch->setChecked(d->settings.value("theme/blinkCursor").toBool());
}

TextSettingsPane::~TextSettingsPane() {
    delete d;
    delete ui;
}

void TextSettingsPane::on_fontComboBox_currentFontChanged(const QFont& f) {
    d->settings.setValue("theme/fontFamily", f.family());
    TerminalController::updateTerminalStyles();
}

QString TextSettingsPane::paneName() {
    return tr("Text");
}

void TextSettingsPane::on_currentFontSize_valueChanged(int arg1) {
    d->settings.setValue("theme/fontSize", arg1);
    TerminalController::updateTerminalStyles();
}

void TextSettingsPane::on_blockCursor_toggled(bool checked) {
    if (checked) {
        d->settings.setValue("theme/cursorType", 0);
        TerminalController::updateTerminalStyles();
    }
}

void TextSettingsPane::on_underlineCursor_toggled(bool checked) {
    if (checked) {
        d->settings.setValue("theme/cursorType", 1);
        TerminalController::updateTerminalStyles();
    }
}

void TextSettingsPane::on_ibeamCursor_toggled(bool checked) {
    if (checked) {
        d->settings.setValue("theme/cursorType", 2);
        TerminalController::updateTerminalStyles();
    }
}

void TextSettingsPane::on_blinkCursorSwitch_toggled(bool checked) {
    d->settings.setValue("theme/blinkCursor", checked);
}
