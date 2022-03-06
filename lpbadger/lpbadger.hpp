#include "badger2040.hpp"

class LowPowerBadger : public pimoroni::Badger2040
{
  public:
    void wait_for_idle();

    void store_persistent_data(const uint8_t* data, int32_t len);
    const uint8_t* get_persistent_data();
};
