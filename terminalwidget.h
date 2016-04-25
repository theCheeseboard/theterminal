#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <qtermwidget5/qtermwidget.h>
#include <QDebug>

class terminalWidget : public QTermWidget
{
    Q_OBJECT
public:
    explicit terminalWidget(QString workDir = "", QWidget *parent = 0);

signals:

public slots:
};

#endif // TERMINALWIDGET_H
