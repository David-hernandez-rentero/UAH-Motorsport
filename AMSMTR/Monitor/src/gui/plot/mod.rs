use std::ops::RangeInclusive;

use eframe::{
    egui::{Id, Layout, ScrollArea, Ui},
    epaint::Hsva,
};
use egui_plot::{Line, PlotMemory, PlotUi};
use fixedbitset::FixedBitSet;

use crate::{
    DAISY_CHAINNED_ADBMS, Plotter, PlotterImpl, TEMP_CELLS_LAST_ADBMS, TEMP_CELLS_PER_ADBMS,
    VOLT_CELLS_PER_ADBMS,
    gui::{Main, plot::configure::PlotState},
};

mod configure;
mod legend;
mod sync_plots;

macro_rules! gen_lut {
    (cell, $daisy:expr, $cell:expr) => {{
        let mut str = *b"ADBMS #0 Cell #00";
        str["ADBMS #".len()] = b'0' + $daisy as u8 + 1;
        str["ADBMS #0 Cell #0".len()] = b'0' + ($cell as u8 + 1) % 10;
        str["ADBMS #0 Cell #".len()] = b'0' + ($cell as u8 + 1) / 10;
        str
    }};
    (cleanup, $raw:expr, $cell:expr, $hue:expr) => {
        (
            { unsafe { std::str::from_utf8_unchecked(&$raw[$cell]) } },
            Hsva {
                h: $hue,
                s: 1.0,
                v: 1.0,
                a: 1.0,
            },
        )
    };
    (array, $idx:ident, $inner:expr) => {{
        let mut $idx = 0usize;
        let mut res = [$inner; _];
        while {
            $idx += 1;
            $idx < res.len()
        } {
            res[$idx] = $inner;
        }
        res
    }};
    (nested, $adbms:expr, $cells:expr) => {{
        const RAW: [[[u8; "ADBMS #0 Cell #00".len()]; $cells]; $adbms] = {
            gen_lut!(array, daisy, {
                gen_lut!(array, cell, gen_lut!(cell, daisy, cell))
            })
        };
        let mut hue = 0.0;
        gen_lut!(array, daisy, {
            let res = gen_lut!(array, cell, gen_lut!(cleanup, RAW[daisy], cell, hue));
            hue += 1.0 / DAISY_CHAINNED_ADBMS as f32;
            res
        })
    }};
}

