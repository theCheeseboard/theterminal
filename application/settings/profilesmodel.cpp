#include "profilesmodel.h"

#include <QDir>
#include <QFileSystemWatcher>
#include <QStandardPaths>

struct ProfilesModelPrivate {
        QStringList profileUuids;
        QFileSystemWatcher* watcher;
};

ProfilesModel::ProfilesModel(QObject* parent) :
    QAbstractListModel(parent), d{new ProfilesModelPrivate()} {
    auto profilesDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absoluteFilePath("profiles");

    if (!QDir(profilesDir).exists()) {
        QDir::root().mkpath(profilesDir);
    }

    d->watcher = new QFileSystemWatcher(this);
    d->watcher->addPath(profilesDir);
    connect(d->watcher, &QFileSystemWatcher::directoryChanged, this, &ProfilesModel::updateProfiles);

    updateProfiles();
}

ProfilesModel::~ProfilesModel() {
    delete d;
}

int ProfilesModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;

    return d->profileUuids.length();
}

QVariant ProfilesModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};

    auto uuid = d->profileUuids.at(index.row());
    switch (role) {
        case Roles::UuidRole:
            return uuid;
    }
    return {};
}

void ProfilesModel::updateProfiles() {
    QStringList foundProfiles;

    QDir profilesDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absoluteFilePath("profiles");
    for (auto profile : profilesDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        foundProfiles.append(profile.baseName());
        if (!d->profileUuids.contains(profile.baseName())) {
            beginInsertRows({}, d->profileUuids.length(), d->profileUuids.length());
            d->profileUuids.append(profile.baseName());
            endInsertRows();
        }
    }

    for (auto i = 0; i < d->profileUuids.length(); i++) {
        if (!foundProfiles.contains(d->profileUuids.at(i))) {
            beginRemoveRows({}, i, i);
            d->profileUuids.removeAt(i);
            endRemoveRows();
            i--;
        }
    }
}

QHash<int, QByteArray> ProfilesModel::roleNames() const {
    return {
        {Roles::UuidRole, "uuid"}
    };
}
