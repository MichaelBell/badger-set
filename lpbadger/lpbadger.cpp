#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/regs/clocks.h"
#include "hardware/watchdog.h"
#include "hardware/pll.h"
#include "hardware/irq.h"
#include "hardware/flash.h"
#include "string.h"

#include "lpbadger.hpp"

#define ROSC_MHZ 6

extern "C" {
void badger_clocks_init()
{
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
      ROSC_MHZ * MHZ);

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
}

void LowPowerBadger::init()
{

  pimoroni::Badger2040::init();
}

#define FLASH_TARGET_OFFSET 0x100000

void LowPowerBadger::store_persistent_data(const uint32_t* data, uint32_t lenInWords)
{
  // Length in bytes
  int32_t len = lenInWords * 4;

  // Store max of 64KB.
  if (len > 0x10000) len = 0x10000;

  // Round erased length up to multiple of 4096
  const uint32_t erase_len =   (len + 0xfff) & 0x1f000;

  // Disable interrupts
  const uint32_t interrupt_mask = *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET));
  *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ICER_OFFSET)) = interrupt_mask;

  // Erase flash and program new data
  flash_range_erase(FLASH_TARGET_OFFSET, erase_len);
  uint8_t page_data[256];
  while (len > 0) {
    memcpy(page_data, data, (len < 256) ? len : 256);
    flash_range_program(FLASH_TARGET_OFFSET, page_data, 256);
    len -= 256;
  }

  // Enable interrupts
  *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)) = interrupt_mask;
}

const uint32_t* LowPowerBadger::get_persistent_data()
{
  return (const uint32_t*)(XIP_BASE + FLASH_TARGET_OFFSET);
}
