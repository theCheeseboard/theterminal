/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "colorschemeselectiondelegate.h"

#include <QDebug>
#include <QFontDatabase>
#include <QPainter>
#include <libcontemporary_global.h>

ColorSchemeSelectionDelegate::ColorSchemeSelectionDelegate(QObject* parent) :
    QAbstractItemDelegate(parent) {
}

void ColorSchemeSelectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QString item = index.data(Qt::UserRole).toString();
    QSettings itemData(item, QSettings::IniFormat);

    auto getColor = [=, &itemData](QString name) {
        itemData.beginGroup(name);
        QVariant col = itemData.value("Color");
        itemData.endGroup();

        QStringList colours;
        if (col.type() == QVariant::StringList) {
            colours = col.toStringList();
        } else {
            colours = col.toString().split(",");
        }
        if (colours.count() != 3) {
            return QColor();
        } else {
            return QColor(colours.at(0).toInt(), colours.at(1).toInt(), colours.at(2).toInt());
        }
    };

    QString name = itemData.value("Description").toString();

    QColor background = getColor("Background");
    QColor foreground = getColor("Foreground");
    QFont font = getFont();
    QFontMetrics metrics(font);

    painter->setBrush(background);
    painter->setPen(Qt::transparent);
    painter->drawRect(option.rect);

    QRect textRect;
    textRect.setTop(option.rect.top() + 9);
    textRect.setLeft(option.rect.left() + SC_DPI(3) + 9);
    textRect.setRight(option.rect.right() - 9);
    textRect.setHeight(metrics.height());

    painter->setFont(font);
    painter->setBrush(Qt::transparent);
    painter->setPen(foreground);
    painter->drawText(textRect, name);

    textRect.moveTop(textRect.top() + metrics.height());
    painter->drawText(textRect, "[" + qgetenv("USER") + "@" + QSysInfo::machineHostName() + " ~]$");

    if (option.state & QStyle::State_Selected) {
        QColor highlightColor = option.palette.color(QPalette::Highlight);

        painter->setBrush(highlightColor);
        painter->setPen(Qt::transparent);

        for (QRect selectionRect : {
                 QRect(0, option.rect.top(), SC_DPI(3), option.rect.height()),
                 QRect(0, option.rect.bottom() + 1 - SC_DPI(3), option.rect.width(), SC_DPI(3)),
                 QRect(option.rect.right() + 1 - SC_DPI(3), option.rect.top(), SC_DPI(3), option.rect.height()),
                 QRect(0, option.rect.top(), option.rect.width(), SC_DPI(3))}) {
            painter->drawRect(selectionRect);
        }
    }
}

QSize ColorSchemeSelectionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QFontMetrics metrics(getFont());

    QSize sizeHint;
    sizeHint.setWidth(300);
    sizeHint.setHeight(metrics.height() * 2 + 18);
    return sizeHint;
}

QFont ColorSchemeSelectionDelegate::getFont() const {
    QFont defaultFont = QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
    QFont font;
    font.setFamily(settings.value("theme/fontFamily", defaultFont.toString()).toString());
    font.setPointSize(settings.value("theme/fontSize", defaultFont.pointSize()).toInt());
    return font;
}
