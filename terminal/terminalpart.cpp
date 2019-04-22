#include "terminalpart.h"

#include "terminalcontroller.h"
#include <QFontDatabase>
#include <tsystemsound.h>
#include <tnotification.h>

class TerminalPartPrivate {
    public:
        QSettings settings;

        QFont defaultFont;
        int zoomLevel = 0;
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
    //Register this terminal part
    TerminalController::addTerminalPart(this);

    this->setScrollBarPosition(TTTermWidget::ScrollBarRight);
    this->setFlowControlEnabled(true);
    this->setHistorySize(d->settings.value("term/scrollback", -1).toInt());
    this->setShellProgram(d->settings.value("term/program", qgetenv("SHELL")).toString());

    reloadThemeSettings();

    connect(this, &TerminalPart::copyAvailable, [=](bool copyAvailable) {
        d->copyOk = copyAvailable;
    });

    connect(this, &TerminalPart::bell, [=] {
        if (this->hasFocus()) {
            //Active terminal
            if (d->settings.value("theme/bellActiveSound", true).toBool()) {
                //Play an audible bell
                tSystemSound::play("bell");
            }
        } else {
            //Inactive terminal
            bool bellSound = d->settings.value("theme/bellInactiveSound", true).toBool();
            bool notification = d->settings.value("theme/bellInactiveNotification", true).toBool();
            if (bellSound && !notification) {
                //Play an audible bell
                tSystemSound::play("bell");
            } else if (bellSound && notification) {
                //Post a system notification with the bell sound
                tNotification* notification = new tNotification();
                notification->setSummary(tr("Bell"));
                notification->setText(tr("A bell was sounded!"));
                notification->setAppName("theTerminal");
                notification->setAppIcon("utilities-terminal");
                notification->setSound(tSystemSound::soundLocation("bell"));
                notification->post(true);
            }
        }
    });

    this->update();
}

bool TerminalPart::canCopy() {
    return d->copyOk;
}

void TerminalPart::resizeEvent(QResizeEvent *event) {

}

void TerminalPart::zoomIn() {
    d->zoomLevel++;
    QFont f = d->defaultFont;
    f.setPointSize(f.pointSize() + d->zoomLevel);
    this->setTerminalFont(f);
}

void TerminalPart::zoomOut() {
    d->zoomLevel--;
    QFont f = d->defaultFont;
    f.setPointSize(f.pointSize() + d->zoomLevel);
    this->setTerminalFont(f);
}

void TerminalPart::zoom100() {
    d->zoomLevel = 0;
    this->setTerminalFont(d->defaultFont);
}

void TerminalPart::reloadThemeSettings() {
    this->setColorScheme(QString("/usr/share/tttermwidget/color-schemes/%1.colorscheme").arg(d->settings.value("theme/scheme", "Linux").toString()));
    this->setBlinkingCursor(d->settings.value("theme/blinkCursor", true).toBool());

    d->defaultFont.setFamily(d->settings.value("theme/fontFamily", QFontDatabase::systemFont(QFontDatabase::FixedFont).family()).toString());
    d->defaultFont.setPointSize(d->settings.value("theme/fontSize", QFontDatabase::systemFont(QFontDatabase::FixedFont).pointSize()).toInt());

    switch (d->settings.value("theme/cursorType", 0).toInt()) {
        case 0:
            this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::BlockCursor);
            break;
        case 1:
            this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::UnderlineCursor);
            break;
        case 2:
            this->setKeyboardCursorShape(Konsole::Emulation::KeyboardCursorShape::IBeamCursor);
            break;
    }

    QFont f = d->defaultFont;
    f.setPointSize(f.pointSize() + d->zoomLevel);
    this->setTerminalFont(f);
}
