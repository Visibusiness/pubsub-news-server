
#include "utils.hpp"

int recv_all(int sockfd, void *buffer, int len) {
    int bytes_received = 0;
    int bytes_remaining = len;
    char *buff = (char *)buffer;

    while (bytes_remaining > 0) {
        int rc = read(sockfd, buff + bytes_received, bytes_remaining);
        if (rc <= 0) {
            return rc;
        }

        bytes_received += rc;
        bytes_remaining -= rc;
    }

    return bytes_received;
}

int send_all(int sockfd, void *buffer, int len) {
    int bytes_sent = 0;
    int bytes_remaining = len;
    char *buff = (char *)buffer;

    while (bytes_remaining > 0) {
        int rc = write(sockfd, buff + bytes_sent, bytes_remaining);
        if (rc <= 0) {
            return rc;
        }

        bytes_sent += rc;
        bytes_remaining -= rc;
    }

    return bytes_sent;
}