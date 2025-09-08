#include <cstdint>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int send_all(int sockfd, void *buff, int len);

int recv_all(int sockfd, void *buff, int len);

enum request_type {
    EXIT,
    SUBSCRIBE,
    UNSUBSCRIBE,
    CONNECT
};

typedef struct tcp_request_t {
    char id[11];
    in_addr_t ip_server;
    uint16_t port_server;
    char topic[51];
    request_type type;
} tcp_request_t;

typedef struct client_data_t {
    std::string id;
    int fd;
    bool connected;
} client_data_t;
