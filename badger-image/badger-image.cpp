#include "pico/stdlib.h"
#include <stdio.h>
#include <cstring>
#include <string>
#include <algorithm>
#include "pico/time.h"
#include "pico/platform.h"

#include "common/pimoroni_common.hpp"
#include "lpbadger.hpp"

#include "examples/badger2040/badger2040_image_demo_images.hpp"

LowPowerBadger badger;

int main() {
  badger.init();
  badger.led(200);

  uint32_t requested_state = 0;
  if (badger.pressed_to_wake(badger.A)) {
    requested_state = 1UL << badger.A;
  }

  else if(badger.pressed_to_wake(badger.B)) {
    requested_state = 1UL << badger.B;
  }

  else if(badger.pressed_to_wake(badger.C)) {
    requested_state = 1UL << badger.C;
  }

  else {
    badger.pen(15);
    badger.clear();

    badger.pen(0);
    badger.font("sans");
    badger.text("Press A, B, or C", 15, 65, 1.0f);
  }

  bool need_update = true;
  while (need_update) {
    badger.led(100);
    if (requested_state) {
      if (requested_state & (1UL <<  badger.A)) badger.image(shaun);
      else if (requested_state & (1UL << badger.B)) badger.image(paul);
      else if (requested_state & (1UL << badger.C)) badger.image(adam);
    }
    badger.update();
    need_update = false;

    const uint32_t button_mask = (7UL << badger.A) & ~requested_state;
    requested_state = 0;
    while (badger.is_busy()) {
      sleep_ms(10);

      // Detect any pressed buttons and
      // trigger another update if any are pressed
      badger.update_button_states();
      uint32_t button_state = badger.button_states() & button_mask;
      if (button_state) {
        need_update = true;
        badger.led(200);

        // Set requested state to lowest set bit in button state
        requested_state = button_state & -button_state;
      }
    }
  }

  badger.led(0);
  badger.halt();
}

