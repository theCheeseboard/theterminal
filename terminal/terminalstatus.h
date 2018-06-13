#ifndef TERMINALSTATUS_H
#define TERMINALSTATUS_H

#include <QWidget>
#include <QDir>
#include <QLabel>
#include <QProcess>
#include <QIcon>

namespace Ui {
    class TerminalStatus;
}

class TerminalStatus : public QWidget
{
        Q_OBJECT

    public:
        explicit TerminalStatus(QWidget *parent = 0);
        ~TerminalStatus();

    public slots:
        void setDir(QDir dir);

    private:
        Ui::TerminalStatus *ui;
        QDir dir;
};

#endif // TERMINALSTATUS_H
