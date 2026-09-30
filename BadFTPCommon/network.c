#include "network.h"
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define TRANSFER_BUFFER_SIZE 4096

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

void cleanup_socket(SOCKET socket) {
    shutdown(socket, SD_SEND);
    closesocket(socket);
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
            *end = '\0';
            return (int)(end - buffer);
        }
    }
    return -2;
}

int receive_file(SOCKET socket, FILE* file, long file_size)
{
    char buffer[TRANSFER_BUFFER_SIZE];
    long total_received = 0;

    while (total_received < file_size) {

        long remaining = file_size - total_received;

        int to_receive;

        if (remaining < TRANSFER_BUFFER_SIZE) {
            to_receive = (int)remaining;
        }
        else {
            to_receive = TRANSFER_BUFFER_SIZE;
        }

        int received = recv(
            socket,
            buffer,
            to_receive,
            0
        );

        if (received == SOCKET_ERROR) {
            fprintf(
                stderr,
                "recv() failed during file transfer: %d\n",
                WSAGetLastError()
            );

            return -1;
        }

        if (received == 0) {
            fprintf(
                stderr,
                "Client disconnected during file transfer.\n"
            );

            return -2;
        }

        size_t written = fwrite(
            buffer,
            1,
            received,
            file
        );

        if (written != (size_t)received) {
            fprintf(
                stderr,
                "Failed to write received data to file.\n"
            );

            return -3;
        }

        total_received += received;
    }

    return 0;
}

SOCKET connect_server(const char* server_ip, int port)
{
    WSADATA wsaData;
    SOCKET client_socket = INVALID_SOCKET;
    struct sockaddr_in server_address = { 0 };

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        fprintf(stderr,
            "WSAStartup failed: %d\n",
            result);

        return INVALID_SOCKET;
    }

    client_socket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (client_socket == INVALID_SOCKET) {
        fprintf(stderr,
            "socket() failed: %d\n",
            WSAGetLastError());

        WSACleanup();
        return INVALID_SOCKET;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons((u_short)port);

    result = inet_pton(
        AF_INET,
        server_ip,
        &server_address.sin_addr
    );

    if (result != 1) {
        fprintf(stderr,
            "Invalid server address: %s\n",
            server_ip);

        closesocket(client_socket);
        WSACleanup();

        return INVALID_SOCKET;
    }

    result = connect(
        client_socket,
        (struct sockaddr*)&server_address,
        sizeof(server_address)
    );

    if (result == SOCKET_ERROR) {
        fprintf(stderr,
            "connect() failed: %d\n",
            WSAGetLastError());

        closesocket(client_socket);
        WSACleanup();

        return INVALID_SOCKET;
    }

    return client_socket;
}