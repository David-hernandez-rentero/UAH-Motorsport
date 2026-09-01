#![cfg_attr(windows, windows_subsystem = "windows")]

use std::sync::Arc;

use eframe::{NativeOptions, egui::ViewportBuilder};
use egui_plot::PlotPoints;
use enum_dispatch::enum_dispatch;
use serde::{Deserialize, Serialize};
use serialport::SerialPortInfo;

use crate::{gui::Main, serial::SerialPlotter, simulation::Simulation};

mod gui;
mod serial;
mod simulation;

const DAISY_CHAINNED_ADBMS: usize = 7;
const VOLT_CELLS_PER_ADBMS: usize = 16;
const TEMP_CELLS_PER_ADBMS: usize = 24;
const TEMP_CELLS_LAST_ADBMS: usize = 8;
#[enum_dispatch]
pub trait Plotter {
    fn port(&self) -> Port;
    fn prepare_drop(&self);
    fn clear(&mut self);
    fn rightmost_point(&self) -> f64;
    fn volt_points(&self) -> [[PlotPoints<'_>; VOLT_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS];
    fn temp_points(
        &self,
    ) -> (
        [[PlotPoints<'_>; TEMP_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS - 1],
        [PlotPoints<'_>; TEMP_CELLS_LAST_ADBMS],
    );
}

#[enum_dispatch(Plotter)]
pub enum PlotterImpl {
    Simulation,
    SerialPlotter(Arc<SerialPlotter>),
}

#[derive(Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
enum Bounds {
    NoScroll,
    AutoScrollInfiniteWindow,
    AutoScrollFixedWindow,
}

#[derive(Clone, PartialEq, Eq, Serialize, Deserialize)]
pub enum Port {
    Simulation,
    Stdin,
    Serial(SerialPortInfo),
}

fn main() -> eframe::Result {
    eframe::run_native(
        "BMS Monitor",
        NativeOptions {
            viewport: ViewportBuilder::default().with_inner_size([16.0 * 100.0, 9.0 * 100.0]),
            persistence_path: Some("BMS-Monitor.data".into()),
            ..NativeOptions::default()
        },
        Box::new(|cc| Ok(Box::new(Main::new(cc)))),
    )
}
