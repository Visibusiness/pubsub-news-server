#include "tcp_client.hpp"
int sockfd;
char *id_client;

void print_int(const char* topic, const unsigned char* data_ptr) {
    int8_t sign = *(const int8_t*)data_ptr;
    int32_t nr_network = *(const uint32_t*)(data_ptr + sizeof(int8_t));
    int32_t nr_host = ntohl(nr_network);
    printf("%s - INT - %d\n", topic, sign ? -nr_host : nr_host);
}

void print_short_real(const char* topic, const unsigned char* data_ptr) {
    uint16_t nr_network = *(const uint16_t*)data_ptr;
    float nr_host = (float)ntohs(nr_network) / 100.0f;
    printf("%s - SHORT_REAL - %.2f\n", topic, nr_host);
}

void print_float(const char* topic, const unsigned char* data_ptr) {
    int8_t sign = *(const int8_t*)data_ptr;
    int32_t nr_network = *(const uint32_t*)(data_ptr + sizeof(int8_t));
    int32_t nr_host = ntohl(nr_network);
    int8_t pow10 = *(const int8_t*)(data_ptr + sizeof(int8_t) + sizeof(uint32_t));
    printf("%s - FLOAT - %.6f\n", topic, (float)(sign ? -nr_host : nr_host) / pow(10, pow10));
}

void print_string(const char* topic, const unsigned char* data_ptr) {
    printf("%s - STRING - %s\n", topic, (const char*)data_ptr);
}

void generic_print(char *buff) {
    char topic[51];
    strncpy(topic, buff, 50);
    topic[50] = '\0';

    buff += 50;
    uint8_t type = *buff;
    buff += sizeof(uint8_t);

    static const data_handler_t handlers[] = {
        {0, "INT", sizeof(int8_t), print_int},
        {1, "SHORT_REAL", 0, print_short_real},
        {2, "FLOAT", sizeof(int8_t) + sizeof(uint32_t), print_float},
        {3, "STRING", 0, print_string}
    };
    size_t num_handlers = 4;

    for (size_t i = 0; i < num_handlers; ++i) {
        if (handlers[i].type_code == type) {
            handlers[i].print_function(topic, (const unsigned char*)buff);
            return;
        }
    }
}

void run_tcp_client(){

    fd_set read_fds, temp_fds;
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);
    while(true){
        temp_fds = read_fds;
        
        select(sockfd + 1, &temp_fds, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &temp_fds)) {
            int len;
            char buff[2048];

            int rc = recv_all(sockfd, &len, sizeof(len));
            if (!rc)
                return;

            recv_all(sockfd, buff, len);

            generic_print(buff);
        }
    
        if (FD_ISSET(STDIN_FILENO, &temp_fds)) {
            std::string input;
            std::getline(std::cin, input);
            std::istringstream iss(input);
            std::string cmd, topic;
            iss >> cmd;
            tcp_request_t request;
            strcpy(request.id, id_client);
            if(cmd == "exit"){
                request.type = EXIT;
                send_all(sockfd, &request, sizeof(request));
                return;
            }
            if(cmd == "subscribe"){
                iss >> topic;
                strcpy(request.topic, topic.c_str());
                request.type = SUBSCRIBE;
                send_all(sockfd, &request, sizeof(request));
                printf("Subscribed to topic.\n");
                continue;
            }
            if(cmd == "unsubscribe"){
                iss >> topic;
                strcpy(request.topic, topic.c_str());
                request.type = UNSUBSCRIBE;
                send_all(sockfd, &request, sizeof(request));
                printf("Unsubscibed to topic.\n");
                continue;
            }
        }
    
    }
}
void setup_tcp_client(int argc, char *argv[]){
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    uint16_t port_server = atoi(argv[3]);
    id_client = argv[1];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_server);
    inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);

    int enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, TCP_NODELAY, &enable, sizeof(int));
    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    tcp_request_t request;
    strcpy(request.id, id_client);
    request.ip_server = serv_addr.sin_addr.s_addr;
    request.port_server = port_server;
    
    request.type = CONNECT;
    send_all(sockfd, &request, sizeof(request));
}

int main(int argc, char *argv[]){

    if (argc != 4) {
        std::cerr << "Usage: ./subscriber <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n";
        return 1;
    }

    setup_tcp_client(argc, argv);
    run_tcp_client();
    return 0;
}