#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef Q_OS_MAC
#include <QTabBar>
#endif

#include <QMainWindow>
#include <QPushButton>
#include "terminalwidget.h"
#include "about.h"
#include "settingswindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString workDir = "", QWidget *parent = 0);
    ~MainWindow();

public slots:
    void showContextMenu(const QPoint &pos);
    void on_actionNew_Window_triggered();
    void on_actionExit_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();

    void addTerminal(QString workDir = "");
    void changeToTerminal(TerminalWidget* widget);
    void changeToTerminal(int index);
    void closeTerminal(TerminalWidget* widget);

private slots:
    void on_actionNew_Tab_triggered();

    void on_actionClose_Tab_triggered();

    void on_actionFind_triggered();

    void on_actionGo_Full_Screen_triggered();

    void on_actionAbout_triggered();

    void on_actionSettings_triggered();

    void on_actionZoomIn_triggered();

    void on_actionZoomOut_triggered();

    void on_actionResetZoom_triggered();

    private:
    Ui::MainWindow *ui;

    QList<TerminalWidget*> allTerminals;
    TerminalWidget* currentTerminal;
    void closeEvent(QCloseEvent* event);

#ifdef Q_OS_MAC
    QTabBar* tabBar;
#else
    QMap<TerminalWidget*, QPushButton*> terminalButtons;
#endif
};

#endif // MAINWINDOW_H
