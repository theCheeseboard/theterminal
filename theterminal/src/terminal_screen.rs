mod color_scheme;
mod keyboard;
mod run_calculator;

use crate::actions::PasteAction;
use crate::terminal_screen::color_scheme::ColorScheme;
use crate::terminal_screen::keyboard::Keyboard;
use crate::terminal_screen::run_calculator::RunCalculator;
use async_channel::Sender;
use cntp_i18n::tr;
use gpui::{
    App, AppContext, AsyncApp, BorderStyle, Bounds, Context, Corners, CursorStyle, Element, Entity,
    EntityInputHandler, FocusHandle, Focusable, Hitbox, HitboxBehavior, Hsla, InteractiveElement,
    IntoElement, KeyBinding, KeyDownEvent, ParentElement, Pixels, Point, Refineable, Render, Style,
    StyleRefinement, Styled, TextAlign, TextRun, UTF16Selection, WeakEntity, Window, WrappedLine,
    actions, bounds, canvas, div, point, px, quad, rgb, size, transparent_black,
};
use portable_pty::{CommandBuilder, PtySize, native_pty_system};
use smol::io::BufReader;
use std::cell::RefCell;
use std::io::{Read, Write};
use std::ops::Range;
use std::ptr::read;
use std::rc::Rc;
use std::sync::{Arc, Mutex, RwLock};
use std::thread;
use tracing::{info, warn};
use vt100::{Callbacks, Parser, Screen};

actions!(terminal_screen, [Backspace, Delete, Left, Right]);

pub fn bind_terminal_screen_keys(cx: &mut App) {
    cx.bind_keys([
        KeyBinding::new("backspace", Backspace, None),
        KeyBinding::new("delete", Delete, None),
        KeyBinding::new("left", Left, None),
        KeyBinding::new("right", Right, None),
    ]);
}

pub struct TerminalScreen {
    screen: Entity<Parser<TerminalScreenCallbacks>>,
    style: StyleRefinement,
    screen_size: Entity<ScreenSize>,
    writer: Rc<RefCell<Box<dyn Write + Send>>>,
    focus_handle: FocusHandle,
    color_scheme: ColorScheme,
    keyboard: Keyboard,
}

pub struct TerminalScreenPrepaint {
    screen: Screen,
    screen_hitbox: Hitbox,
    screen_lines: Vec<Vec<WrappedLine>>,
    style: Style,
    background: Hsla,
    caret_rect: Bounds<Pixels>,
    caret_color: Hsla,
}

#[derive(PartialEq, Copy, Clone)]
pub struct ScreenSize {
    columns: u16,
    lines: u16,
}

impl TerminalScreen {
    pub fn new(cx: &mut App) -> Entity<TerminalScreen> {
        let screen_size_entity = cx.new(|_| ScreenSize {
            columns: 40,
            lines: 20,
        });

        cx.new(|cx| {
            let mut writer = None;

            let (tx_read, rx_read) = async_channel::unbounded();
            let (tx_write, rx_write) = async_channel::unbounded();

            let weak_terminal_screen = cx.weak_entity();

            let parser = cx.new(|cx| {
                let screen_size = screen_size_entity.read(cx);
                let mut parser = Parser::new_with_callbacks(
                    screen_size.lines,
                    screen_size.columns,
                    1000,
                    TerminalScreenCallbacks { tx_write },
                );

                let pty = native_pty_system();
                let pty_pair = match pty.openpty(PtySize {
                    rows: screen_size.lines,
                    cols: screen_size.columns,
                    pixel_width: 0,
                    pixel_height: 0,
                }) {
                    Ok(pty_pair) => pty_pair,
                    Err(error) => {
                        warn!("Unable to open pty: {error}");
                        parser.process(
                            tr!("PTY_OPEN_ERROR", "Unable to open pty")
                                .to_string()
                                .as_bytes(),
                        );
                        return parser;
                    }
                };

                let mut cmd = CommandBuilder::new(default_shell());
                cmd.env("TERM", "xterm-256color");
                if let Err(error) = pty_pair.slave.spawn_command(cmd) {
                    warn!("Unable to spawn process: {error}");
                    parser.process(
                        tr!("PTY_SPAWN_ERROR", "Unable to spawn process")
                            .to_string()
                            .as_bytes(),
                    );
                    return parser;
                }

                let mut reader = pty_pair.master.try_clone_reader().unwrap();
                thread::spawn(move || {
                    loop {
                        let mut buf = [0_u8; 1024];
                        let read_len = reader.read(&mut buf).unwrap();
                        smol::block_on(tx_read.send(buf[0..read_len].to_vec())).unwrap();
                    }
                });

                let raw_writer = Rc::new(RefCell::new(pty_pair.master.take_writer().unwrap()));
                writer = Some(raw_writer.clone());

                cx.spawn(async move |_, _| {
                    loop {
                        let bytes = rx_write.recv().await.unwrap();
                        raw_writer.borrow_mut().write_all(&bytes).unwrap();
                    }
                })
                .detach();

                cx.observe(&screen_size_entity, move |parser, screen_size, cx| {
                    let screen_size = screen_size.read(cx);
                    pty_pair
                        .master
                        .resize(PtySize {
                            rows: screen_size.lines,
                            cols: screen_size.columns,
                            pixel_width: 0,
                            pixel_height: 0,
                        })
                        .unwrap();

                    parser
                        .screen_mut()
                        .set_size(screen_size.lines, screen_size.columns);
                })
                .detach();

                parser
            });

            cx.observe(&parser, |_, _, cx| cx.notify()).detach();

            let parser_entity_clone = parser.clone();

            cx.spawn(async move |_, cx: &mut AsyncApp| {
                loop {
                    let bytes = rx_read.recv().await.unwrap();

                    cx.update_entity(&parser_entity_clone, |parser, cx| {
                        parser.process(bytes.as_slice());
                        cx.notify();
                    })
                    .unwrap();
                }
            })
            .detach();

            TerminalScreen {
                screen: parser,
                style: StyleRefinement::default().font_family("JetBrains Mono"),
                screen_size: screen_size_entity,
                writer: writer.take().unwrap(),
                focus_handle: cx.focus_handle(),
                color_scheme: ColorScheme::default(),
                keyboard: Keyboard::default(),
            }
        })
    }

