/*
	voltmeter
*/

/*!
	Simple socket serverr based on poll()
*/

#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>

#include "voltio.hpp"
#include "queue.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>


namespace voltio {


/*!
	Thread safe queue for thread pool
*/


/*!
    epoll class to implement simple thread pool to works
    with epoll file descriptors
    class has CRTP idiom to avoid virtual methods (see receive_handler).
    receive_handler is the one method of CRTP interface to receive data
    TODO: make support to accept interface
*/
template<class T>
class epoll
{

    //struct sockaddr_un address;
    struct sockaddr_un address;
    int listenfd = -1, // listen socket fd
        epfd = -1;     //  epoll fd for listen-accept processing
        
    std::atomic<bool> isDone = false;
    std::string       sock_path;
    
    // to listen and accept new connections
    std::thread acceptor_thread;
    // to process clients request
    std::vector<std::thread> threads;

    using woker_queue = bounded_queue<int>;
    // this is queue for woker threads. we will pass
    // accepted socket using one queue per thread. 
    // we should use non removable conteiner, 
    // pointer to conteiner's data will be non changed
    // we initialize vector and dont change one alonj lifetime!!!
    // because we use reference on queue and pass it to worker threads  
    std::vector<woker_queue *> queues; 
    
    
    // static helpers
    inline static int setnonblocking(int fd)
    {
        int flags;
        if((flags = fcntl(fd, F_GETFD, 0)) < 0){
            printf("get falg error\n");
            return -1;
        }

        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) < 0) {
            printf("set nonblock fail\n");
            return -1;
        }
        return 0;
    }

    inline static
    int epoll_control(int epoolfd, int action, int events, int clfd) {
        
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = clfd;

        int r = epoll_ctl( epoolfd, action, clfd, &ev);
        return r;
    
    }

    inline static
    int add_to_epool(int epoolfd, int events, int clfd) {
        
        int r = epoll_control(epoolfd, EPOLL_CTL_ADD, events, clfd);
        
        return r;
    }

    inline static
    int delete_from_epool(int epoolfd, int clfd) {
        
        
        int r = epoll_control(epoolfd, EPOLL_CTL_DEL, 0, clfd);
        
        return r;
    }


    inline 
    int add_to_accept_epool(int events, int clfd) {

        return epoll::add_to_epool(epfd, events, clfd);
    }

    
