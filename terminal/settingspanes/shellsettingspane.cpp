#include "shellsettingspane.h"
#include "ui_shellsettingspane.h"

#include <tsettings.h>

struct ShellSettingsPanePrivate {
        tSettings settings;
};

ShellSettingsPane::ShellSettingsPane(QWidget* parent) :
    tSettingsPane(parent), ui(new Ui::ShellSettingsPane) {
    ui->setupUi(this);

    d = new ShellSettingsPanePrivate();

    QString termProgram = d->settings.value("term/program").toString();
    if (termProgram.isEmpty())
        termProgram = qEnvironmentVariable("SHELL");
    ui->shellLineEdit->setText(termProgram);
}

ShellSettingsPane::~ShellSettingsPane() {
    delete d;
    delete ui;
}

QString ShellSettingsPane::paneName() {
    return tr("Shell");
}

void ShellSettingsPane::on_shellLineEdit_editingFinished() {
    d->settings.setValue("term/program", ui->shellLineEdit->text());
}
