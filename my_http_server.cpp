#include <functional>
#include <iostream>
#include <string>
#include "my_socket.h"
#include "my_http_server.h"


http_server::http_request::http_request(http_server & outer) : outer(outer), socket(outer.tcpserv) {}
http_server::http_request::http_request(http_server & outer,
                     const std::function<void(const char*, int)> & head_callb,
                     const std::function<void(const char*, int)> & body_callb,
                     const std::string& domain,
                     const std::string& request,
                     const std::string& headers) : outer(outer), socket(outer.tcpserv, domain, 80, cb)
{
    socket.send_request(request + " HTTP/1.0\nHost: " + domain + "\n" + headers + "\n\n");
    this -> head_callb = head_callb;
    this -> body_callb = body_callb;
}

