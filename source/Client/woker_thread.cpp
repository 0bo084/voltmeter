#include "woker_thread.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "../command.hpp"

#include <unistd.h>
#include <fcntl.h>


static inline bool is_valid_fd(int fd)
{
    return (fcntl(fd, F_GETFL) != -1) || (errno != EBADF);
}

WokerThread::WokerThread(const std::string& _path, QObject *parent)
    : QThread(parent)
    , path(_path)
    , isDone(false)
    , queue(WokerThread::QueueSize)
{
    
}

WokerThread::~WokerThread()
{
    isDone.store(true);

    wait();
}


void WokerThread::runCommand(std::unique_ptr<voltio::command>&& cmd)
{
    queue.push(std::move(cmd));  
}


void WokerThread::run()
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

        emit sendFatalError( interThreadData("fatal error: cant connect to server"));

        return;
    }

    guard.detach();

 

    std::string ser;
    ser.reserve(512);


    
    while(!isDone.load())
    {

        std::unique_ptr<voltio::command> cmd;

        if(!queue.pop(cmd)) {
            
            std::this_thread::yield();
            continue;
        }    
        voltio::cmdFactory::make(ser,*cmd);   

        
        ret = send(fd, ser.c_str(), ser.length(), MSG_NOSIGNAL);
        if(ret <= 0) {
            emit sendFatalError( interThreadData("fatal error: connection lost"));
            return;
        }
 
        
        char buff[1024];
        ret = recv(fd, buff, sizeof(buff), 0);
        // static_cast here is not too dangerous because we cant get overflow here
        if((ret > 0)&&(ret < static_cast<int>(sizeof(buff)))) {
            buff[ret] = '\x0';
            emit sendResponse(interThreadData(buff));
        }
        else if (ret <= 0) {
            emit sendFatalError( interThreadData("fatal error: connection lost"));
            return;
        }
        else 
            emit sendResponse(interThreadData("Response too big: protocol error"));
    }
    
}
//![processing the image (finish)]

void WokerThread::stopProcess()
{
    isDone.store(true);
}

