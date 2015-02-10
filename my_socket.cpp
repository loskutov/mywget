#include <assert.h>
#include <unistd.h>
#include <iostream>
#include "my_socket.h"

static const int TIMEOUT = 2'000;

tcp_server::tcp_server()
{
    epollfd = epoll_create(MAX_EVENTS);
}

void tcp_server::start()
{
    int n;
    char buffer[1500];
    ssize_t size;
    do {
        n = epoll_wait(epollfd, events, MAX_EVENTS, TIMEOUT);
        if (n == -1) {
            throw std::runtime_error("epoll_wait returned -1");
        }
        assert (n >= 0);
        if (n == 0) {
            debug_out("No more data in epoll\n");
        }
        for (int i = 0; i < n; i++) {
            do {
                size = read(events[i].data.fd, buffer, 1500 - 1);
                //for (int i = 0; i < size; i++) putchar(buffer[i]);
                debug_out("callin da callback\n");
                callbacks[events[i].data.fd](buffer, size);
            } while (size > 0);
        }
    } while (n > 0);
    debug_out("started tcp server\n");
    //TODO complete
}

tcp_server::tcp_socket::tcp_socket(tcp_server & outer) : outer(outer)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        throw std::runtime_error("couldn't create socket");
}

tcp_server::tcp_socket::tcp_socket(tcp_server& outer,
           std::string url,
           uint16_t port,
           std::function<void(const char* buffer, int size)> cb)
               : tcp_socket(outer) {
    hostent *server = gethostbyname(url.c_str());
    if (server == nullptr)
        throw std::runtime_error("couldn't figure out the host by its name");
    sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    if (connect(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        throw std::runtime_error("couldn't connect");
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;//| EPOLLPRI | EPOLLERR | EPOLLHUP;
    ev.data.fd = sockfd;
    int ii = epoll_ctl(outer.epollfd, EPOLL_CTL_ADD, sockfd, &ev);
    assert(ii == 0);
    debug_out("socket added to epoll\n");
    outer.callbacks[sockfd] = cb;
}

tcp_server::tcp_socket::~tcp_socket() {
    int n = close(sockfd);
    assert(n == 0);
}

void tcp_server::tcp_socket::send_request(const std::string & request) {
    write(sockfd, request.c_str(), request.size());
    //TODO write some other code
}

