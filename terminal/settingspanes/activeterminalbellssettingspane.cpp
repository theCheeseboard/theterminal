#include "activeterminalbellssettingspane.h"
#include "ui_activeterminalbellssettingspane.h"

#include <tsettings.h>

struct ActiveTerminalBellsSettingsPanePrivate {
        tSettings settings;
};

ActiveTerminalBellsSettingsPane::ActiveTerminalBellsSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::ActiveTerminalBellsSettingsPane) {
    ui->setupUi(this);

    d = new ActiveTerminalBellsSettingsPanePrivate();
    ui->bellActiveSoundSwitch->setChecked(d->settings.value("bell/bellActiveSound").toBool());
}

ActiveTerminalBellsSettingsPane::~ActiveTerminalBellsSettingsPane() {
    delete d;
    delete ui;
}

QString ActiveTerminalBellsSettingsPane::paneName() {
    return tr("Active Terminal Bells");
}

void ActiveTerminalBellsSettingsPane::on_bellActiveSoundSwitch_toggled(bool checked) {
    d->settings.setValue("bell/bellActiveSound", checked);
}
