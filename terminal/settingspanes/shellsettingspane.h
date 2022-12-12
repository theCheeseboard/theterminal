#ifndef SHELLSETTINGSPANE_H
#define SHELLSETTINGSPANE_H

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
class ShellSettingsPane;
}

struct ShellSettingsPanePrivate;
class ShellSettingsPane : public tSettingsPane {
  Q_OBJECT

public:
  explicit ShellSettingsPane(QWidget *parent = nullptr);
  ~ShellSettingsPane();

private:
  Ui::ShellSettingsPane *ui;
  ShellSettingsPanePrivate *d;

  // tSettingsPane interface
public:
  QString paneName();
private slots:
  void on_shellLineEdit_editingFinished();
};

#endif // SHELLSETTINGSPANE_H
