#ifndef ACTIVETERMINALBELLSSETTINGSPANE_H
#define ACTIVETERMINALBELLSSETTINGSPANE_H

#include <QWidget>

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class ActiveTerminalBellsSettingsPane;
}

struct ActiveTerminalBellsSettingsPanePrivate;
class ActiveTerminalBellsSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit ActiveTerminalBellsSettingsPane(QWidget* parent = nullptr);
        ~ActiveTerminalBellsSettingsPane();

    private:
        Ui::ActiveTerminalBellsSettingsPane* ui;
        ActiveTerminalBellsSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_bellActiveSoundSwitch_toggled(bool checked);
};

#endif // ACTIVETERMINALBELLSSETTINGSPANE_H
