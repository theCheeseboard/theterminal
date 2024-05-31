#include "profilehelper.h"

#include <QUuid>

ProfileHelper::ProfileHelper(QObject* parent) :
    QObject{parent} {
}

QString ProfileHelper::newProfileUuid() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}
