#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/regs/clocks.h"
#include "hardware/pll.h"
#include "hardware/irq.h"
#include "hardware/flash.h"
#include "string.h"

#include "lpbadger.hpp"

void LowPowerBadger::init()
{
    // Configure all clocks to run off the USB PLL, and shut down
  // the SYS PLL to save power.
  // 24MHz seems to be the slowest we can run the system and still
  // talk to the display - I'm not quite sure why it doesn't work
  // with a slower system clock
  clock_stop(clk_usb);
  clock_stop(clk_adc);
  clock_stop(clk_rtc);

  pll_init(pll_usb, 1, 480 * MHZ, 5, 4);

  clock_configure(clk_sys,
      CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
      CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
      24 * MHZ,
      24 * MHZ);

  clock_configure(clk_peri,
      0,
      CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
      24 * MHZ,
      24 * MHZ);

  clock_configure(clk_adc,
      0,
      CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
      24 * MHZ,
      24 * MHZ);

  clock_configure(clk_rtc,
      0,
      CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
      24 * MHZ,
      46875);

  pll_deinit(pll_sys);

  // Disable unused clock regions
  clocks_hw->wake_en1 = 0x703f;
  clocks_hw->wake_en0 = 0xf3ef8f3f;

  sleep_ms(1);

  pimoroni::Badger2040::init();

  sleep_ms(1);
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
