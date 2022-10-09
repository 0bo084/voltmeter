/*!

    Simple load tests for the server
*/


#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#include <string>
#include <thread>
#include <memory>


// will use client tide 
#define VOLTMETER_CLIENT
#include "../source/command.hpp"
#include "../source/voltio.hpp"


int connect(std::string path);


int connect(std::string path)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    fd_guard guard(fd);
    
    
    sockaddr_un address;
    memset (&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path.c_str());

    int ret = ::connect( fd, reinterpret_cast<const sockaddr *>(&address), SUN_LEN(&address));
    
    if (ret < 0) {
        ret = errno;
    }

    guard.detach();
    return fd;
}

static constexpr size_t NumberOfThread = 4096;

int main(int , char** )
{
    
    std::array<std::thread, NumberOfThread>  threads;

    for(auto& t: threads)
        t = std::thread(
            [](){
                int fd = ::connect(voltio::default_socket_path);
                std::string str = std::string("get_result channel63, range2");

                int res = ::send(fd, str.c_str(), str.length(), 0);
                if(res < 0)
                    printf("send error %d \n", errno);

                char buff[1024];
                res = ::recv(fd, buff, sizeof(buff), 0);
                if(res < 0)
                    printf("send error %d \n", errno);
                else
                    printf("recv  %d bytes\n", res);
                
                close(fd);


            }
        );

    for(auto& t: threads)
        t.join();


    std::array<std::thread, NumberOfThread>  _threads;

    size_t i = 0;
    for(auto& t: _threads) {
        
        t = std::thread(
            [i](){
                int fd = ::connect(voltio::default_socket_path);
                std::string str = std::string("start_measure channel");
                str += std::to_string(i%64);
                
                int res = ::send(fd, str.c_str(), str.length(), 0);
                if(res < 0)
                    printf("send error %d \n", errno);

                char buff[1024];
                res = ::recv(fd, buff, sizeof(buff), 0);
                if(res < 0)
                    printf("send error %d \n", errno);
                else
                    printf("recv  %d bytes\n", res);
                
                str = std::string("get_result channel");
                str += std::to_string(i%64);
                str += ", range2";

                res = ::send(fd, str.c_str(), str.length(), 0);
                if(res < 0)
                    printf("send error %d \n", errno);
                else {
                    buff[res] = 0;
                    printf("recv  %d bytes: %s\n", res, buff);
                }
                
                res = ::recv(fd, buff, sizeof(buff), 0);
                if(res < 0)
                    printf("send error %d \n", errno);
                else {
                    buff[res] = 0;
                    printf("recv  %d bytes: %s\n", res, buff);
                }
                close(fd);


            }
        );
        ++i;
    }

    for(auto& t: _threads)
        t.join();



    return 0;
}