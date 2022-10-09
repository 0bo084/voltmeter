/*
	voltio
*/

/*!
	Common structures of Voltmeter protocol 
*/

#pragma once

#include <cstddef>
#include <string>
#include "exception.hpp"



namespace voltio 
{

    constexpr char default_socket_path[] = {"/tmp/9Lq7BNBnBycd6nxy.socket"};

    /*!
        
        The right command seqence is:
        1. set_range(range)
        2. start_measure
        3. get_result
        4. stop_measure

        - you can call get_status in any time
        - you can call get_result only after start_measure
        - set_range(range) reset channel to idle_state
        - start_measure sets the channel to measure_state
        - stop_measure sets channel to idle_state
        - set_range(range) works in all statuses and does not change it. 
        it only sets the new range
    */




    //! ranges of mesured values in volts
    enum class range_t
    {

        range0 = 0, //[0.0000001 ... 0.0010000)
        range1 = 1, //[0.001 ... 1)
        range2 = 2, //[1 ... 1000)
        range3 = 3, //[1000 ... 1000000)

    };

    //! Status of device cannel
    enum class status_t
    {
        idle = 0, //! default state of channel
        measure = 1, //! ready to get result
        busy = 2, /*! 
                    internal driven state of device. 
                    Device cant get result now. you can 
                    retry request to get success later. 
                    Channel able to reset this error itself 
                */
        error = 3,  /*! 
                    error state of device. To reset this error client should
                    reinitialize a device.  Channel COULD NOT reset 
                    this error itself! 
                */
    };

    constexpr size_t NumberOfChannels = 64;

    // static helpers    
    inline static size_t range_to(range_t range) noexcept 
    {
        return static_cast<size_t>(range);
    } 

    inline static range_t to_range(size_t r)  // exeption
    {
        if (r > static_cast<size_t>(range_t::range3)) 
            throw exception_t("Invalid range name!"); // exception
        return static_cast<range_t>(r);
    } 

    inline static std::string status_to(status_t status) // exception
    {
        switch(status) 
        {
            case status_t::idle : return "idle";
            case status_t::measure : return "measure";
            case status_t::busy : return "busy";
            case status_t::error : return "error";      
            default: return "unknown status";
        }
    }


} // namespace voltio



#include <unistd.h>

/*!
    helper classes to guard resources along RAII
*/
template<typename T>
class fd_guard {
    T fd;
    bool isOwner;
public:
    
    fd_guard(T _fd) noexcept : fd(_fd), isOwner(true) {}
    ~fd_guard() noexcept { if (isOwner) close(fd);}

    void detach() noexcept {isOwner = false;}
    
};
