#pragma once

#include <sys/epoll.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>

#include <string>
#include <unordered_map>
#include <functional>
#include <iostream> // for debug out only

static const int MAX_EVENTS = 10;

struct tcp_server {
    int epollfd;
    epoll_event events[MAX_EVENTS];
    std::unordered_map<int, std::function<void(const char*, int)>> callbacks;
    tcp_server();
    void start();
    struct tcp_socket {
        void send_request(const std::string&);
        int sockfd;
        tcp_server & outer;
        epoll_event ev;

        tcp_socket(tcp_server& outer);
        tcp_socket(tcp_server&, std::string, uint16_t,
                   std::function<void(const char*, int)>);
        ~tcp_socket();
    };
};

