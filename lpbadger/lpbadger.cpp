#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/flash.h"
#include "string.h"

#include "lpbadger.hpp"

#define FLASH_TARGET_OFFSET 0x100000

void LowPowerBadger::wait_for_idle()
{
  while (is_busy()) sleep_ms(5);
}

void LowPowerBadger::store_persistent_data(const uint8_t* data, int32_t len)
{
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

const uint8_t* LowPowerBadger::get_persistent_data()
{
  return (const uint8_t*)(XIP_BASE + FLASH_TARGET_OFFSET);
}
