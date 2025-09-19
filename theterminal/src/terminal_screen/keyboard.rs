use gpui::private::anyhow;
use gpui::{KeyDownEvent, KeyEvent};
use serde::Deserialize;
use std::collections::HashMap;

pub struct Keyboard {
    keymap: HashMap<String, KeyValue>,
}

#[derive(Deserialize)]
#[serde(untagged)]
enum KeyValue {
    Seq(String),
    KeySeq(Vec<KeySeq>),
}

#[derive(Deserialize)]
struct KeySeq {
    control: Option<bool>,
    alt: Option<bool>,
    shift: Option<bool>,
    seq: String,
}

impl Keyboard {
    pub fn new_from_string(string: &str) -> anyhow::Result<Keyboard> {
        Ok(Keyboard {
            keymap: serde_json::from_str(string)?,
        })
    }

    pub fn get_escape_sequence(&self, event: &KeyDownEvent) -> Option<String> {
        let modifiers = event.keystroke.modifiers;
        match self.keymap.get(&event.keystroke.key)? {
            KeyValue::Seq(seq_string) => Some(seq_string.clone()),
            KeyValue::KeySeq(seqs) => Some(
                seqs.iter()
                    .find(|seq| {
                        seq.control
                            .is_none_or(|control| control == modifiers.control)
                            && seq.alt.is_none_or(|alt| alt == modifiers.alt)
                            && seq.shift.is_none_or(|shift| shift == modifiers.shift)
                    })?
                    .seq
                    .clone(),
            ),
        }
    }
}

impl Default for Keyboard {
    fn default() -> Self {
        Keyboard::new_from_string(include_str!("../../dist/keyboard/default.keyboard")).unwrap()
    }
}
