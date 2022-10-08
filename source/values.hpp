/*
	voltio
*/

/*!
	Device channel
*/

#pragma once


#include <array>
#include <unordered_map>
#include <random>
#include <cmath>
#include <functional>
#include <mutex>

#include "voltio.hpp"
#include "timer.hpp"


namespace voltio
{

/*!
    Class is container of pseudo values to emulate measure process  
*/
class pseudoMeasureValues
{
    static constexpr size_t sizeOfData = 256;
    
    using  pseudoValues = std::array<float,sizeOfData>;
    
    
    pseudoValues range0Values; 
    pseudoValues range1Values; 
    pseudoValues range2Values; 
    pseudoValues range3Values; 

    std::mutex  mtx;
    
    const std::unordered_map< range_t, const pseudoValues&> mapValues = {
        {range_t::range0, range0Values},
        {range_t::range1, range1Values},
        {range_t::range2, range2Values},
        {range_t::range3, range3Values}
        };

    //! counters is position in each range. then it reaches the end, it set to begining  
    std::unordered_map<range_t, size_t> mapCounters = {
        {range_t::range0, 0ul},
        {range_t::range1, 0ul},
        {range_t::range2, 0ul},
        {range_t::range3, 0ul}
        };

    void setRand(pseudoValues& vals, size_t min, size_t max, size_t divider)
    {
        // Seed with a real random value, if available
        std::random_device r;
        
        // Choose a random mean between 1 and 6
        std::default_random_engine e1(r());
        std::uniform_int_distribution<size_t> uniform_dist(min, max);
        
        for(auto& i : vals)
            i = static_cast<pseudoValues_type>(static_cast<double>(uniform_dist(e1))/divider);
    }  
    
    
    public:

        using  pseudoValues_type = std::array<float,sizeOfData>::value_type;

        pseudoMeasureValues()
        {
            setRand(range0Values, 1ul, 9999ul, 10000000ul);
            setRand(range1Values, 1ul, 999ul, 1000ul);
            setRand(range2Values, 1ul, 999ul, 1ul);
            setRand(range3Values, 1000ul, 999999ul, 1ul);
        }

        /*!
            gets pseudo value for measuring
            Shoult be mt-safe
        */
        pseudoValues_type getValue(range_t range) 
        {
            std::lock_guard<std::mutex> guard(mtx);

            size_t i = mapCounters.at(range);
            const pseudoValues_type val =  mapValues.at(range).at(i++);
            mapCounters[range] = (i < mapValues.at(range).size()) ? i : 0ul;
            
            return val;
        }




};

// forward declaration
class voltmeter;

/*!
    Controller class uses as provider pseudo measure data for device channel 
*/


class dataProvider : private timer, private pseudoMeasureValues {

    
public:

    dataProvider() = default;
    ~dataProvider() 
    {
        timer::stop();
        timer::join();
        printf("dataProvider has been closed...\n");
    }

    template<typename Callable, typename ... Args>
    void start(Callable&& function, size_t intervalMS, Args&&... args)
    {
        timer::setInterval(std::forward<Callable>(function), intervalMS, std::forward<Args>(args)...);       
    }

    pseudoMeasureValues::pseudoValues_type getValue(range_t r)  {return pseudoMeasureValues::getValue(r);} 
    void stop() {timer::stop();}
};



} // namespace voltio
