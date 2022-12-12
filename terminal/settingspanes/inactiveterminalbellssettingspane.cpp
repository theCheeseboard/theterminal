#include "inactiveterminalbellssettingspane.h"
#include "ui_inactiveterminalbellssettingspane.h"

#include <tsettings.h>

struct InactiveTerminalBellsSettingsPanePrivate {
        tSettings settings;
};

InactiveTerminalBellsSettingsPane::InactiveTerminalBellsSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::InactiveTerminalBellsSettingsPane) {
    ui->setupUi(this);
    d = new InactiveTerminalBellsSettingsPanePrivate();
    ui->bellInactiveSoundSwitch->setChecked(d->settings.value("bell/bellInactiveSound").toBool());
    ui->bellInactiveNotificationSwitch->setChecked(d->settings.value("bell/bellInactiveNotification").toBool());
}

InactiveTerminalBellsSettingsPane::~InactiveTerminalBellsSettingsPane() {
    delete d;
    delete ui;
}

QString InactiveTerminalBellsSettingsPane::paneName() {
    return tr("Inactive Terminal Bells");
}

void InactiveTerminalBellsSettingsPane::on_bellInactiveSoundSwitch_toggled(bool checked) {
    d->settings.setValue("bell/bellInactiveSound", checked);
}

void InactiveTerminalBellsSettingsPane::on_bellInactiveNotificationSwitch_toggled(bool checked) {
    d->settings.setValue("bell/bellInactiveNotification", checked);
}
