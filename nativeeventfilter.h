#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QX11Info>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

#undef None
#undef Status

class NativeEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit NativeEventFilter(QObject *parent = 0);

signals:
    void toggleTerminal();

public slots:

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
};

#endif // NATIVEEVENTFILTER_H
