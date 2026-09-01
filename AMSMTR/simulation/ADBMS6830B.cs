/*
 * https://www.analog.com/media/en/technical-documentation/data-sheets/adbms6830b.pdf
 */

using Antmicro.Renode.Exceptions;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.SPI;
using Antmicro.Renode.Utilities;
using System;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class ADBMS6830B : ISPIPeripheral
    {
        [InlineArray(8)] private struct U8x8 { private byte _element0; }
        [InlineArray(6)] private struct U8x6 { private byte _element0; }

        private enum SpiState { CmdIn, DataIn, DataOut }

#nullable enable
        private record Cmd(string Bits, string Name, Action<U8x6>? DataIn, Func<U8x6>? DataOut, Action? CmdIn);
#nullable restore
        private Cmd CmdGet(ushort x) =>
            commands.AsEnumerable().FirstOrDefault(cmd =>
            cmd.Bits
            .Select((c, i) => c == 'X' || c - '0' == ((x >> (cmd.Bits.Length - i - 1)) & 1))
            .All(x => x));


        private readonly Cmd[] commands;
        private readonly uint daisies;
        private uint daisyIdx;
        private SpiState spiState;
        private U8x8 pecBuffer;
        private U8x6[] buffers;
        private byte bufferIdx;
        private byte commandCounter;
#nullable enable
        private Cmd? cmd;
#nullable restore

        public void FinishTransmission()
        {
        }

        public byte Transmit(byte data)
        {
            this.DebugLog($"ADBMS6830B RX 0x{data:X2} SpiState={spiState} DaisyIdx={daisyIdx} BufferIdx={bufferIdx}");

            switch (spiState)
            {
                case SpiState.CmdIn:
                    {
                        pecBuffer[bufferIdx++] = data;
                        if (bufferIdx < 4) return 0;
                        bufferIdx = 0;
                        var pec15 = pec_15_crc(pecBuffer, byte_len: 4);
                        if (pec15 != 0)
                        {
                            this.WarningLog($"ADBMS6830B Wrong PEC 15 CRC {pec15:X2}");
                            // TODO SPIFLT
                            return 0;
                        }
                        ushort cmdId = (ushort)((pecBuffer[0] << 8) | pecBuffer[1]);
                        cmd = CmdGet(cmdId);
                        if (cmd is null)
                        {
                            this.WarningLog($"ADBMS6830B Unknown CMD {cmdId:b11}");
                            // TODO SPIFLT
                            return 0;
                        }

                        this.InfoLog($"ADBMS6830B Received CMD {cmd.Name}");
                        if (cmd.DataIn is not null)
                        {
                            daisyIdx = daisies - 1;
                            spiState = SpiState.DataIn;
                        }
                        else if (cmd.DataOut is not null)
                        {
                            daisyIdx = 0;
                            spiState = SpiState.DataOut;
                        }
                        else if (++commandCounter >= 64)
                        {
                            commandCounter = 1;
                        }
                        if (cmd.CmdIn is not null)
                        {
                            cmd.CmdIn();
                        }
                        else if (cmd.DataIn is null && cmd.DataOut is null)
                        {
                            this.WarningLog($"ADBMS6830B Unimplemented command CMD {cmd.Name}");
                            // TODO SPIFLT
                            return 0;
                        }
                        return 0;
                    }
                case SpiState.DataIn:
                    {
                        pecBuffer[bufferIdx++] = data;
                        if (bufferIdx < 8) return 0;
                        bufferIdx = 0;
                        this.InfoLog($"ADBMS6830B RX Data:PEC {cmd.Name}:{daisyIdx}/{daisies} {pecBuffer[0]:X2}{pecBuffer[1]:X2}{pecBuffer[2]:X2}{pecBuffer[3]:X2}{pecBuffer[4]:X2}{pecBuffer[5]:X2}:{pecBuffer[6]:X2}{pecBuffer[7]:X2}");
                        var pec10 = pec_10_crc(pecBuffer, bit_len: 8 * 8);
                        if (pec10 != 0)
                        {
                            this.WarningLog($"ADBMS6830B Wrong PEC 10 CRC {pec10:X3}");
                            // TODO SPIFLT
                            --daisyIdx; // TODO each ADBMS crashes but they all still receive their crashed packet, right?
                            return 0;
                        }
                        pecBuffer[..6].CopyTo(buffers[daisyIdx]);
                        cmd.DataIn(buffers[daisyIdx]);
                        if (daisyIdx-- > 0) return 0;
                        System.Diagnostics.Debug.Assert(cmd.DataOut is null);
                        spiState = SpiState.CmdIn;
                        if (++commandCounter >= 64) commandCounter = 1;
                        return 0;
                    }
                case SpiState.DataOut:
                    {
                        if (bufferIdx == 0)
                        {
                            if (cmd.Name == "PLAUX")
                            {
                                this.InfoLog($"ADBMS6830B Recover from PLAUX");
                                spiState = SpiState.CmdIn;
                                return 1;
                            }
                            cmd.DataOut().AsRawBytes().CopyTo(pecBuffer);
                            pecBuffer[6] = (byte)(commandCounter << 2);
                            pecBuffer[7] = 0;
                            var pec10 = pec_10_crc(pecBuffer, bit_len: 6 * 8 + 6);
                            pecBuffer[6] |= (byte)(pec10 >> 8);
                            pecBuffer[7] = (byte)(pec10 & 0xFF);
                            System.Diagnostics.Debug.Assert(pec_10_crc(pecBuffer, bit_len: 8 * 8) == 0);
                        }
                        var res = pecBuffer[bufferIdx++];
                        if (bufferIdx >= 8)
                        {
                            bufferIdx = 0;
                            if (++daisyIdx == daisies) spiState = SpiState.CmdIn;
                            this.InfoLog($"ADBMS6830B TX Data:PEC {cmd.Name}:{daisyIdx}/{daisies} {pecBuffer[0]:X2}{pecBuffer[1]:X2}{pecBuffer[2]:X2}{pecBuffer[3]:X2}{pecBuffer[4]:X2}{pecBuffer[5]:X2}:{pecBuffer[6]:X2}{pecBuffer[7]:X2}");
                        }
                        return res;
                    }
                default:
                    throw new InvalidEnumArgumentException();
            }
        }

        // uvx --from git+https://github.com/tpircher/pycrc pycrc --generate=c
        // --algorithm=bit-by-bit-fast --width=15 --poly=0xC599 --xor-in=0x10
        // --xor-out=0 --reflect-in=false --reflect-out=false
        private static ushort pec_15_crc(U8x8 data, byte byte_len)
        {
            ushort crc = 0x10;
            byte dataIdx = 0;

            while (byte_len-- != 0)
            {
                var c = data[dataIdx++];
                for (var i = 0x80; i > 0; i >>= 1)
                {
                    var bit = (crc & 0x4000) ^ ((c & i) != 0 ? 0x4000 : 0);
                    crc <<= 1;
                    if (bit != 0)
                    {
                        crc ^= 0x4599;
                    }
                }
                crc &= 0x7fff;
            }
            return crc &= 0x7fff;
        }

        // uvx --from git+https://github.com/tpircher/pycrc pycrc --generate=c
        // --algorithm=bit-by-bit-fast --width=10 --poly=0x48F --xor-in=0x10
        // --xor-out=0 --reflect-in=false --reflect-out=false
        private static ushort pec_10_crc(U8x8 data, byte bit_len)
        {
            ushort crc = 0x10;
            byte dataIdx = 0;

            while (bit_len > 0)
            {
                var c = data[dataIdx++];
                for (var i = 0x80; i > 0 && bit_len > 0; i >>= 1, --bit_len)
                {
                    var bit = (crc & 0x200) ^ ((c & i) != 0 ? 0x200 : 0);
                    crc <<= 1;
                    if (bit != 0)
                    {
                        crc ^= 0x08f;
                    }
                }
                crc &= 0x3ff;
            }
            return crc &= 0x3ff;
        }

        private U8x6[] configA;
        private U8x6[] configB;
        private uint timer;
        private uint tempOffset;
        private const uint VOLT_CELLS_PER_ADBMS = 16;
        private const uint TEMP_CELLS_PER_ADBMS = 24;

        private static U8x6 sin(float offset, float time, float inc)
        {
            U8x6 res = new();
            for (var i = 6; i > 0;)
            {
                var value = (ushort)((short)(Math.Sin((time / 16 + (offset + i / 2) * inc) * Math.Tau) * short.MaxValue) & 0xFFFF);
                res[--i] = (byte)(value >> 8);
                res[--i] = (byte)(value & 0xFF);
            }
            return res;
        }

        public void Reset()
        {
            spiState = SpiState.CmdIn;
            bufferIdx = new();
            configA.Fill(new());
            configB.Fill(new());
            timer = new();
            tempOffset = new();
        }

        public ADBMS6830B(uint daisies)
        {
            if (daisies <= 0)
                throw new ConstructionException($"There must be at least one ADBMS6830B");
            this.DebugLog($"ADBMS6830B configured with {daisies} daisy-chainned units");
            this.daisies = daisies;
            buffers = new U8x6[daisies];
            configA = new U8x6[daisies];
            configB = new U8x6[daisies];
            Reset();

            commands =
            [
                new Cmd("00000000001", "WRCFGA",    (x) => configA[daisyIdx] = x, null, null),
                new Cmd("00000100100", "WRCFGB",    (x) => configB[daisyIdx] = x, null, null),
                new Cmd("00000000010", "RDCFGA",    null, () => configA[daisyIdx], null),
                new Cmd("00000100110", "RDCFGB",    null, () => configB[daisyIdx], null),
                new Cmd("00000000100", "RDCVA",     null, null, null),
                new Cmd("00000000110", "RDCVB",     null, null, null),
                new Cmd("00000001000", "RDCVC",     null, null, null),
                new Cmd("00000001010", "RDCVD",     null, null, null),
                new Cmd("00000001001", "RDCVE",     null, null, null),
                new Cmd("00000001011", "RDCVF",     null, null, null),
                new Cmd("00000001100", "RDCVALL",   null, null, null),
                new Cmd("00001000100", "RDACA",     null, () => sin(daisyIdx * VOLT_CELLS_PER_ADBMS +  0, timer, 1f / (daisies * VOLT_CELLS_PER_ADBMS)), null),
                new Cmd("00001000110", "RDACB",     null, () => sin(daisyIdx * VOLT_CELLS_PER_ADBMS +  3, timer, 1f / (daisies * VOLT_CELLS_PER_ADBMS)), null),
                new Cmd("00001001000", "RDACC",     null, () => sin(daisyIdx * VOLT_CELLS_PER_ADBMS +  6, timer, 1f / (daisies * VOLT_CELLS_PER_ADBMS)), null),
                new Cmd("00001001010", "RDACD",     null, () => sin(daisyIdx * VOLT_CELLS_PER_ADBMS +  9, timer, 1f / (daisies * VOLT_CELLS_PER_ADBMS)), null),
                new Cmd("00001001001", "RDACE",     null, () => sin(daisyIdx * VOLT_CELLS_PER_ADBMS + 12, timer, 1f / (daisies * VOLT_CELLS_PER_ADBMS)), null),
                new Cmd("00001001011", "RDACF",     null, () => { var res = sin(daisyIdx * VOLT_CELLS_PER_ADBMS + 15, timer, 1f / (daisies * VOLT_CELLS_PER_ADBMS)); res[2..].Fill(0xFF); return res; }, null),
                new Cmd("00001001100", "RDACALL",   null, null, null),
                new Cmd("00000000011", "RDSVA",     null, null, null),
                new Cmd("00000000101", "RDSVB",     null, null, null),
                new Cmd("00000000111", "RDSVC",     null, null, null),
                new Cmd("00000001101", "RDSVD",     null, null, null),
                new Cmd("00000001110", "RDSVE",     null, null, null),
                new Cmd("00000001111", "RDSVF",     null, null, null),
                new Cmd("00000010000", "RDSALL",    null, null, null),
                new Cmd("00000010001", "RDCSALL",   null, null, null),
                new Cmd("00001010001", "RDACSALL",  null, null, null),
                new Cmd("00000010010", "RDFCA",     null, null, null),
                new Cmd("00000010011", "RDFCB",     null, null, null),
                new Cmd("00000010100", "RDFCC",     null, null, null),
                new Cmd("00000010101", "RDFCD",     null, null, null),
                new Cmd("00000010110", "RDFCE",     null, null, null),
                new Cmd("00000010111", "RDFCF",     null, null, null),
                new Cmd("00000011000", "RDFCALL",   null, null, null),
                new Cmd("00000011001", "RDAUXA",    null, null, null),
                new Cmd("00000011010", "RDAUXB",    null, null, null),
                new Cmd("00000011011", "RDAUXC",    null, () => sin(daisyIdx * TEMP_CELLS_PER_ADBMS + (tempOffset % TEMP_CELLS_PER_ADBMS), timer, 1f / (daisies * TEMP_CELLS_PER_ADBMS)), null),
                new Cmd("00000011111", "RDAUXD",    null, null, null),
                new Cmd("00000011100", "RDRAXA",    null, null, null),
                new Cmd("00000011101", "RDRAXB",    null, null, null),
                new Cmd("00000011110", "RDRAXC",    null, null, null),
                new Cmd("00000100101", "RDRAXD",    null, null, null),
                new Cmd("00000110000", "RDSTATA",   null, null, null),
                new Cmd("00000110001", "RDSTATB",   null, null, null),
                new Cmd("0000X110010", "RDSTATC",   null, null, null),
                new Cmd("00000110011", "RDSTATD",   null, null, null),
                new Cmd("00000110100", "RDSTATE",   null, null, null),
                new Cmd("00000110101", "RDASALL",   null, null, null),
                new Cmd("00000100000", "WRPWMA",    null, null, null),
                new Cmd("00000100010", "RDPWMA",    null, null, null),
                new Cmd("00000100001", "WRPWMB",    null, null, null),
                new Cmd("00000100011", "RDPWMB",    null, null, null),
                new Cmd("00001000000", "CMDIS",     null, null, null),
                new Cmd("00001000001", "CMEN",      null, null, null),
                new Cmd("00001000011", "CMHB2",     null, null, null),
                new Cmd("00001011000", "WRCMCFG",   null, null, null),
                new Cmd("00001011001", "RDCMCFG",   null, null, null),
                new Cmd("00001011010", "WRCMCELLT", null, null, null),
                new Cmd("00001011011", "RDCMCELLT", null, null, null),
                new Cmd("00001011100", "WRCMGPIOT", null, null, null),
                new Cmd("00001011101", "RDCMGPIOT", null, null, null),
                new Cmd("00001011110", "CLRCMFLAG", null, null, null),
                new Cmd("00001011111", "RDCMFLAG",  null, null, null),
                new Cmd("01XX11X0XXX", "ADCV",      null, null, null),
                new Cmd("001X11X10XX", "ADSV",      null, null, null),
                new Cmd("10XXX01XXXX", "ADAX",      null, null, () => tempOffset += 3),
                new Cmd("1000000XXXX", "ADAX2",     null, null, null),
                new Cmd("11100010001", "CLRCELL",   null, null, null),
                new Cmd("11100010100", "CLRFC",     null, null, null),
                new Cmd("11100010010", "CLRAUX",    null, null, null),
                new Cmd("11100010110", "CLRSPIN",   null, null, null),
                new Cmd("11100010111", "CLRFLAG",   null, null, null),
                new Cmd("11100010101", "CLOVUV",    null, null, null),
                new Cmd("11100011000", "PLADC",     null, null, null),
                new Cmd("11100011100", "PLCADC",    null, null, null),
                new Cmd("11100011101", "PLSADC",    null, null, null),
                new Cmd("11100011110", "PLAUX",     null, () => new(), null),
                new Cmd("11100011111", "PLAUX2",    null, null, null),
                new Cmd("11100100001", "WRCOMM",    null, null, null),
                new Cmd("11100100010", "RDCOMM",    null, null, null),
                new Cmd("11100100011", "STCOMM",    null, null, null),
                new Cmd("00000101000", "MUTE",      null, null, null),
                new Cmd("00000101001", "UNMUTE",    null, null, null),
                new Cmd("00000101100", "RDSID",     null, null, null),
                new Cmd("00000101110", "RSTCC",     null, null, () => commandCounter = 0),
                new Cmd("00000101101", "SNAP",      null, null, () => ++timer),
                new Cmd("00000101111", "UNSNAP",    null, null, () => {}),
                new Cmd("00000100111", "SRST",      null, null, null),
                new Cmd("00000111000", "ULRR",      null, null, null),
                new Cmd("00000111001", "WRRR",      null, null, null),
                new Cmd("00000111010", "RDRR",      null, null, null),
            ];
        }
    }
}
