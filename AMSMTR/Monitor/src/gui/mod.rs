use std::{ops::RangeInclusive, sync::Arc, thread::JoinHandle};

use eframe::{
    CreationContext,
    egui::{CentralPanel, Context, Id, Panel, TextStyle, Ui},
};
use fixedbitset::FixedBitSet;
use parking_lot::Mutex;
use serialport::SerialPortInfo;

use crate::{
    Bounds, DAISY_CHAINNED_ADBMS, Plotter, PlotterImpl, Port, TEMP_CELLS_LAST_ADBMS,
    TEMP_CELLS_PER_ADBMS, VOLT_CELLS_PER_ADBMS,
    serial::{SerialPlotter, SerialPort, Stdin},
    simulation::Simulation,
};

mod controls;
mod plot;

pub(super) struct Main {
    available_ports: (Option<JoinHandle<Vec<SerialPortInfo>>>, Vec<SerialPortInfo>),
    custom_baud_rate: String,
    baud_rate: u32,
    plotter: Option<PlotterImpl>,
    logs: Arc<Mutex<Vec<String>>>,

    expanded_volt: Option<Id>,
    expanded_temp: Option<Id>,
    hidden_volt: FixedBitSet,
    hidden_temp: FixedBitSet,
    manual_x_bounds: RangeInclusive<f64>,
    manual_y_zoom: (f32, f64), // (y_zoom_factor, y_center_percent)
    auto_bounds: Bounds,
    x_width: (bool, f64),
    show_hover: bool,

    reset: bool,
}

impl eframe::App for Main {
    fn ui(&mut self, ui: &mut Ui, _frame: &mut eframe::Frame) {
        CentralPanel::default().show(ui, |ui| {
            Panel::left("controls+logs").show(ui, |ui| {
                ui.vertical(|ui| {
                    self.render_controls(ui);
                    ui.separator();
                    self.render_logs(ui);
                });
            });
            self.show_plots(ui);
        });
    }

    fn save(&mut self, storage: &mut dyn eframe::Storage) {
        eframe::set_value(storage, "baud_rate", &self.baud_rate);
        eframe::set_value(storage, "port", &self.plotter.as_ref().map(Plotter::port));
        eframe::set_value(storage, "auto_bounds", &self.auto_bounds);
        eframe::set_value(storage, "x_width", &self.x_width);
        eframe::set_value(storage, "show_hover", &self.show_hover);
    }
}

impl Main {
    pub fn new(cc: &CreationContext) -> Self {
        let ctx = &cc.egui_ctx;
        ctx.global_style_mut(|style| {
            style.text_styles.insert(
                TextStyle::Body,
                style.text_styles[&TextStyle::Monospace].clone(),
            );
        });
        let storage = cc.storage.unwrap();
        use eframe::get_value;
        let mut res = Self {
            available_ports: Default::default(),
            custom_baud_rate: String::new(),
            baud_rate: get_value(storage, "baud_rate").unwrap_or(115_200),
            plotter: None,
            logs: Arc::new(Mutex::new(Vec::with_capacity(1024 * 4))),

            expanded_volt: Default::default(),
            expanded_temp: Default::default(),
            hidden_volt: FixedBitSet::with_capacity(DAISY_CHAINNED_ADBMS * VOLT_CELLS_PER_ADBMS),
            hidden_temp: FixedBitSet::with_capacity(
                (DAISY_CHAINNED_ADBMS - 1) * TEMP_CELLS_PER_ADBMS + TEMP_CELLS_LAST_ADBMS,
            ),
            manual_y_zoom: (1.0, 0.0),
            manual_x_bounds: 0.0..=0.0,
            auto_bounds: get_value(storage, "auto_bounds").unwrap_or(Bounds::AutoScrollFixedWindow),
            x_width: get_value(storage, "x_width").unwrap_or((true, 5.0)),
            show_hover: get_value(storage, "show_hover").unwrap_or(true),

            reset: true,
        };
        res.set_plotter_from_port(ctx.clone(), get_value(storage, "port").unwrap_or_default());
        res
    }

    fn set_plotter_from_port(&mut self, ctx: Context, selected: Option<Port>) {
        let plotter = match selected {
            None => Ok(None),
            Some(Port::Simulation) => {
                Ok(Some(Simulation::new(ctx.clone(), self.logs.clone()).into()))
            }
            Some(Port::Stdin) => Ok(Some(
                SerialPlotter::new(ctx.clone(), self.logs.clone(), Port::Stdin, Stdin::new).into(),
            )),
            Some(Port::Serial(port)) => serialport::new(&port.port_name, self.baud_rate)
                .open()
                .map(|input| {
                    Some(
                        SerialPlotter::new(
                            ctx.clone(),
                            self.logs.clone(),
                            Port::Serial(port),
                            |queued_error| SerialPort::new(queued_error, input),
                        )
                        .into(),
                    )
                }),
        };
        self.clear();
        if let Some(plotter) = &self.plotter {
            plotter.prepare_drop();
        }
        self.plotter = match plotter {
            Ok(plotter) => plotter,
            Err(err) => {
                ctx.request_repaint();
                self.logs.lock().push(format!("[ERROR] {err}"));
                None
            }
        }
    }

    fn clear(&mut self) {
        let mut logs = self.logs.lock();
        if let Some(plotter) = &mut self.plotter {
            plotter.clear();
        }
        logs.clear();
    }
}
