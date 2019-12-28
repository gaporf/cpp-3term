//
// Created by gaporf on 12.12.2019.
//

#include "epoll_raii.h"

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
        throw std::runtime_error("Could not set all signal as ignored");
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
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = signal_fd;
    if (epoll_ctl(fd, EPOLL_CTL_ADD, signal_fd, &ev) != 0) {
        throw std::runtime_error("Could not add signal to epoll");
    }
}

epoll_raii::~epoll_raii() {
    if (fd != -1) {
        close(fd);
    }
}

void epoll_raii::add_event(int socket, std::function<void()> *ptr, int name_of_event) {
    epoll_event ev{};
    ev.events = name_of_event;
    ev.data.ptr = ptr;
    int status = epoll_ctl(fd, EPOLL_CTL_ADD, socket, &ev);
    if (status != 0) {
        std::cout << "Could not add event with socket " << socket << std::endl;
    }
}

void epoll_raii::delete_event(int socket) {
    epoll_ctl(fd, EPOLL_CTL_DEL, socket, nullptr);
}

void epoll_raii::execute() {
    for (;;) {
        const size_t K = 32;
        const size_t TIMEOUT = 60 * 1000;
        epoll_event events[K];
        int n = epoll_wait(fd, events, K, TIMEOUT);
        if (n < 0) {
            std::cout << "Could not get current events from epoll, the error is " + std::to_string(n) << std::endl;
        }
        for (int i = 0; i < n; i++) {
            int cur_socket = events[i].data.fd;
            if (cur_socket == signal_fd) {
                exit(0);
            } else if (events[i].events) {
                auto *ptr = reinterpret_cast<std::function<void()> *>(events[i].data.ptr);
                (*ptr)();
            }
        }
    }
}