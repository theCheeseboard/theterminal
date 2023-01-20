#include "colourssettingspane.h"
#include "ui_colourssettingspane.h"

#include "models/colorschemeselectiondelegate.h"
#include "terminalcontroller.h"
#include <QDir>
#include <lib/tttermwidget.h>
#include <tsettings.h>

struct ColoursSettingsPanePrivate {
        tSettings settings;
};

ColoursSettingsPane::ColoursSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::ColoursSettingsPane) {
    ui->setupUi(this);

    d = new ColoursSettingsPanePrivate();

    ui->coloursComboBox->setItemDelegate(new ColorSchemeSelectionDelegate());
    ui->coloursComboBox->blockSignals(true);
    for (QString systemColorsDir : TTTermWidget::colorSchemeDirs()) {
        QDir systemColors(systemColorsDir);
        for (QFileInfo col : systemColors.entryInfoList()) {
            if (col.suffix() == "colorscheme") {
                ui->coloursComboBox->addItem(col.baseName(), col.filePath());
                if (col.baseName() == d->settings.value("theme/scheme").toString()) {
                    ui->coloursComboBox->setCurrentIndex(ui->coloursComboBox->count() - 1);
                }
            }
        }
    }
    ui->coloursComboBox->blockSignals(false);

    ui->opacitySlider->setValue(d->settings.value("theme/opacity").toInt());
}

ColoursSettingsPane::~ColoursSettingsPane() {
    delete d;
    delete ui;
}

QString ColoursSettingsPane::paneName() {
    return tr("Colours");
}

void ColoursSettingsPane::on_opacitySlider_valueChanged(int value) {
    d->settings.setValue("theme/opacity", value);
    TerminalController::updateTerminalStyles();
}

void ColoursSettingsPane::on_coloursComboBox_currentIndexChanged(int index) {
    d->settings.setValue("theme/scheme", ui->coloursComboBox->itemText(index));
    TerminalController::updateTerminalStyles();
}
