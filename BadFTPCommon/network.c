#include "network.h"
#include <stdio.h>

SOCKET setup_server(int port) {
    WSADATA wsaData;
    SOCKET server_fd = INVALID_SOCKET;
    SOCKET new_socket = INVALID_SOCKET;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // create socket
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        fprintf(stderr, "socket creation failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Configure the server address structure
    address.sin_family = AF_INET;          // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
    address.sin_port = htons(port);       // Convert port to network byte order

    // bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        fprintf(stderr, "bind failed with error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // listen for incoming connections
    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "listen failed with error : % d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return server_fd;
}

SOCKET accept_client(SOCKET server_socket)
{
    struct sockaddr_in client_address;
    int client_address_len = sizeof(client_address);

    SOCKET client_socket = accept(
        server_socket,
        (struct sockaddr*)&client_address,
        &client_address_len
    );

    if (client_socket == INVALID_SOCKET) {
        fprintf(stderr, "accept() failed with error: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }
    return client_socket;
}

int send_all(SOCKET socket, const char* buffer, int length)
{
    int total_sent = 0;

    while (total_sent < length) {

        int sent = send(
            socket,
            buffer + total_sent,
            length - total_sent,
            0
        );

        if (sent == SOCKET_ERROR) {
            fprintf(stderr,
                "send failed: %d\n",
                WSAGetLastError());

            return -1;
        }

        total_sent += sent;
    }

    return 0;
}

void cleanup_server(SOCKET server) {
    closesocket(server);
    WSACleanup();
}

int recv_line(SOCKET socket, char* buffer) {
    int used = 0;
    while (used < 1023) {

        int valread = recv(
            socket,
            buffer + used,
            1023 - used,
            0
        );

        if (valread == SOCKET_ERROR) {
            fprintf(
                stderr,
                "recv() failed: %d\n",
                WSAGetLastError()
            );

            return -1;
        }

        if (valread <= 0) {
            return 0;
        }

        used += valread;
        buffer[used] = '\0';

        char* end = strstr(buffer, "\r\n");

        if (end != NULL) {

            // Replace \r with \0
            // This turns "upload\r\n" into "upload"
            *end = '\0';

            printf("Command: [%s]\n", buffer);
            used = 0;
            return (int)(end - buffer);
        }
    }
    return -2;
}

int receive_file(SOCKET socket, FILE* file, long file_size) {
    long bytes_received = 0;
    while (bytes_received < file_size) {

    }
}