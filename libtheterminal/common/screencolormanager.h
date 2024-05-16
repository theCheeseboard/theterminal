#ifndef SCREENCOLORMANAGER_H
#define SCREENCOLORMANAGER_H

#include "libtheterminal-common-exports.h"
#include <QObject>

struct ScreenColorManagerPrivate;
class LIBTHETERMINAL_COMMON_EXPORT ScreenColorManager : public QObject {
        Q_OBJECT
    public:
        explicit ScreenColorManager(QObject* parent = nullptr);
        ~ScreenColorManager();

        static QStringList definitions();
        static QString name(QString definition);

        using Color = quint32;
        using ColorSection = quint8;

        QColor decodeColor(Color color);
        QColor background();
        QColor foreground();

        // Colours are implemented as follows:
        // Mode is determined by the last two bits:
        //   0x00: 24 bit colour - Interpret the bits as 0xRRGGBBXX
        //   0x01:  8 bit colour - Interpret the bits as 0xXXXXIIXX
        //                         where II is the index of the colour
        //          4 bit colour is encoded as an 8 bit colour.
        //   0x02: Default color - 0x00000002 is the default background colour
        //                         0x00000102 is the default foreground colour
        static Color colorDefault(bool background);
        static Color color8bit(ColorSection index);
        static Color color24bit(ColorSection r, ColorSection g, ColorSection b);
        static ColorSection decodeColorMode(Color color);
        static bool decodeColorDefault(Color color); // true for foreground, false for background
        static ColorSection decodeColor8bit(Color color);
        static QColor decodeColor24bit(Color color);

        void loadColorDefinition(QString colorDefinition);

    signals:
        void colorDefinitionChanged();

    private:
        ScreenColorManagerPrivate* d;
};

#endif // SCREENCOLORMANAGER_H
