#include "terminalprofile.h"

#include <QFontDatabase>

struct TerminalProfilePrivate {
        QString profileName = "default";
        QFont font;
        qreal zoom = 1;
};

TerminalProfile::TerminalProfile(QObject* parent) :
    QObject{parent}, d{new TerminalProfilePrivate()} {
    d->font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
}

TerminalProfile::~TerminalProfile() {
    delete d;
}

QString TerminalProfile::profileName() {
    return d->profileName;
}

void TerminalProfile::setProfileName(QString profileName) {
    // TODO: Load profile information
    d->profileName = profileName;
    emit profileNameChanged();
}

QFont TerminalProfile::font() {
    auto font = d->font;
    font.setPointSizeF(font.pointSizeF() * d->zoom);
    return font;
}

void TerminalProfile::setFont(QFont font) {
    d->font = font;
    emit fontChanged();
}

qreal TerminalProfile::zoom() {
    return d->zoom;
}

void TerminalProfile::setZoom(qreal zoom) {
    d->zoom = zoom;
    emit zoomChanged();
    emit fontChanged();
}
