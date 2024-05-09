# Basic Client-Server Communication in C

This repository contains simple client and server programs implemented in C that demonstrate basic socket communication. The client can connect to the server and send shell commands, and the server can execute these commands and send back the output to the client.

## Features

- Allows clients to connect to the server and when a client writes 'run shell', the client can run shell commands.
- Server executes received shell commands and sends back the output to clients.
- Provides a basic command-line interface for the client to interact with the server.

## Requirements

- Compiler version GNU gcc11+

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
./server
```
4. Run the client in another terminal window:
```
./client
```
## Files
- **client.c**: Implements the client-side program that connects to the server, sends commands, and receives responses.
- __server.c__: Implements the server-side program that listens for client connections, handles incoming connections using threads, and executes shell commands.