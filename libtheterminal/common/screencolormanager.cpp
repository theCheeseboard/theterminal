#include "screencolormanager.h"

#include <QColor>
#include <QFile>
#include <QSettings>

struct ScreenColorManagerPrivate {
        QByteArray colors;
};

ScreenColorManager::ScreenColorManager(QObject* parent) :
    QObject{parent}, d{new ScreenColorManagerPrivate()} {
    d->colors.resize((256 + 4) * 4);
    this->loadColorDefinition("Linux");
}

ScreenColorManager::~ScreenColorManager() {
    delete d;
}

QColor ScreenColorManager::decodeColor(quint32 color) {
    switch (decodeColorMode(color)) {
        case 0: // 24 bit color
            return decodeColor24bit(color);
        case 1: // 8 bit color
            {
                auto index = decodeColor8bit(color);
                return QColor(*(reinterpret_cast<QRgb*>(d->colors.data() + 4 * index)));
            }
        case 2: // Default color
            {
                auto foreground = decodeColorDefault(color);
                return foreground ? this->foreground() : this->background();
            }
    }
    return {};
}

QColor ScreenColorManager::background() {
    // The 256th colour is the background colour
    return QColor(*(reinterpret_cast<QRgb*>(d->colors.data() + 4 * 256)));
}

QColor ScreenColorManager::foreground() {
    // The 258th colour is the foreground colour
    return QColor(*(reinterpret_cast<QRgb*>(d->colors.data() + 4 * 258)));
}

ScreenColorManager::Color ScreenColorManager::colorDefault(bool background) {
    return background ? 0x00000002 : 0x00000102;
}

ScreenColorManager::Color ScreenColorManager::color8bit(ColorSection index) {
    return (index << 8) | 0x01;
}

ScreenColorManager::Color ScreenColorManager::color24bit(ColorSection r, ColorSection g, ColorSection b) {
    return (r << 24) | (g << 16) | (b << 8) | 0x00;
}

bool ScreenColorManager::decodeColorDefault(Color color) {
    return color >> 8;
}

ScreenColorManager::ColorSection ScreenColorManager::decodeColorMode(Color color) {
    return (color & 0xFF);
}

ScreenColorManager::ColorSection ScreenColorManager::decodeColor8bit(Color color) {
    return (color >> 8) & 0xFF;
}

QColor ScreenColorManager::decodeColor24bit(Color color) {
    return {static_cast<int>(color >> 24), static_cast<int>((color >> 16) & 0xFF), static_cast<int>((color >> 8) & 0xFF)};
}

void ScreenColorManager::loadColorDefinition(QString colorDefinition) {
    if (!colorDefinition.startsWith("/")) {
        // Interpret as an internal resource
        colorDefinition = QStringLiteral(":/com/vicr123/theterminal/libtheterminal/colorschemes/%1.colorscheme").arg(colorDefinition);
    }

    // Colors are packed into d->colors as 0xAARRGGBB

    QSettings settings(colorDefinition, QSettings::IniFormat);
    char buf[4];
    buf[3] = 0xFF;
    for (auto i = 0; i <= 9; i++) {
        auto baseName = QStringLiteral("Color%1").arg(i);
        auto index = i;
        auto intenseIndex = i + 8;
        if (i == 8) {
            baseName = QStringLiteral("Background");
            index = 256;
            intenseIndex = 257;
        }
        if (i == 9) {
            baseName = QStringLiteral("Foreground");
            index = 258;
            intenseIndex = 259;
        }

        auto color = settings.value(QStringLiteral("%1/Color").arg(baseName)).toStringList();
        buf[0] = color.at(0).toInt();
        buf[1] = color.at(1).toInt();
        buf[2] = color.at(2).toInt();
        memcpy(d->colors.data() + index * 4, buf, 4);
        auto colorIntense = settings.value(QStringLiteral("%1Intense/Color").arg(baseName)).toStringList();
        buf[0] = colorIntense.at(0).toInt();
        buf[1] = colorIntense.at(1).toInt();
        buf[2] = colorIntense.at(2).toInt();
        memcpy(d->colors.data() + intenseIndex * 4, buf, 4);
    }

    emit colorDefinitionChanged();
}
