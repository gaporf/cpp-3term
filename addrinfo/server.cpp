//
// Created by gaporf on 12.12.2019.
//

#include "server.h"
#include "addrinfo_raii.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdexcept>
#include <netdb.h>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <iostream>
#include <csignal>
#include <zconf.h>

server::server(uint16_t port) : epfd(-1), port(port), server_socket(), is_ready(), m(), handle_clients([this] {
    for (;;) {
        std::unique_lock<std::mutex> lg(m);
        if (ready_clients.empty()) {
            is_ready.wait(lg);
        }
        while (!ready_clients.empty()) {
            client *cur_client = ready_clients.front();
            ready_clients.pop();
            if (cur_client->get_pid.has_value()) {
                threads.erase(threads.erase(cur_client->get_pid.value()));
                cur_client->get_pid.reset();
            }
            if (cur_client->client_requests.empty()) {
                cur_client->is_queued = false;
                cur_client->timer_fd.set_timer(epfd);
            } else {
                std::mutex &cur_mutex = m;
                std::condition_variable &cur_condition = is_ready;
                std::queue<client *> &cur_queue = ready_clients;
                std::string &request = cur_client->client_requests.front();
                cur_client->client_requests.pop();
                std::optional<pid_t> &cur_pid = cur_client->get_pid;
                std::thread th([&cur_mutex, &cur_condition, &cur_queue, cur_client, request, &cur_pid] {
                    cur_pid.emplace(getpid());
                    std::vector<std::string> ans = handle(request);
                    for (std::string &str : ans) {
                        int res = send(cur_client->get_fd(), str.c_str(), str.length(), 0);
                        if (res < 0) {
                            std::cout << "Could not send data to client " << cur_client->get_fd() << std::endl;
                        }
                    }
                    std::unique_lock<std::mutex> lg(cur_mutex);
                    cur_queue.push(cur_client);
                    if (cur_queue.size() == 1) {
                        cur_condition.notify_one();
                    }
                });
                th.detach();
            }
        }
    }
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

}

server::~server() {
    if (epfd != -1 && server_socket.get_fd() != -1) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, server_socket.get_fd(), nullptr);
    }
    while (!threads.empty()) {
        pid_t a = *threads.begin();
        threads.erase(threads.begin());
        kill(a, SIGKILL);
    }
}

int server::get_fd() const {
    return server_socket.get_fd();
}

void server::check_socket(int socket) {
    char buf[1024];
    int rc = recv(socket, buf, sizeof(buf), 0);
    if (rc < 0) {
        std::cout << "Could not read from socket " << socket << std::endl;
    }
    std::string request(buf, rc - 2);
    add_request(socket, request);
}

std::pair<int, int> server::create_new_socket() {
    list_of_clients.emplace_back(server_socket.get_fd());
    link_to_client[list_of_clients.back().get_fd()] = &list_of_clients.back();
    return {list_of_clients.back().get_fd(), list_of_clients.back().timer_fd.get_fd()};
}

int server::get_timer_fd(int socket) {
    client *c = link_to_client[socket];
    return c->timer_fd.get_fd();
}

void server::kill_client(int socket) {
    client *cur_client = link_to_client[socket];
    list_of_clients.remove(*cur_client);
    link_to_client.erase(link_to_client.find(socket));
}

void server::set_epfd(int new_epfd) {
    epfd = new_epfd;
}

server::client::client(int socket) : is_queued(false), timer_fd(), fd(socket) {}

server::client::~client() = default;

bool server::client::operator==(const struct server::client &rhs) {
    return fd == rhs.fd;
}

int server::client::get_fd() const {
    return fd.get_fd();
}

void server::add_request(int socket, std::string const &request) {
    client *cur_client = link_to_client[socket];
    std::unique_lock<std::mutex> lg(m);
    cur_client->client_requests.push(request);
    if (!cur_client->is_queued) {
        ready_clients.push(cur_client);
        cur_client->is_queued = true;
    }
    lg.unlock();
    if (ready_clients.size() == 1) {
        is_ready.notify_one();
    }
}

std::vector<std::string> server::handle(const std::string &request) {
    int status;
    addrinfo hints{};
    memset(&hints, 0, sizeof(addrinfo));
    addrinfo_raii server_info;
    auto servinfo = server_info.get_addrinfo();
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    status = getaddrinfo(request.c_str(), "http", &hints, &servinfo);
    if (status != 0) {
        if (status != -2) {
            return {"There is a mistake while getting information about website " + request + ", the error code is " + std::to_string(status) + "\n"};
        } else {
            return {"Incorrect address of website " + request + '\n'};
        }
    }
    std::vector<std::string> ans;
    ans.emplace_back("The IP addresses for " + request + '\n');
    for (auto p = servinfo; p != nullptr; p = p->ai_next) {
        char buf[1024];
        inet_ntop(p->ai_family, &(reinterpret_cast<sockaddr_in *>(p->ai_addr)->sin_addr),
                  buf, sizeof(buf));
        std::string address(buf);
        address.append("\n");
        ans.emplace_back(address);
    }
    server_info.free();
    return ans;
}