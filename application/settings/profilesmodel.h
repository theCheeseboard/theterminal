#ifndef PROFILESMODEL_H
#define PROFILESMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

struct ProfilesModelPrivate;
class ProfilesModel : public QAbstractListModel {
        Q_OBJECT
        QML_ELEMENT

    public:
        explicit ProfilesModel(QObject* parent = nullptr);
        ~ProfilesModel();

        enum Roles {
            UuidRole = Qt::UserRole
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    private:
        ProfilesModelPrivate* d;

        void updateProfiles();

        // QAbstractItemModel interface
    public:
        QHash<int, QByteArray> roleNames() const override;
};

#endif // PROFILESMODEL_H
