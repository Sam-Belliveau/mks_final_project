#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#ifndef NETWORKING_H
#define NETWORKING_H

#define PIPE_INPUT 1
#define PIPE_OUTPUT 0

#define ACK "HOLA"
#define WKP "multi_shell_pipe"
#define PANIC "THE_PROGRAM_IS_ENDING_AND_YOU_NEED_TO_CLOSE"

#define server_printf(args...) fprintf(stderr, "[SERVER] " args)
#define client_printf(args...) fprintf(stderr, "[CLIENT] " args)

#define HANDSHAKE_BUFFER_SIZE 10
#define BUFFER_SIZE 1000

int server_handshake(int *to_client);
int client_handshake(int *to_server);

#endif