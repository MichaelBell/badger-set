#include "hardware/clocks.h"
#include "hardware/regs/clocks.h"
#include "hardware/watchdog.h"
#include "hardware/pll.h"
#include "hardware/vreg.h"
#include "hardware/structs/rosc.h"
#include "hardware/structs/syscfg.h"
#include "hardware/structs/xip_ctrl.h"

#define ROSC_MHZ 10

void badger_clocks_init()
{
    // Reduce voltage to 0.95 and reduce brownout detection voltage to 
    // avoid tripping it due to small fluctuations.
#if 1
    hw_write_masked(&vreg_and_chip_reset_hw->bod, 0b1000 << VREG_AND_CHIP_RESET_BOD_VSEL_LSB, VREG_AND_CHIP_RESET_BOD_VSEL_BITS);
    vreg_set_voltage(VREG_VOLTAGE_0_95);

    // Change ROSC divider to 8 to give a clock around 10MHz
    // (guaranteed between 3.6 and 24MHz).
    rosc_hw->div = 0xaa0 + 8;
#else
    // This version reduces voltage to 0.80V
    // This works for me but is quite a long way below spec!
    hw_write_masked(&vreg_and_chip_reset_hw->bod, 0b0100 << VREG_AND_CHIP_RESET_BOD_VSEL_LSB, VREG_AND_CHIP_RESET_BOD_VSEL_BITS);
    hw_write_masked(&vreg_and_chip_reset_hw->vreg, 0b0101 << VREG_AND_CHIP_RESET_VREG_VSEL_LSB, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);

    // Change ROSC divider to 5 to give a clock around 10MHz
    // (guaranteed between 3.6 and 24MHz).
    rosc_hw->div = 0xaa0 + 5;
#endif

    // Start tick in watchdog
    watchdog_start_tick(ROSC_MHZ);

    // Disable resus that may be enabled from previous software
    clocks_hw->resus.ctrl = 0;

    // Before we touch PLLs, switch sys and ref cleanly away from their aux sources.
    hw_clear_bits(&clocks_hw->clk[clk_sys].ctrl, CLOCKS_CLK_SYS_CTRL_SRC_BITS);
    while (clocks_hw->clk[clk_sys].selected != 0x1)
        tight_loop_contents();
    hw_clear_bits(&clocks_hw->clk[clk_ref].ctrl, CLOCKS_CLK_REF_CTRL_SRC_BITS);
    while (clocks_hw->clk[clk_ref].selected != 0x1)
        tight_loop_contents();

    // Configure clocks
    clock_configure(clk_ref,
                    CLOCKS_CLK_REF_CTRL_SRC_VALUE_ROSC_CLKSRC_PH,
                    0, // No aux mux
                    ROSC_MHZ * MHZ,
                    ROSC_MHZ * MHZ);

  clock_configure(clk_sys,
      CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
      CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_ROSC_CLKSRC,
      ROSC_MHZ * MHZ,
      (ROSC_MHZ / 2) * MHZ);

  clock_configure(clk_peri,
      0,
      CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_ROSC_CLKSRC_PH,
      ROSC_MHZ * MHZ,
      ROSC_MHZ * MHZ);

  clock_configure(clk_adc,
      0,
      CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_ROSC_CLKSRC_PH,
      ROSC_MHZ * MHZ,
      ROSC_MHZ * MHZ);

  clock_configure(clk_rtc,
      0,
      CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_ROSC_CLKSRC_PH,
      ROSC_MHZ * MHZ,
      46875);

  // The PLLs shouldn't be initialized, but this does no harm
  pll_deinit(pll_sys);
  pll_deinit(pll_usb);

#ifdef PICO_COPY_TO_RAM
  // Shutdown unused memory banks
  syscfg_hw->mempowerdown = 0x58;

  // And disable unused clock regions
  clocks_hw->wake_en1 = 0x303e;
  clocks_hw->wake_en0 = 0x73ef0f3f;
#else
  // Shutdown unused memory banks
  syscfg_hw->mempowerdown = 0x5e;

  // And disable unused clock regions
  clocks_hw->wake_en1 = 0x303e;
  clocks_hw->wake_en0 = 0x13ef0f3f;
#endif
}
