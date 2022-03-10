#pragma once

#include "badger2040.hpp"

class LowPowerBadger : public pimoroni::Badger2040
{
  public:
    LowPowerBadger() : Badger2040(frame_buffer) {}

    void init();
    void halt();

    // Wait for e-ink disaply to finish updating
    void wait_for_idle();

    // Wait and timeout after approximately n seconds, or wait forever if seconds is zero
    // Returns false on timeout, otherwise true.
    // Note that unlike the normal Badger2040 this does not wait for the button to be released
    bool wait_for_press(int seconds = 0);

    // Wait for all buttons to be released
    void wait_for_no_press();

    // Override update methods to use our lower power wait
    void update(bool blocking=false);
    void partial_update(int x, int y, int w, int h, bool blocking=false);
    void update_speed(uint8_t speed);

    void led(uint8_t brightness);
    void fast_clear(bool white = true);

    void store_persistent_data(const uint8_t* data, int32_t len);
    const uint8_t* get_persistent_data();

  private:
    uint8_t frame_buffer[296*128 / 8];
};
