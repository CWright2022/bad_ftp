#include <stdio.h>
#include "network.h"
int main(void)
{
    char filename[256];

    printf("Enter file path to send: ");
    scanf_s("%256s", filename, (unsigned)sizeof(filename));

    printf("Opening %s\n", filename);
    //FILE* fptr;
    //fopen_s(&fptr, filename, "rb");

    SOCKET socket = connect_server("127.0.0.1", 1234);

    strcat_s(filename, sizeof(filename), "\r\n");

    send_all(socket, filename, (int)strlen(filename));

    cleanup_socket(socket);

    return 0;
}