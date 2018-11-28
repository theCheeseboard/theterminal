#include "terminalpart.h"

#include <QFontDatabase>

class TerminalPartPrivate {
    public:
        QSettings settings;

        QFont defaultFont;
        bool copyOk = false;
};

TerminalPart::TerminalPart(bool connectPty, QWidget* parent) : TTTermWidget(0, connectPty, parent) {
    d = new TerminalPartPrivate();
    setup();
}

TerminalPart::TerminalPart(QString workDir, QWidget *parent) : TTTermWidget(0, true, parent)
{
    d = new TerminalPartPrivate();
    setup();

    QStringList environment;
    environment.append("TERM=xterm");

    this->setEnvironment(environment);

    if (workDir != "") {
        this->setWorkingDirectory(workDir);
    }

    this->update();
    this->startShellProgram();
}

TerminalPart::~TerminalPart() {
    delete d;
}

void TerminalPart::setup() {
    this->setScrollBarPosition(TTTermWidget::ScrollBarRight);
    this->setFlowControlEnabled(true);
    this->setColorScheme(QString("/usr/share/tttermwidget/color-schemes/%1.colorscheme").arg(d->settings.value("theme/scheme", "Linux").toString()));
    this->setHistorySize(d->settings.value("term/scrollback", -1).toInt());
    this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::IBeamCursor);
    this->setShellProgram(d->settings.value("term/program", qgetenv("SHELL")).toString());

    connect(this, &TerminalPart::copyAvailable, [=](bool copyAvailable) {
        d->copyOk = copyAvailable;
    });

    d->defaultFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    this->setTerminalFont(d->defaultFont);

    this->update();
}

bool TerminalPart::canCopy() {
    return d->copyOk;
}

void TerminalPart::resizeEvent(QResizeEvent *event) {

}

void TerminalPart::zoomIn() {
    QFont f = this->getTerminalFont();
    f.setPointSize(f.pointSize() + 1);
    this->setTerminalFont(f);
}

void TerminalPart::zoomOut() {
    QFont f = this->getTerminalFont();
    f.setPointSize(f.pointSize() - 1);
    this->setTerminalFont(f);
}

void TerminalPart::zoom100() {
    this->setTerminalFont(d->defaultFont);
}
