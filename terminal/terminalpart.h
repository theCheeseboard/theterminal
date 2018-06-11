#ifndef TERMINALPART_H
#define TERMINALPART_H

#include <qtermwidget.h>
#include <QDebug>
#include <QSettings>

class TerminalPart : public QTermWidget
{
    Q_OBJECT
public:
    explicit TerminalPart(QWidget* parent);
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
