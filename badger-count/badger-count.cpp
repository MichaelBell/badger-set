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

  uint32_t count = *badger.get_persistent_data();
  if (badger.pressed_to_wake(badger.A)) {
    ++count;
  }
  else if(badger.pressed_to_wake(badger.C)) {
    count = 0;
  }

  badger.pen(15);
  badger.clear();

  badger.pen(0);
  badger.font("sans");

  char text_buf[80];
  sprintf(text_buf, "Count: %d", count);
  badger.text(text_buf, 15, 65, 1.0f);

  badger.update();
  badger.store_persistent_data(&count, 1);

  badger.led(0);

  while (badger.is_busy()) sleep_ms(10);

  badger.halt();
}

