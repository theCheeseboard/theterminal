#ifndef INACTIVETERMINALBELLSSETTINGSPANE_H
#define INACTIVETERMINALBELLSSETTINGSPANE_H

#include <QWidget>

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class InactiveTerminalBellsSettingsPane;
}

struct InactiveTerminalBellsSettingsPanePrivate;
class InactiveTerminalBellsSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit InactiveTerminalBellsSettingsPane(QWidget* parent = nullptr);
        ~InactiveTerminalBellsSettingsPane();

    private:
        Ui::InactiveTerminalBellsSettingsPane* ui;
        InactiveTerminalBellsSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_bellInactiveSoundSwitch_toggled(bool checked);
        void on_bellInactiveNotificationSwitch_toggled(bool checked);
};

#endif // INACTIVETERMINALBELLSSETTINGSPANE_H
