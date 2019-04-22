#ifndef TERMINALPART_H
#define TERMINALPART_H

#include <tttermwidget/tttermwidget.h>
#include <QDebug>
#include <QSettings>

class TerminalPartPrivate;

class TerminalPart : public TTTermWidget
{
    Q_OBJECT
public:
    explicit TerminalPart(bool connectPty, QWidget* parent);
    explicit TerminalPart(QString workDir = "", QWidget *parent = 0);
    ~TerminalPart();

signals:

public slots:
    bool canCopy();

    void zoomIn();
    void zoomOut();
    void zoom100();

    void reloadThemeSettings();

private:
    TerminalPartPrivate* d;

    void setup();
    void resizeEvent(QResizeEvent* event);
};

#endif // TERMINALPART_H
