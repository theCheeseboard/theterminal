#include "terminalpart.h"

TerminalPart::TerminalPart(QWidget* parent) : QTermWidget(0, parent) {
    setup();
}

TerminalPart::TerminalPart(QString workDir, QWidget *parent) : QTermWidget(0, parent)
{
    setup();

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

void TerminalPart::setup() {
    this->setScrollBarPosition(QTermWidget::ScrollBarRight);
    this->setFlowControlEnabled(true);
    this->setColorScheme("/usr/share/tttermwidget/color-schemes/Linux.colorscheme");
    this->setHistorySize(settings.value("term/scrollback", -1).toInt());
    this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::IBeamCursor);

    connect(this, &TerminalPart::copyAvailable, [=](bool copyAvailable) {
        this->copyOk = copyAvailable;
    });

    this->update();
}

bool TerminalPart::canCopy() {
    return copyOk;
}

void TerminalPart::resizeEvent(QResizeEvent *event) {

}
