#include "fontmodel.h"
#include <QFontDatabase>

struct FontModelPrivate {
        QStringList fonts;
};

FontModel::FontModel(QObject* parent) :
    QAbstractListModel(parent), d{new FontModelPrivate()} {
    for (auto family : QFontDatabase::families()) {
        if (QFontDatabase::isFixedPitch(family)) {
            d->fonts.append(family);
        }
    }
}

FontModel::~FontModel() {
    delete d;
}

int FontModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;

    return d->fonts.length();
}

QVariant FontModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    switch (role) {
        case Qt::DisplayRole:
            return d->fonts.at(index.row());
        default:
            return {};
    }
}

QHash<int, QByteArray> FontModel::roleNames() const {
    return {
        {FamilyRole, "family"}
    };
}
