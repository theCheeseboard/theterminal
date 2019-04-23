#ifndef DROPDOWN_H
#define DROPDOWN_H

#include <QDialog>
#include <QApplication>
#include <QDesktopWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QScreen>
#include <QDir>
#include <QPaintEvent>
#include <QPainter>
#include <QProcess>
#include <QMenu>
#include "nativeeventfilter.h"
#include "terminalwidget.h"
#include <ttoast.h>
#include <tpropertyanimation.h>
#include "settingswindow.h"
#include <QX11Info>

namespace Ui {
class Dropdown;
}

class Dropdown : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit Dropdown(QString workDir, QWidget *parent = 0);
    ~Dropdown();

    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

    TerminalWidget* currentTerminal();

public slots:
    void show();
    void hide();
    void newTab(QString workDir);
    void closeTab(TerminalWidget* widget);

private slots:
    void on_AddTab_clicked();

    void on_CloseTab_clicked();

    void on_stackedTabs_currentChanged(int arg1);

    void on_expand_clicked();

    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionFind_triggered();

    private:
    Ui::Dropdown *ui;

    QMap<TerminalWidget*, QPushButton*> terminalButtons;
    bool isExpanded = false;
    QScreen* currentScreen = NULL;
    QSettings settings;

    void paintEvent(QPaintEvent* event);
};

#endif // DROPDOWN_H
