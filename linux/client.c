#define _POSIX_C_SOURCE 200809L

#include "network.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 1234

int main(void)
{
    char filename[256];
    char cwd[PATH_MAX];
    FILE *file;
    long file_size;
    badftp_socket_t socket;
    char command[1024];
    char response[256];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Working directory: %s\n", cwd);
    }

    printf("Enter file path to send: ");
    if (scanf("%255s", filename) != 1) {
        fprintf(stderr, "Could not read file path\n");
        return EXIT_FAILURE;
    }

    printf("Opening %s\n", filename);
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Could not open file");
        return EXIT_FAILURE;
    }

    if (fseek(file, 0, SEEK_END) != 0 || (file_size = ftell(file)) < 0) {
        perror("Could not determine file size");
        fclose(file);
        return EXIT_FAILURE;
    }
    rewind(file);
    printf("File length is %ld\n", file_size);

    socket = connect_server("127.0.0.1", PORT);
    if (socket == BADFTP_INVALID_SOCKET) {
        fclose(file);
        return EXIT_FAILURE;
    }

    if (snprintf(command, sizeof(command), "UPLOAD %s %ld\r\n",
                 filename, file_size) >= (int)sizeof(command) ||
        send_all(socket, command, strlen(command)) != 0) {
        fprintf(stderr, "Failed to send upload command\n");
        fclose(file);
        cleanup_socket(socket);
        return EXIT_FAILURE;
    }

    printf("Awaiting server response...\n");
    if (recv_line(socket, response, sizeof(response)) <= 0 ||
        strcmp(response, "READY") != 0) {
        fprintf(stderr, "Server did not accept the upload\n");
        fclose(file);
        cleanup_socket(socket);
        return EXIT_FAILURE;
    }

    if (send_file(socket, file, file_size) != 0) {
        fclose(file);
        cleanup_socket(socket);
        return EXIT_FAILURE;
    }

    fclose(file);
    cleanup_socket(socket);
    return EXIT_SUCCESS;
}
