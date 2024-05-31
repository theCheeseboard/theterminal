#ifndef PROFILEHELPER_H
#define PROFILEHELPER_H

#include <QObject>
#include <QQmlEngine>

class ProfileHelper : public QObject {
        Q_OBJECT
        QML_ELEMENT
    public:
        explicit ProfileHelper(QObject* parent = nullptr);

        Q_SCRIPTABLE QString newProfileUuid();

    signals:
};

#endif // PROFILEHELPER_H
