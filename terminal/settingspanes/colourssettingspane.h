#ifndef COLOURSSETTINGSPANE_H
#define COLOURSSETTINGSPANE_H

#include <QWidget>

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class ColoursSettingsPane;
}

struct ColoursSettingsPanePrivate;
class ColoursSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit ColoursSettingsPane(QWidget* parent = nullptr);
        ~ColoursSettingsPane();

    private:
        Ui::ColoursSettingsPane* ui;
        ColoursSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_opacitySlider_valueChanged(int value);
        void on_coloursComboBox_currentIndexChanged(int index);
};

#endif // COLOURSSETTINGSPANE_H
