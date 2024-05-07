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

    signals:
        void profileNameChanged();
        void fontChanged();
        void zoomChanged();

    private:
        TerminalProfilePrivate* d;
};

#endif // TERMINALPROFILE_H
