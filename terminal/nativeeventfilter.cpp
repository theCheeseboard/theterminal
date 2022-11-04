#include "nativeeventfilter.h"
#include <tx11info.h>

#ifdef Q_OS_LINUX
    //#include <QX11Info>
    #include <X11/XF86keysym.h>
    #include <X11/XKBlib.h>
    #include <X11/Xlib.h>
    #include <X11/keysym.h>
    #include <xcb/xcb.h>
    #include <xcb/xcb_atom.h>
#endif

NativeEventFilter::NativeEventFilter(QObject* parent) :
    QObject(parent) {
}

bool NativeEventFilter::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
#ifndef Q_OS_MAC
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == XCB_KEY_RELEASE) {
            xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);

            if (capturingKeyPress) {
                KeySym keyPressed = XkbKeycodeToKeysym(tX11Info::display(), button->detail, 0, button->state & ShiftMask ? 1 : 0);

                settings.setValue("dropdown/key", QVariant::fromValue(keyPressed));
                settings.setValue("dropdown/keyState", button->state);
                settings.setValue("dropdown/keyString", QString::fromUtf8(XKeysymToString(keyPressed)));
                captureKeyPresses(false);
                emit keypressCaptureComplete();
                return true;
            } else {
                if (button->detail == XKeysymToKeycode(tX11Info::display(), settings.value("dropdown/key", XK_F12).toLongLong())) {
                    emit toggleTerminal();
                    return true;
                }
            }
        }
    }
#endif
    return false;
}

void NativeEventFilter::captureKeyPresses(bool capture) {
    capturingKeyPress = capture;
}
