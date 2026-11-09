#ifndef BADFTP_NETWORK_H
#define BADFTP_NETWORK_H

#include <stdio.h>

typedef int badftp_socket_t;

#define BADFTP_INVALID_SOCKET (-1)

badftp_socket_t setup_server(int port);
badftp_socket_t accept_client(badftp_socket_t server_socket);
badftp_socket_t connect_server(const char *server_ip, int port);

int send_all(badftp_socket_t socket, const char *buffer, size_t length);
int recv_line(badftp_socket_t socket, char *buffer, size_t buffer_size);
int send_file(badftp_socket_t socket, FILE *file, long file_size);
int receive_file(badftp_socket_t socket, FILE *file, long file_size);
void cleanup_socket(badftp_socket_t socket);

#endif