    pub fn backspace(&mut self, _: &Backspace, window: &mut Window, cx: &mut Context<Self>) {
        self.write_to_pty(&[0x08_u8]);
    }

    pub fn delete(&mut self, _: &Delete, window: &mut Window, cx: &mut Context<Self>) {}

    pub fn paste(&mut self, _: &PasteAction, window: &mut Window, cx: &mut Context<Self>) {
        if let Some(clipboard_contents) = cx
            .read_from_clipboard()
            .and_then(|clipboard| clipboard.text())
        {
            let screen = self.screen.read(cx);
            if screen.screen().bracketed_paste() {
                self.write_to_pty(b"\x1B[200~");
            }
            self.write_to_pty(clipboard_contents.as_bytes());
            if screen.screen().bracketed_paste() {
                self.write_to_pty(b"\x1B[201~");
            }
        }
    }

    pub fn write_to_pty(&self, bytes: &[u8]) {
        let _ = self.writer.borrow_mut().write_all(bytes);
    }

    fn process_key_press(&mut self, event: &KeyDownEvent, _: &mut Window, cx: &mut Context<Self>) {
        if let Some(sequence) = self.keyboard.get_escape_sequence(event) {
            self.write_to_pty(sequence.as_bytes());
        } else if event.keystroke.modifiers.control {
            match event.keystroke.key.as_str() {
                key if key.len() == 1 => {
                    let ch = key.chars().next().unwrap().to_ascii_uppercase();
                    if ch.is_ascii_uppercase() {
                        let ctrl_char = (ch as u8) - b'A' + 1;
                        self.write_to_pty(&[ctrl_char]);
                    } else if ch == '[' {
                        self.write_to_pty(b"\x1B");
                    } else if ch == '\\' {
                        self.write_to_pty(b"\x1C");
                    } else if ch == ']' {
                        self.write_to_pty(b"\x1D");
                    } else if ch == '^' {
                        self.write_to_pty(b"\x1E");
                    } else if ch == '_' {
                        self.write_to_pty(b"\x1F");
                    } else if ch == ' ' || ch == '@' {
                        self.write_to_pty(b"\x00");
                    }
                }
                _ => {}
            }
        } else if let Some(bytes) = match event.keystroke.key.as_str() {
            "enter" => {
                let screen = self.screen.read(cx);
                if screen.screen().new_line_mode() {
                    Some(b"\r\n".as_slice().to_vec())
                } else {
                    Some(b"\r".as_slice().to_vec())
                }
            }
            _ => {
                let kc = event.keystroke.key_char.clone().unwrap_or_default();
                Some(kc.as_bytes().to_vec())
            }
        } {
            self.write_to_pty(bytes.as_slice());
        }
    }
}

impl Render for TerminalScreen {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let screen = self.screen.clone();
        let screen_size = self.screen_size.clone();
        let style = self.style.clone();
        let color_scheme = self.color_scheme;

