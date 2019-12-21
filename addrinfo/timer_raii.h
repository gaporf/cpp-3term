//
// Created by gaporf on 20.12.2019.
//

#ifndef ADDRINFO_SERVER_TIMER_RAII_H
#define ADDRINFO_SERVER_TIMER_RAII_H


#include <sys/timerfd.h>

struct timer_raii {
    timer_raii();

    ~timer_raii();

    timer_raii(timer_raii const &rhs) = delete;

    timer_raii &operator=(timer_raii const &rhs) = delete;

    [[nodiscard]] int get_fd() const;

    void set_timer(int epfd);

private:
    void set_timer();

    int const TIMEOUT = 60 * 10;

    itimerspec ts{};
    int fd;
};


#endif //ADDRINFO_SERVER_TIMER_RAII_H
