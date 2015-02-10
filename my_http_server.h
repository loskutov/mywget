#pragma once

#include "my_socket.h"
#include <functional>
#include <iostream>
#include <string>
#include <stdexcept>


class http_server
{

    public:
    class http_request
    {
    public:

        /**
         * @brief Almost default constructor
         * @param outer The corresponding HTTP server
         */
        http_request(http_server & outer);

        /**
         * @brief Another constructor
         * @param outer The corresponding HTTP server
         * @param head_callb The callback to call on HTTP response head
         * @param body_callb The callback to call on HTTP response body
         * @param domain The domain
         * @param request The HTTP request
         * @param headers The additional headers
         */
        http_request(http_server & outer,
                     const std::function<void(const char*, int)> & head_callb,
                     const std::function<void(const char*, int)> & body_callb,
                     const std::string& domain,
                     const std::string& request,
                     const std::string& headers = "");

    private:
        enum position {HEAD, BODY};
        std::function<void(const char*, int)> cb = [this](const char* a, int n)
        {
            std::string head;
            std::string body;
            char p1 = '\0';
            char p2 = '\0';
            char p3 = '\0';
            int beg = -1;
            if (pos == BODY) {
                body_callb(a, n);
            } else {
                for (int i = 0; i < n; i++) {
                    if ((p3 == '\r' && p2 == '\n' && p1 == '\r' && a[i] == '\n') // CRLF CRLF
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
                if (beg == -1)
                    head_callb(a, n);
                else if (beg != n)
                    body_callb(&a[beg], n - beg);
            }
        };
        std::function<void(const char*, int)> head_callb;
        std::function<void(const char*, int)> body_callb;
        http_server & outer; // can't access the outer class without it
        tcp_server::tcp_socket socket;
        position pos = HEAD;
    };

    /**
     * @brief Start the internal tcp server (which means start listening the epoll)
     */
    inline void run() {
        debug_out("starting tcpserv");
        tcpserv.start();
    }

    private:

    tcp_server tcpserv;
};
