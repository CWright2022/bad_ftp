#pragma once
#ifndef NETWORK_H
#define NETWORK_H

#include <winsock2.h>
#include <stdio.h>

SOCKET setup_server(int port);
SOCKET accept_client(SOCKET server);

int send_all(SOCKET socket, const char* buffer, int length);
int recv_line(SOCKET socket, char* buffer);

void cleanup_server(SOCKET server);

int receive_file(SOCKET socket, FILE* file, long file_size);

int send_file(SOCKET socket, FILE* file, long file_size);

#endif