        div()
            .h_full()
            .w_full()
            .key_context("TerminalScreen")
            .track_focus(&self.focus_handle(cx))
            .on_action(cx.listener(Self::delete))
            .on_action(cx.listener(Self::paste))
            .on_key_down(cx.listener(|this, event: &KeyDownEvent, window, cx| {
                this.process_key_press(event, window, cx)
            }))
            .child(
                canvas(
                    move |bounds, window, cx| {
                        prepaint_terminal_screen(
                            screen,
                            screen_size,
                            style,
                            color_scheme,
                            bounds,
                            window,
                            cx,
                        )
                    },
                    paint_terminal_screen,
                )
                .w_full()
                .h_full(),
            )
    }
}

impl Focusable for TerminalScreen {
    fn focus_handle(&self, _: &App) -> FocusHandle {
        self.focus_handle.clone()
    }
}

impl EntityInputHandler for TerminalScreen {
    fn text_for_range(
        &mut self,
        range: Range<usize>,
        adjusted_range: &mut Option<Range<usize>>,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) -> Option<String> {
        None
    }

    fn selected_text_range(
        &mut self,
        ignore_disabled_input: bool,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) -> Option<UTF16Selection> {
        None
    }

    fn marked_text_range(
        &self,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) -> Option<Range<usize>> {
        None
    }

    fn unmark_text(&mut self, window: &mut Window, cx: &mut Context<Self>) {}

    fn replace_text_in_range(
        &mut self,
        range: Option<Range<usize>>,
        text: &str,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) {
        info!("Typed {text}");
        self.write_to_pty(text.as_bytes());
    }

    fn replace_and_mark_text_in_range(
        &mut self,
        range: Option<Range<usize>>,
        new_text: &str,
        new_selected_range: Option<Range<usize>>,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) {
        info!("Typed2 {new_text}");
        self.write_to_pty(new_text.as_bytes());
    }

    fn bounds_for_range(
        &mut self,
        range_utf16: Range<usize>,
        element_bounds: Bounds<Pixels>,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) -> Option<Bounds<Pixels>> {
        None
    }

    fn character_index_for_point(
        &mut self,
        point: Point<Pixels>,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) -> Option<usize> {
        None
    }
}

impl Styled for TerminalScreen {
    fn style(&mut self) -> &mut StyleRefinement {
        &mut self.style
    }
}

fn prepaint_terminal_screen(
    parser_entity: Entity<Parser<TerminalScreenCallbacks>>,
    screen_size: Entity<ScreenSize>,
    style_refinement: StyleRefinement,
    color_scheme: ColorScheme,
    bounds: Bounds<Pixels>,
    window: &mut Window,
    cx: &mut App,
) -> TerminalScreenPrepaint {
    let screen_hitbox = window.insert_hitbox(bounds, HitboxBehavior::Normal);

    let style = Style::default().refined(style_refinement);

    let screen = parser_entity.read(cx).screen().clone();
    let color_scheme = color_scheme.reverse_when(screen.reverse_video());

    let (screen_lines, caret_rect) =
        window.with_text_style(style.text_style().cloned(), |window| {
            let text_style = window.text_style();
            let line_height = text_style.line_height_in_pixels(window.rem_size());

            let character_size = window
                .text_system()
                .shape_text(
                    "E".into(),
                    text_style.font_size.to_pixels(window.rem_size()),
                    &[text_style.to_run(1)],
                    None,
                    None,
                )
                .unwrap()[0]
                .size(line_height);

            let new_screen_size = ScreenSize {
                columns: (bounds.size.width / character_size.width).floor() as u16,
                lines: (bounds.size.height / character_size.height).floor() as u16,
            };

            if *screen_size.read(cx) != new_screen_size {
                let screen_size = screen_size.clone();
                cx.spawn(async move |cx: &mut AsyncApp| {
                    cx.update_entity(&screen_size, |screen_size, cx| {
                        screen_size.columns = new_screen_size.columns;
                        screen_size.lines = new_screen_size.lines;
                        cx.notify();
                    })
                    .unwrap();
                })
                .detach();
            }

            let mut screen_lines = Vec::new();
            for line in 0..screen.size().0 {
                let mut run_calculator = RunCalculator::new(
                    window.text_system().clone(),
                    text_style.clone(),
                    window.rem_size(),
                    color_scheme,
                );

                for column in 0..screen.size().1 {
                    let cell = screen.cell(line, column).unwrap();
                    run_calculator.push_cell(cell.clone());
                }

                screen_lines.push(run_calculator.runs());
            }

            let caret_rect = Bounds {
                origin: point(
                    screen.cursor_position().1 as f32 * character_size.width,
                    screen.cursor_position().0 as f32 * character_size.height,
                ) + bounds.origin,
                size: size(px(1.), character_size.height),
            };

            (screen_lines, caret_rect)
        });

    let caret_color = color_scheme.parse_color(screen.fgcolor(), color_scheme.foreground);

    TerminalScreenPrepaint {
        screen,
        screen_hitbox,
        style,
        screen_lines,
        background: color_scheme.background,
        caret_rect,
        caret_color,
    }
}

