all: subscriber server

subscriber: subscriber.cpp
	g++ -Wall subscriber.cpp -o subscriber

server : server.cpp
	g++ -Wall server.cpp -o server

clean: 
	rm -rf subscriber
