use gpui::http_client::anyhow;
use gpui::private::anyhow;
use gpui::{Hsla, Rgba};
use ini::inistr;
use std::collections::HashMap;
use std::mem;
use vt100::Color;

#[derive(Copy, Clone)]
pub struct ColorScheme {
    pub background: Hsla,
    pub foreground: Hsla,
    pub indexed: [Hsla; 16],
}

impl ColorScheme {
    pub fn new_from_inbuilt(color_scheme: InbuiltColorScheme) -> anyhow::Result<ColorScheme> {
        Self::new_from_string(color_scheme.color_scheme_file())
    }

    fn new_from_string(string: &str) -> anyhow::Result<ColorScheme> {
        let data =
            inistr!(safe string).map_err(|_| anyhow!("Unable to parse color scheme file"))?;

        let reader = ColorSchemeReader { data };

        Ok(ColorScheme {
            background: reader.read("background")?,
            foreground: reader.read("foreground")?,
            indexed: [
                reader.read("color0")?,
                reader.read("color1")?,
                reader.read("color2")?,
                reader.read("color3")?,
                reader.read("color4")?,
                reader.read("color5")?,
                reader.read("color6")?,
                reader.read("color7")?,
                reader.read("color0intense")?,
                reader.read("color1intense")?,
                reader.read("color2intense")?,
                reader.read("color3intense")?,
                reader.read("color4intense")?,
                reader.read("color5intense")?,
                reader.read("color6intense")?,
                reader.read("color7intense")?,
            ],
        })
    }

    pub fn reverse_when(mut self, reverse: bool) -> ColorScheme {
        if reverse {
            mem::swap(&mut self.background, &mut self.foreground);
        }
        self
    }

    pub fn parse_color(&self, color: Color, default_color: Hsla) -> Hsla {
        match color {
            Color::Default => default_color,
            Color::Idx(idx) if idx < 16 => {
                *self.indexed.get(idx as usize).unwrap_or(&default_color)
            }
            Color::Idx(idx) if idx < 232 => {
                let color = idx - 16;
                let b = color % 6;
                let g = color / 6 % 6;
                let r = color / 36;

                Rgba {
                    r: if r == 0 {
                        0.
                    } else {
                        (r * 40 + 55) as f32 / 255.
                    },
                    g: if g == 0 {
                        0.
                    } else {
                        (g * 40 + 55) as f32 / 255.
                    },
                    b: if b == 0 {
                        0.
                    } else {
                        (b * 40 + 55) as f32 / 255.
                    },
                    a: 1.,
                }
                .into()
            }
            Color::Idx(idx) => {
                let color = idx - 232;
                let intensity = (color * 10 + 8) as f32 / 255.;
                Rgba {
                    r: intensity,
                    g: intensity,
                    b: intensity,
                    a: 1.,
                }
                .into()
            }
            Color::Rgb(r, g, b) => Rgba {
                r: r as f32 / 255.,
                g: g as f32 / 255.,
                b: b as f32 / 255.,
                a: 1.,
            }
            .into(),
        }
    }
}

impl Default for ColorScheme {
    fn default() -> Self {
        if cfg!(target_os = "windows") {
            ColorScheme::new_from_inbuilt(InbuiltColorScheme::Campbell).unwrap()
        } else {
            ColorScheme::new_from_inbuilt(InbuiltColorScheme::Linux).unwrap()
        }
    }
}

pub enum InbuiltColorScheme {
    Linux,
    PowerShell,
    Ubuntu,
    Campbell,
    Solarized,
    SolarizedLight,
    GreenOnBlack,
}

impl InbuiltColorScheme {
    pub fn color_scheme_file(self) -> &'static str {
        match self {
            InbuiltColorScheme::Linux => include_str!("../../dist/color_schemes/Linux.colorscheme"),
            InbuiltColorScheme::PowerShell => {
                include_str!("../../dist/color_schemes/PowerShell.colorscheme")
            }
            InbuiltColorScheme::Ubuntu => {
                include_str!("../../dist/color_schemes/Ubuntu.colorscheme")
            }
            InbuiltColorScheme::Campbell => {
                include_str!("../../dist/color_schemes/Campbell.colorscheme")
            }
            InbuiltColorScheme::Solarized => {
                include_str!("../../dist/color_schemes/Solarized.colorscheme")
            }
            InbuiltColorScheme::SolarizedLight => {
                include_str!("../../dist/color_schemes/SolarizedLight.colorscheme")
            }
            InbuiltColorScheme::GreenOnBlack => {
                include_str!("../../dist/color_schemes/GreenOnBlack.colorscheme")
            }
        }
    }
}

struct ColorSchemeReader {
    data: HashMap<String, HashMap<String, Option<String>>>,
}

impl ColorSchemeReader {
    pub fn read(&self, section: &'static str) -> anyhow::Result<Hsla> {
        let color_string = self
            .data
            .get(section)
            .and_then(|section| section.get("color").cloned())
            .flatten()
            .ok_or(anyhow!("Unable to find color in color scheme"))?;

        // Interpret as RGB
        let parts = color_string.split(',').collect::<Vec<&str>>();
        if parts.len() != 3 {
            return Err(anyhow!("Color string is not in the correct format"));
        }

        let r = parts[0].parse::<u8>()?;
        let g = parts[1].parse::<u8>()?;
        let b = parts[2].parse::<u8>()?;

        Ok(Rgba {
            r: r as f32 / 255.,
            g: g as f32 / 255.,
            b: b as f32 / 255.,
            a: 1.,
        }
        .into())
    }
}
