//
// Created by gaporf on 12.12.2019.
//

#ifndef ADDRINFO_SERVER_EPOLL_RAII_H
#define ADDRINFO_SERVER_EPOLL_RAII_H

#include <map>
#include <functional>

struct epoll_raii {
    epoll_raii();

    ~epoll_raii();

    void add_event(int socket, std::function<void()> *ptr);

    void delete_event(int socket);

    void execute();

private:

    int fd, signal_fd;
};


#endif //ADDRINFO_SERVER_EPOLL_RAII_H
