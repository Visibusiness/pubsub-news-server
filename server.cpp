#include "server.hpp"
#include "utils.hpp"
int tcp_sock_fd;
int udp_sock_fd;
fd_set all_fd;
fd_set read_fds;

int fd_max;

std::map<std::string, client_data_t *> clients;
std::map<std::string, std::vector<client_data_t *>> topics_to_clients;

std::vector<std::string> split_topic(const std::string& topic) {
    std::vector<std::string> tokens;
    std::stringstream ss(topic);
    std::string segment;

    while (std::getline(ss, segment, '/')) {
        tokens.push_back(segment);
    }

    return tokens;
}

bool topic_matches(const std::vector<std::string>& topic, const std::vector<std::string>& pattern) {
    size_t topicIndex = 0;
    size_t patternIndex = 0;

    while (topicIndex < topic.size() && patternIndex < pattern.size()) {
        const std::string& pat = pattern[patternIndex];

        if (pat == "+") {
            topicIndex++;
            patternIndex++;
        } else if (pat == "*") {
            if (patternIndex == pattern.size() - 1)
                return true;

            patternIndex++;
            while (topicIndex < topic.size() && topic[topicIndex] != pattern[patternIndex]) {
                topicIndex++;
            }
            if (topicIndex == topic.size())
                return false;

            topicIndex++;
            patternIndex++;
        } else {
            if (topic[topicIndex] != pat)
                return false;
            topicIndex++;
            patternIndex++;
        }
    }

    return topicIndex == topic.size() && patternIndex == pattern.size();
}

void init_server() {
    FD_ZERO(&all_fd);
    FD_SET(STDIN_FILENO, &all_fd);
    FD_SET(tcp_sock_fd, &all_fd);
    FD_SET(udp_sock_fd, &all_fd);
    fd_max = std::max(tcp_sock_fd, udp_sock_fd);
}

void setup_server(uint16_t port_server) {
    tcp_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    udp_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_server);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bind(tcp_sock_fd, (const sockaddr *)&serv_addr, sizeof(serv_addr));
    bind(udp_sock_fd, (const sockaddr *)&serv_addr, sizeof(serv_addr));

    listen(tcp_sock_fd, 10000);
}

void handle_tcp() {
    struct sockaddr_in tcp_cli_addr;
    socklen_t tcp_cli_len = sizeof(tcp_cli_addr);
    int tcp_client_fd = accept(tcp_sock_fd, (struct sockaddr *)&tcp_cli_addr, &tcp_cli_len);
    int enable = 1;
    setsockopt(tcp_client_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    setsockopt(tcp_client_fd, SOL_SOCKET, TCP_NODELAY, &enable, sizeof(int));
    FD_SET(tcp_client_fd, &all_fd);
    fd_max = std::max(fd_max, tcp_client_fd);
}

void handle_udp() {
    char buffer[2048];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char message_topic[51];

    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recvfrom(udp_sock_fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_len);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';

        size_t topic_len = std::min((size_t)bytes_received, sizeof(message_topic) - 1);
        strncpy(message_topic, buffer, topic_len);
        message_topic[topic_len] = '\0';

        std::set<client_data_t *> unique_clients;

        for (auto const& [topic_pattern, list_clients] : topics_to_clients) {
            if (topic_matches(split_topic(std::string(message_topic)), split_topic(topic_pattern))) {
                for (auto const& client : list_clients) {
                    if (client->connected) {
                        unique_clients.insert(client);
                    }
                }
            }
        }

        for (auto const& client : unique_clients) {
            send_all(client->fd, &bytes_received, sizeof(bytes_received));
            send_all(client->fd, buffer, bytes_received);
        }
    }
}

void handle_connect(int fd, const tcp_request_t &request) {
    if (clients.find(std::string(request.id)) == clients.end()) {
        printf("New client %s connected from %s:%hu.\n",
               request.id,
               inet_ntoa(*(struct in_addr *)&request.ip_server),
               request.port_server);
        client_data_t *new_client = new client_data_t;
        new_client->connected = true;
        new_client->id = std::string(request.id);
        new_client->fd = fd;
        clients[new_client->id] = new_client;
        return;
    }

    client_data_t *client = clients[std::string(request.id)];
    if (client->connected) {
        printf("Client %s already connected.\n", request.id);
        close(fd);
        FD_CLR(fd, &all_fd);
        return;
    }

    printf("New client %s connected from %s:%hu.\n",
           request.id,
           inet_ntoa(*(struct in_addr *)&request.ip_server),
           request.port_server);

    client->connected = true;
    client->fd = fd;
}

void handle_subscribe(const tcp_request_t &request) {
    client_data_t *client = clients[std::string(request.id)];
    topics_to_clients[std::string(request.topic)].push_back(client);
}

void handle_unsubscribe(const tcp_request_t &request) {
    std::string topic_to_remove_pattern = std::string(request.topic);

    for (auto &[topic, list_clients] : topics_to_clients) {
        if (topic_matches(split_topic(topic), split_topic(topic_to_remove_pattern))) {
            list_clients.erase(
                std::remove_if(list_clients.begin(), list_clients.end(),
                               [&](client_data_t *client) {
                                   return client->id == std::string(request.id);
                               }),
                list_clients.end());
        }
    }
}

void handle_exit(int fd, const tcp_request_t &request) {
    printf("Client %s disconnected.\n", request.id);
    FD_CLR(fd, &all_fd);
    client_data_t *client = clients[std::string(request.id)];
    client->connected = false;
    close(fd);
}

void handle_clients(int fd) {
    tcp_request_t request;
    recv_all(fd, &request, sizeof(request));

    switch (request.type) {
    case CONNECT:
        handle_connect(fd, request);
        break;
    case SUBSCRIBE:
        handle_subscribe(request);
        break;
    case UNSUBSCRIBE:
        handle_unsubscribe(request);
        break;
    case EXIT:
        handle_exit(fd, request);
        break;
    }
}

void exit_program() {
    for (auto &[id, client] : clients)
        if (FD_ISSET(client->fd, &all_fd))
            close(client->fd);
    close(tcp_sock_fd);
    close(udp_sock_fd);
    exit(0);
}

void run_server() {
    init_server();
    while (true) {
        read_fds = all_fd;
        select(fd_max + 1, &read_fds, nullptr, nullptr, nullptr);
        for (int fd = 0; fd <= fd_max; fd++) {
            if (!FD_ISSET(fd, &read_fds))
                continue;
            if (FD_ISSET(tcp_sock_fd, &read_fds)) {
                handle_tcp();
                continue;
            }

            if (FD_ISSET(udp_sock_fd, &read_fds)) {
                handle_udp();
                continue;
            }
            if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                std::string input;
                std::getline(std::cin, input);
                if (input == "exit")
                    exit_program();
            }
            handle_clients(fd);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./server <PORT_SERVER>\n";
        return 1;
    }

    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    uint16_t port_server = atoi(argv[1]);

    setup_server(port_server);
    run_server();

    return 0;
}