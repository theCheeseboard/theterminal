#include "terminalpart.h"

TerminalPart::TerminalPart(bool connectPty, QWidget* parent) : TTTermWidget(0, connectPty, parent) {
    setup();
}

TerminalPart::TerminalPart(QString workDir, QWidget *parent) : TTTermWidget(0, true, parent)
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
    this->setScrollBarPosition(TTTermWidget::ScrollBarRight);
    this->setFlowControlEnabled(true);
    this->setColorScheme("/usr/share/tttermwidget/color-schemes/Linux.colorscheme");
    this->setHistorySize(settings.value("term/scrollback", -1).toInt());
    this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::IBeamCursor);

    connect(this, &TerminalPart::copyAvailable, [=](bool copyAvailable) {
        this->copyOk = copyAvailable;
    });

#ifdef Q_OS_MAC
    this->setTerminalFont(QFont("Monaco", 10));
#endif

    this->update();
}

bool TerminalPart::canCopy() {
    return copyOk;
}

void TerminalPart::resizeEvent(QResizeEvent *event) {

}
