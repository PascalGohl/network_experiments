
server: server.o
		g++ server.o -pg -o server -lboost_system -lboost_thread -lboost_system -lpthread

client: client.o
		g++ client.o -o client -lboost_system -lboost_thread -lboost_system -lpthread
		
all: client server