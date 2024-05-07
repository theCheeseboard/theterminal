#include "terminalprofile.h"

#include <QFontDatabase>
#include <QJsonObject>

struct TerminalProfilePrivate {
        QString profileName = "default";
        QFont font;
        qreal zoom = 1;
        QString colorName = "Linux";
        QString shell;
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

QString TerminalProfile::colorName() {
    return d->colorName;
}

void TerminalProfile::setColorName(QString colorName) {
    d->colorName = colorName;
    emit colorNameChanged();
}

QString TerminalProfile::shell() {
    if (d->shell.isEmpty()) return defaultShell();
    return d->shell;
}

void TerminalProfile::setShell(QString shell) {
    d->shell = shell;
    emit shellChanged();
}

QJsonObject TerminalProfile::save() {
    return {
        {"font",  QJsonObject({{"family", d->font.family()},
                     {"size", d->font.pointSizeF()}})},
        {"color", d->colorName                                       },
        {"shell", d->shell                                           }
    };
}

void TerminalProfile::load(QJsonObject object) {
    auto fontObject = object.value("font").toObject();

    QFont font;
    if (fontObject.contains("family")) {
        font.setFamily(fontObject.value("family").toString());
        font.setPointSizeF(fontObject.value("size").toDouble());
    } else {
        font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    }
    setFont(font);
    setColorName(object.value("color").toString());
    setShell(object.value("shell").toString());
}

// macOS implementation lives in terminalprofile-objc.mm
#ifndef Q_OS_MAC
QString TerminalProfile::defaultShell() {
    #ifdef Q_OS_WIN
    return QStringLiteral("C:/Windows/System32/powershell.exe");
    #else
    return "/bin/bash";
    #endif
}
#endif
