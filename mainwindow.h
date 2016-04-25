#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "terminalwidget.h"
#include "about.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_terminal_finished();
    void on_terminal_customContextMenuRequested(const QPoint &pos);
    void on_terminal_copyAvailable(bool canCopy);
    void on_actionNew_Window_triggered();
    void on_actionExit_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();

    void addTerminal();
    void changeToTerminal(terminalWidget* widget);
    void changeToTerminal(int index);
    void closeTerminal(terminalWidget* widget);

private slots:
    void on_actionNew_Tab_triggered();

    void on_actionClose_Tab_triggered();

    void on_actionFind_triggered();

    void on_actionGo_Full_Screen_triggered();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;

    QList<terminalWidget*> allTerminals;
    QMap<terminalWidget*, QPushButton*> terminalButtons;
    terminalWidget* currentTerminal;
};

#endif // MAINWINDOW_H
