#include <catch2/catch_test_macros.hpp>

// test client side classes 
#define VOLTMETER_CLIENT
#include "../source/command.hpp"

using namespace voltio;


template <class Cmd>
void checkCmdSerialization(size_t ch)
{
    Cmd cmd( ch, nullptr);

    REQUIRE(cmd.get_channel() == ch );
    REQUIRE(&cmd.name[0] == &Cmd::name[0]);
    
    std::string planned_ser(Cmd::name);
    planned_ser += " channel";
    planned_ser += std::to_string(ch);

    std::string serialization;
    cmd.serialize(serialization);
    

    REQUIRE(planned_ser == serialization );
}


TEST_CASE( "check response serialiation", "[object -> string]" ) {
    
    constexpr char _cmd[] = {"get_status"};
    constexpr char _info[] = {"idle"};
    constexpr char _err[] = {"unknown status!"};

    std::string serialization;
    bool isOk = true;
    std::string cmd{_cmd};
    std::string info{_info};
    std::string err{_err};

    const size_t ch = 63;
        
    SECTION( "positive response" ) {
        
        response resp( isOk, std::move(info), ch,  std::move(cmd));

        REQUIRE(resp.get_channel() == ch );
        REQUIRE(resp.get_isOk() == isOk );
        REQUIRE(resp.get_info() ==  _info);
        REQUIRE(resp.get_cmd() == _cmd );

        std::string planned_ser("[63] ");
        planned_ser += _cmd;
        planned_ser += ": Ok, ";
        planned_ser += _info;

        resp.serialize(serialization);
        

        REQUIRE(planned_ser == serialization );
    }

    SECTION( "negative response" ) {
        isOk = false;
        response resp( isOk, std::move(err), ch,  std::move(cmd));
        
        REQUIRE(resp.get_channel() == ch );
        REQUIRE(resp.get_isOk() == isOk );
        REQUIRE(resp.get_info() ==  _err);
        REQUIRE(resp.get_cmd() == _cmd );

        resp.serialize(serialization);
        std::string planned_ser("[63] ");
        planned_ser += _cmd;
        planned_ser += ": Fail, ";
        planned_ser += _err;


        REQUIRE(planned_ser == serialization );
    }
}


TEST_CASE( "check set_range_cmd serialiation", "[object -> string]" ) {
    
    
    
    
    const size_t ch = 63;
    const range_t range{range_t::range3};
    
    

    SECTION( "construct set_range_cmd and serialize" ) {    

        set_range_cmd cmd( ch, range,  nullptr);
        
        REQUIRE(cmd.get_channel() == ch );
        REQUIRE(cmd.get_range() == range );
        REQUIRE(&cmd.name[0] == &set_range::name[0]);
        
        std::string planned_ser(set_range::name);
        planned_ser += " channel63, range3";
        
        std::string serialization;
        cmd.serialize(serialization);

        REQUIRE(planned_ser == serialization );
    }

    SECTION( "construct start_measure_cmd and serialize" ) {    

        checkCmdSerialization<start_measure_cmd>(ch);
    }

    SECTION( "construct stop_measure_cmd and serialize" ) {    

        checkCmdSerialization<stop_measure_cmd>(ch);
    }
    
    SECTION( "construct get_status_cmd and serialize" ) {    

        checkCmdSerialization<get_status_cmd>(ch);
    }
    
    SECTION( "construct get_result_cmd and serialize" ) {    

        checkCmdSerialization<get_result_cmd>(ch);
    }
}


