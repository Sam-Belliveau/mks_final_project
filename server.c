#include "./src/shell.h"
#include "./src/pipe_networking.h"
#include "./src/shell_command.h"

#include <stdio.h>
#include <signal.h>

#define MAX_CLIENTS 256

typedef union {
    struct {
        int from;
        int to;
    };

    int pipe[2];
} bi_file;

int shell_loop(int* input);
int handle_client(bi_file shell, int shell_chain, bi_file client, bi_file prev_client);
void close_all_fds();

int main(int argc, char** argv)
{
    int read_size;
    char buffer[BUFFER_SIZE];

    int client_id = 0;

    bi_file client, prev_client;
    bi_file shell, shell_chain;

    int t, last_server = -1;

    shell.from = shell_loop(&shell.to);

    client.from = -1;
    client.to = -1;

    while(1)
    {
        pipe(prev_client.pipe);
        pipe(shell_chain.pipe);

        t = last_server;
        last_server = prev_client.to;
        prev_client.to = t;

        client.from = server_handshake( &client.to );
        ++client_id;

        if(fork() == 0)
        {
            server_printf("Started Client Handle [ID: #%d]\n", client.from);
            while(handle_client(shell, shell_chain.to, client, prev_client));
            close_all_fds();

            server_printf("Killing Client Handle [ID: #%d]\n", client.from);
            kill(getppid(), SIGKILL);
            exit(0);
        }

        else
        {
            close(shell.from);
            close(shell_chain.to);
            shell.from = shell_chain.from;
            close(prev_client.from);
            close(prev_client.to);
            close(client.from);
            close(client.to);
        }
    }
}

// Handle SIGINT by not closing if it is the parent process
// and exiting if it is the child. The child usually overwrites this however.
static void signal_handler(int signal) 
{
}

int shell_loop(int* input)
{
    int i;
    char* command_str;
    struct shell_command* command;

    int server_to_shell[2];
    int shell_to_server[2];

    pipe(server_to_shell);
    pipe(shell_to_server);

    if(fork() == 0)
    {
        dup2(server_to_shell[PIPE_OUTPUT], STDIN_FILENO); close(server_to_shell[PIPE_OUTPUT]);
        dup2(shell_to_server[PIPE_INPUT], STDOUT_FILENO); 
        dup2(shell_to_server[PIPE_INPUT], STDERR_FILENO); close(shell_to_server[PIPE_INPUT]);

        signal(SIGINT, signal_handler);

        // Very Simple Shell Loop
        while(1)
        {
            // Read command from GNU readline
            command = shell_readline();
            
            shell_execute_commands(command);
            shell_command_free(command);
            free(command_str);
        }

        exit(0);
    }

    else
    {
        *input = server_to_shell[PIPE_INPUT];
        return shell_to_server[PIPE_OUTPUT];
    }

}

#define MAX_DESC(a, b) ((a) > (b) ? (a) : (b))

int handle_client(bi_file shell, int shell_chain, bi_file client, bi_file prev_client)
{
    int read_size;
    char buffer[BUFFER_SIZE] = {};

    fd_set read_fds;

    FD_ZERO(&read_fds);

    FD_SET(shell.from, &read_fds);
    FD_SET(client.from, &read_fds);
    FD_SET(prev_client.from, &read_fds);

    int max_desc = MAX_DESC(MAX_DESC(shell.from, client.from), prev_client.from);

    int i = select(max_desc+1, &read_fds, NULL, NULL, NULL);

    if(FD_ISSET(shell.from, &read_fds)) 
    { 
        read_size = read(shell.from, buffer, BUFFER_SIZE);

        if(read_size && (strncmp(buffer, PANIC, sizeof(PANIC)) != 0)) 
        {
            write(client.to, buffer, read_size); 
            write(shell_chain, buffer, read_size); 
            return 1;
        }
        else 
        {
            server_printf("Closed: shell.from [ID: #%d]\n", client.from);
            goto close_fds;
        }
    }

    if(FD_ISSET(client.from, &read_fds))
    {
        read_size = read(client.from, buffer, BUFFER_SIZE);

        if(read_size && (strncmp(buffer, PANIC, sizeof(PANIC)) != 0)) 
        {
            write(shell.to, buffer, read_size); 
            write(shell_chain, buffer, read_size); 
            write(prev_client.to, buffer, read_size); 
            return 1;
        }
        else
        {
            server_printf("Closed: client.from [ID: #%d]\n", client.from);
            goto close_fds;
        }
    }

    if(FD_ISSET(prev_client.from, &read_fds))
    {
        read_size = read(prev_client.from, buffer, BUFFER_SIZE);
        
        if(read_size && (strncmp(buffer, PANIC, sizeof(PANIC)) != 0)) 
        {
            write(client.to, buffer, read_size); 
            write(prev_client.to, buffer, read_size); 
            return 1;
        }
        else 
        {
            server_printf("Closed: prev_client.from [ID: #%d]\n", client.from);
            goto close_fds;
        }
    }

    close_fds:

    server_printf("\t- Closing: shell.to\n");
    close(shell.to);

    server_printf("\t- Closing: shell_chain\n");
    close(shell_chain);

    server_printf("\t- Closing: client\n");
    close(client.from);
    close(client.to);

    server_printf("\t- PANICING: prev_client\n");
    write(prev_client.to, PANIC, sizeof(PANIC)); 

    server_printf("\t- Closing: prev_client\n");
    close(prev_client.from);         
    close(prev_client.to);        

    server_printf("\t- Running Exit Process\n\nß");
    return 0;
}

void close_all_fds()
{
    int i, fdlimit = (int)sysconf(_SC_OPEN_MAX);
    for (i = STDERR_FILENO + 1; i < fdlimit; i++) close(i);   
}