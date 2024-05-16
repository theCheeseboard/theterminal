#ifndef FONTMODEL_H
#define FONTMODEL_H

#include <QAbstractListModel>
#include <QQmlComponent>

struct FontModelPrivate;
class FontModel : public QAbstractListModel {
        Q_OBJECT
        QML_ELEMENT

    public:
        explicit FontModel(QObject* parent = nullptr);
        ~FontModel();

        enum FontRoles {
            FamilyRole = Qt::DisplayRole
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

    private:
        FontModelPrivate* d;
};

#endif // FONTMODEL_H