protected:

    static constexpr std::size_t  Backlog = 1024;
    static constexpr std::size_t  TimeoutPollMS = 100;
    static constexpr std::size_t  MaxNumberOfEpollEvents = Backlog*2;
    static constexpr std::size_t  SizeOfRecvBuffer = 1024;

    

    // should run in ONLY THE ONE thread !!!
    void acceptor_loop() {

        struct epoll_event events[epoll::MaxNumberOfEpollEvents];

        while (!isDone.load()) {

            int ready = epoll_wait( epfd, events, epoll::MaxNumberOfEpollEvents, 10000);
            
            if (ready == 0) 
                /* timeout, no ready */
                continue;     
            
            else if (ready > 0){             
                for (int i = 0; i < ready; i++) {
                    if (events[i].data.fd == listenfd)
                        accept();  // do not need pass fd to accept, we can get once inside
                }
            }
            else if (ready == -1) {
                ready = errno;
                if ((ready != EAGAIN) && (ready != EINTR)){
                    perror("epoll_wait:");
                    return;
                }
            } 
        }
    }


    // Each woker loop has own epool on stack and own arayy of events on stack
    // acceptor loop pass acepted fd to the queue of the woker loop
    // woker loop takes fds adds it to the own epool  
    void woker_loop(woker_queue* queue) {

        struct epoll_event events[epoll::MaxNumberOfEpollEvents];
        
        int epollWokerFd = epoll_create(1);
        if (epollWokerFd < 0) {
            perror("epoll create");
            return;
        }

        fd_guard guard(epollWokerFd);


        int fd;
        while (!isDone.load()) {
            
            while( queue->pop(fd)) {
                int res = epoll::add_to_epool(epollWokerFd, EPOLLIN, fd);
                if (res < 0){
                    perror("epoll::add_to_epool:");
                    return;
                }
            }

            int ready = epoll_wait( epollWokerFd, events, epoll::MaxNumberOfEpollEvents, epoll::TimeoutPollMS);
            
            if (ready == 0) 
                /* timeout, no ready */
                continue;     
            
            else if (ready > 0){  

                          
                for (int i = 0; i < ready; i++) {
                    handle_message(epollWokerFd, events[i].data.fd);
                }
            }
            else if (ready == -1) {

                ready = errno;
                if ((ready != EAGAIN) && (ready != EINTR)){
                    perror("epoll_wait:");
                    return;
                }
            } 
        }
    }

    void handle_message(int epollfd, int fd)
    {
        char recvbuf[SizeOfRecvBuffer];
        int len;
        // we have enotht recv buffer to get all data at once.
        // if buffer to small (len == sizeof(recvbuf)) then close channel
        // because someone does against our protocol rules 
        len = recv(fd, recvbuf, sizeof(recvbuf), 0);
            
      
        if ((len == 0) || (len == sizeof(recvbuf))) { 
            // close connection    
            int res = delete_from_epool(epollfd, fd);
            if (res < 0){
                perror("delete_from_epool:");
                return;
            }
            res = close(fd);
            
            printf("[%d] closing connection... \n", fd); 
        } 
        else if (len < 0 && errno != EAGAIN) {
            
            printf("recv error:%s\n", strerror(errno));
        }
        else {
            printf("[%d]New data have received %d bytes\n", fd, len); 
            recvbuf[len] = '\x0';
            self()->receive_handler(fd, recvbuf, len);
        }
    }

    void accept()
    {
        int cliFd;
        struct sockaddr_in cliaddr;
        int socklen = sizeof(struct sockaddr_in);
        
        cliFd = ::accept( 
            listenfd, 
            reinterpret_cast<struct sockaddr*>(&cliaddr), 
            reinterpret_cast<socklen_t *>(&socklen)
            );
        if (cliFd < 0) {
            perror("accept");
            return;
        }

        if (setnonblocking(cliFd) < 0) {
            perror("setnonblocking error");
            return;
        } 

        /*!
            Use conter to pass socket to the each woker thread
            in sequence; 
        */
        static size_t counter = 0;
        bool isOk = queues[counter%queues.size()]->push(cliFd);
        if (!isOk) {
            perror("Cant process accepted socket");
            shutdown(cliFd, SHUT_RDWR); // close connection
            close(cliFd);
            return;
        } 
        ++counter;

        printf("New client connection [%d]\n", cliFd);

    }

    

    int create_bind_listen_socket(const std::string& path)
    {
        unlink(path.c_str());
        listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
        fd_guard guard_listen(listenfd);
        
        int ret;
        if ((ret = setnonblocking(listenfd)) < 0) {
            perror("setnonblock error");
            return ret;
        }

        memset (&address, 0, sizeof(address));
        address.sun_family = AF_UNIX;
        strcpy(address.sun_path, path.c_str());
    
        ret = bind (
            listenfd, 
            reinterpret_cast<const sockaddr*>(&address), 
            SUN_LEN(&address)
            );
        if (ret < 0) {
            perror("bind");
            return ret;
        }

        ret = listen (listenfd, epoll::Backlog);
            if (ret < 0) {
                perror("listen");
            return -1;
        }

        guard_listen.detach();

        return 0;
    }

protected:

    // CRTP support 
    T* self() { return static_cast<T* >(this); }
    
    // CRTP stub to override 
    void receive_handler(int, char *, int) {}

public:    
    
    epoll(const std::string& path)  // exception
        : sock_path(path)
    {       
        int ret = create_bind_listen_socket(path);
        if (ret < 0) {
            perror("create_bind_listen_socket");
            throw exception_t("create_bind_listen_socket error");
        }

        epfd = epoll_create(1);
        if (epfd < 0) {
            perror("epoll create");
            throw exception_t("epoll create error");
        }
        fd_guard guard_epoll(epfd);
        

        ret = add_to_accept_epool( EPOLLIN, listenfd);
        if (ret < 0) {
            perror("epoll_ctl");
            return;
        }
     
        guard_epoll.detach();
        
        // make the one thread for listen-accept connection
        acceptor_thread = std::thread(
            [this](){ 
                epoll::acceptor_loop();
            }
            );
        // make poll for processing clients reqs 
        int n = std::thread::hardware_concurrency();
        for (int i = 0; i < n; i++ ) {
            // create queue for woker thread
            woker_queue* q = new woker_queue(epoll::Backlog);
            queues.push_back(q);
            // create woker thread and pass ref on queue (to pass socket fd from listen-accept thead)             
            threads.emplace_back(
                //std::thread(
                [this, q]() {
                    epoll::woker_loop(q);
                }
                );
                //);
        }

    }

    ~epoll()
    {
        stop();

        if(acceptor_thread.joinable())
            acceptor_thread.join();
        printf("Acceptor thread has been stopped...\n");

        close(listenfd);
        close(epfd);

        for ( auto& t : threads)
            if(t.joinable())
                t.join();   
        printf("All woker threads have been stopped...\n");
        
        for ( woker_queue* q : queues)
            delete q;
        
        unlink(sock_path.c_str());
        printf("All woker queues have been closed...\n");
          
        printf("Epoll has been closed...\n");
    }

    void stop()
    {
        isDone.store(true);
    }

};
    


} // namespace voltio



 
 

 
 
