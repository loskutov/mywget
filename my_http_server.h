#pragma once

#include "my_socket.h"
#include <functional>
#include <iostream>
#include <string>
#include <stdexcept>


class http_server {
    enum position {HEAD, BODY};

    tcp_socket_factory tcpserv;
    public:
    class http_request {
        std::function<void(const char*, int)> cb = [this](const char* a, int n) {
            std::string head;
            std::string body;
            char p1 = '\0';
            char p2 = '\0';
            char p3 = '\0';
            int beg = -1;
            if(pos == BODY) {
                body_callb(a, n);
            } else {
                for(int i = 0; i < n; i++) {
                    if((p3 == '\r' && p2 == '\n' && p1 == '\r' && a[i] == '\n') // CRLF CRLF
                    || (p1 == '\n' && a[i] == '\n')) { // LF LF
                        pos = BODY;
                        beg = i + 1;
                        head_callb(a, beg);
                        break;
                    }
                    p3 = p2;
                    p2 = p1;
                    p1 = a[i];
                }
                if(beg == -1)
                    head_callb(a, n);
                else if(beg != n) {
                    body_callb(&a[beg], n - beg);
                }
            }
        };
        std::function<void(const char*, int)> head_callb;
        std::function<void(const char*, int)> body_callb;
        http_server & outer; // can't access the outer class without it
        tcp_socket_factory::tcp_socket socket;
        public:
        http_request(http_server & outer);
        http_request(http_server & outer,
                     std::function<void(const char*, int)> head_callb,
                     std::function<void(const char*, int)> body_callb,
                     std::string domain,
                     std::string request,
                     std::string headers = "");
        position pos = HEAD;
    };
    void run();
};
