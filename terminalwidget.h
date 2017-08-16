#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <qtermwidget5/qtermwidget.h>
#include <QDebug>
#include <QSettings>

class terminalWidget : public QTermWidget
{
    Q_OBJECT
public:
    explicit terminalWidget(QString workDir = "", QWidget *parent = 0);

signals:

public slots:
    bool canCopy();

private:
    QSettings settings;

    bool copyOk = false;
};

#endif // TERMINALWIDGET_H
