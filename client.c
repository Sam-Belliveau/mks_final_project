#include "./src/pipe_networking.h"

int direct_read();

// Handle SIGINT so that the shell can survive a ctrl+c
static void signal_handler(int);

int to_server;
int from_server;

int main() 
{
    signal(SIGINT, signal_handler);
    from_server = client_handshake( &to_server );
    while(direct_read(from_server, to_server, STDIN_FILENO, STDOUT_FILENO));
    signal_handler(-1);
}

// Handle SIGINT by not closing if it is the parent process
// and exiting if it is the child. The child usually overwrites this however.
static void signal_handler(int signal) 
{
    write(to_server, PANIC, sizeof(PANIC)); 
    close(to_server); to_server = -1;
    close(from_server); from_server = -1;
    exit(0);
}

int direct_read(int from_server, int to_server, int from_user, int to_user)
{
    int read_size;
    char buffer[BUFFER_SIZE] = {};
    fd_set read_fds;

    FD_ZERO(&read_fds);

    FD_SET(from_server, &read_fds);
    FD_SET(from_user, &read_fds);

    int max_desc = from_server > from_user ? from_server : from_user;

    int i = select(max_desc+1, &read_fds, NULL, NULL, NULL);

    if(FD_ISSET(from_user, &read_fds)) 
    { 
        read_size = read(from_user, buffer, BUFFER_SIZE);
        if(read_size)
        {
            write(to_server, buffer, read_size);
            return 1;
        } 
        else
        {
            client_printf("STDIN doesn't work anymore???\n");
            return 0;
        }
    }

    if(FD_ISSET(from_server, &read_fds))
    {
        read_size = read(from_server, buffer, BUFFER_SIZE);
            
        if(read_size)
        {
            write(to_user, buffer, read_size);
            return 1;
        } 
        else
        {
            client_printf("Server Closed!\n");
            return 0;
        }
    }

    return 0;
}