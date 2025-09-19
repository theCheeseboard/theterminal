// On Windows do NOT show a console window when opening the app
#![cfg_attr(all(not(test), target_os = "windows"), windows_subsystem = "windows")]

mod main_surface;
mod main_window;

use crate::main_window::MainWindow;
use cntp_i18n::{I18N_MANAGER, tr, tr_load};
use cntp_icon_tool_macros::application_icon;
use contemporary::application::{ApplicationLink, Details, License, new_contemporary_application};
use contemporary::macros::application_details;
use contemporary::setup::{Contemporary, ContemporaryMenus, setup_contemporary};
use contemporary::window::contemporary_window_options;
use gpui::{App, Bounds, Menu, MenuItem, WindowBounds, WindowOptions, px, size};
use smol_macros::main;
use std::any::TypeId;
use std::rc::Rc;
use theterminal::actions::{
    CloseTabAction, CopyAction, CutAction, NewTabAction, PasteAction, register_actions,
};
use theterminal::terminal_screen::bind_terminal_screen_keys;

fn mane() {
    application_icon!("../dist/baseicon.svg");

    new_contemporary_application().run(|cx: &mut App| {
        I18N_MANAGER.write().unwrap().load_source(tr_load!());
        let bounds = Bounds::centered(None, size(px(800.0), px(600.0)), cx);

        let default_window_options = contemporary_window_options(cx, "theTerminal".into());
        register_actions(cx);
        bind_terminal_screen_keys(cx);
        cx.open_window(
            WindowOptions {
                window_bounds: Some(WindowBounds::Windowed(bounds)),
                ..default_window_options
            },
            |_, cx| {
                let window = MainWindow::new(cx);
                let weak_window = window.downgrade();
                let weak_windew = window.downgrade();
                let weak_windaw = window.downgrade();

                setup_contemporary(
                    cx,
                    Contemporary {
                        details: Details {
                            generatable: application_details!(),
                            copyright_holder: "Victor Tran",
                            copyright_year: "2025",
                            application_version: "5.0",
                            license: License::Gpl3OrLater,
                            links: [
                                (
                                    ApplicationLink::FileBug,
                                    "https://github.com/vicr123/theterminal/issues",
                                ),
                                (
                                    ApplicationLink::SourceCode,
                                    "https://github.com/vicr123/theterminal",
                                ),
                            ]
                            .into(),
                        },
                        menus: ContemporaryMenus {
                            menus: vec![
                                Menu {
                                    name: tr!("MENU_FILE", "File").into(),
                                    items: vec![
                                        MenuItem::action(
                                            tr!("FILE_NEW_TAB", "New Tab"),
                                            NewTabAction,
                                        ),
                                        MenuItem::action(
                                            tr!("FILE_CLOSE_TAB", "Close Tab"),
                                            CloseTabAction,
                                        ),
                                    ],
                                },
                                Menu {
                                    name: tr!("MENU_EDIT", "Edit").into(),
                                    items: vec![
                                        MenuItem::action(tr!("EDIT_COPY", "Copy"), CopyAction),
                                        MenuItem::action(tr!("EDIT_CUT", "Cut"), CutAction),
                                        MenuItem::action(tr!("EDIT_PASTE", "Paste"), PasteAction),
                                    ],
                                },
                            ],
                            on_about: Rc::new(move |cx| {
                                weak_window.upgrade().unwrap().update(cx, |window, cx| {
                                    window.about_surface_open(true);
                                    cx.notify()
                                })
                            }),
                            on_settings: None,
                        },
                    },
                );

                window
            },
        )
        .unwrap();
        cx.activate(true);
    });
}

main! {
    async fn main() {
        mane()
    }
}
