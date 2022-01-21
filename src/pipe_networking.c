#include "pipe_networking.h"

/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
    // Reset File Descriptors
    int from_client;
    
    // Create Buffer
    char private_pipe[BUFFER_SIZE] = {}, ack[HANDSHAKE_BUFFER_SIZE];
    int bytes_read;

    // Reset File Descriptors
    from_client = -1;
    *to_client = -1;

    // Create WKP
    remove(WKP);
    if(mkfifo(WKP, 0666))
    {
        server_printf("Error when creating WKP: %s [%d]\n", strerror(errno), errno);
        exit(-1);
    }
    else server_printf("Created WKP\n");

    // Open the WKP
    from_client = open(WKP, O_RDONLY);
    if(from_client < 0)
    {
        server_printf("Error when opening WKP: %s [%d]\n", strerror(errno), errno);
        exit(-1);
    }
    else server_printf("Opened WKP\n");
    remove(WKP);

    // Read name of private pipe from client and open it
    bytes_read = read(from_client, private_pipe, BUFFER_SIZE);
    server_printf("Recieved %d bytes of input from WKP, closing\n", bytes_read);
    *to_client = open(private_pipe, O_WRONLY);

    if(*to_client < 0)
         server_printf("Error Opening Pipe %s: %s [%d]\n", private_pipe, strerror(errno), errno);
    else server_printf("Opened Pipe %s\n", private_pipe);

    // Write ACK to server
    write(*to_client, ACK, sizeof(ACK));
    server_printf("Sent ACK\n");

    // Recieve ACK from client
    if(read(from_client, ack, HANDSHAKE_BUFFER_SIZE) != HANDSHAKE_BUFFER_SIZE)
         server_printf("Error Recieving ACK, but I don't care [%s]\n", ack);
    else server_printf("Recieved ACK [%s]\n", ack); 

    return from_client;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
    int from_server;

    // Create Buffer
    char private_pipe[BUFFER_SIZE] = {}, ack[HANDSHAKE_BUFFER_SIZE];
    sprintf(private_pipe, "%d", getpid());

    // Reset File Descriptors
    from_server = -1;
    *to_server = -1;

    // Set Private Pipe
    sprintf(private_pipe, "%d", getpid());

    // Try To Open WKP
    *to_server = open(WKP, O_WRONLY);
    
    // Create Pipe
    if(*to_server < 0)
    {
        client_printf("Error when opening WKP: %s [%d]\n", strerror(errno), errno);
        return from_server;
    }
    else client_printf("Opened WKP\n");

    // Create private pipe
    remove(private_pipe);
    if(mkfifo(private_pipe, 0666))
    {
        client_printf("Error when creating private pipe %s: %s [%d]\n", private_pipe, strerror(errno), errno);
        return from_server;
    }
    else client_printf("Created private pipe %s\n", private_pipe);

    // Write name of private_pipe to server
    write(*to_server, private_pipe, BUFFER_SIZE);
    client_printf("Wrote %s to WKP\n", private_pipe);

    // Open private pipe to read from server
    from_server = open(private_pipe, O_RDONLY);
    if(from_server < 0)
    {
        client_printf("Error Opening Pipe %s: %s [%d]\n", private_pipe, strerror(errno), errno);
        return from_server;
    }
    else client_printf("Opened Pipe %s\n", private_pipe);
    remove(private_pipe);

    // Wait for ACK from server
    if(read(from_server, ack, HANDSHAKE_BUFFER_SIZE) != HANDSHAKE_BUFFER_SIZE)
         client_printf("Error Recieving ACK, but I don't care [%s]\n", ack);
    else client_printf("Recieved ACK [%s]\n", ack); 

    // Write ACK to server
    write(*to_server, ACK, sizeof(ACK));
    client_printf("Sent ACK\n");

    return from_server;
}