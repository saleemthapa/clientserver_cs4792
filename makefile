CC := g++
CFLAGS := -std=c++11 -Wall

all: server client
server: server.cpp
	$(CC) $(CFLAGS) -o server server.cpp -lpthread
client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp -lpthread 
clean:
	rm -f server client

