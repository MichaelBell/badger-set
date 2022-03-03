#include "badger2040.hpp"

class LowPowerBadger : public pimoroni::Badger2040
{
  public:
    void init();

    void store_persistent_data(const uint32_t* data, uint32_t lenInWords);
    const uint32_t* get_persistent_data();
};
