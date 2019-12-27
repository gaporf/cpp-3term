//
// Created by gaporf on 12.12.2019.
//

#include "socket_raii.h"

#include <sys/socket.h>
#include <unistd.h>

socket_raii::socket_raii() : fd(socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) {}

socket_raii::socket_raii(int server_socket) : fd(accept(server_socket, nullptr, nullptr)) {}

socket_raii::socket_raii(int timeout, itimerspec &ts) : fd(timerfd_create(CLOCK_MONOTONIC, 0)) {
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = timeout;
    ts.it_value.tv_nsec = 0;
}

socket_raii::~socket_raii() {
    if (fd != -1) {
        close(fd);
    }
}

bool socket_raii::operator==(const struct socket_raii &rhs) {
    return rhs.get_fd() == fd;
}

int socket_raii::get_fd() const {
    return fd;
}