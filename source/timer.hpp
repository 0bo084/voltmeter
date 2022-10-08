
/*
	voltio
*/

/*!
	simple timer. It uses the one thread. 
*/

#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <type_traits>




class timer {
	
    std::atomic<bool>   active{true};
    std::thread         thread;
	
public:
    timer() = default;

    timer(const timer&) = delete;
    timer(timer&&) = delete;
    timer& operator=(const timer) = delete;
    timer& operator=(timer&&) = delete;

    

    ~timer() 
    {
        stop();

        if (thread.joinable())
            thread.join();
    }

    template<typename Callable, typename ... Args>
    void setTimeout(Callable&& function, size_t delayMS, Args&&... args){

        active = true;
        
        std::thread t([=]() {
            if(!active.load()) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMS));
            if(!active.load()) return;
            function(std::forward<Args>(args)...);
        },
        std::forward<Args>(args)...);

        thread = std::move(t); 
               
    }

    template<typename Callable, typename ... Args>
    void setInterval(Callable&& function, size_t intervalMS, Args&&... args){
        active = true;
        std::thread t([=]() {
            while(active.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(intervalMS));
                if(!active.load()) return;
                function(std::forward<Args>(args)...);
            }
        },
        std::forward<Args>(args)...);

        thread = std::move(t);     
    }

    void stop() {

        active.store(false);
    }

    void join() {

        if (thread.joinable())
            thread.join();
    }

};
