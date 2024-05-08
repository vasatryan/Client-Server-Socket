# Basic Client-Server Communication in C

This repository contains simple client and server programs implemented in C that demonstrate basic socket communication. The client can connect to the server and send shell commands, and the server can execute these commands and send back the output to the client.

## Features

- Allows clients to connect to the server and send shell commands.
- Server executes received shell commands and sends back the output to clients.
- Supports handling multiple clients using threads.
- Provides a basic command-line interface for the client to interact with the server.

## Requirements

- Compiler version GNU gcc11+
- OS version MacOs Ventura 13.2 and Linux Ubuntu 22.04

## Prerequisites

- GCC (GNU Compiler Collection) is required to compile the programs. If you don't have GCC installed, you can download it from [here](https://gcc.gnu.org/).

## Compilation and Usage

1. Clone the repository:

   ```bash
   git clone https://github.com/vasatryan/Client-Server-Socket.git
   cd Client-Server-Socket
2. Compile the client and server programs:
 ```bash
make
```
3. Run the server in one terminal window:
```
./bin/server
```
4. Run the client in another terminal window:
```
./bin/client
```
## Files
- **client.c**: Implements the client-side program that connects to the server, sends commands, and receives responses.
- __server.c__: Implements the server-side program that listens for client connections, handles incoming connections using threads, and executes shell commands.
## Makefile
The Makefile simplifies compilation. It includes the following targets:

****
- __all__ (default): Compiles both the client and server programs.
- __client__: Compiles only the client program.
- __server__: Compiles only the server program.
- __clean__: Removes compiled binaries and object files.




