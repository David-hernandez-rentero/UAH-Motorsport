/*
 * SPI model for STM32G4 series, specifically RM0440 sections
 * 42.5.9."Data packing" and 42.9.3."FRLVL"
 * https://www.st.com/resource/en/reference_manual/rm0440-stm32g4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
 * This delegates everything to Renode's STM32SPI with `STM32Series.G0` while
 * adding support for "Data packing" aka "packing mode".
 */

using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.SPI
{
    public sealed class STM32SPIPacking :
        IPeripheralContainer<ISPIPeripheral, NullRegistrationPoint>, IPeripheral,
        IDoubleWordPeripheral, IWordPeripheral, IBytePeripheral, IKnownSize
    {
#nullable enable
        private IMachine? registrationMachine;

#nullable restore
        private readonly STM32SPI inner;
        private readonly int receptionBufferCapacity;

        private int receptionBufferLen;

        public long Size => inner.Size;
        public GPIO IRQ => inner.IRQ;
        public GPIO DMAReceive => inner.DMAReceive;
        public GPIO DMASend => inner.DMASend;
        public IEnumerable<NullRegistrationPoint> GetRegistrationPoints(ISPIPeripheral peripheral) => inner.GetRegistrationPoints(peripheral);
        public IEnumerable<IRegistered<ISPIPeripheral, NullRegistrationPoint>> Children => inner.Children;
        public void Register(ISPIPeripheral peripheral, NullRegistrationPoint registrationPoint)
        {
            if (registrationMachine is not null)
            {
                registrationMachine.RegisterAsAChildOf(this, inner, NullRegistrationPoint.Instance);
                registrationMachine = null;
            }
            inner.Register(peripheral, registrationPoint);
        }
        public void Unregister(ISPIPeripheral peripheral) => inner.Unregister(peripheral);


        public STM32SPIPacking(IMachine machine, int bufferCapacity = 4)
        {
            registrationMachine = machine;
            receptionBufferCapacity = bufferCapacity;
            inner = new STM32SPI(machine, STM32Series.L5, bufferCapacity);
            Reset();
        }

        private const long SPIx_SR = 0x08; // RM0440 42.9.3
        private const long SPIx_DR = 0x0C; // RM0440 42.9.4
        private bool nonByteMode = true;

        public uint ReadDoubleWord(long offset)
        {
            var res = inner.ReadDoubleWord(offset);
            if (offset == SPIx_DR)
            {
                --receptionBufferLen;
                if (nonByteMode)
                {
                    --receptionBufferLen;
                    res |= inner.ReadDoubleWord(offset) << 8;
                }
                if (receptionBufferLen < 0)
                    receptionBufferLen = 0;
            }
            else if (offset == SPIx_SR)
            {
                res &= ~(uint)0b11_000000000;
                if (2 * receptionBufferLen > receptionBufferCapacity)
                    res |= 0b11_000000000;
                else if (4 * receptionBufferLen > receptionBufferCapacity)
                    res |= 0b10_000000000;
                else if (receptionBufferLen > 0)
                    res |= 0b01_000000000;
            }
            return res;
        }

        public void WriteDoubleWord(long offset, uint value)
        {
            inner.WriteDoubleWord(offset, value);
            if (offset == SPIx_DR)
            {
                ++receptionBufferLen;
                if (nonByteMode)
                {
                    ++receptionBufferLen;
                    inner.WriteDoubleWord(offset, value >> 8);
                }
                if (receptionBufferLen > receptionBufferCapacity)
                    receptionBufferLen = receptionBufferCapacity;
            }
        }

        public void Reset()
        {
            receptionBufferLen = 0;
            inner.Reset();
        }
        public ushort ReadWord(long offset) => (ushort)ReadDoubleWord(offset);
        public void WriteWord(long offset, ushort value) => WriteDoubleWord(offset, value);
        public byte ReadByte(long offset)
        {
            if (offset % 4 == 0)
            {
                nonByteMode = false;
                var res = (byte)ReadDoubleWord(offset);
                nonByteMode = true;
                return res;
            }
            else
            {
                return inner.ReadByte(offset);
            }
        }
        public void WriteByte(long offset, byte value)
        {
            if (offset % 4 == 0)
            {
                nonByteMode = false;
                WriteDoubleWord(offset, value);
                nonByteMode = true;
            }
            else
            {
                inner.WriteByte(offset, value);
            }
        }
    }
}
