#include "terminalwidget.h"

terminalWidget::terminalWidget(QString workDir, QWidget *parent) : QTermWidget(0, parent)
{
    this->setScrollBarPosition(QTermWidget::ScrollBarRight);
    this->setFlowControlEnabled(true);
    this->setColorScheme("Linux");
    this->setHistorySize(-1);

    QStringList environment;
    environment.append("TERM=xterm");

    this->setEnvironment(environment);

    this->update();
    this->startShellProgram();
}