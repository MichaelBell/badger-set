#include "badger2040.hpp"

class LowPowerBadger : public pimoroni::Badger2040
{
  public:
    void init();

    // Wait for e-ink disaply to finish updating
    void wait_for_idle();

    // Override update methods to use our lower power wait
    void update(bool blocking=false);
    void partial_update(int x, int y, int w, int h, bool blocking=false);

    void led(uint8_t brightness);

    void store_persistent_data(const uint8_t* data, int32_t len);
    const uint8_t* get_persistent_data();
};
