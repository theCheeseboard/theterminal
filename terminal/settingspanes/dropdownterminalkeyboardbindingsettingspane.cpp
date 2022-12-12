#include "dropdownterminalkeyboardbindingsettingspane.h"
#include "ui_dropdownterminalkeyboardbindingsettingspane.h"

#include <tsettings.h>
#include <tx11info.h>

#ifndef Q_OS_MAC
    #include <X11/XF86keysym.h>
    #include <X11/XKBlib.h>
    #include <X11/Xatom.h>
    #include <X11/Xlib.h>
    #include <xcb/xcb.h>
#endif

struct DropdownTerminalKeyboardBindingSettingsPanePrivate {
        tSettings settings;
};

DropdownTerminalKeyboardBindingSettingsPane::DropdownTerminalKeyboardBindingSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::DropdownTerminalKeyboardBindingSettingsPane) {
    ui->setupUi(this);
    d = new DropdownTerminalKeyboardBindingSettingsPanePrivate();

    if (tX11Info::isPlatformX11()) {
        on_keybindingButton_toggled(false);
    } else {
        ui->keybindingButton->setVisible(false);
    }
}

DropdownTerminalKeyboardBindingSettingsPane::~DropdownTerminalKeyboardBindingSettingsPane() {
    delete d;
    delete ui;
}

QString DropdownTerminalKeyboardBindingSettingsPane::paneName() {
    return tr("Keyboard Binding");
}

void DropdownTerminalKeyboardBindingSettingsPane::on_keybindingButton_toggled(bool checked) {
#ifndef Q_OS_MAC
    if (checked) {
        // Capture keyboard
        XGrabKeyboard(tX11Info::display(), RootWindow(tX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync, CurrentTime);
        tApplication::instance()->installNativeEventFilter(this);

        ui->keybindingButton->setText("Strike a key!");
    } else {
        XUngrabKeyboard(tX11Info::display(), CurrentTime);
        tApplication::instance()->removeNativeEventFilter(this);

        ui->keybindingButton->setText(d->settings.value("dropdown/keyString").toString());
    }
#endif
}

bool DropdownTerminalKeyboardBindingSettingsPane::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == XCB_KEY_RELEASE) {
            xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);

            KeySym keyPressed = XkbKeycodeToKeysym(tX11Info::display(), button->detail, 0, button->state & ShiftMask ? 1 : 0);

            d->settings.setValue("dropdown/key", QVariant::fromValue(keyPressed));
            d->settings.setValue("dropdown/keyState", button->state);
            d->settings.setValue("dropdown/keyString", QString::fromUtf8(XKeysymToString(keyPressed)));
            ui->keybindingButton->setChecked(false);
            return true;
        }
    }
}
