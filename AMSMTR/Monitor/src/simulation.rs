use std::{f64::consts::TAU, sync::Arc, time::Instant};

use eframe::egui::Context;
use egui_plot::PlotPoints;
use parking_lot::Mutex;

use crate::{
    DAISY_CHAINNED_ADBMS, Plotter, Port, TEMP_CELLS_LAST_ADBMS, TEMP_CELLS_PER_ADBMS,
    VOLT_CELLS_PER_ADBMS,
};

pub struct Simulation {
    ctx: Context,
    logs: Arc<Mutex<Vec<String>>>,
    start: Instant,
}

impl Simulation {
    pub fn new(ctx: Context, logs: Arc<Mutex<Vec<String>>>) -> Self {
        Self {
            ctx,
            logs,
            start: Instant::now(),
        }
    }
}

fn for_daisy<const CELLS_PER_ADBMS: usize, const N: usize>(
    daisy: usize,
    max_time: f64,
    amplitude: f64,
    offset: f64,
) -> [PlotPoints<'static>; N] {
    std::array::from_fn(|cell| {
        PlotPoints::from_explicit_callback(
            move |time| {
                (TAU * (time / 1000.0
                    + (daisy * CELLS_PER_ADBMS + cell) as f64
                        / (CELLS_PER_ADBMS * DAISY_CHAINNED_ADBMS) as f64))
                    .sin()
                    * amplitude
                    + offset
            },
            0.0..max_time,
            250,
        )
    })
}

impl Plotter for Simulation {
    fn port(&self) -> Port {
        Port::Simulation
    }
    fn prepare_drop(&self) {}
    fn clear(&mut self) {
        self.start = Instant::now();
    }
    fn rightmost_point(&self) -> f64 {
        Instant::now().duration_since(self.start).as_millis() as f64
    }

    fn volt_points(&self) -> [[PlotPoints<'_>; VOLT_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS] {
        self.ctx.request_repaint();
        let max_time = self.rightmost_point();
        self.logs.lock().push(format!(
            "New voltage values @ {:02}:{:06.3}",
            max_time as usize / (60 * 1000),
            max_time / 1000.0 % (60.0)
        ));
        std::array::from_fn(|daisy| for_daisy::<VOLT_CELLS_PER_ADBMS, _>(daisy, max_time, 3.0, 2.5))
    }
    fn temp_points(
        &self,
    ) -> (
        [[PlotPoints<'_>; TEMP_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS - 1],
        [PlotPoints<'_>; TEMP_CELLS_LAST_ADBMS],
    ) {
        self.ctx.request_repaint();
        let max_time = self.rightmost_point();
        self.logs.lock().push(format!(
            "New temperature values @ {:02}:{:06.3}",
            max_time as usize / (60 * 1000),
            max_time / 1000.0 % (60.0)
        ));
        (
            std::array::from_fn(|daisy| {
                for_daisy::<TEMP_CELLS_PER_ADBMS, _>(daisy, max_time, 10.0, 20.0)
            }),
            for_daisy::<TEMP_CELLS_PER_ADBMS, _>(DAISY_CHAINNED_ADBMS - 1, max_time, 10.0, 20.0),
        )
    }
}
