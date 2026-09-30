#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "network.h"
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 1234
#define BUFFER_LEN 1024
#define UPLOAD_CMD "UPLOAD"
#define EXIT_CMD "exit"

int main() {
    SOCKET server_socket = setup_server(1234);
    SOCKET client_socket = accept_client(server_socket);
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_LEN] = { 0 };
    const char* hello = "Hello from Windows Server!\r\n";
    const char* uploadResponse = "upload!\r\n";
    const char* goodbye = "goodbye!\r\n";

    // receive, account for /r/n

    while (true) {
        char buffer[1024] = { 0 };

        int result = recv_line(client_socket, buffer);

        if (result == 0) {
            printf("Client disconnected.\n");
            break;
        }

        if (result < 0) {
            fprintf(stderr, "Client connection error.\n");
            break;
        }

        printf("Received: [%s]\n", buffer);

        char cmd[256];
        char filename[256];
        char filesize_s[256];

        long filesize;

        char* context = NULL;
        char* token = NULL;

        /*
         * Parse command
         */
        token = strtok_s(buffer, " ", &context);

        if (token == NULL) {
            fprintf(stderr, "Missing command.\n");
            return;
        }

        strcpy_s(cmd, sizeof(cmd), token);

        /*
         * Parse filename
         */
        token = strtok_s(NULL, " ", &context);

        if (token == NULL) {
            fprintf(stderr, "Missing filename.\n");
            return;
        }

        strcpy_s(filename, sizeof(filename), token);

        /*
         * Parse file size
         */
        token = strtok_s(NULL, " ", &context);

        if (token == NULL) {
            fprintf(stderr, "Missing file size.\n");
            return;
        }

        strcpy_s(filesize_s, sizeof(filesize_s), token);

        /*
         * Convert file size string to long.
         */
        char* endptr = NULL;

        filesize = strtol(
            filesize_s,
            &endptr,
            10
        );

        if (endptr == filesize_s || *endptr != '\0' || filesize < 0) {
            fprintf(stderr, "Invalid file size: %s\n", filesize_s);
            return;
        }

        printf("Command:  %s\n", cmd);
        printf("Filename: %s\n", filename);
        printf("Filesize: %ld\n", filesize);

        FILE* file = NULL;
        if (fopen_s(&file, filename, "wb") != 0) {
            fprintf(stderr, "Could not open file: %s\n", filename);
            return EXIT_FAILURE;
        }
        printf("opened file\n");
        char message[] = "READY\r\n";
        send_all(client_socket, message, strlen(message));
        printf("sent ready\n");
        printf("all done!\n");
    }


    // clean up
    closesocket(client_socket);
    cleanup_socket(server_socket);

    return 0;
}