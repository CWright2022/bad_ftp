#define _POSIX_C_SOURCE 200809L

#include "network.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define PORT 1234
#define BUFFER_LEN 1024

static int parse_upload_command(char *command, char *filename,
                                size_t filename_size, long *file_size)
{
    char *save_pointer;
    char *token;
    char *end_pointer;

    token = strtok_r(command, " ", &save_pointer);
    if (token == NULL || strcmp(token, "UPLOAD") != 0) {
        fprintf(stderr, "Invalid or missing command\n");
        return -1;
    }

    token = strtok_r(NULL, " ", &save_pointer);
    if (token == NULL || strlen(token) >= filename_size) {
        fprintf(stderr, "Invalid or missing filename\n");
        return -1;
    }
    strcpy(filename, token);

    token = strtok_r(NULL, " ", &save_pointer);
    if (token == NULL) {
        fprintf(stderr, "Invalid or missing file size\n");
        return -1;
    }

    errno = 0;
    *file_size = strtol(token, &end_pointer, 10);
    if (errno == ERANGE || end_pointer == token || *end_pointer != '\0' ||
        *file_size < 0) {
        fprintf(stderr, "Invalid file size: %s\n", token);
        return -1;
    }

    return 0;
}

int main(void)
{
    char cwd[PATH_MAX];
    char command[BUFFER_LEN];
    char filename[256];
    long file_size;
    badftp_socket_t server_socket;
    badftp_socket_t client_socket;

    signal(SIGPIPE, SIG_IGN);

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Working directory: %s\n", cwd);
    }

    server_socket = setup_server(PORT);
    if (server_socket == BADFTP_INVALID_SOCKET) {
        return EXIT_FAILURE;
    }
    printf("Listening on port %d...\n", PORT);

    client_socket = accept_client(server_socket);
    if (client_socket == BADFTP_INVALID_SOCKET) {
        cleanup_socket(server_socket);
        return EXIT_FAILURE;
    }

    for (;;) {
        int result = recv_line(client_socket, command, sizeof(command));
        if (result == 0) {
            printf("Client disconnected.\n");
            break;
        }
        if (result < 0) {
            fprintf(stderr, "Client connection error.\n");
            break;
        }

        printf("Received: [%s]\n", command);
        if (parse_upload_command(command, filename, sizeof(filename),
                                 &file_size) != 0) {
            break;
        }

        printf("Filename: %s\nFilesize: %ld\n", filename, file_size);
        FILE *file = fopen(filename, "wb");
        if (file == NULL) {
            perror("Could not open file");
            break;
        }

        if (send_all(client_socket, "READY\r\n", 7) != 0 ||
            receive_file(client_socket, file, file_size) != 0) {
            fclose(file);
            break;
        }

        fclose(file);
        printf("Upload complete.\n");
    }

    cleanup_socket(client_socket);
    cleanup_socket(server_socket);
    return EXIT_SUCCESS;
}
