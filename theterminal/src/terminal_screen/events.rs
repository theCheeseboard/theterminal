use crate::terminal_screen::TerminalScreen;
use gpui::{App, Entity, Window};
use std::rc::Rc;

pub struct TerminalScreenCloseEvent {
    pub terminal: Entity<TerminalScreen>,
}

#[derive(Clone)]
pub struct TerminalScreenEvents {
    pub close_requested: Rc<dyn Fn(&TerminalScreenCloseEvent, &mut Window, &mut App)>,
}
