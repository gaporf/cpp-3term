//
// Created by gaporf on 19.12.2019.
//

#ifndef ADDRINFO_SERVER_ADDRINFO_RAII_H
#define ADDRINFO_SERVER_ADDRINFO_RAII_H


#include <netdb.h>

struct addrinfo_raii {
    addrinfo_raii();

    ~addrinfo_raii();

    void free();

    addrinfo *get_addrinfo();

private:
    addrinfo *servinfo;
};


#endif //ADDRINFO_SERVER_ADDRINFO_RAII_H
