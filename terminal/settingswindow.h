#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <tsettings.h>

namespace Ui {
    class SettingsWindow;
}

class SettingsWindow : public QDialog {
        Q_OBJECT

    public:
        explicit SettingsWindow(QWidget* parent = 0);
        ~SettingsWindow();

    private slots:
        void on_settingsLists_currentRowChanged(int currentRow);

        void on_DoneButton_clicked();

        void on_keybindingButton_toggled(bool checked);

        void keypressCaptureComplete();

        void on_scrollbackSpin_valueChanged(int arg1);

        void on_termTypeComboBox_currentIndexChanged(int index);

        void on_coloursComboBox_currentIndexChanged(const QString& arg1);

        void on_shellLineEdit_editingFinished();

        void on_fontComboBox_currentFontChanged(const QFont& f);

        void on_currentFontSize_valueChanged(int arg1);

        void on_blockCursor_toggled(bool checked);

        void on_underlineCursor_toggled(bool checked);

        void on_ibeamCursor_toggled(bool checked);

        void on_blinkCursorSwitch_toggled(bool checked);

        void on_bellActiveSoundSwitch_toggled(bool checked);

        void on_bellInactiveSoundSwitch_toggled(bool checked);

        void on_bellInactiveNotificationSwitch_toggled(bool checked);

        void on_opacitySlider_valueChanged(int value);

        void on_noScrollbackRadioButton_toggled(bool checked);

        void on_limitScrollbackRadioButton_toggled(bool checked);

        void on_infiniteScrollbackRadioButton_toggled(bool checked);

        void on_scrollKeystrokeSwitch_toggled(bool checked);

        void on_systemTitlebarsCheckbox_toggled(bool checked);

    private:
        Ui::SettingsWindow* ui;

        tSettings settings;
};

#endif // SETTINGSWINDOW_H
