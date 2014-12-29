all:
	clang++ -O2 -DDEBUG -lgumbo -lboost_system -lboost_filesystem -Wall -pedantic -std=c++14 my_socket.cpp my_http_server.cpp mywget.cpp -o mywget
