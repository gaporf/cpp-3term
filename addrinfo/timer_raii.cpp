//
// Created by gaporf on 20.12.2019.
//

#include "timer_raii.h"

#include <sys/timerfd.h>
#include <stdexcept>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>

timer_raii::timer_raii() : ts{} {
    fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (fd < 0) {
        std::cout << "Could not create timer, the error code is " + std::to_string(fd) << std::endl;
    }
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = TIMEOUT;
    ts.it_value.tv_nsec = 0;
    set_timer();
}

timer_raii::~timer_raii() {
    if (fd != -1) {
        close(fd);
    }
}

int timer_raii::get_fd() const {
    return fd;
}

void timer_raii::set_timer(int epfd) {
    set_timer();
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    int res = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    if (res != 0) {
        std::cout << "Could not add timer to epoll with socket " << fd << std::endl;
    }
}

void timer_raii::set_timer() {
    int res = timerfd_settime(fd, 0, &ts, nullptr);
    if (res != 0) {
        std::cout << "Could not set timer, the error code is " + std::to_string(res) << std::endl;
    }
}