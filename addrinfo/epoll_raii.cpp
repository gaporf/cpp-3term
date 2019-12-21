//
// Created by gaporf on 12.12.2019.
//

#include "epoll_raii.h"

#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <stdexcept>
#include <unistd.h>
#include <csignal>
#include <iostream>

epoll_raii::epoll_raii() : fd(epoll_create(1)) {
    if (fd == -1) {
        throw std::runtime_error("Could not create epoll");
    }
    sigset_t all;
    sigfillset(&all);
    if (sigprocmask(SIG_SETMASK, &all, nullptr) == -1) {
        throw std::runtime_error("Could not set all signal are ignored");
    }
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
        throw std::runtime_error("Could not set SIGINT and SIGTERM as blocked");
    }
    signal_fd = signalfd(-1, &mask, 0);
    if (signal_fd == -1) {
        throw std::runtime_error("Could not create a signal");
    }
    add_event(signal_fd);
}

epoll_raii::~epoll_raii() {
    if (fd != -1) {
        close(fd);
    }
}

void epoll_raii::add_server(server &srv) {
    int server_socket = srv.get_fd();
    add_event(server_socket);
    get_server[server_socket] = &srv;
    srv.set_epfd(fd);
}

void epoll_raii::execute() {
    for (;;) {
        const size_t K = 32;
        const size_t TIMEOUT = 60 * 1000;
        epoll_event events[K];
        int n = epoll_wait(fd, events, K, TIMEOUT);
        if (n < 0) {
            throw std::runtime_error("Could not get current events from epoll, the error is "
                                     + std::to_string(n));
        }
        for (int i = 0; i < n; i++) {
            int cur_socket = events[i].data.fd;
            if (cur_socket == signal_fd) {
                exit(0);
            } else if (get_server.find(cur_socket) != get_server.end()) {
                server *cur_server = get_server[cur_socket];
                if (cur_server->get_fd() == cur_socket) {
                    std::pair<int, int> new_socket_pair = cur_server->create_new_socket();
                    get_timer[new_socket_pair.second] = new_socket_pair.first;
                    add_event(new_socket_pair.second);
                    get_server[new_socket_pair.first] = cur_server;
                    add_event(new_socket_pair.first);
                } else {
                    epoll_ctl(fd, EPOLL_CTL_DEL, cur_server->get_timer_fd(cur_socket), nullptr);
                    cur_server->check_socket(cur_socket);
                }
            } else {
                int client_socket = get_timer[cur_socket];
                server *cur_server = get_server[client_socket];
                cur_server->kill_client(client_socket);
                epoll_ctl(fd, EPOLL_CTL_DEL, cur_socket, nullptr);
                epoll_ctl(fd, EPOLL_CTL_DEL, client_socket, nullptr);
                get_server.erase(get_server.find(client_socket));
                get_timer.erase(get_timer.find(cur_socket));
            }
        }
    }
}

void epoll_raii::add_event(int socket) {
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = socket;
    int res = epoll_ctl(fd, EPOLL_CTL_ADD, socket, &ev);
    if (res != 0) {
        throw std::runtime_error("Could not add event " + std::to_string(socket) + " to epoll, the error code is " + std::to_string(res));
    }
}