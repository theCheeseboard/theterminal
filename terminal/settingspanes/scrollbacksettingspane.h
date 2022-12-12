#ifndef SCROLLBACKSETTINGSPANE_H
#define SCROLLBACKSETTINGSPANE_H

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class ScrollbackSettingsPane;
}

struct ScrollbackSettingsPanePrivate;
class ScrollbackSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit ScrollbackSettingsPane(QWidget* parent = nullptr);
        ~ScrollbackSettingsPane();

    private:
        Ui::ScrollbackSettingsPane* ui;
        ScrollbackSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_noScrollbackRadioButton_toggled(bool checked);
        void on_limitScrollbackRadioButton_toggled(bool checked);
        void on_infiniteScrollbackRadioButton_toggled(bool checked);
        void on_scrollbackSpin_valueChanged(int arg1);
};

#endif // SCROLLBACKSETTINGSPANE_H
