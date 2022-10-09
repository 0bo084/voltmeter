#include <catch2/catch_test_macros.hpp>

/*!

    Checking construction and deserialization server command's objects 

*/


// test server side classes 
#undef VOLTMETER_CLIENT
#include "../source/command.cpp"
#include "../source/voltmeter.hpp"

using namespace voltio;

template <class Cmd>
void checkDeserialization(size_t ch) 
{
    voltmeter device;

    std::string planned_ser(Cmd::name);
    planned_ser += " channel";
    planned_ser += std::to_string(ch);
    
    
    std::unique_ptr<command> c =  
            cmdFactory::make( planned_ser.c_str(), planned_ser.length(), device);

    Cmd* cmd = reinterpret_cast<Cmd *>(c.get());
    REQUIRE(nullptr != cmd);
    REQUIRE(cmd->get_channel() == ch );
    REQUIRE(&(cmd->name[0]) == &Cmd::name[0]);
    REQUIRE(cmd->get_device() == &device);    
}

TEST_CASE( "check command serialiation", "[string -> object]" ) {
    
    std::string serialization;
    
    const size_t ch = 63;
    const range_t range{range_t::range3};
        

    SECTION( "deserialize set_range_cmd (string -> object)" ) {    
        
        voltmeter device;

        std::string planned_ser(set_range::name);
        planned_ser += " channel63, range3";
        
        std::unique_ptr<command> c =  
                cmdFactory::make( planned_ser.c_str(), planned_ser.length(), device);

        set_range_cmd* cmd = reinterpret_cast<set_range_cmd *>(c.get());
        REQUIRE(nullptr != cmd);
        REQUIRE(cmd->get_channel() == ch );
        REQUIRE(cmd->get_range() == range );
        REQUIRE(&(cmd->name[0]) == &set_range::name[0]);
        REQUIRE(cmd->get_device() == &device);    
    }

    SECTION( "deserialize start_measure_cmd (string -> object)" ) {    
        
        checkDeserialization<start_measure_cmd>(ch);
    }

    SECTION( "deserialize stop_measure_cmd (string -> object)" ) {    
        
        checkDeserialization<stop_measure_cmd>(ch);
    }
    SECTION( "deserialize get_status_cmd (string -> object)" ) {    
        
        checkDeserialization<get_status_cmd>(ch);
    }
    SECTION( "deserialize get_result_cmd (string -> object)" ) {    
        
        checkDeserialization<get_result_cmd>(ch);
    }
    
}
