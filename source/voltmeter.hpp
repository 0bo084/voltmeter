/*
	voltio
*/

/*!
	Voltmeter Device class
    Contains channels 
*/

#pragma once

#include <array>

#include "channel.hpp"
#include "command.hpp"

#define CHECK_CHANNEL_IDX(idx, err) if(!isOkIndex((idx))) { (err) = "Invalid channel name!"; return false;}

namespace voltio
{

    class dataProvider;
    /*!
        Device
    */
    class voltmeter 
    {
        static constexpr size_t channelsNumber = 64;
        std::array<deviceChannel, channelsNumber> channels;

        bool isOkIndex(size_t devIdx) const noexcept 
        {
            return (devIdx < channels.size()) ?  true : false;
        }
        
    public:

        voltmeter() = default;

        void set(dataProvider& prov) {
            
            for(auto& ch : channels) 
                ch.set(prov);
        }
        
        // protocol
        bool start(size_t channel, std::string& err)
        {
            CHECK_CHANNEL_IDX(channel, err);

            bool isOk = channels[channel].start(err);
            return isOk;
        }
        
        bool set_range(size_t channel, range_t range, std::string& err)
        {
            CHECK_CHANNEL_IDX(channel, err);
            channels[channel].set_range(range);
            return true;
        }

        bool stop(size_t channel, std::string& err)
        {
            CHECK_CHANNEL_IDX(channel, err);

            bool isOk = channels[channel].stop(err);
            return isOk;
        }

        bool get_status(size_t channel, std::string& info)
        {
            CHECK_CHANNEL_IDX(channel, info);
            status_t status;
            channels[channel].get_status(status);
            
            return true;
        }

        bool get_result(size_t channel, std::string& info)
        {
            CHECK_CHANNEL_IDX(channel, info);
            float val = 0.0;
            bool isOk = channels[channel].get_result(val, info);
            //! make positive resp. Negative has aldeady done in @info
            if (isOk) { 
                info += std::to_string( val);
                info += " volts";
            }
            
            return isOk;
        }
    };


} // namespace voltio
