#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef Q_OS_MAC
    #include <QTabBar>
#endif

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class TerminalWidget;
class TerminalTabber;
struct MainWindowPrivate;
class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QString workDir = "", QString cmd = "", QWidget* parent = 0);
        ~MainWindow();

    public slots:
        void on_actionNew_Window_triggered();
        void on_actionExit_triggered();
        void on_actionCopy_triggered();
        void on_actionPaste_triggered();

        void addTerminal(QString workDir = "", QString cmd = "");
        void addTerminal(TerminalWidget* widget);
        void closeTerminal(TerminalWidget* widget);

        TerminalTabber* splitVertically();
        TerminalTabber* splitHorizontally();

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

        void on_actionSplitVertically_triggered();

        void on_actionSplitHorizontally_triggered();

        void on_actionFileBug_triggered();

        void on_actionSources_triggered();

        void on_actionPrint_triggered();

        void on_actionConnect_to_Server_triggered();

    private:
        Ui::MainWindow* ui;
        MainWindowPrivate* d;

        void closeEvent(QCloseEvent* event);
        TerminalTabber* currentTabber();
        TerminalWidget* currentTerminal();

        TerminalTabber* newTabber();
        void moveTabberButtons();
};

#endif // MAINWINDOW_H
