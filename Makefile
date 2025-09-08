CC=g++
CFLAGS=-Wall -Werror -Wno-error=unused-variable -g

build: server subscriber

server: server.cpp utils.cpp
	$(CC) -o $@ $^ $(CFLAGS)

subscriber: tcp_client.cpp utils.cpp
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf server subscriber