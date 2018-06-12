#ifndef TERMINALPART_H
#define TERMINALPART_H

#include <tttermwidget/tttermwidget.h>
#include <QDebug>
#include <QSettings>

class TerminalPart : public TTTermWidget
{
    Q_OBJECT
public:
    explicit TerminalPart(bool connectPty, QWidget* parent);
    explicit TerminalPart(QString workDir = "", QWidget *parent = 0);

signals:

public slots:
    bool canCopy();

private:
    QSettings settings;
    void setup();

    bool copyOk = false;

    void resizeEvent(QResizeEvent* event);
};

#endif // TERMINALPART_H
