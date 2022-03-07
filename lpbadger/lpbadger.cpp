#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/flash.h"
#include <string.h>
#include <math.h>

#include "lpbadger.hpp"

#define FLASH_TARGET_OFFSET 0x100000

class LowPowerBadgerGoSlow
{
public:
  LowPowerBadgerGoSlow()
  {
    // Increase the clock divider directly rather than using clock_configure
    // It doesn't matter that the rest of the system doesn't know the frequency
    // as we are in a tight loop.
    //
    // Disable interrupts while waiting as any interrupts would take
    // a very long time to execute running at 4kHz!
    interrupt_mask = *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET));
    *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ICER_OFFSET)) = interrupt_mask;

    clocks_hw->clk[clk_sys].div = 2500 << 8;
  }

  ~LowPowerBadgerGoSlow()
  {
    clocks_hw->clk[clk_sys].div = 2 << 8;

    // Enable interrupts
    *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)) = interrupt_mask;
  }

private:
  uint32_t interrupt_mask;
};

void LowPowerBadger::wait_for_idle()
{
  if (!is_busy()) return;

  LowPowerBadgerGoSlow slow;
  while (is_busy());
}

void LowPowerBadger::update(bool blocking)
{
  if (blocking) wait_for_idle();
  hw_set_bits(&clocks_hw->clk[clk_peri].ctrl, CLOCKS_CLK_PERI_CTRL_ENABLE_BITS);
  Badger2040::update(false);
  hw_clear_bits(&clocks_hw->clk[clk_peri].ctrl, CLOCKS_CLK_PERI_CTRL_ENABLE_BITS);
  if (blocking) wait_for_idle();
}

void LowPowerBadger::partial_update(int x, int y, int w, int h, bool blocking) 
{
  if (blocking) wait_for_idle();
  hw_set_bits(&clocks_hw->clk[clk_peri].ctrl, CLOCKS_CLK_PERI_CTRL_ENABLE_BITS);
  Badger2040::partial_update(x, y, w, h, false);
  hw_clear_bits(&clocks_hw->clk[clk_peri].ctrl, CLOCKS_CLK_PERI_CTRL_ENABLE_BITS);
  if (blocking) wait_for_idle();
}

void LowPowerBadger::update_speed(uint8_t speed)
{
  hw_set_bits(&clocks_hw->clk[clk_peri].ctrl, CLOCKS_CLK_PERI_CTRL_ENABLE_BITS);
  Badger2040::update_speed(speed);
  hw_clear_bits(&clocks_hw->clk[clk_peri].ctrl, CLOCKS_CLK_PERI_CTRL_ENABLE_BITS);
}

void LowPowerBadger::init()
{
  Badger2040::init();

  // Use a much shorter PWM wrap so that we can still control the brightness
  // at a very low system clock
  pwm_set_wrap(pwm_gpio_to_slice_num(LED), 60);

  // Disable PERI CLK
  hw_clear_bits(&clocks_hw->clk[clk_peri].ctrl, CLOCKS_CLK_PERI_CTRL_ENABLE_BITS);
}

void LowPowerBadger::halt()
{
  uint32_t interrupt_mask = *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET));
  *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ICER_OFFSET)) = interrupt_mask;

  gpio_put(ENABLE_3V3, 0);
  clocks_hw->clk[clk_sys].div = 2500 << 8;

  // don't allow any more code to execute while power rail drops
  while(true) {}
}

bool LowPowerBadger::wait_for_press(int timeout) {
  update_button_states();
  if (button_states()) return true;

  LowPowerBadgerGoSlow slow;
  int i = timeout * 100;
  do {
    update_button_states();
  } while (button_states() == 0 && (!timeout || --i));

  return button_states() != 0;
}

void LowPowerBadger::wait_for_no_press() {
  update_button_states();
  if (button_states() == 0) return;

  LowPowerBadgerGoSlow slow;
  do {
    update_button_states();
  } while (button_states() != 0);
}

void LowPowerBadger::led(uint8_t brightness) {
   // set the led brightness from 1 to 256 with gamma correction
   float gamma = 2.8;
   uint16_t v = (uint16_t)(pow((float)(brightness) / 256.0f, gamma) * 60.0f + 0.5f);
   pwm_set_gpio_level(LED, v);
}

void LowPowerBadger::fast_clear(bool white) {
  memset(frame_buffer, white ? 0x0 : 0xff, sizeof(frame_buffer));
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
