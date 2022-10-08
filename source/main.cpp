#include <iostream>
#include <signal.h> 
#include <unistd.h> 
#include <stdio.h> 

#include "values.hpp"
#include "voltmeter.hpp"
#include "server.hpp"




using namespace voltio;


volatile sig_atomic_t flag = 0;

void my_function(int sig)
{ // can be called asynchronously
  
    flag = 1; // set flag
}


int main(int, char**) 
{

    std::string path = voltio::default_socket_path;
    voltmeter_server srv(path);

    // Register signals 
    __sighandler_t h = signal(SIGINT, my_function); 
    if (SIG_ERR == h)
        printf("Setup signal handler error in signal");
    //      ^          ^
    //  Which-Signal   |-- which user defined function registered
    while(!flag)  
        sleep(100);
    
    srv.stop();

    return 0;
    
    
    
}
