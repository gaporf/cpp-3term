//
// Created by gaporf on 12.12.2019.
//

#ifndef ADDRINFO_SERVER_SOCKET_RAII_H
#define ADDRINFO_SERVER_SOCKET_RAII_H


#include <cstdint>

struct socket_raii {
    socket_raii();

    explicit socket_raii(int server_socket);

    socket_raii(socket_raii const &rhs) = delete;

    ~socket_raii();

    bool operator==(socket_raii const &rhs);

    socket_raii &operator=(socket_raii const &rhs) = delete;

    [[nodiscard]] int get_fd() const;

private:
    int fd;
};


#endif //ADDRINFO_SERVER_SOCKET_RAII_H
