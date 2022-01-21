#include "./src/pipe_networking.h"

int main() 
{
    int to_server;
    int from_server;

    int read_size;
    char buffer[BUFFER_SIZE] = {};

    from_server = client_handshake( &to_server );

    if(fork())
    {
        close(to_server);
        while((read_size = read(from_server, buffer, BUFFER_SIZE)) > 0)
        { write(STDOUT_FILENO, buffer, read_size); }
        close(from_server);
    }

    else
    {
        close(from_server);
        while((read_size = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0)
        { write(to_server, buffer, read_size); }
        close(to_server);
    }
}