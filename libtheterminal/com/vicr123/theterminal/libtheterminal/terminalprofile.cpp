#include "terminalprofile.h"

#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#ifdef Q_OS_UNIX
    #include <pwd.h>
    #include <unistd.h>
#endif

struct TerminalProfilePrivate {
        QFileSystemWatcher profileWatcher;
        QString profileUuid = "EB31ADE1-9342-43E9-9E9C-811CCB978F64";
        QString profileName = "Default";
        QFont font;
        qreal zoom = 1;
        QString colorName = "Linux";
        QString shell;

        bool inSetup = false;
};

TerminalProfile::TerminalProfile(QObject* parent) :
    QObject{parent}, d{new TerminalProfilePrivate()} {
    d->font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    connect(&d->profileWatcher, &QFileSystemWatcher::fileChanged, this, [this] {
        // Load changes to the profile
        this->loadProfile();
    });
    setProfileUuid(d->profileUuid);
}

TerminalProfile::~TerminalProfile() {
    delete d;
}

QString TerminalProfile::profileUuid() {
    return d->profileUuid;
}

void TerminalProfile::setProfileUuid(QString profileUuid) {
    d->profileUuid = profileUuid;

    if (!QFile::exists(this->profilePath())) {
        // Create the profile file
        QFile file(this->profilePath());
        file.open(QFile::WriteOnly);
        file.close();
    }

    d->profileWatcher.removePaths(d->profileWatcher.files());
    d->profileWatcher.addPath(this->profilePath());

    this->loadProfile();
}

QString TerminalProfile::profileName() {
    return d->profileName;
}

void TerminalProfile::setProfileName(QString profileName) {
    d->profileName = profileName;
    emit profileNameChanged();
}

QString TerminalProfile::profilePath() {
    auto profiles = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absoluteFilePath("profiles");

    if (!QDir(profiles).exists()) {
        QDir::root().mkpath(profiles);
    }

    return QDir(profiles).absoluteFilePath(QStringLiteral("%1.json").arg(d->profileUuid));
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
        {"name",  d->profileName                                     },
        {"font",  QJsonObject({{"family", d->font.family()},
                     {"size", d->font.pointSizeF()}})},
        {"color", d->colorName                                       },
        {"shell", d->shell                                           }
    };
}

void TerminalProfile::load(QJsonObject object) {
    d->inSetup = true;
    auto fontObject = object.value("font").toObject();
    setProfileName(d->profileName);

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
    d->inSetup = false;
}

void TerminalProfile::saveProfile() {
    if (d->inSetup) return;

    QFile file(this->profilePath());
    file.open(QFile::WriteOnly);
    file.write(QJsonDocument(this->save()).toJson());
    file.close();
}

void TerminalProfile::loadProfile() {
    QFile file(this->profilePath());
    if (!file.exists()) return;

    QJsonParseError error;
    file.open(QFile::ReadOnly);
    auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError) return;
    load(doc.object());
}

// macOS implementation lives in terminalprofile-objc.mm
#ifndef Q_OS_MAC
QString TerminalProfile::defaultShell() {
    #ifdef Q_OS_WIN
    return QStringLiteral("C:/Windows/System32/powershell.exe");
    #else
    auto passwd = getpwuid(getegid());
    return QString(passwd->pw_shell);
    #endif
}
#endif
