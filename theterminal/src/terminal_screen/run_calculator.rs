use crate::terminal_screen::color_scheme;
use crate::terminal_screen::color_scheme::ColorScheme;
use gpui::{
    FontWeight, Hsla, Pixels, Refineable, Rgba, TextStyle, TextStyleRefinement, UnderlineStyle,
    WindowTextSystem, WrappedLine, px, rgb,
};
use std::mem;
use std::sync::Arc;
use vt100::{Cell, Color};

pub struct RunCalculator {
    current_string: String,
    current_cell_attributes: CellAttributes,
    current_runs: Vec<WrappedLine>,
    window_text_system: Arc<WindowTextSystem>,
    text_style: TextStyle,
    rem_size: Pixels,
    color_scheme: ColorScheme,
}

impl RunCalculator {
    pub fn new(
        text_system: Arc<WindowTextSystem>,
        text_style: TextStyle,
        rem_size: Pixels,
        color_scheme: ColorScheme,
    ) -> Self {
        Self {
            current_string: String::new(),
            current_cell_attributes: CellAttributes::default(),
            current_runs: Vec::new(),
            window_text_system: text_system,
            text_style,
            rem_size,
            color_scheme,
        }
    }

    pub fn push_cell(&mut self, cell: Option<Cell>) {
        if let Some(cell) = cell {
            let attributes = cell.clone().into();
            if self.current_cell_attributes != attributes {
                self.finalise_run();
                self.current_string = String::new();
                self.current_cell_attributes = attributes;
            }

            self.current_string += if cell.has_contents() {
                cell.contents()
            } else {
                " "
            }
        } else {
            self.current_string += " ";
        }
    }

    fn finalise_run(&mut self) {
        if self.current_string.is_empty() {
            return;
        }

        let string = mem::take(&mut self.current_string);

        let text_style = self.text_style.clone().refined(
            self.current_cell_attributes
                .text_style_refinement(self.color_scheme),
        );

        let run_length = string.len();
        self.current_runs.push(
            self.window_text_system
                .shape_text(
                    string.into(),
                    text_style.font_size.to_pixels(self.rem_size),
                    &[text_style.to_run(run_length)],
                    None,
                    None,
                )
                .unwrap()[0]
                .clone(),
        )
    }

    pub fn runs(mut self) -> Vec<WrappedLine> {
        self.finalise_run();
        self.current_runs
    }
}

#[derive(PartialEq, Default)]
struct CellAttributes {
    fg: Color,
    bg: Color,
    bold: bool,
    dim: bool,
    underline: bool,
    inverse: bool,
}

impl CellAttributes {
    pub fn text_style_refinement(&self, color_scheme: ColorScheme) -> TextStyleRefinement {
        let mut foreground = Some(color_scheme.parse_color(self.fg, color_scheme.foreground));
        let mut background = Some(color_scheme.parse_color(self.bg, color_scheme.background));
        if self.inverse {
            mem::swap(&mut foreground, &mut background);
        }

        TextStyleRefinement {
            color: foreground,
            background_color: background,
            font_weight: Some(if self.bold {
                FontWeight::BOLD
            } else {
                FontWeight::NORMAL
            }),
            underline: if self.underline {
                Some(UnderlineStyle {
                    color: foreground,
                    thickness: px(1.),
                    ..UnderlineStyle::default()
                })
            } else {
                None
            },
            ..TextStyleRefinement::default()
        }
    }
}

impl From<Cell> for CellAttributes {
    fn from(value: Cell) -> Self {
        Self {
            fg: value.fgcolor(),
            bg: value.bgcolor(),
            bold: value.bold(),
            dim: value.dim(),
            underline: value.underline(),
            inverse: value.inverse(),
        }
    }
}
