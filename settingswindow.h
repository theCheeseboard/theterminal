#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QStackedWidget>
#include <QListWidget>
#include <QKeySequenceEdit>
#include <QX11Info>
#include "nativeeventfilter.h"
#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <X11/Xatom.h>

#undef Unsorted
#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef Expose
#undef Bool

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = 0);
    ~SettingsWindow();

private slots:
    void on_settingsLists_currentRowChanged(int currentRow);

    void on_DoneButton_clicked();

    void on_keybindingButton_toggled(bool checked);

    void keypressCaptureComplete();

private:
    Ui::SettingsWindow *ui;

    QSettings settings;
};

#endif // SETTINGSWINDOW_H
