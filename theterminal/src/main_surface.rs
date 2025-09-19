use cntp_i18n::tr;
use contemporary::components::application_menu::ApplicationMenu;
use contemporary::components::pager::pager;
use contemporary::components::pager::slide_horizontal_animation::SlideHorizontalAnimation;
use contemporary::styling::theme::Theme;
use contemporary::surface::surface;
use gpui::{
    App, AppContext, Context, Entity, InteractiveElement, IntoElement, Menu, MenuItem,
    ParentElement, Render, Styled, Window, div, px,
};
use theterminal::actions::NewTabAction;
use theterminal::terminal_screen::TerminalScreen;

pub struct MainSurface {
    application_menu: Entity<ApplicationMenu>,

    terminal_screen: Entity<TerminalScreen>,
}

impl MainSurface {
    pub fn new(cx: &mut App) -> Entity<MainSurface> {
        cx.new(|cx| MainSurface {
            application_menu: ApplicationMenu::new(
                cx,
                Menu {
                    name: "Application Menu".into(),
                    items: vec![MenuItem::action(tr!("FILE_NEW_TAB"), NewTabAction)],
                },
            ),

            terminal_screen: TerminalScreen::new(cx),
        })
    }
}

impl Render for MainSurface {
    fn render(&mut self, _: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        surface()
            .actions(
                div().occlude().flex().content_stretch().child(
                    div()
                        .flex()
                        .id("action-bar")
                        .bg(theme.button_background)
                        .rounded(theme.border_radius)
                        .gap(px(2.))
                        .content_stretch(), // TODO: Tab buttons
                ),
            )
            .child(
                pager("main-pager", 0)
                    .flex_grow()
                    .h_full()
                    .animation(SlideHorizontalAnimation::new())
                    .page(
                        div()
                            .w_full()
                            .h_full()
                            .pt(px(40.))
                            .child(self.terminal_screen.clone())
                            .into_any_element(),
                    ),
            )
            .application_menu(self.application_menu.clone())
    }
}
