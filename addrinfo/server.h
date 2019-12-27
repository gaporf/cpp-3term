//
// Created by gaporf on 12.12.2019.
//

#ifndef ADDRINFO_SERVER_SERVER_H
#define ADDRINFO_SERVER_SERVER_H

#include "socket_raii.h"
#include "epoll_raii.h"

#include <map>
#include <list>

#include <cstdint>

#include <vector>
#include <queue>
#include <set>

#include <mutex>
#include <condition_variable>
#include <thread>

struct server {
    explicit server(uint16_t port, epoll_raii &e);

    ~server();

private:
    struct client {
        client(int socket, epoll_raii &epfd, std::list<client> *ptr_to_list);

        ~client();

        bool operator==(client const &rhs);

        void set_timer();

        void delete_timer();

        std::list<client> *ptr_to_list;
        epoll_raii &epfd;
        socket_raii fd;

        int const TIMEOUT = 60 * 10;
        itimerspec ts;
        bool is_waiting;
        socket_raii timer_fd;

        bool is_queued;
        std::optional<pid_t> pid;
        std::mutex m;
        std::queue<std::string> client_requests;
        std::function<void()> handle_client, kill_client;
    };

    epoll_raii &epfd;
    uint16_t port;
    socket_raii server_socket;

    std::list<client> list_of_clients;

    std::function<void()> add_new_socket;

    static std::vector<std::string> handle(std::string const &request);
};


#endif //ADDRINFO_SERVER_SERVER_H
