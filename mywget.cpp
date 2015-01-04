#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <unordered_set>
#include <set>

#include "my_http_server.h"
#include "downloadable.cpp"

#include <sys/stat.h>
#include <signal.h>
#include "gumbo.h"

using namespace std;

unordered_set<string> dloaded;
void split(const string & s, char delim, list<string> & ans) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        ans.push_back(item);
    }
}

string unsplit(const list<string> & l) {
    if(l.empty())
        return "";
    auto it = l.begin();
    string s = *it;
    it++;
    for(;it != l.end(); it++) {
        s += '/' + *it;
    }
    return s;
}

string absolute_url(const string & url = "", const string & path = "") {
    if(url == "")
        return path;
    size_t slash = url.find_last_of('/');
    string s = url.substr(0, slash) + '/' + path;
    list<string> l;
    split(s, '/', l);
    if (!path.empty() && path[0] == '/') {
        return l.front() + path;
    }

    for (auto it = l.begin(); it != l.end();) {
        if (*it == "." || ((it == l.begin() || prev(it) == l.begin()) && *it == "..")) {
            it = l.erase(it);
        } else if (it != l.begin() && *it == "..") {
            it = l.erase(prev(it), next(it));
        } else it++;
    }
    return unsplit(l);
}

void my_handler(int sig) {
    switch(sig) {
        case SIGTERM:
            cerr << "\nSEGTERM caught; aborting\n";
            break;
        case SIGINT:
            cerr << "\nSIGINT caught; aborting\n";
            break;
    }
    exit(-1);
}


vector<string> search_for_links(GumboNode* node) {
    vector<string> ans;
    if (node->type != GUMBO_NODE_ELEMENT) {
        return ans;
    }
    GumboAttribute* href;
    if (node->v.element.tag == GUMBO_TAG_A &&
            (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        ans.push_back(href->value);
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        vector<string> v = search_for_links(static_cast<GumboNode*>(children->data[i]));
        ans.insert(ans.end(), v.begin(), v.end());
    }
    return ans;
}

void recget(int level, const vector<string> & urls, const string & parent = "") {
    if(level < 0)
        return;
    http_server serv;
    vector<downloadable *> ds;
    for(auto i : urls) {
        if(i.substr(0, 7) == "http://") {
            if(dloaded.find(i.substr(0, 7)) != dloaded.end())
                continue;
            cerr << "Gonna dl " << i.substr(7) << '\n';
            ds.push_back(new downloadable(serv, i.substr(7).c_str(), "Referer: " + i));
            dloaded.emplace(i.substr(7));
        } else if(i.substr(0, 2) == "//") {
            if(dloaded.find(i.substr(0, 2)) != dloaded.end())
                continue;
            cerr << "Gonna dl " << i.substr(2) << '\n';
            ds.push_back(new downloadable(serv, i.substr(2).c_str(), "Referer: " + i));
            dloaded.emplace(i.substr(2));
        } else if(i.find(':') != string::npos || (!i.empty() && i[0] == '#')) { // another proto used or a link to a part of this document
            cerr << "not gonna dl " << i << endl;
        } else {
            string s = absolute_url(parent, i);
            if(dloaded.find(s) != dloaded.end())
                continue;
            cerr << "don't know how to parse " << i << endl;
            cerr << "but i'll try.. maybe it's " << absolute_url(parent, i) << endl;
            ds.push_back(new downloadable(serv, s, "Referer: " + i));
            dloaded.emplace(s);
        }
    }
    vector<pair<string, string>> nextgen;
    try {
        serv.run();
    } catch(const string & e) {
        cout << "moved to " << e;
        vector<string> v {e};
        recget(level, v);
    }
    for(auto d : ds) {
        if(d -> ishtml)
            nextgen.emplace_back(d -> url, d -> data);
        delete d;
    }
    for(auto s : nextgen) {
        GumboOutput* output = gumbo_parse(s.second.c_str());
        recget(level - 1, search_for_links(output->root), s.first);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    }
}


int main(int argc, char* args[]) {
    //cout << absolute_url("tbp.karelia.ru/", "/img/foto");
    //return 0;
    // handle SIGINT and SIGTERM signals
    struct sigaction signal_handler;
    signal_handler.sa_handler = my_handler;
    sigemptyset(&signal_handler.sa_mask);
    signal_handler.sa_flags = 0;
    sigaction(SIGINT, &signal_handler, NULL);
    sigaction(SIGTERM, &signal_handler, NULL);

    // handle commandline options
    if (argc < 2) {
        cout << "No URL specified\n";
        cout << "USAGE: " << args[0] << " [OPTION] [URL]\n";
        return -2;
    }
    assert (argc >= 2);
    bool recursive = false;
    int level = 0;
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(args[i], "-R") == 0)
            recursive = true;
        else if (strncmp(args[i], "--level=", 8) == 0)
            level = atoi(args[i] + 8);
        else {
            cout << "Invalid option: " << args[i] << '\n';
            return 2;
        }
    }

    vector<string> urls;
    urls.push_back(args[argc - 1]);
    recget(1, urls);

    for(auto i : dloaded) {
        cout << "downloaded: " << i << endl;
    }

    return 0;
}
