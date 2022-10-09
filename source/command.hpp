/*
	voltio
*/

/*!
	Command objects of Voltmeter protocol 
*/

#pragma once


#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <algorithm>

#include "voltio.hpp"
#include "exception.hpp"


namespace voltio 
{


class response {
    
    
    bool        isOk;
    size_t      channel;
    std::string info;
    std::string cmd;


public:

    response() 
        : isOk(false)
        , channel(0)
        {}

    response(bool _isOk, std::string&& _info, size_t ch, std::string&& _cmd) 
        : isOk(_isOk)
        , channel(ch)
        , info(std::move(_info))
        , cmd(std::move(_cmd))
        {}

    response(const response& other)
        : isOk(other.isOk)
        , channel(other.channel)
        , info(other.info)
        , cmd(other.cmd)
        {}    

    response(response&& other) 
        : isOk(std::move(other.isOk))
        , channel(std::move(other.channel))
        , info(std::move(other.info))
        , cmd(std::move(other.cmd))
        {}

    //! accesors
    size_t get_channel() const noexcept {return channel;}
    bool get_isOk() const noexcept {return isOk;}
    const std::string& get_info() const noexcept {return info;}
    const std::string& get_cmd() const noexcept {return cmd;}
    

    //! serialization
    bool serialize(std::string& ser) const {

        ser = "[";
        ser += std::to_string(get_channel());
        ser += "] ";
        ser += get_cmd();

        ser += (get_isOk()) ? ": Ok" : ": Fail";
        if(get_info().size()) {
            ser += ", ";
            ser += get_info();
        }
        
        return true;
    }
};
 


class command {

public:
    
    virtual bool serialize(std::string& ser) const = 0;
    
    
    
    virtual ~command() noexcept {}

#ifndef VOLTMETER_CLIENT
    virtual response execute() = 0;
#endif

};

struct start_measure {

    static constexpr char name[] = "start_measure";
};

struct set_range {

    static constexpr char name[] = "set_range";
};

struct stop_measure {

    static constexpr char name[] = "stop_measure";
};


struct get_status {

    static constexpr char name[] = "get_status";
};

struct get_result {

    static constexpr char name[] = "get_result";
    
};


// forward declaration
class voltmeter;

template <typename T>
class cmd: public command, public T {

    size_t      channel;
    voltmeter*  device;


protected:

    

    cmd(size_t ch, voltmeter* dev) noexcept : channel(ch), device(dev) {} 

public:

    auto get_device() noexcept {return device;}
    
    auto get_channel() const noexcept {return channel;}
    

    virtual bool serialize(std::string& ser) const override 
    {
        ser = T::name;
        ser += " channel";
        ser += std::to_string(get_channel());


        return true;
    }

};

class start_measure_cmd: public cmd<start_measure> {

   
    
public:

    virtual bool serialize(std::string& ser) const final
    {
        bool isOk = cmd<start_measure>::serialize(ser);
        return isOk;
    }


    start_measure_cmd( size_t channel, voltmeter* dev) noexcept : cmd<start_measure>(channel,dev) {}
    
    std::string get_name() const {return start_measure::name;}

#ifndef VOLTMETER_CLIENT   
    virtual response execute() final;
#endif        

        
};

class set_range_cmd: public cmd<set_range> {

        
    range_t range;


public:

    virtual bool serialize(std::string& ser) const final
    {
        bool isOk = cmd<set_range>::serialize(ser);

        ser += ", range";
        ser += std::to_string(range_to(get_range()));

        return isOk;
    }
        
   
    set_range_cmd(size_t channel, range_t _range, voltmeter* dev) noexcept 
        : cmd<set_range>(channel, dev)
        , range(_range) 
    {
    }

    
    std::string get_name() const noexcept {return set_range::name;}
    range_t get_range() const noexcept {return range;}
    
        
#ifndef VOLTMETER_CLIENT   
        virtual response execute() final;
#endif        

};

class stop_measure_cmd : public cmd<stop_measure> {

public:
    
    virtual bool serialize(std::string& ser) const final
    {
        bool isOk = cmd<stop_measure>::serialize(ser);
        return isOk;
    }



    stop_measure_cmd(size_t channel, voltmeter* dev) noexcept : cmd<stop_measure>(channel,dev) {}

    std::string get_name() const {return stop_measure::name;}

#ifndef VOLTMETER_CLIENT   
    virtual response execute() final;
#endif        

};

class get_status_cmd : public cmd<get_status> {

public:
    
    virtual bool serialize(std::string& ser) const final
    {
        bool isOk = cmd<get_status>::serialize(ser);
        return isOk;
    }

    get_status_cmd(size_t channel, voltmeter* dev) noexcept : cmd<get_status>(channel, dev) {}

    std::string get_name() const {return get_status::name;}


#ifndef VOLTMETER_CLIENT   
    virtual response execute() final;
#endif  
};

class get_result_cmd : public cmd<get_result> {
    
public:

    virtual bool serialize(std::string& ser) const final
    {
        bool isOk = cmd<get_result>::serialize(ser);
        return isOk;
    }

