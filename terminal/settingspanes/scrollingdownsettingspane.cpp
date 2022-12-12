#include "scrollingdownsettingspane.h"
#include "ui_scrollingdownsettingspane.h"

#include "terminalcontroller.h"
#include <tsettings.h>

struct ScrollingDownSettingsPanePrivate {
        tSettings settings;
};

ScrollingDownSettingsPane::ScrollingDownSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::ScrollingDownSettingsPane) {
    ui->setupUi(this);

    d = new ScrollingDownSettingsPanePrivate();
    ui->scrollKeystrokeSwitch->setChecked(d->settings.value("scrolling/scrollOnKeystroke").toBool());
}

ScrollingDownSettingsPane::~ScrollingDownSettingsPane() {
    delete d;
    delete ui;
}

QString ScrollingDownSettingsPane::paneName() {
    return tr("Scrolling Down");
}

void ScrollingDownSettingsPane::on_scrollKeystrokeSwitch_toggled(bool checked) {
    d->settings.setValue("scrolling/scrollOnKeystroke", checked);
    TerminalController::updateTerminalStyles();
}
