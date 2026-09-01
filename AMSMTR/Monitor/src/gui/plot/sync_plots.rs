use eframe::egui::Vec2;
use egui_plot::{PlotPoint, PlotUi};

use crate::{
    Bounds, Main,
    gui::plot::{PlotInfo, PlotState},
};

impl Main {
    pub(super) fn sync_plots(
        &self,
        ui: &mut PlotUi,
        plot_info: PlotInfo,
        rightmost_point: Option<f64>,
        new_state: &mut PlotState,
    ) {
        new_state.reset |= match (self.auto_bounds != Bounds::NoScroll, ui.auto_bounds().any()) {
            (false, false) | (true, true) => false,
            (false, true) => {
                new_state.auto_bounds = if self.x_width.0 {
                    Bounds::AutoScrollFixedWindow
                } else {
                    Bounds::AutoScrollInfiniteWindow
                };
                true
            }
            (true, false) => {
                new_state.auto_bounds = Bounds::NoScroll;
                true
            }
        };
        match new_state.auto_bounds {
            Bounds::AutoScrollFixedWindow => {
                if let Some(last) = rightmost_point {
                    ui.set_plot_bounds_x((last - self.x_width.1 * 1000.0).max(0.0)..=last);
                } else {
                    ui.set_plot_bounds_y(plot_info.default_y_bounds);
                }
            }
            Bounds::NoScroll => {
                if ui.response().clicked()
                    || ui.response().drag_stopped()
                    || ui.response().is_pointer_button_down_on()
                    || (ui.response().contains_pointer()
                        && (ui.ctx().input(|i| i.zoom_delta_2d()) != Vec2::ONE
                            || ui.ctx().input(|i| i.smooth_scroll_delta) != Vec2::ZERO))
                {
                    new_state.manual_x_bounds = ui.plot_bounds().range_x();
                    new_state.manual_y_zoom = (
                        ui.ctx().input(|i| i.zoom_delta_2d()).y,
                        ui.pointer_coordinate()
                            .map(|pos| {
                                (pos.y - ui.plot_bounds().min()[1]) / ui.plot_bounds().height()
                            })
                            .unwrap_or(0.5),
                    );
                } else {
                    ui.zoom_bounds(
                        [1.0, new_state.manual_y_zoom.0].into(),
                        PlotPoint {
                            x: 0.0,
                            y: new_state.manual_y_zoom.1 * ui.plot_bounds().height()
                                + ui.plot_bounds().min()[1],
                        },
                    );
                    ui.set_plot_bounds_x(new_state.manual_x_bounds.clone());
                }
            }
            _ => (),
        }
    }
}
