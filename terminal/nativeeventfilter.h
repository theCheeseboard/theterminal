#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QSettings>

#ifdef Q_OS_LINUX
#include <QX11Info>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#undef None
#undef Status
#undef Bool
#endif

class NativeEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit NativeEventFilter(QObject *parent = 0);

signals:
    void toggleTerminal();
    void keypressCaptureComplete();

public slots:
    void captureKeyPresses(bool capture);

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

    bool capturingKeyPress = false;
    QSettings settings;
};

#endif // NATIVEEVENTFILTER_H
