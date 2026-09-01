use std::{
    io::ErrorKind,
    ops::{Deref, DerefMut},
    sync::{
        Arc,
        atomic::{AtomicI8, AtomicU32, Ordering},
    },
};

use eframe::egui::Context;
use egui_plot::{PlotPoint, PlotPoints};
use ordered_float::NotNan;
use parking_lot::Mutex;

use crate::{
    DAISY_CHAINNED_ADBMS, Plotter, Port, TEMP_CELLS_LAST_ADBMS, TEMP_CELLS_PER_ADBMS,
    VOLT_CELLS_PER_ADBMS,
    serial::{ReadErr, SerialReader},
};

pub struct SerialPlotter {
    ctx: Context,
    port: Port,
    volt_last: AtomicU32,
    temp_last: AtomicU32,
    volt_points: Mutex<[[Vec<PlotPoint>; VOLT_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS]>,
    temp_points: Mutex<(
        [[Vec<PlotPoint>; TEMP_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS - 1],
        [Vec<PlotPoint>; TEMP_CELLS_LAST_ADBMS],
    )>,
    logs: Arc<Mutex<Vec<String>>>,

    /// `queued_error > 0` => [`ReadErr::StopThread`]
    ///
    /// `queued_error < 0` => [`ReadErr::SilentClearScreen`]
    pub(super) queued_error: AtomicI8,
}

impl Plotter for Arc<SerialPlotter> {
    fn port(&self) -> Port {
        self.port.clone()
    }
    fn prepare_drop(&self) {
        self.queued_error.store(1, Ordering::SeqCst);
    }
    fn clear(&mut self) {
        let mut volt = self.volt_points.lock();
        let mut temp = self.temp_points.lock();
        self.queued_error.store(-1, Ordering::SeqCst);
        volt.iter_mut()
            .for_each(|points| points.iter_mut().for_each(Vec::clear));
        temp.0
            .iter_mut()
            .for_each(|points| points.iter_mut().for_each(Vec::clear));
        temp.1.iter_mut().for_each(Vec::clear);
    }
    fn rightmost_point(&self) -> f64 {
        Ord::max(
            self.volt_last.load(Ordering::SeqCst),
            self.temp_last.load(Ordering::SeqCst),
        ) as _
    }

    fn volt_points(&self) -> [[PlotPoints<'_>; VOLT_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS] {
        self.volt_points
            .lock()
            .clone()
            .map(|points| points.map(PlotPoints::Owned))
    }
    fn temp_points(
        &self,
    ) -> (
        [[PlotPoints<'_>; TEMP_CELLS_PER_ADBMS]; DAISY_CHAINNED_ADBMS - 1],
        [PlotPoints<'_>; TEMP_CELLS_LAST_ADBMS],
    ) {
        let clone = { self.temp_points.lock().clone() };
        (
            clone.0.map(|points| points.map(PlotPoints::Owned)),
            clone.1.map(PlotPoints::Owned),
        )
    }
}

impl SerialPlotter {
    pub fn new<Input: SerialReader>(
        ctx: Context,
        logs: Arc<Mutex<Vec<String>>>,
        port: Port,
        input: impl 'static + Send + FnOnce(Arc<Self>) -> Input,
    ) -> Arc<Self> {
        let res = Arc::new(Self {
            ctx,
            port,
            volt_last: Default::default(),
            temp_last: Default::default(),
            volt_points: Default::default(),
            temp_points: Default::default(),
            logs,
            queued_error: Default::default(),
        });
        let res_clone = res.clone();
        std::thread::spawn(move || res_clone.parse_thread(input(res_clone.clone())));
        res
    }

    fn non_blocking_parse(
        &self,
        input: &mut impl SerialReader,
        first_byte: u8,
    ) -> Result<(), ReadErr> {
        if first_byte & 0x80 != 0 {
            let mut len = (first_byte & 0x3f) as _;
            if (first_byte & 0x40) != 0 {
                let mut shift = 6;
                loop {
                    let next_byte = input.non_blocking_read()?;
                    len |= ((next_byte & 0x7f) as usize) << shift;
                    if (next_byte & 0x80) == 0 {
                        break;
                    }
                    shift += 7;
                }
            }

            if len > u16::MAX as _ {
                return Err(ReadErr::Io(ErrorKind::OutOfMemory.into()));
            }
            let mut str = Vec::new();
            str.reserve_exact(len);
            (0..len)
                .rev()
                .try_for_each(|_| input.non_blocking_read().map(|byte| str.push(byte)))?;
            str.pop_if(|&mut last| b'\n' == last);
            let str =
                String::from_utf8(str).map_err(|_| ReadErr::Io(ErrorKind::InvalidData.into()))?;
            self.logs.lock().push(str);
            self.ctx.request_repaint();
            return Ok(());
        }

        fn read_cells<'a, const N: usize>(
            points: &mut [SortedVecGuard<'a>; N],
            timestamp: f64,
            input: &mut impl SerialReader,
        ) -> Result<(), ReadErr> {
            points.iter_mut().try_for_each(|points| {
                let raw =
                    i16::from_le_bytes([input.non_blocking_read()?, input.non_blocking_read()?]);
                Ok(points.push(PlotPoint {
                    x: timestamp,
                    y: raw as f64 * 150.0e-6 + 1.5,
                }))
            })
        }
        fn read_daisies<'a, const N: usize, const M: usize>(
            points: &mut [[SortedVecGuard<'a>; N]; M],
            timestamp: f64,
            input: &mut impl SerialReader,
        ) -> Result<(), ReadErr> {
            points
                .iter_mut()
                .try_for_each(|points| read_cells(points, timestamp, input))
        }

        let timestamp = u32::from_le_bytes([
            first_byte << 2,
            input.non_blocking_read()?,
            input.non_blocking_read()?,
            input.non_blocking_read()?,
        ]) >> 2;
        let timestamp_f64 = timestamp as _;
        let atomic_timestamp = if first_byte & 0x40 == 0 {
            let guard = &mut *self.volt_points.lock();
            let mut guard = guard
                .each_mut()
                .map(|points| points.each_mut().map(SortedVecGuard::from));
            read_daisies(&mut guard, timestamp_f64, input)?;
            guard
                .into_iter()
                .for_each(|points| points.into_iter().for_each(SortedVecGuard::commit));
            &self.volt_last
        } else {
            let guard = &mut *self.temp_points.lock();
            let mut guard = (
                guard
                    .0
                    .each_mut()
                    .map(|points| points.each_mut().map(SortedVecGuard::from)),
                guard.1.each_mut().map(SortedVecGuard::from),
            );
            read_daisies(&mut guard.0, timestamp_f64, input)?;
            read_cells(&mut guard.1, timestamp_f64, input)?;
            guard
                .0
                .into_iter()
                .for_each(|points| points.into_iter().for_each(SortedVecGuard::commit));
            guard.1.into_iter().for_each(SortedVecGuard::commit);
            &self.temp_last
        };
        loop {
            let old = atomic_timestamp.load(Ordering::SeqCst);
            if timestamp <= old
                || atomic_timestamp
                    .compare_exchange_weak(old, timestamp, Ordering::SeqCst, Ordering::SeqCst)
                    .is_ok()
            {
                break;
            }
        }
        self.ctx.request_repaint();
        Ok(())
    }

    fn parse_thread(&self, mut input: impl SerialReader) {
        'thread_loop: loop {
            let mut result = match input.blocking_read() {
                Ok(first_byte) => self.non_blocking_parse(&mut input, first_byte),
                Err(err) => Err(err),
            };
            loop {
                match result {
                    Ok(()) => continue 'thread_loop,
                    Err(ReadErr::SilentClearScreen) => (),
                    Err(ReadErr::StopThread) => break 'thread_loop,
                    Err(ReadErr::Io(err)) => {
                        self.logs
                            .lock()
                            .push(format!("[ERROR] {err}. Ignoring bytes in buffer..."));
                        self.ctx.request_repaint();
                    }
                }
                result = input.clear_input_buffer();
            }
        }
    }
}

struct SortedVecGuard<'a> {
    inner: &'a mut Vec<PlotPoint>,
    prev_size: usize,
    committed: bool,
}
impl<'a> From<&'a mut Vec<PlotPoint>> for SortedVecGuard<'a> {
    fn from(inner: &'a mut Vec<PlotPoint>) -> Self {
        Self {
            prev_size: inner.len(),
            inner,
            committed: false,
        }
    }
}
impl<'a> SortedVecGuard<'a> {
    pub fn commit(mut self) {
        self.committed = true;
    }
}
impl<'a> Deref for SortedVecGuard<'a> {
    type Target = Vec<PlotPoint>;

    fn deref(&self) -> &Vec<PlotPoint> {
        self.inner
    }
}
impl<'a> DerefMut for SortedVecGuard<'a> {
    fn deref_mut(&mut self) -> &mut Vec<PlotPoint> {
        self.inner
    }
}
impl<'a> Drop for SortedVecGuard<'a> {
    fn drop(&mut self) {
        if self.committed {
            if (self.inner.len() != self.prev_size + 1
                && self.inner.get(self.prev_size..).is_none_or(|slice| {
                    slice.is_empty()
                        || !slice
                            .is_sorted_by_key(|point| unsafe { NotNan::new_unchecked(point.x) })
                }))
                || (self.inner.len() > self.prev_size
                    && self.inner.len() > 1
                    && self.inner[self.prev_size - 1].x > self.inner[self.prev_size].x)
            {
                // TODO WARN that the output from BMS was not monotonic
                self.inner
                    .sort_unstable_by_key(|point| unsafe { NotNan::new_unchecked(point.x) });
            }
        } else {
            self.inner.truncate(self.prev_size);
        }
    }
}
