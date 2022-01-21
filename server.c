#include "./src/shell.h"
#include "./src/pipe_networking.h"
#include "./src/shell_command.h"

#include <stdio.h>

#define MAX_CLIENTS 256

// Handle SIGINT so that the shell can survive a ctrl+c
static void signal_handler(int);

int shell_loop(int* input);

int main(int argc, char** argv)
{
    int read_size;
    char buffer[BUFFER_SIZE];

    int to_client;
    int from_client;

    int to_shell;
    int from_shell;

    int last_server = -1;

    from_shell = shell_loop(&to_shell);

    while(1)
    {
        from_client = server_handshake( &to_client );

        int prev_chain[2], next_chain[2];
        pipe(prev_chain);
        pipe(next_chain);

        if(fork() == 0)
        {
            close(next_chain[PIPE_OUTPUT]);
            close(prev_chain[PIPE_INPUT]);

            if(fork())
            {
                if(fork())
                {
                    close(from_shell);

                    while((read_size = read(from_client, buffer, BUFFER_SIZE)))
                    { 
                        write(to_shell, buffer, read_size); 
                        write(next_chain[PIPE_INPUT], buffer, read_size); 
                        if(last_server > 0) write(last_server, buffer, read_size); 
                    }

                    close(from_client);
                    close(next_chain[PIPE_INPUT]);
                    close(last_server);

                    write(to_shell, "quit", 5);
                    exit(0);
                }

                else
                {
                    close(from_client);
                    close(from_shell);
                    close(next_chain[PIPE_INPUT]);

                    while((read_size = read(prev_chain[PIPE_OUTPUT], buffer, BUFFER_SIZE)))
                    { 
                        write(to_client, buffer, read_size); 
                        if(last_server > 0) write(last_server, buffer, read_size); 
                    }

                    close(to_client);
                    close(last_server);
                    close(prev_chain[PIPE_OUTPUT]);

                    write(to_shell, "quit", 5);
                    exit(0);
                }
            }

            else
            {
                close(from_client);
                close(last_server);
                close(prev_chain[PIPE_OUTPUT]);

                while((read_size = read(from_shell, buffer, BUFFER_SIZE)))
                { 
                    write(to_client, buffer, read_size); 
                    write(next_chain[PIPE_INPUT], buffer, read_size); 
                }

                close(from_shell);
                close(next_chain[PIPE_INPUT]);

                write(to_shell, "quit", 5);
                exit(0);
            }
        }

        else
        {
            close(prev_chain[PIPE_OUTPUT]);
            close(next_chain[PIPE_INPUT]);
            close(from_client); from_client = -1;
            close(to_client); to_client = -1;
            from_shell = next_chain[PIPE_OUTPUT];
            last_server = prev_chain[PIPE_INPUT];
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