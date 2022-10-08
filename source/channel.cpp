#include "channel.hpp"
#include "values.hpp"



using namespace voltio;


// used to set pseudo values. Emulate changing volts in channel
void deviceChannel::set(dataProvider& prov)
{
    // use exclusive lock while writing data
    std::unique_lock<std::shared_mutex> lock(mtx);
    if (getStatus() == status_t::measure) {
        
        range_t r = getRange();
        float val = prov.getValue(r);
        setValue(val);
    }

}
