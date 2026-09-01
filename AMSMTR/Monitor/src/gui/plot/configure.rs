use std::ops::RangeInclusive;

use eframe::egui::Ui;
use egui_plot::{HoverPosition, Plot, PlotMemory};

use crate::{Bounds, Main, gui::plot::PlotInfo};

#[derive(PartialEq)]
pub(super) struct PlotState {
    pub manual_x_bounds: RangeInclusive<f64>,
    pub manual_y_zoom: (f32, f64),
    pub auto_bounds: Bounds,
    pub reset: bool,
}
impl PlotState {
    pub(super) fn save(self, save_to: &mut Main) -> bool {
        let different = self
            != (PlotState {
                manual_x_bounds: save_to.manual_x_bounds.clone(),
                manual_y_zoom: save_to.manual_y_zoom,
                auto_bounds: save_to.auto_bounds,
                reset: save_to.reset,
            });
        if different {
            save_to.reset = self.reset;
            save_to.auto_bounds = self.auto_bounds;
            save_to.manual_x_bounds = self.manual_x_bounds;
            save_to.manual_y_zoom = self.manual_y_zoom;
        }
        different
    }
}

impl Main {
    pub(super) fn configure_plot<'a>(
        &self,
        ui: &Ui,
        plot_info: PlotInfo,
        height: f32,
        new_state: &mut PlotState,
    ) -> Plot<'a> {
        let mut plot = Plot::new(plot_info.id)
            .id(plot_info.id)
            .height(height)
            .allow_axis_zoom_drag(false)
            .allow_boxed_zoom(false)
            .x_axis_label("Time")
            .x_axis_formatter(|position, _| {
                format!(
                    "{:02}:{:06.3}",
                    position.value as usize / (60 * 1000),
                    position.value / 1000.0 % (60.0),
                )
            })
            .y_axis_label(plot_info.title)
            .y_axis_formatter(move |position, _| {
                format!("{:6.3}{}", position.value, plot_info.unit)
            });
        if self.reset {
            new_state.reset = false;
            let auto_bounds = self.auto_bounds != Bounds::NoScroll;
            plot = plot.reset().auto_bounds(auto_bounds);
            if !auto_bounds {
                plot = if let Some(mem) = PlotMemory::load(ui, plot_info.id) {
                    new_state.manual_x_bounds = mem.bounds().range_x();
                    plot.default_x_bounds(mem.bounds().min()[0], mem.bounds().max()[0])
                        .default_y_bounds(mem.bounds().min()[1], mem.bounds().max()[1])
                } else {
                    new_state.manual_x_bounds = 0.0..=1000.0;
                    plot.default_x_bounds(0.0, 1000.0).default_y_bounds(
                        *plot_info.default_y_bounds.start(),
                        *plot_info.default_y_bounds.end(),
                    )
                };
            }
        }
        if self.show_hover {
            plot = plot.label_formatter(move |pos| {
                Some(match pos {
                    HoverPosition::NearDataPoint {
                        plot_name,
                        position,
                        index: _,
                    } => format!(
                        "{plot_name}\n{:02}:{:06.3} {:6.3}{}",
                        position.x as usize / (60 * 1000),
                        position.x / 1000.0 % (60.0),
                        position.y,
                        plot_info.unit,
                    ),
                    HoverPosition::Elsewhere { position } => format!(
                        "{:02}:{:06.3}",
                        position.x as usize / (60 * 1000),
                        position.x / 1000.0 % (60.0),
                    ),
                })
            });
        }
        plot
    }
}
