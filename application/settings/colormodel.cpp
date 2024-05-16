#include "colormodel.h"

#include <screencolormanager.h>

struct ColorModelPrivate {
};

ColorModel::ColorModel(QObject* parent) :
    QAbstractListModel(parent), d{new ColorModelPrivate()} {
}

ColorModel::~ColorModel() {
    delete d;
}

int ColorModel::rowCount(const QModelIndex& parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return ScreenColorManager::definitions().length();
}

QVariant ColorModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto definition = ScreenColorManager::definitions().value(index.row());
    switch (role) {
        case Description:
            return ScreenColorManager::name(definition);
        case Identifier:
            return definition;
    }

    return QVariant();
}

QHash<int, QByteArray> ColorModel::roleNames() const {
    return {
        {Description, "description"},
        {Identifier,  "identifier" }
    };
}
