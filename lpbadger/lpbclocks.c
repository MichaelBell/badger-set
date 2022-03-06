#include "hardware/clocks.h"
#include "hardware/regs/clocks.h"
#include "hardware/watchdog.h"
#include "hardware/pll.h"
#include "hardware/structs/rosc.h"

#define ROSC_MHZ 10

void badger_clocks_init()
{
    // Change ROSC divider to 10 to give a clock around 10MHz 
    // (guaranteed between 1.92 and 19.2MHz).
    rosc_hw->div = 0xaa0 + 10;

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

  // And disable unused clock regions
  clocks_hw->wake_en1 = 0x303f;
  clocks_hw->wake_en0 = 0xf3ef0f3f;
}
