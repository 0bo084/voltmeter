#include "channel.hpp"
#include "values.hpp"



using namespace voltio;


// used to set pseudo values. Emulate changing volts in channel
void deviceChannel::set(dataProvider& prov)
{
    // use exclusive lock while writing data
    std::unique_lock<std::shared_mutex> lock(mtx);
    

    static constexpr double probability = 0.05;
    //divide to get a number between 0 and 1
    double result = rand() / static_cast<double>(RAND_MAX);
   
    if(getStatus() == status_t::busy)
        setStatus(prevstatus);
    else if((result < probability)&&(getStatus() != status_t::busy)){
        // emulate probabitity error
        prevstatus = getStatus();
        setStatus(status_t::busy);
    }
    
    if (getStatus() == status_t::measure) {
        
        range_t r = getRange();
        float val = prov.getValue(r);
        setValue(val);
    }

}
