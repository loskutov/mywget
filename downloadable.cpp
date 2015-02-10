#pragma once

#include <string>
#include <boost/filesystem.hpp>
#include <boost/range/algorithm/search.hpp>
#include <boost/range/as_literal.hpp>
#include <fstream>


#include "my_http_server.h"
using namespace std;

std::string nametosave(const std::string & uri)
{
    int slash = uri.find('/');
    if (slash >= uri.size() - 1) {
        boost::filesystem::create_directories(uri);
        return uri + "/index.html";
    } else {
        std::string s = uri;
        slash = s.find_last_of('/');
        if (slash != std::string::npos) {
            if(!boost::filesystem::exists(s.substr(0, slash))) 
                boost::filesystem::create_directories(s.substr(0, slash));
        }
        debug_out(("gonna save to " + s + '\n').c_str());
        return s;
    }
}

class downloadable
{
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
        std::cerr << "GET " << nameathost << std::endl;
        return http_server::http_request(
            serv,
            //[this](const char* a, int n) {if(strstr(a, "text/html")) ishtml = true;},
            [this](const char* a, int n) {
                if (boost::search(make_pair(a, a + n), boost::as_literal("text/html")) != a + n)
                    ishtml = true;
                auto http = boost::search(make_pair(a, a + n), boost::as_literal("HTTP/"));
                if (http != a + n && http + 9 < a + n)
                    classofresponse = http[9] - '0';
                auto loc = boost::search(make_pair(a, a + n), boost::as_literal("Location: "));
                if (loc != a + n) {
                    for(int i = 10; loc[i] != '\n' && loc + i < a + n; i++)
                        location += loc[i];
                }
                
            },
            [this, url](const char* a, int n) {
                switch (classofresponse) {
                case 2:
                //case 3:
                //if (true || classofresponse == 2) { // FIXME: not workin w/o 'true' O_o
                    if (ishtml)
                        data.insert(data.end(), a, a + n);
                    if (classofresponse != 2) {
                        debug_out(("class=" + to_string(classofresponse)).c_str());
                    }
                    //std::cout << a[i];
                    file.write(a, n);
                    break;


                //} else if (classofresponse == 3) { // Moved
                case 3:
                    debug_out(("Location=" + location + '\n').c_str());
                    break;
                //}
                default:
                    std::cerr << "class is" << classofresponse << std::endl;
                    break;
                }
            },
            host.c_str(),
            ("GET " + nameathost).c_str(),
            header
        );
    }
    downloadable(http_server & serv, const std::string & url,
                 const std::string & header = "") :
        serv(serv),
        req(request(url, header)),
        url(url) {
            name = nametosave(url);
            file.open(name.c_str());
        }

};
