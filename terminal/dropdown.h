#ifndef DROPDOWN_H
#define DROPDOWN_H

#include "nativeeventfilter.h"
#include "settingswindow.h"
#include "terminalwidget.h"
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QStackedWidget>
#include <tpropertyanimation.h>
#include <ttoast.h>

namespace Ui {
    class Dropdown;
}

struct DropdownPrivate;
class Dropdown : public QDialog {
        Q_OBJECT
        Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

    public:
        explicit Dropdown(QString workDir, QWidget* parent = 0);
        ~Dropdown();

        void setGeometry(int x, int y, int w, int h);
        void setGeometry(QRect geometry);

        TerminalWidget* currentTerminal();

    public slots:
        void show();
        void hide();
        void newTab(QString workDir);
        void newTab(TerminalWidget* widget);
        void closeTab(TerminalWidget* widget);

    private slots:
        void on_CloseTab_clicked();

        void on_expand_clicked();

        void on_actionCopy_triggered();

        void on_actionPaste_triggered();

        void on_actionFind_triggered();

    private:
        Ui::Dropdown* ui;
        DropdownPrivate* d;

        void paintEvent(QPaintEvent* event);
};

#endif // DROPDOWN_H
