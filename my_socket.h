#pragma once

#include <sys/epoll.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>

#include <string>
#include <unordered_map>
#include <functional>

inline void debug_out(const char* a)
{
#ifdef DEBUG
    printf("\033[0;31m%s\033[0m", a);
#endif
}

static const int MAX_EVENTS = 10;

struct tcp_server
{
public:
    /**
     * @brief Default constructor
     */
    tcp_server();

    /**
     * @brief Start the corresponding epoll
     */
    void start();

    struct tcp_socket
    {
    public:

        /**
         * @brief Send a request
         * @param request Request body
         */
        void send_request(const std::string& request);

        /**
         * @brief Almost default constructor
         * @param outer The corresponding TCP server
         */
        tcp_socket(tcp_server& outer);

        /**
         * @brief Another constructor
         * @param outer The corresponding TCP server
         * @param url The URL
         * @param port The TCP port
         * @param cb The callback to call on the response
         */
        tcp_socket(tcp_server& outer, std::string url, uint16_t port,
                   std::function<void(const char*, int)> cb);
        ~tcp_socket();
    private:
        int sockfd;
        const tcp_server & outer; // can't access the outer class without it
    };
private:
    int epollfd;
    epoll_event events[MAX_EVENTS];
    std::unordered_map<int, std::function<void(const char*, int)>> callbacks;
};

