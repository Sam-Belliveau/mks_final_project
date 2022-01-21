#ifndef SHELL_COMMAND_HEADER_FILE
#define SHELL_COMMAND_HEADER_FILE 1

#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "constants.h"

struct shell_command 
{
    int argc;
    char* argv[SH_MAX_ARGS + 1];

    int redir_stdin;
    int redir_stdout;
    int redir_stderr;

    struct shell_command* next_command;
};

// Initialize shell command
struct shell_command* shell_command_create(char *);

// Scan arguments for redirections, and add them to the command
struct shell_command* shell_command_add_redirects(struct shell_command*);

// Free command and return the next command in the chain
struct shell_command* shell_command_free_individual(struct shell_command*);

// Free the entire chain of commands.
void shell_command_free(struct shell_command*);

#endif