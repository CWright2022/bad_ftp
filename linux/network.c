#include "network.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define TRANSFER_BUFFER_SIZE 4096

static int retryable_error(void)
{
    return errno == EINTR;
}

badftp_socket_t setup_server(int port)
{
    badftp_socket_t server_socket;
    struct sockaddr_in address;
    int reuse_address = 1;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == BADFTP_INVALID_SOCKET) {
        perror("socket");
        return BADFTP_INVALID_SOCKET;
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
                   &reuse_address, sizeof(reuse_address)) < 0) {
        perror("setsockopt");
        cleanup_socket(server_socket);
        return BADFTP_INVALID_SOCKET;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((uint16_t)port);

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        cleanup_socket(server_socket);
        return BADFTP_INVALID_SOCKET;
    }

    if (listen(server_socket, SOMAXCONN) < 0) {
        perror("listen");
        cleanup_socket(server_socket);
        return BADFTP_INVALID_SOCKET;
    }

    return server_socket;
}

badftp_socket_t accept_client(badftp_socket_t server_socket)
{
    struct sockaddr_in client_address;
    socklen_t address_length = sizeof(client_address);
    badftp_socket_t client_socket;

    client_socket = accept(server_socket, (struct sockaddr *)&client_address,
                           &address_length);
    if (client_socket == BADFTP_INVALID_SOCKET) {
        perror("accept");
    }

    return client_socket;
}

int send_all(badftp_socket_t socket, const char *buffer, size_t length)
{
    size_t total_sent = 0;

    while (total_sent < length) {
        ssize_t sent = send(socket, buffer + total_sent, length - total_sent,
                            MSG_NOSIGNAL);
        if (sent < 0) {
            if (retryable_error()) {
                continue;
            }
            perror("send");
            return -1;
        }
        if (sent == 0) {
            fprintf(stderr, "send returned zero bytes\n");
            return -1;
        }
        total_sent += (size_t)sent;
    }

    return 0;
}

int recv_line(badftp_socket_t socket, char *buffer, size_t buffer_size)
{
    size_t used = 0;
    char character;

    if (buffer_size == 0) {
        return -2;
    }

    while (used + 1 < buffer_size) {
        ssize_t received = recv(socket, &character, 1, 0);
        if (received < 0) {
            if (retryable_error()) {
                continue;
            }
            perror("recv");
            return -1;
        }
        if (received == 0) {
            buffer[used] = '\0';
            return 0;
        }

        if (character == '\r') {
            continue;
        }
        if (character == '\n') {
            buffer[used] = '\0';
            return (int)used;
        }
        buffer[used++] = character;
    }

    buffer[used] = '\0';
    return -2;
}

int receive_file(badftp_socket_t socket, FILE *file, long file_size)
{
    char buffer[TRANSFER_BUFFER_SIZE];
    long total_received = 0;

    while (total_received < file_size) {
        long remaining = file_size - total_received;
        size_t requested = remaining < TRANSFER_BUFFER_SIZE
                               ? (size_t)remaining
                               : TRANSFER_BUFFER_SIZE;
        ssize_t received = recv(socket, buffer, requested, 0);

        if (received < 0) {
            if (retryable_error()) {
                continue;
            }
            perror("recv during file transfer");
            return -1;
        }
        if (received == 0) {
            fprintf(stderr, "Client disconnected during file transfer.\n");
            return -2;
        }
        if (fwrite(buffer, 1, (size_t)received, file) != (size_t)received) {
            perror("write received file");
            return -3;
        }
        total_received += (long)received;
    }

    return 0;
}

int send_file(badftp_socket_t socket, FILE *file, long file_size)
{
    char buffer[TRANSFER_BUFFER_SIZE];
    long total_sent = 0;

    while (total_sent < file_size) {
        long remaining = file_size - total_sent;
        size_t requested = remaining < TRANSFER_BUFFER_SIZE
                               ? (size_t)remaining
                               : TRANSFER_BUFFER_SIZE;
        size_t bytes_read = fread(buffer, 1, requested, file);

        if (bytes_read == 0) {
            if (ferror(file)) {
                perror("read file");
            } else {
                fprintf(stderr, "Unexpected end of file\n");
            }
            return -1;
        }
        if (send_all(socket, buffer, bytes_read) != 0) {
            fprintf(stderr, "Failed to send file data\n");
            return -2;
        }
        total_sent += (long)bytes_read;
    }

    return 0;
}

badftp_socket_t connect_server(const char *server_ip, int port)
{
    badftp_socket_t client_socket;
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == BADFTP_INVALID_SOCKET) {
        perror("socket");
        return BADFTP_INVALID_SOCKET;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) != 1) {
        fprintf(stderr, "Invalid server address: %s\n", server_ip);
        cleanup_socket(client_socket);
        return BADFTP_INVALID_SOCKET;
    }

    if (connect(client_socket, (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0) {
        perror("connect");
        cleanup_socket(client_socket);
        return BADFTP_INVALID_SOCKET;
    }

    return client_socket;
}

void cleanup_socket(badftp_socket_t socket)
{
    if (socket != BADFTP_INVALID_SOCKET) {
        shutdown(socket, SHUT_RDWR);
        close(socket);
    }
}
