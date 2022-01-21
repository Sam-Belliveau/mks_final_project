# mks_final_project | Shared Shell

## How to use

#### Start Server

To start the server run `make run_server`

#### Start a Client

To start the client, run `make run_client`

## Information

The shared shell is a project that will merge two of the previous assignments:

    - The forking pipe based server
    - The shell program

Merging these two will create a shell that can be accessed from two separate windows on the computer. This could be helpful if... you wanted the same shell in two separate windows. I'm not sure how, but it has to fit into SOMEBODY's workflow.

Because I have COVID right now, I am choosing to work alone so I can go at my own pace.

This project will use:
* Allocating memory
* Processes (fork, exec etc.)
* Signals
* Pipes (named and unnamed)

It should look like the shell we created before, but everything that gets typed on one window, should then appear on the other window.
