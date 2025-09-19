use gpui::{App, KeyBinding, actions};

actions!(
    theterminal,
    [NewTabAction, CopyAction, CutAction, PasteAction]
);

fn theterminal_keybindings(keystrokes: &str) -> String {
    if cfg!(target_os = "macos") {
        keystrokes.to_string()
    } else {
        keystrokes
            .replace("secondary", "secondary-shift")
            .to_string()
    }
}

pub fn register_actions(cx: &mut App) {
    cx.on_action(new_tab);
    cx.bind_keys([
        KeyBinding::new(
            theterminal_keybindings("secondary-t").as_str(),
            NewTabAction,
            None,
        ),
        KeyBinding::new(
            theterminal_keybindings("secondary-c").as_str(),
            CopyAction,
            None,
        ),
        KeyBinding::new(
            theterminal_keybindings("secondary-x").as_str(),
            CutAction,
            None,
        ),
        KeyBinding::new(
            theterminal_keybindings("secondary-v").as_str(),
            PasteAction,
            None,
        ),
    ])
}

fn new_tab(_: &NewTabAction, cx: &mut App) {}
