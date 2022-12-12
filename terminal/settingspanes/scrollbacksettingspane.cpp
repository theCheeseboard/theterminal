#include "scrollbacksettingspane.h"
#include "ui_scrollbacksettingspane.h"

#include <tsettings.h>

struct ScrollbackSettingsPanePrivate {
        tSettings settings;
};

ScrollbackSettingsPane::ScrollbackSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::ScrollbackSettingsPane) {
    ui->setupUi(this);

    d = new ScrollbackSettingsPanePrivate();
    int scrollback = d->settings.value("term/scrollback").toInt();
    if (scrollback == -1) {
        ui->infiniteScrollbackRadioButton->setChecked(true);
    } else if (scrollback == 0) {
        ui->noScrollbackRadioButton->setChecked(true);
    } else {
        ui->limitScrollbackRadioButton->setChecked(true);
        ui->scrollbackSpin->setValue(scrollback);
    }
}

ScrollbackSettingsPane::~ScrollbackSettingsPane() {
    delete d;
    delete ui;
}

QString ScrollbackSettingsPane::paneName() {
    return tr("Scrollback");
}

void ScrollbackSettingsPane::on_noScrollbackRadioButton_toggled(bool checked) {
    if (checked) {
        d->settings.setValue("term/scrollback", 0);
    }
}

void ScrollbackSettingsPane::on_limitScrollbackRadioButton_toggled(bool checked) {
    if (checked) {
        d->settings.setValue("term/scrollback", ui->scrollbackSpin->value());
        ui->scrollbackSpin->setEnabled(true);
    } else {
        ui->scrollbackSpin->setEnabled(false);
    }
}

void ScrollbackSettingsPane::on_infiniteScrollbackRadioButton_toggled(bool checked) {
    if (checked) {
        d->settings.setValue("term/scrollback", -1);
    }
}

void ScrollbackSettingsPane::on_scrollbackSpin_valueChanged(int arg1) {
    d->settings.setValue("term/scrollback", arg1);
}
