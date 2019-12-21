//
// Created by gaporf on 12.12.2019.
//

#ifndef ADDRINFO_SERVER_EPOLL_RAII_H
#define ADDRINFO_SERVER_EPOLL_RAII_H

#include <map>

#include "server.h"
#include "socket_raii.h"

struct epoll_raii {
    epoll_raii();

    ~epoll_raii();

    void add_server(server &srv);

    void execute();

private:
    void add_event(int socket);

    int fd, signal_fd;
    std::map<int, server *> get_server;
    std::map<int, int> get_timer;
};


#endif //ADDRINFO_SERVER_EPOLL_RAII_H
