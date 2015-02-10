all:
	clang++ -O0 -DDEBUG -lgumbo -lboost_system -lboost_filesystem -Wall -pedantic -std=c++14 my_socket.cpp my_http_server.cpp mywget.cpp -o mywget -g
clean:
	rm -rf crap/*
demo:
	clang++ -O2 -DDEBUG --std=c++14 my_socket.cpp demo.cpp -o demo
demo-nodebug:
	clang++ -O2 --std=c++14 my_socket.cpp demo.cpp -o demo
