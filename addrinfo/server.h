//
// Created by gaporf on 12.12.2019.
//

#ifndef ADDRINFO_SERVER_SERVER_H
#define ADDRINFO_SERVER_SERVER_H

#include "socket_raii.h"
#include "timer_raii.h"

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
    explicit server(uint16_t port);

    ~server();

    [[nodiscard]] int get_fd() const;

    void check_socket(int socket);

    std::pair<int, int> create_new_socket();

    [[nodiscard]] int get_timer_fd(int socket);

    void kill_client(int socket);

    void set_epfd(int epfd);

private:
    struct client {
        explicit client(int socket);

        ~client();

        bool operator==(client const &rhs);

        [[nodiscard]] int get_fd() const;

        bool is_queued;
        std::optional<pid_t> get_pid;
        timer_raii timer_fd;
        socket_raii fd;
        std::queue<std::string> client_requests;
    };

    int epfd;
    uint16_t port;
    socket_raii server_socket;

    std::map<int, client *> link_to_client;
    std::list<client> list_of_clients;

    std::set<pthread_t> threads;
    std::condition_variable is_ready;
    std::mutex m;
    std::queue<client *> ready_clients;
    std::thread handle_clients;

    void add_request(int socket, std::string const &request);

    static std::vector<std::string> handle(std::string const &request);
};


#endif //ADDRINFO_SERVER_SERVER_H
