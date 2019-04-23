#ifndef TERMINALPART_H
#define TERMINALPART_H

#include <tttermwidget/tttermwidget.h>
#include <QDebug>
#include <QSettings>

class TerminalPartPrivate;

struct TerminalPartConstruct {
    QString workDir = "";
    QString manPage = "";
    bool connectPty = true;
    bool startShell = true;
};

class TerminalPart : public TTTermWidget
{
    Q_OBJECT
    public:
        //explicit TerminalPart(bool connectPty, QWidget* parent);
        //explicit TerminalPart(QString workDir = "", QWidget *parent = 0);
        explicit TerminalPart(TerminalPartConstruct args, QWidget* parent = nullptr);
        ~TerminalPart();

    signals:
        void closeTerminal();
        void openNewTerminal(TerminalPart* part);

    public slots:
        bool canCopy();

        void zoomIn();
        void zoomOut();
        void zoom100();

        void reloadThemeSettings();
        void tryClose();

    private:
        TerminalPartPrivate* d;

        void setup();
        void resizeEvent(QResizeEvent* event);
};

#endif // TERMINALPART_H