    get_result_cmd(size_t channel, voltmeter* dev) noexcept : cmd<get_result>(channel, dev) {}

    std::string get_name() const {return get_result::name;}

#ifndef VOLTMETER_CLIENT   
    virtual response execute() final;
#endif        

};

class cmdFactory {

        static constexpr size_t Size = 1024;
        
        // helpers
        // return true/false and position after spaces
        static int skip_spaces(const char* buff, const size_t len)
        {
            size_t i = 0;
            //! skip spaces 
            while((i < len)&&(buff[i] == ' '))
                ++i;
            return i;
        }   

        //! NOTE: if function returns false, @param pos is not valid!
        static bool search_and_skip(size_t* pos, const char* buff, size_t len, const char symbol)
        {
            size_t i = 0;
            bool isFound = false;
            //! skip spaces 
            while(i < len) {
                if (buff[i++] == symbol){
                    isFound = true;
                    break;
                }
            }
            
            *pos = i;

            return isFound;
        }   

    public:

#ifndef VOLTMETER_CLIENT 
        //! server side serialization helpers
        
        //! make cmd object from binary buffer
        static std::unique_ptr<command> make(const char* buff, size_t len, voltmeter& device)
        {
            // typedef lymbda
            using construct = std::unique_ptr<command> (*)(
                size_t ch, voltmeter& device, const char* buff, size_t len); 
            // constructors
            auto construct_start_measure = 
                [](size_t ch, voltmeter& device, const char*, size_t ) -> std::unique_ptr<command> 
                {
                    return std::make_unique<start_measure_cmd>(ch, &device);
                };
            auto construct_stop_measure = 
                [](size_t ch, voltmeter& device, const char* , size_t ) -> std::unique_ptr<command> 
                {
                    return std::make_unique<stop_measure_cmd>(ch, &device);
                };
            auto construct_set_range = 
                [](size_t ch, voltmeter& device, const char* buff, size_t len) -> std::unique_ptr<command> 
                {
                    // parse range id 
                    size_t i = 0;
                    bool delimeterFound = cmdFactory::search_and_skip(&i, buff, len, ',');
                    if (!delimeterFound) return std::unique_ptr<command>();
                    
                    i += cmdFactory::skip_spaces(buff + i, len - i);
                    std::string_view ch_view("range");
                    if (0 != ch_view.compare( 
                        0 , ch_view.length(), buff + i, std::min(ch_view.length(),len - i)))
                        return std::unique_ptr<command>();  // range id has not found...

                    i += ch_view.length();
                    size_t n = 0;
                    size_t r  = std::stoul(std::string( buff + i), &n);
                    i += n;
                    return std::make_unique<set_range_cmd>(ch, to_range(r), &device);
                };
            
            auto construct_get_status = 
                [](size_t ch, voltmeter& device, const char* , size_t ) -> std::unique_ptr<command> 
                {
                    return std::make_unique<get_status_cmd>(ch, &device); 
                };
            
            auto construct_get_result = 
                [](size_t ch, voltmeter& device, const char* , size_t ) -> std::unique_ptr<command> 
                {
                    return std::make_unique<get_result_cmd>(ch, &device); 
                };
            
            
            constexpr std::array<std::pair<std::string_view, construct>, 5> cmds {
                std::pair{start_measure::name, construct_start_measure},
                std::pair{stop_measure::name, construct_stop_measure},
                std::pair{set_range::name, construct_set_range},
                std::pair{get_status::name, construct_get_status},
                std::pair{get_result::name, construct_get_result},
                };
            
            size_t i = cmdFactory::skip_spaces( buff, len);
            //! parsing buffer
            // fist, we should find the command name
            bool isFound = false;
            size_t index = 0;
            for(auto& c : cmds) {
                if(0 == c.first.compare(i, c.first.length(), buff + i, std::min(c.first.length(),len - i))) {
                    isFound = true;
                    i += c.first.length();
                    break;
                }
                ++index;
            }
            if (!isFound) return std::unique_ptr<command>();  // command has not found...

            // index points on useful record in cmds 
            
            // second, parse channel id
            int s = cmdFactory::skip_spaces( buff + i, len - i);
            i += s;
            std::string_view ch_view("channel");
            if (0 != ch_view.compare( 0 , ch_view.length(), buff + i, std::min(ch_view.length(), len - i)))
                return std::unique_ptr<command>();  // channel id has not found...

            i += ch_view.length();
            size_t n = 0;
            size_t ch  = std::stoul(std::string( buff + i), &n);
            i += n;

            return cmds[index].second(ch, device, buff + i, len - i);    
        }
        
        //! make binary stream from reponse obj
        static void make( std::string& ser, const response& resp)
        {  
            resp.serialize(ser);
             
        }
        
#else
        //! client side serialization helpers
        
        //! make binary stream from cmd obj
        static bool make( std::string& ser, const command& cmd)
        {
            bool isOk = cmd.serialize(ser);
            return isOk;
        }

#endif
        

};

} // namespace voltio