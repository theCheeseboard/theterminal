#ifndef DROPDOWNTERMINALKEYBOARDBINDINGSETTINGSPANE_H
#define DROPDOWNTERMINALKEYBOARDBINDINGSETTINGSPANE_H

#include <QAbstractNativeEventFilter>
#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class DropdownTerminalKeyboardBindingSettingsPane;
}

struct DropdownTerminalKeyboardBindingSettingsPanePrivate;
class DropdownTerminalKeyboardBindingSettingsPane : public tSettingsPane,
                                                    public QAbstractNativeEventFilter {
        Q_OBJECT

    public:
        explicit DropdownTerminalKeyboardBindingSettingsPane(QWidget* parent = nullptr);
        ~DropdownTerminalKeyboardBindingSettingsPane();

    private:
        Ui::DropdownTerminalKeyboardBindingSettingsPane* ui;
        DropdownTerminalKeyboardBindingSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_keybindingButton_toggled(bool checked);

        // QAbstractNativeEventFilter interface
    public:
        bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result);
};

#endif // DROPDOWNTERMINALKEYBOARDBINDINGSETTINGSPANE_H
