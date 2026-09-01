/*
 * RCC model for STM32G4 series, RM0440 section 7.4
 * https://www.st.com/resource/en/reference_manual/rm0440-stm32g4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
 * This is essentially a memory map (every write is left as is), with the
 * distinction that the RCC's XXX_ON bits (set by software) are copied to RCC's
 * XXX_RDY bits (ready bits, set by hardware). It also copies
 * RCC's SW (set by software) to RCC's SWS (set by hardware).
 */

using Antmicro.Renode.Core;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Peripherals.Memory;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32G4_RCC : IDoubleWordPeripheral, IKnownSize
    {
        public long Size { get; }

        private readonly ArrayMemory memory;

        // RCC_CR (RM0440 7.4.1): XXX_RDY mirrors XXX_ON
        private const uint HSI_ON_MASK = 1u << 8; // bit 8
        private const int HSI_ON_TO_RDY_SHIFT = 2; // bit 10
        private const uint HSI_RDY_MASK = HSI_ON_MASK << HSI_ON_TO_RDY_SHIFT;
        //
        private const uint HSE_ON_MASK = 1u << 16; // bit 16
        private const int HSE_ON_TO_RDY_SHIFT = 1; // bit 17
        private const uint HSE_RDY_MASK = HSE_ON_MASK << HSE_ON_TO_RDY_SHIFT;
        //
        private const uint PLL_ON_MASK = 1u << 24; // bit 24
        private const int PLL_ON_TO_RDY_SHIFT = 1; // bit 25
        private const uint PLL_RDY_MASK = PLL_ON_MASK << PLL_ON_TO_RDY_SHIFT;
        //
        private const uint RDY_MASK = HSI_RDY_MASK | HSE_RDY_MASK | PLL_RDY_MASK;

        // RCC_CFGR (RM0440 7.4.3): SWS mirrors SW
        private const uint SW_MASK = 0x3u; // bits 0,1
        private const int SW_TO_SWS_SHIFT = 2; // bits 2,3
        private const uint SWS_MASK = SW_MASK << SW_TO_SWS_SHIFT;

        private enum Registers
        {
            CR = 0x00, // RM0440 7.4.1
            CFGR = 0x08, // RM0440 7.4.3
        }

        public STM32G4_RCC(IMachine machine, long size)
        {
            Size = size;
            memory = new ArrayMemory((ulong)size);
        }

        public void Reset()
        {
            memory.Reset();
            // TODO reset to their default values, not just 0
        }

        public uint ReadDoubleWord(long offset)
        {
            return memory.ReadDoubleWord(offset);
        }

        public void WriteDoubleWord(long offset, uint value)
        {
            switch ((Registers)offset)
            {
                case Registers.CR:
                    value = (value & ~RDY_MASK)
                        | ((value & HSI_ON_MASK) << HSI_ON_TO_RDY_SHIFT)
                        | ((value & HSE_ON_MASK) << HSE_ON_TO_RDY_SHIFT)
                        | ((value & PLL_ON_MASK) << PLL_ON_TO_RDY_SHIFT);
                    break;
                case Registers.CFGR:
                    value = (value & ~SWS_MASK)
                        | ((value & SW_MASK) << SW_TO_SWS_SHIFT);
                    break;
            }
            memory.WriteDoubleWord(offset, value);
        }
    }
}
