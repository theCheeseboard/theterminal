#ifndef SCROLLINGDOWNSETTINGSPANE_H
#define SCROLLINGDOWNSETTINGSPANE_H

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class ScrollingDownSettingsPane;
}

struct ScrollingDownSettingsPanePrivate;
class ScrollingDownSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit ScrollingDownSettingsPane(QWidget* parent = nullptr);
        ~ScrollingDownSettingsPane();

    private:
        Ui::ScrollingDownSettingsPane* ui;
        ScrollingDownSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_scrollKeystrokeSwitch_toggled(bool checked);
};

#endif // SCROLLINGDOWNSETTINGSPANE_H
