#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <netinet/tcp.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cmath>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include "utils.hpp"

extern int sockfd;
extern char *id_client;

void generic_print(char *buff);
void run_tcp_client();
void setup_tcp_client();

typedef struct {
    uint8_t type_code;
    const char* type_name;
    size_t data_offset;
    void (*print_function)(const char* topic, const unsigned char* data_ptr);
} data_handler_t;


#endif