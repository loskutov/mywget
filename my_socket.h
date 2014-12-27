#pragma once

#include <sys/epoll.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
struct tcp_socket_factory {
    struct tcp_socket {
        int sockfd;
        int epollfd;
        epoll_event ev;
        tcp_socket() {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sockfd < 0)
                throw "couldn't create socket";
        }

        tcp_socket(std::string url, uint16_t port) : tcp_socket() {
            hostent *server = gethostbyname(url.c_str());
            if (server == NULL)
                throw "couldn't figure out the host by its name";
            sockaddr_in serv_addr;
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port);

            bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

            if (connect(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                throw "couldn't connect";


            ev.events = EPOLLIN ;//| EPOLLPRI | EPOLLERR | EPOLLHUP;
            ev.data.fd = sockfd;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
            std::cout << "added!1\n";
        }
    };
};

