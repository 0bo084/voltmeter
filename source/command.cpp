
/*
	voltio
*/

/*!
	Command objects of Voltmeter protocol 
*/

#include "command.hpp"
#include "voltmeter.hpp"
#include "voltio.hpp"



using namespace voltio;


response start_measure_cmd::execute() { 
    std::string err;
    bool isOk = get_device()->start(get_channel(), err); 

    return response( isOk, std::move(err), get_channel(), get_name());
}

response stop_measure_cmd::execute() { 
    std::string err;
    bool isOk = get_device()->stop(get_channel(), err); 

    return response( isOk, std::move(err), get_channel(), std::move(get_name()));
}



response set_range_cmd::execute() { 
    std::string err;
    bool isOk = get_device()->set_range( get_channel(), get_range(), err); 

    return response( isOk, std::move(err), get_channel(), std::move(get_name()));
}


response get_status_cmd::execute() { 
            
    std::string info;
    bool isOk = get_device()->get_status( get_channel(), info);

    return response( isOk, std::move(info), get_channel(), std::move(get_name()));
}


response get_result_cmd::execute() { 
            
    std::string info;
    bool isOk = get_device()->get_result(get_channel(), info);

    return response( isOk, std::move(info), get_channel(), std::move(get_name()));
}
