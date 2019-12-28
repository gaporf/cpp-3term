//
// Created by gaporf on 12.12.2019.
//

#include "server.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <netinet/in.h>
#include <stdexcept>
#include <netdb.h>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <iostream>
#include <csignal>
#include <zconf.h>

server::server(uint16_t port, epoll_raii &e) : epfd(e),
                                               port(port),
                                               server_socket(),
                                               add_new_socket([this]() {
                                                   list_of_clients.emplace_back(server_socket.get_fd(), epfd, &list_of_clients);
                                               }) {
    if (server_socket.get_fd() == -1) {
        throw std::runtime_error("Could not create a socket, the server with port " + std::to_string(port) + " wasn't created");
    }
    sockaddr_in server_address{};
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = 0;
    server_address.sin_family = AF_INET;
    for (;;) {
        int res = bind(server_socket.get_fd(), reinterpret_cast<sockaddr const *>(&server_address), sizeof(server_address));
        if (res != 0) {
            port++;
            server_address.sin_port = htons(port);
        } else {
            std::cout << "OK, the server has port " << port << std::endl;
            break;
        }
    }
    int res = listen(server_socket.get_fd(), SOMAXCONN);
    if (res != 0) {
        throw std::runtime_error("Could not listen, the server with port " + std::to_string(port) +
                                 " wasn't created, the error code is " + std::to_string(res));
    }
    epfd.add_event(server_socket.get_fd(), &add_new_socket);
}

server::~server() {
    if (server_socket.get_fd() != -1) {
        epfd.delete_event(server_socket.get_fd());
    }
}

server::client::client(int socket, epoll_raii &epfd, std::list<client> *ptr_to_list) :
        ptr_to_list(ptr_to_list),
        epfd(epfd),
        fd(socket),
        ts(),
        is_waiting(false),
        timer_fd(TIMEOUT, ts),
        handle_client([this]() {
            char buf[1024];
            int rc = recv(fd.get_fd(), buf, sizeof(buf), 0);
            if (rc < 0) {
                std::cout << "Could not get information by socket " << fd.get_fd();
            }
            if (strncmp(buf, "exit", 4) == 0) {
                kill_client();
                return;
            }
            std::string request(buf, rc - 2);
            std::unique_lock<std::mutex> lg(m);
            client_requests.push(request);
            if (!pid.has_value()) {
                delete_timer();
                std::thread th([this] {
                    pid.emplace(getpid());
                    for (;;) {
                        std::unique_lock<std::mutex> lg(m);
                        if (client_requests.empty()) {
                            pid.reset();
                            set_timer();
                            break;
                        } else {    
                            std::string request = client_requests.front();
                            client_requests.pop();
                            lg.unlock();
                            std::vector<std::string> ans = handle(request);
                            for (std::string &str : ans) {
                                int status = send(fd.get_fd(), str.c_str(),
                                                  str.length(), 0);
                                if (status < 0) {
                                    std::cout << "Could not send information to socket " << fd.get_fd() << std::endl;

                                }
                            }
                        }
                    }
                });
                th.detach();
            }
        }),
        kill_client([this]() {
            this->ptr_to_list->remove(*this);
        }) {
    epfd.add_event(fd.get_fd(), &handle_client);
    set_timer();
}

server::client::~client() {
    if (fd.get_fd() != -1) {
        this->epfd.delete_event(fd.get_fd());
    }
    std::unique_lock<std::mutex> lg(m);
    if (is_waiting) {
        delete_timer();
    }
    if (pid.has_value()) {
        kill(pid.value(), SIGTERM);
    }
}

bool server::client::operator==(const struct server::client &rhs) {
    return fd == rhs.fd;
}

void server::client::set_timer() {
    int status = timerfd_settime(timer_fd.get_fd(), 0, &ts, nullptr);
    if (status != 0) {
        std::cout << "Could not set timer" << std::endl;
    }
    is_waiting = true;
    epfd.add_event(timer_fd.get_fd(), &kill_client);
}

void server::client::delete_timer() {
    if (is_waiting) {
        epfd.delete_event(timer_fd.get_fd());
    }
    is_waiting = false;
}

std::vector<std::string> server::handle(const std::string &request) {
    addrinfo hints{};
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    addrinfo *server_info;
    int status = getaddrinfo(request.c_str(), "http", &hints, &server_info);
    if (status != 0) {
        if (status != -2) {
            return {"There is a mistake while getting information about website " + request + ", the error code is " + std::to_string(status) +
                    "\n"};
        } else {
            return {"Incorrect address of website " + request + '\n'};
        }
    }
    std::vector<std::string> ans;
    ans.emplace_back("The IP addresses for " + request + '\n');
    for (auto p = server_info; p != nullptr; p = p->ai_next) {
        char buf[1024];
        inet_ntop(p->ai_family, &(reinterpret_cast<sockaddr_in *>(p->ai_addr)->sin_addr),
                  buf, sizeof(buf));
        std::string address(buf);
        address.append("\n");
        ans.emplace_back(address);
    }
    freeaddrinfo(server_info);
    return ans;
}