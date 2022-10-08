/*
	voltio
*/

/*!
	Device channel
*/

#pragma once

#include "voltio.hpp"
#include "exception.hpp"


#include <atomic>
#include <mutex>
#include <shared_mutex>


namespace voltio
{


// forvard declaration
class dataProvider;

/*!
	Device channel

*/
class deviceChannel 
{
    status_t                    status{status_t::error};
    std::atomic<float>          value{0.0};
    range_t                     range{range_t::range0};
    mutable std::shared_mutex   mtx;


    // private getter/setter. NOT thread safe!
    float getValue() const noexcept {return value;};
    void setValue(float val) noexcept {value = val;}
    range_t getRange() const noexcept {return range;};
    void setRange(range_t r) noexcept {range = r;};
    status_t getStatus() const noexcept { return status; }
    void setStatus(status_t st) noexcept { status = st; }

public:

    /*! 
        protocol commands for channel
    */
    
    bool start(std::string& err) noexcept 
    {
        // use exclusive lock while writing data
        std::unique_lock<std::shared_mutex> lock(mtx);
        // analyse current state
        switch (getStatus())
        {
        case status_t::busy :
        case status_t::error :
            // reset channel, now we are ready to measure
            setStatus(status_t::measure);
            break;
        case status_t::measure :
            // do nothing...
            break;
        
        default:
            err = "Unknown channel status!";
            setStatus(status_t::error);
            return false;
        } 

        return true;
    }


    bool stop(std::string& err) noexcept
    {
        // use exclusive lock while writing data
        std::unique_lock<std::shared_mutex> lock(mtx);
        // analyse current state
        switch (getStatus())
        {
        case status_t::busy :
        case status_t::error :
        case status_t::measure :
            // reset channel, now we are ready to measure
            setStatus(status_t::idle);
            break;
        case status_t::idle :
            // do nothing...
            break;
        
        default:
            err = "Unknown channel status!";
            setStatus(status_t::error);
            return false;
        } 

        return true;
    }


    void set_range(range_t range) noexcept 
    {
        // use exclusive lock while writing data
        std::unique_lock<std::shared_mutex> lock(mtx);
        // do not needed analyse current state. Command works in all state
        setRange(range);
    }



    bool get_status(status_t& st) const noexcept 
    { 
        // use shared lock while reading data
        std::shared_lock<std::shared_mutex> lock(mtx);
        st = getStatus();
        return true;
    }

    bool get_result(float& val, std::string& err) const noexcept
    { 
        // use shared lock while reading data
        std::shared_lock<std::shared_mutex> lock(mtx);
        
        if (getStatus() == status_t::measure) {
            val = getValue();
            return true;
        }
        else {
            err = "bad stastus!";
            return false;
        }
    }

    // used to set pseudo values. Emulate changing volts in channel
    void set(dataProvider& prov);

};

}