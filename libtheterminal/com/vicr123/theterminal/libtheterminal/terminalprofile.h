#ifndef TERMINALPROFILE_H
#define TERMINALPROFILE_H

#include <QFont>
#include <QObject>
#include <QQmlEngine>

struct TerminalProfilePrivate;
class TerminalProfile : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString profileName READ profileName WRITE setProfileName NOTIFY profileNameChanged FINAL)
        Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged FINAL)
        Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged FINAL)
        Q_PROPERTY(QString colorName READ colorName WRITE setColorName NOTIFY colorNameChanged FINAL)
        Q_PROPERTY(QString shell READ shell WRITE setShell NOTIFY shellChanged FINAL)
        QML_ELEMENT
    public:
        explicit TerminalProfile(QObject* parent = nullptr);
        ~TerminalProfile();

        QString profileName();
        void setProfileName(QString profileName);

        QFont font();
        void setFont(QFont font);

        qreal zoom();
        void setZoom(qreal zoom);

        QString colorName();
        void setColorName(QString colorName);

        QString shell();
        void setShell(QString shell);

    signals:
        void profileNameChanged();
        void fontChanged();
        void zoomChanged();
        void colorNameChanged();
        void shellChanged();

    private:
        TerminalProfilePrivate* d;

        QJsonObject save();
        void load(QJsonObject object);

        QString defaultShell();
};

#endif // TERMINALPROFILE_H
