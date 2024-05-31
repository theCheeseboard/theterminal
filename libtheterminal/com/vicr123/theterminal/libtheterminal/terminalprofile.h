#ifndef TERMINALPROFILE_H
#define TERMINALPROFILE_H

#include <QFont>
#include <QObject>
#include <QQmlEngine>

struct TerminalProfilePrivate;
class TerminalProfile : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString profileName READ profileName WRITE setProfileName NOTIFY profileNameChanged FINAL)
        Q_PROPERTY(QString profileUuid READ profileUuid WRITE setProfileUuid NOTIFY profileUuidChanged FINAL)
        Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged FINAL)
        Q_PROPERTY(QString fontFamily WRITE setFontFamily)
        Q_PROPERTY(qreal fontPointSize WRITE setFontPointSize)
        Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged FINAL)
        Q_PROPERTY(QString colorName READ colorName WRITE setColorName NOTIFY colorNameChanged FINAL)
        Q_PROPERTY(QString shell READ shell WRITE setShell NOTIFY shellChanged FINAL)
        Q_PROPERTY(QString defaultShell READ defaultShell);
        QML_ELEMENT
    public:
        explicit TerminalProfile(QObject* parent = nullptr);
        ~TerminalProfile();

        static QString profilesLocation();

        QString profileUuid();
        void setProfileUuid(QString profileUuid);

        QString profileName();
        void setProfileName(QString profileName);

        QString profilePath();

        QFont font();
        void setFont(QFont font);
        void setFontFamily(QString fontFamily);
        void setFontPointSize(qreal pointSize);

        qreal zoom();
        void setZoom(qreal zoom);

        QString colorName();
        void setColorName(QString colorName);

        QString shell();
        void setShell(QString shell);

        Q_SCRIPTABLE void saveProfile();
        Q_SCRIPTABLE void loadProfile();

        QString defaultShell();

    signals:
        void profileNameChanged();
        void fontChanged();
        void zoomChanged();
        void colorNameChanged();
        void shellChanged();
        void profileUuidChanged();

    private:
        TerminalProfilePrivate* d;

        QJsonObject save();
        void load(QJsonObject object);
};

#endif // TERMINALPROFILE_H
