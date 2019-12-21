//
// Created by gaporf on 19.12.2019.
//

#include "addrinfo_raii.h"

addrinfo_raii::addrinfo_raii() : servinfo(nullptr) {}

addrinfo_raii::~addrinfo_raii() {
    free();
}

void addrinfo_raii::free() {
    if (servinfo != nullptr) {
        freeaddrinfo(servinfo);
    }
    servinfo = nullptr;
}

addrinfo *addrinfo_raii::get_addrinfo() {
    return servinfo;
}