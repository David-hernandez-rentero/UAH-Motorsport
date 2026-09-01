use std::{
    io::{BufRead, BufReader, Error, ErrorKind, Read, StdinLock},
    sync::{Arc, atomic::Ordering},
};

use serialport::ClearBuffer;

use crate::serial::SerialPlotter;

pub enum ReadErr {
    StopThread,
    SilentClearScreen,
    Io(Error),
}
impl From<serialport::Error> for ReadErr {
    fn from(value: serialport::Error) -> Self {
        match value.kind {
            serialport::ErrorKind::InvalidInput | serialport::ErrorKind::Unknown => panic!(),
            serialport::ErrorKind::NoDevice => ReadErr::Io(ErrorKind::NotConnected.into()),
            serialport::ErrorKind::Io(io) => ReadErr::Io(io.into()),
        }
    }
}
pub trait SerialReader {
    const PACKETS_WITHOUT_PAUSES: bool = false; // TODO `Clear` might not work if false
    fn blocking_read(&mut self) -> Result<u8, ReadErr>;
    fn non_blocking_read(&mut self) -> Result<u8, ReadErr>;
    fn clear_input_buffer(&mut self) -> Result<(), ReadErr>;
}
pub struct SerialPort {
    inner: BufReader<Box<dyn serialport::SerialPort>>,
    plotter: Arc<SerialPlotter>,
}
impl SerialPort {
    pub fn new(plotter: Arc<SerialPlotter>, input: Box<dyn serialport::SerialPort>) -> Self {
        Self {
            inner: BufReader::with_capacity(512, input),
            plotter,
        }
    }
}
impl SerialReader for SerialPort {
    fn blocking_read(&mut self) -> Result<u8, ReadErr> {
        match self.plotter.queued_error.swap(0, Ordering::SeqCst).cmp(&0) {
            std::cmp::Ordering::Equal => (),
            std::cmp::Ordering::Less => return Err(ReadErr::SilentClearScreen),
            std::cmp::Ordering::Greater => return Err(ReadErr::StopThread),
        }
        let mut byte = 0;
        match self.inner.read_exact(std::slice::from_mut(&mut byte)) {
            Ok(()) => Ok(byte),
            Err(err) => Err(ReadErr::Io(err)),
        }
    }
    fn non_blocking_read(&mut self) -> Result<u8, ReadErr> {
        match self.plotter.queued_error.swap(0, Ordering::SeqCst).cmp(&0) {
            std::cmp::Ordering::Equal => (),
            std::cmp::Ordering::Less => return Err(ReadErr::SilentClearScreen),
            std::cmp::Ordering::Greater => return Err(ReadErr::StopThread),
        }
        if Self::PACKETS_WITHOUT_PAUSES
            && self.inner.buffer().len() == 0
            && self.inner.get_ref().bytes_to_read()? == 0
        {
            Err(ReadErr::Io(ErrorKind::WouldBlock.into()))
        } else {
            self.blocking_read()
        }
    }
    fn clear_input_buffer(&mut self) -> Result<(), ReadErr> {
        self.inner.consume(self.inner.buffer().len());
        self.inner
            .get_ref()
            .clear(ClearBuffer::Input)
            .map_err(Into::into)
    }
}

pub struct Stdin {
    inner: StdinLock<'static>,
    available: usize,
    plotter: Arc<SerialPlotter>,
}
impl Stdin {
    pub fn new(plotter: Arc<SerialPlotter>) -> Self {
        Self {
            inner: std::io::stdin().lock(),
            available: 0,
            plotter,
        }
    }
}
impl SerialReader for Stdin {
    fn blocking_read(&mut self) -> Result<u8, ReadErr> {
        match self.plotter.queued_error.swap(0, Ordering::SeqCst).cmp(&0) {
            std::cmp::Ordering::Equal => (),
            std::cmp::Ordering::Less => return Err(ReadErr::SilentClearScreen),
            std::cmp::Ordering::Greater => return Err(ReadErr::StopThread),
        }
        if self.available == 0 {
            self.available = self.inner.fill_buf().map_err(ReadErr::Io)?.len();
        }
        let mut byte = 0;
        match self.inner.read_exact(std::slice::from_mut(&mut byte)) {
            Ok(()) => {
                self.available -= 1;
                Ok(byte)
            }
            Err(err) => Err(ReadErr::Io(err)),
        }
    }
    fn non_blocking_read(&mut self) -> Result<u8, ReadErr> {
        match self.plotter.queued_error.swap(0, Ordering::SeqCst).cmp(&0) {
            std::cmp::Ordering::Equal => (),
            std::cmp::Ordering::Less => return Err(ReadErr::SilentClearScreen),
            std::cmp::Ordering::Greater => return Err(ReadErr::StopThread),
        }
        if Self::PACKETS_WITHOUT_PAUSES && self.available == 0 {
            Err(ReadErr::Io(ErrorKind::WouldBlock.into()))
        } else {
            self.blocking_read()
        }
    }
    fn clear_input_buffer(&mut self) -> Result<(), ReadErr> {
        Ok(self.inner.consume(self.available))
    }
}
