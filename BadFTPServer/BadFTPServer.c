#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "network.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT 1234
#define BUFFER_LEN 1024
#define UPLOAD_CMD "upload"
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
        char buffer[1024];
        recv_line(client_socket, buffer);


        if (strcmp(buffer, UPLOAD_CMD) == 0) {
            send(client_socket, uploadResponse, (int)strlen(uploadResponse), 0);
            printf("sent goodbye\n");
        }
        else if (strcmp(buffer, EXIT_CMD) == 0) {
            send(client_socket, goodbye, (int)strlen(goodbye), 0);
            printf("sent goodbye.\n");
            break;
        }
        else {
            send(client_socket, hello, (int)strlen(hello), 0);
            printf("Hello message sent.\n");
        }

    }


    // clean up
    closesocket(client_socket);
    cleanup_server(server_socket);

    return 0;
}