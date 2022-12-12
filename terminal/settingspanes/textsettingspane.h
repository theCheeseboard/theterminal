#ifndef TEXTSETTINGSPANE_H
#define TEXTSETTINGSPANE_H

#include <QWidget>

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class TextSettingsPane;
}

struct TextSettingsPanePrivate;
class TextSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit TextSettingsPane(QWidget* parent = nullptr);
        ~TextSettingsPane();

    private slots:
        void on_fontComboBox_currentFontChanged(const QFont& f);

        void on_currentFontSize_valueChanged(int arg1);

        void on_blockCursor_toggled(bool checked);

        void on_underlineCursor_toggled(bool checked);

        void on_ibeamCursor_toggled(bool checked);

        void on_blinkCursorSwitch_toggled(bool checked);

    private:
        Ui::TextSettingsPane* ui;
        TextSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
};

#endif // TEXTSETTINGSPANE_H
