#include "terminalwidget.h"

terminalWidget::terminalWidget(QString workDir, QWidget *parent) : QTermWidget(0, parent)
{
    this->setScrollBarPosition(QTermWidget::ScrollBarRight);
    this->setFlowControlEnabled(true);
    this->setColorScheme("Linux");
    this->setHistorySize(settings.value("term/scrollback", -1).toInt());
    this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::IBeamCursor);

    connect(this, &terminalWidget::copyAvailable, [=](bool copyAvailable) {
        this->copyOk = copyAvailable;
    });

    QFont font;
    font.setFamily(settings.value("appearance/font", "Monospace").toString());
    font.setPointSize(settings.value("appearance/fontSize", 10).toInt());
    this->setTerminalFont(font);

    QStringList environment;
    environment.append("TERM=xterm");

    this->setEnvironment(environment);

    if (workDir != "") {
        this->setWorkingDirectory(workDir);
    }

    this->setShellProgram(settings.value("term/program", "/bin/bash").toString());

    this->update();
    this->startShellProgram();
}

bool terminalWidget::canCopy() {
    return copyOk;
}
