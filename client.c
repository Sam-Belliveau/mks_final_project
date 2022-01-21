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
        while((read_size = read(from_server, buffer, BUFFER_SIZE)))
        { write(STDOUT_FILENO, buffer, read_size); }
    }

    else
    {
        while((read_size = read(STDIN_FILENO, buffer, BUFFER_SIZE)))
        { write(to_server, buffer, read_size); }
    }
}