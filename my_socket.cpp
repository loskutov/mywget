#include "my_socket.h"
tcp_server::tcp_server() {
    epollfd = epoll_create(MAX_EVENTS);
}

void tcp_server::start() {
    int n = epoll_wait(epollfd, events, MAX_EVENTS, TIMEOUT);
    if(n == -1) {
        throw std::runtime_error("epoll_wait returned -1");
    }
    //TODO complete
}

tcp_server::tcp_socket::tcp_socket(tcp_server & outer) : outer(outer) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        throw std::runtime_error("couldn't create socket");
}

tcp_server::tcp_socket::tcp_socket(tcp_server& outer,
           std::string url,
           uint16_t port,
           std::function<void(const char* buffer, int size)> cb)
               : tcp_socket(outer) {
    hostent *server = gethostbyname(url.c_str());
    if (server == NULL)
        throw std::runtime_error("couldn't figure out the host by its name");
    sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    if (connect(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        throw "couldn't connect";
    ev.events = EPOLLIN ;//| EPOLLPRI | EPOLLERR | EPOLLHUP;
    ev.data.fd = sockfd;
    epoll_ctl(outer.epollfd, EPOLL_CTL_ADD, sockfd, &ev);
    outer.callbacks[sockfd] = cb;
    std::cout << "added!1\n";
}

void tcp_server::tcp_socket::send_request(const std::string & request) {
    //TODO write the code lol
    send(sockfd, request.c_str(), request.size(), 0);
}

/* struct tcp_socket {
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
*/
