use eframe::egui::{
    Button, Checkbox, ComboBox, DragValue, Layout, PopupCloseBehavior, ScrollArea, TextEdit,
    TextStyle, TextWrapMode, Ui,
};

use crate::{Bounds, Plotter, Port, gui::Main};

impl Main {
    pub(super) fn render_logs(&self, ui: &mut Ui) {
        let logs = self.logs.lock();
        ScrollArea::both()
            .stick_to_bottom(true)
            .auto_shrink(false)
            .show_rows(
                ui,
                ui.text_style_height(&TextStyle::Monospace).round(),
                logs.len(),
                |ui, rows| {
                    ui.style_mut().wrap_mode = Some(TextWrapMode::Extend);
                    logs.get(rows).unwrap().iter().for_each(|log| {
                        ui.monospace(log);
                    });
                },
            );
    }

    pub(super) fn render_controls(&mut self, ui: &mut Ui) {
        ui.horizontal(|ui| {
            self.render_bounds(ui);
            self.render_extras(ui);
            ui.vertical(|ui| {
                self.render_baud_picker(ui);
                self.render_port_picker(ui);
            });
        });
    }
    fn render_bounds(&mut self, ui: &mut Ui) {
        ui.group(|ui| {
            ui.vertical(|ui| {
                let mut auto_bounds = self.auto_bounds != Bounds::NoScroll;
                if ui.checkbox(&mut auto_bounds, "Auto bounds").changed() {
                    self.reset = true;
                    self.auto_bounds = match (auto_bounds, self.x_width.0) {
                        (false, _) => Bounds::NoScroll,
                        (true, false) => Bounds::AutoScrollInfiniteWindow,
                        (true, true) => Bounds::AutoScrollFixedWindow,
                    };
                }
                ui.horizontal(|ui| {
                    if ui
                        .add_enabled(
                            auto_bounds,
                            Checkbox::new(&mut self.x_width.0, "Scrolling bounds"),
                        )
                        .changed()
                    {
                        self.reset = true;
                        self.auto_bounds = if self.x_width.0 {
                            Bounds::AutoScrollFixedWindow
                        } else {
                            Bounds::AutoScrollInfiniteWindow
                        };
                    }
                    ui.add_enabled(
                        self.auto_bounds == Bounds::AutoScrollFixedWindow,
                        DragValue::new(&mut self.x_width.1)
                            .range(0.001..=f64::MAX)
                            .speed(0.01)
                            .fixed_decimals(3),
                    );
                });
            });
        });
    }

    fn render_extras(&mut self, ui: &mut Ui) {
        ui.with_layout(Layout::bottom_up(Default::default()), |ui| {
            let width = ui
                .horizontal(|ui| ui.checkbox(&mut self.show_hover, "Show hover info"))
                .response
                .rect
                .width();
            if ui
                .add_enabled(
                    self.plotter.is_some(),
                    Button::new("Clear").min_size([width, ui.available_height()].into()),
                )
                .clicked()
            {
                self.clear();
            }
        });
    }

    fn render_baud_picker(&mut self, ui: &mut Ui) {
        ui.add_enabled_ui(
            matches!(
                self.plotter.as_ref().map(Plotter::port),
                Some(Port::Serial(_))
            ),
            |ui| {
                let prev_baud = self.baud_rate;
                ComboBox::from_label("Baud rate")
                    .close_behavior(PopupCloseBehavior::CloseOnClickOutside)
                    .selected_text(self.baud_rate.to_string())
                    .show_ui(ui, |ui| {
                        for rate in [
                            9_600, 19_200, 38_400, 57_600, 115_200, 230_400, 460_800, 921_600,
                        ] {
                            if ui
                                .selectable_value(&mut self.baud_rate, rate, rate.to_string())
                                .clicked()
                            {
                                ui.close();
                            }
                        }

                        ui.separator();

                        if ui
                            .add(
                                TextEdit::singleline(&mut self.custom_baud_rate)
                                    .hint_text("Custom"),
                            )
                            .lost_focus()
                        {
                            if let Ok(baud_rate) = self.custom_baud_rate.trim().parse::<u32>()
                                && baud_rate != 0
                            {
                                self.baud_rate = baud_rate;
                            }
                            ui.close();
                        }
                    });
                if prev_baud != self.baud_rate {
                    self.custom_baud_rate.clear();
                    self.set_plotter_from_port(
                        ui.clone(),
                        self.plotter.as_ref().map(Plotter::port),
                    );
                }
            },
        );
    }

    fn render_port_picker(&mut self, ui: &mut Ui) {
        ComboBox::from_label("Serial port")
            .selected_text(
                self.plotter
                    .as_ref()
                    .map(|plotter| match plotter.port() {
                        Port::Simulation => "Simulation".to_owned(),
                        Port::Stdin => "STDIN".to_owned(),
                        Port::Serial(port) => port.port_name.clone(),
                    })
                    .unwrap_or_default(),
            )
            .show_ui(ui, |ui| {
                let prev_selected = self.plotter.as_ref().map(Plotter::port);
                let mut selected = prev_selected.clone();
                if self.available_ports.0.is_some() {
                    if let Some(thr) = self.available_ports.0.take_if(|thr| thr.is_finished())
                        && let Ok(ports) = thr.join()
                    {
                        self.available_ports.1 = ports;
                    }
                } else {
                    self.available_ports.0 = Some(std::thread::spawn(|| {
                        serialport::available_ports().unwrap_or_default()
                    }));
                }
                ui.selectable_value(&mut selected, None, "");
                ui.selectable_value(&mut selected, Some(Port::Simulation), "Simulation");
                ui.selectable_value(&mut selected, Some(Port::Stdin), "STDIN");
                self.available_ports.1.iter().for_each(|port| {
                    ui.selectable_value(
                        &mut selected,
                        Some(Port::Serial(port.clone())),
                        &port.port_name,
                    );
                });
                if selected != prev_selected {
                    self.set_plotter_from_port(ui.clone(), selected);
                }
            });
    }
}
