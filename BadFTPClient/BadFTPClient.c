#include <stdio.h>
#include "network.h"
int main(void)
{
    char filename[256];

    char cwd[1024];

    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Working directory: %s\n", cwd);
    }

    printf("Enter file path to send: ");
    scanf_s("%255s", filename, (unsigned)sizeof(filename));

    printf("Opening %s\n", filename);
    FILE* file = NULL;

    if (fopen_s(&file, filename, "rb") != 0) {
        fprintf(stderr, "Could not open file: %s\n", filename);
        return EXIT_FAILURE;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    printf("file length is %ld\n", file_size);
    char size_str[32];
    sprintf_s(size_str, sizeof(size_str), "%ld", file_size);

    SOCKET socket = connect_server("127.0.0.1", 1234);

    char command_to_send[1024];
    sprintf_s(
        command_to_send,
        sizeof(command_to_send),
        "UPLOAD %s %ld\r\n",
        filename,
        file_size
    );

    // send upload command (UPLOAD filename size)
    send_all(socket, command_to_send, (int)strlen(command_to_send));

    printf("awaiting server response...");
    char buffer[256];
    recv_line(socket, buffer);
    send_file(socket, file, file_size);

    fclose(file);
    cleanup_socket(socket);

    return 0;
}