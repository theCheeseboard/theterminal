#ifndef COLORMODEL_H
#define COLORMODEL_H

#include <QAbstractListModel>
#include <QQmlComponent>

struct ColorModelPrivate;
class ColorModel : public QAbstractListModel {
        Q_OBJECT
        QML_ELEMENT

    public:
        explicit ColorModel(QObject* parent = nullptr);
        ~ColorModel();

        enum ColorRoles {
            Description = Qt::DisplayRole,
            Identifier = Qt::UserRole
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

    private:
        ColorModelPrivate* d;
};

#endif // COLORMODEL_H
