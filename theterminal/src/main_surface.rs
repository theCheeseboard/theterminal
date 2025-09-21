use cntp_i18n::tr;
use contemporary::components::application_menu::ApplicationMenu;
use contemporary::components::button::button;
use contemporary::components::icon::icon;
use contemporary::components::pager::pager;
use contemporary::components::pager::slide_horizontal_animation::SlideHorizontalAnimation;
use contemporary::styling::theme::Theme;
use contemporary::surface::surface;
use gpui::prelude::FluentBuilder;
use gpui::{
    App, AppContext, Context, Entity, InteractiveElement, IntoElement, Menu, MenuItem,
    ParentElement, Render, Styled, Window, div, px,
};
use std::rc::Rc;
use theterminal::actions::{CloseTabAction, CopyAction, NewTabAction, PasteAction};
use theterminal::terminal_screen::TerminalScreen;
use theterminal::terminal_screen::events::{TerminalScreenCloseEvent, TerminalScreenEvents};

pub struct MainSurface {
    application_menu: Entity<ApplicationMenu>,
    terminal_screens: Vec<Entity<TerminalScreen>>,
    current_terminal_screen: usize,
    events: TerminalScreenEvents,
}

impl MainSurface {
    pub fn new(cx: &mut App) -> Entity<MainSurface> {
        cx.new(|cx| {
            let events = TerminalScreenEvents {
                close_requested: Rc::new(cx.listener(Self::terminal_tab_closed)),
            };

            MainSurface {
                application_menu: ApplicationMenu::new(
                    cx,
                    Menu {
                        name: "Application Menu".into(),
                        items: vec![
                            MenuItem::action(tr!("FILE_NEW_TAB"), NewTabAction),
                            MenuItem::action(tr!("FILE_CLOSE_TAB"), CloseTabAction),
                            MenuItem::separator(),
                            MenuItem::action(tr!("EDIT_COPY"), CopyAction),
                            MenuItem::action(tr!("EDIT_PASTE"), PasteAction),
                        ],
                    },
                ),

                terminal_screens: vec![TerminalScreen::new(cx, events.clone())],
                current_terminal_screen: 0,
                events,
            }
        })
    }
}

impl MainSurface {
    fn new_tab(&mut self, _: &NewTabAction, window: &mut Window, cx: &mut Context<Self>) {
        self.terminal_screens
            .push(TerminalScreen::new(cx, self.events.clone()));
        self.current_terminal_screen = self.terminal_screens.len() - 1;
        cx.notify()
    }

    fn close_tab(&mut self, _: &CloseTabAction, window: &mut Window, cx: &mut Context<Self>) {
        let Some(current_terminal_screen) = self
            .terminal_screens
            .get(self.current_terminal_screen)
            .cloned()
        else {
            return;
        };

        window.on_next_frame(move |window, cx| {
            current_terminal_screen.update(cx, |terminal_screen, cx| {
                terminal_screen.request_close(window, cx);
            });
        });
    }

    fn terminal_tab_closed(
        &mut self,
        event: &TerminalScreenCloseEvent,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) {
        let Some(index) = self
            .terminal_screens
            .iter()
            .position(|terminal| terminal.entity_id() == event.terminal.entity_id())
        else {
            return;
        };

        if self.terminal_screens.len() == 1 {
            window.remove_window();
            return;
        }

        self.terminal_screens.remove(index);
        if self.current_terminal_screen != 0 && self.current_terminal_screen >= index {
            self.current_terminal_screen -= 1;
        }
        cx.notify();
    }
}

impl Render for MainSurface {
    fn render(&mut self, _: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        div()
            .size_full()
            .key_context("MainSurface")
            .on_action(cx.listener(Self::new_tab))
            .on_action(cx.listener(Self::close_tab))
            .child(
                surface()
                    .actions(
                        div()
                            .occlude()
                            .flex()
                            .gap(px(2.))
                            .content_stretch()
                            .child(
                                self.terminal_screens.iter().enumerate().fold(
                                    div()
                                        .flex()
                                        .id("action-bar")
                                        .bg(theme.button_background)
                                        .rounded(theme.border_radius)
                                        .gap(px(2.))
                                        .content_stretch(),
                                    |david, (i, terminal_screen)| {
                                        let terminal_screen = terminal_screen.read(cx);

                                        let tab_subtext = terminal_screen
                                            .working_directory()
                                            .and_then(|path| {
                                                path.iter()
                                                    .next_back()
                                                    .map(|str| str.to_string_lossy().to_string())
                                            })
                                            .unwrap_or_else(|| "".to_string());

                                        david.child(
                                            button(i)
                                                .child(
                                                    div()
                                                        .flex()
                                                        .flex_col()
                                                        .child(
                                                            if terminal_screen.title().is_empty() {
                                                                tr!("TERMINAL_DEFAULT_TITLE")
                                                                    .to_string()
                                                            } else {
                                                                terminal_screen.title()
                                                            },
                                                        )
                                                        .when(!tab_subtext.is_empty(), |david| {
                                                            david.child(
                                                                div()
                                                                    .text_size(
                                                                        theme.system_font_size
                                                                            * 0.6,
                                                                    )
                                                                    .child(tab_subtext),
                                                            )
                                                        }),
                                                )
                                                .on_click(cx.listener(move |this, _, _, cx| {
                                                    this.current_terminal_screen = i;
                                                    cx.notify()
                                                }))
                                                .when(
                                                    self.current_terminal_screen == i,
                                                    |button| button.checked(),
                                                ),
                                        )
                                    },
                                ),
                            )
                            .child(
                                button("new-tab-button")
                                    .child(icon("list-add".into()))
                                    .flat()
                                    .on_click(cx.listener(|this, _, window, cx| {
                                        this.new_tab(&NewTabAction, window, cx);
                                    })),
                            ),
                    )
                    .child(
                        self.terminal_screens.iter().fold(
                            pager("main-pager", self.current_terminal_screen)
                                .flex_grow()
                                .h_full()
                                .animation(SlideHorizontalAnimation::new()),
                            |pager, terminal_screen| {
                                pager.page(
                                    div()
                                        .w_full()
                                        .h_full()
                                        .pt(px(40.))
                                        .child(terminal_screen.clone())
                                        .into_any_element(),
                                )
                            },
                        ),
                    )
                    .application_menu(self.application_menu.clone()),
            )
    }
}