const VOLT_LUT: [[(&str, Hsva); VOLT_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS] =
    gen_lut!(nested, DAISY_CHAINNED_ADBMS, VOLT_CELLS_PER_ADBMS);
const TEMP_LUT: (
    [[(&str, Hsva); TEMP_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS - 1],
    [(&str, Hsva); TEMP_CELLS_LAST_ADBMS],
) = (
    gen_lut!(nested, DAISY_CHAINNED_ADBMS - 1, TEMP_CELLS_PER_ADBMS),
    {
        const RAW: [[u8; "ADBMS #0 Cell #00".len()]; TEMP_CELLS_LAST_ADBMS] =
            gen_lut!(array, cell, {
                gen_lut!(cell, DAISY_CHAINNED_ADBMS - 1, cell)
            });
        gen_lut!(array, cell, {
            gen_lut!(cleanup, RAW, cell, 1.0 - 1.0 / DAISY_CHAINNED_ADBMS as f32)
        })
    },
);
const VOLT_LUT_SLICE: [&[(&str, Hsva)]; DAISY_CHAINNED_ADBMS] =
    gen_lut!(array, daisy, VOLT_LUT[daisy].as_slice());
const TEMP_LUT_SLICE: [&[(&str, Hsva)]; DAISY_CHAINNED_ADBMS] = gen_lut!(array, daisy, {
    if daisy < TEMP_LUT.0.len() {
        TEMP_LUT.0[daisy].as_slice()
    } else {
        TEMP_LUT.1.as_slice()
    }
});

#[derive(Clone)]
struct PlotInfo {
    id: Id,
    title: &'static str,
    unit: &'static str,
    default_y_bounds: RangeInclusive<f64>,
    legend_entries: &'static [&'static [(&'static str, Hsva)]; DAISY_CHAINNED_ADBMS],
    expanded_group_hidden_entries: fn(&mut Main) -> (&mut Option<Id>, &mut FixedBitSet),
}

impl Main {
    pub(super) fn show_plots(&mut self, ui: &mut Ui) {
        let rightmost_point = self.plotter.as_ref().map(Plotter::rightmost_point);
        let height = ((ui.available_height() - ui.style().spacing.item_spacing.y) * 0.5).max(
            ui.input(|i| i.viewport().monitor_size.map(|s| s.y))
                .unwrap_or(1080.0)
                * 0.25,
        );
        let mut new_state = PlotState {
            manual_x_bounds: self.manual_x_bounds.clone(),
            manual_y_zoom: self.manual_y_zoom,
            auto_bounds: self.auto_bounds,
            reset: self.reset,
        };

        ScrollArea::vertical().show(ui, |ui| {
            macro_rules! plot {
                (cells, $lut:expr, $iter:expr, $plot_ui:ident) => {
                    $iter.into_iter().enumerate().for_each(|(cell, points)| {
                        $plot_ui.line(Line::new($lut[cell].0, points).color($lut[cell].1));
                    })
                };
                (daisies, $lut:expr, $iter:expr, $plot_ui:ident) => {
                    $iter.into_iter().enumerate().for_each(|(daisy, points)| {
                        plot!(cells, $lut[daisy], points, $plot_ui);
                    })
                };
                (plot, $plot_info:expr, $renderer:expr) => {
                    self.render_single_plot(
                        ui,
                        $plot_info,
                        $renderer,
                        height,
                        &mut new_state,
                        rightmost_point,
                    );
                };
            }
            plot!(
                plot,
                PlotInfo {
                    id: "volt_plot".into(),
                    title: "Voltages",
                    unit: "V ",
                    default_y_bounds: -3.5..=6.5,
                    legend_entries: &VOLT_LUT_SLICE,
                    expanded_group_hidden_entries: |this| (
                        &mut this.expanded_volt,
                        &mut this.hidden_volt
                    )
                },
                |plot_ui, plotter| plot!(daisies, VOLT_LUT, plotter.volt_points(), plot_ui)
            );
            plot!(
                plot,
                PlotInfo {
                    id: "temp_plot".into(),
                    title: "Temperatures",
                    unit: "ºC",
                    default_y_bounds: 10.0..=30.0,
                    legend_entries: &TEMP_LUT_SLICE,
                    expanded_group_hidden_entries: |this| (
                        &mut this.expanded_temp,
                        &mut this.hidden_temp
                    )
                },
                |plot_ui, plotter| {
                    let (daisies, last_daisy) = plotter.temp_points();
                    plot!(daisies, TEMP_LUT.0, daisies, plot_ui);
                    plot!(cells, TEMP_LUT.1, last_daisy, plot_ui);
                }
            );
        });

        if new_state.save(self) {
            ui.request_repaint();
        } else {
            self.manual_y_zoom.0 = 1.0;
        }
    }

    fn render_single_plot(
        &mut self,
        ui: &mut Ui,
        plot_info: PlotInfo,
        render: for<'a> fn(&mut PlotUi<'a>, &'a PlotterImpl),
        height: f32,
        new_state: &mut PlotState,
        rightmost_point: Option<f64>,
    ) {
        ui.with_layout(Layout::right_to_left(Default::default()), |ui| {
            let (expanded_legend_group, hidden_entries) =
                (plot_info.expanded_group_hidden_entries)(self);
            legend::render(ui, plot_info.clone(), expanded_legend_group, hidden_entries);

            self.configure_plot(ui, plot_info.clone(), height, new_state)
                .show(ui, |plot_ui| {
                    self.sync_plots(plot_ui, plot_info.clone(), rightmost_point, new_state);
                    if let Some(plotter) = &self.plotter {
                        render(plot_ui, plotter);
                    }
                });
        });

        if self.reset
            && let Some(mut mem) = PlotMemory::load(ui, plot_info.id)
        {
            assert!(mem.hidden_items.is_empty());
            let (_, hidden_entries) = (plot_info.expanded_group_hidden_entries)(self);
            hidden_entries.ones().for_each(|idx| {
                let cells_per_adbms = plot_info.legend_entries[0].len();
                mem.hidden_items.insert(Id::new(
                    plot_info.legend_entries[idx / cells_per_adbms][idx % cells_per_adbms].0,
                ));
            });
            mem.store(ui, plot_info.id);
        }
    }
}
