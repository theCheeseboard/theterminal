#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QStackedWidget>
#include <QListWidget>
#include <QKeySequenceEdit>
#include <QSpinBox>
#include <QComboBox>
#include "nativeeventfilter.h"

#ifdef Q_OS_LINUX
#include <QX11Info>
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
#endif

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

    void on_scrollbackSpin_valueChanged(int arg1);

    void on_termTypeComboBox_currentIndexChanged(int index);

    void on_coloursComboBox_currentIndexChanged(const QString &arg1);

    void on_shellLineEdit_editingFinished();

    private:
    Ui::SettingsWindow *ui;

    QSettings settings;
};

#endif // SETTINGSWINDOW_H