fn paint_terminal_screen(
    bounds: Bounds<Pixels>,
    prepaint_state: TerminalScreenPrepaint,
    window: &mut Window,
    cx: &mut App,
) {
    window.paint_quad(quad(
        bounds,
        Corners::all(px(0.)),
        prepaint_state.background,
        px(0.),
        transparent_black(),
        BorderStyle::Solid,
    ));

    window.set_cursor_style(
        if cfg!(target_os = "windows") {
            CursorStyle::Arrow
        } else {
            CursorStyle::IBeam
        },
        &prepaint_state.screen_hitbox,
    );

    window.with_text_style(prepaint_state.style.text_style().cloned(), |window| {
        let text_style = window.text_style();
        let line_height = text_style.line_height_in_pixels(window.rem_size());

        let mut y = bounds.origin.y;
        for line in prepaint_state.screen_lines {
            let mut x = bounds.origin.x;
            for shaped_text in line {
                shaped_text
                    .paint_background(
                        Point::new(x, y),
                        line_height,
                        TextAlign::Left,
                        None,
                        window,
                        cx,
                    )
                    .unwrap();

                shaped_text
                    .paint(
                        Point::new(x, y),
                        line_height,
                        TextAlign::Left,
                        None,
                        window,
                        cx,
                    )
                    .unwrap();

                x += shaped_text.width();
            }

            y += line_height;
        }
    });

    window.paint_quad(quad(
        prepaint_state.caret_rect,
        Corners::all(px(0.)),
        prepaint_state.caret_color,
        px(0.),
        transparent_black(),
        BorderStyle::Solid,
    ));
}

struct TerminalScreenCallbacks {
    tx_write: Sender<Vec<u8>>,
}

impl Callbacks for TerminalScreenCallbacks {
    fn unhandled_control(&mut self, _: &mut Screen, b: u8) {
        warn!("Unhandled control: {b:?}");
    }

    fn unhandled_escape(&mut self, _: &mut Screen, i1: Option<u8>, i2: Option<u8>, b: u8) {
        warn!("Unhandled escape: {i1:?} {i2:?} {b:?}")
    }

    fn unhandled_csi(
        &mut self,
        _: &mut Screen,
        i1: Option<u8>,
        i2: Option<u8>,
        params: &[&[u16]],
        c: char,
    ) {
        warn!(
            "Unhandled csi: {:?} {:?} {params:?} {c:?}",
            i1.map(|i| i as char),
            i2.map(|i| i as char)
        )
    }

    fn unhandled_osc(&mut self, _: &mut Screen, params: &[&[u8]]) {
        warn!("Unhandled osc: {params:?}")
    }

    fn write_to_pty(&mut self, _: &mut Screen, bytes: &[u8]) {
        smol::block_on(self.tx_write.send(bytes.to_vec())).unwrap();
    }
}

fn default_shell() -> String {
    if cfg!(target_os = "windows") {
        "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe".to_string()
    } else if cfg!(target_os = "macos") {
        #[cfg(target_os = "macos")]
        unsafe {
            use objc2_foundation::{NSString, NSUserName};
            use objc2_open_directory::{
                ODNode, ODSession, kODAttributeTypeUserShell, kODRecordTypeUsers,
            };

            let default_session = ODSession::defaultSession().unwrap();
            let node = ODNode::nodeWithSession_name_error(
                Some(default_session.as_ref()),
                Some(&NSString::from_str("/Local/Default")),
                None,
            )
            .unwrap();
            let record = node
                .recordWithRecordType_name_attributes_error(
                    kODRecordTypeUsers,
                    Some(&*NSUserName()),
                    None,
                    None,
                )
                .unwrap();
            let values = record
                .valuesForAttribute_error(kODAttributeTypeUserShell, None)
                .unwrap();
            let string = values
                .firstObject()
                .unwrap()
                .downcast::<NSString>()
                .unwrap();
            string.to_string()
        }
    } else {
        "bash".to_string()
    }
}
