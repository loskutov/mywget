#pragma once

#include <string>
#include <boost/filesystem.hpp>
#include <fstream>


#include "my_http_server.h"

std::string nametosave(const std::string & uri) {
    int slash = uri.find('/');
    if (slash == std::string::npos) {
        return uri + "/index.html";
    } else {
        std::string s = uri.substr();
        slash = s.find_last_of('/');
        if(slash != std::string::npos) {
            boost::filesystem::create_directories(s.substr(0, slash));
        }
        return s;
    }
}

class downloadable {
    std::ofstream file;
    http_server& serv;
    http_server::http_request req;
    std::string name;
    std::string location;
    public:
    std::string url;
    std::string data;
    bool ishtml = 0;
    char classofresponse = 0;
    auto request(const std::string & url, const std::string & header) {
        std::string host;
        std::string nameathost;
        int slash = url.find('/');
        if (slash == std::string::npos) {
            host = url;
            nameathost = "/";
        } else {
            host = url.substr(0, slash);
            nameathost = url.substr(slash);
        }
        return http_server::http_request(
            serv,
            //[this](const char* a, int n) {if(strstr(a, "text/html")) ishtml = true;},
            [this](const char* a, int n) {
                if (strstr(a, "text/html")) ishtml = true;
                const char* http = strstr(a, "HTTP/");
                if(http != nullptr && http + 9 < a + n) classofresponse = http[9];
                const char* loc = strstr(a, "Location: ");
                if(loc != nullptr) {
                    for(int i = 10; loc[i] != '\n' && loc + i < a + n; i++)
                        location += loc[i];
                }
                
            },
            [this,url](const char* a, int n) {
                if(classofresponse == '2') {
                    for (int i = 0; i < n; i++) {
                        if(ishtml)
                            data += a[i];
                        file << a[i];
                    }
                } else {
                    throw location;
                }
            },
            host.c_str(),
            ("GET " + nameathost).c_str(),
            header
        );
    }
    downloadable(http_server & serv, const std::string & url, const std::string & header = "") :
        file(nametosave(url)),
        serv(serv),
        req(request(url, header)),
        name(nametosave(url)),
        url(url) {}
    //downloadable(downloadable && rhs) : serv(rhs.serv), req(rhs.req), file(rhs.file){}
    virtual ~downloadable() {
        file.close();
    }

};
