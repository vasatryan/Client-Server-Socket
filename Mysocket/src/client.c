#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_BUFFER 16384

char buffer[MAX_BUFFER];
int userSocket = -1;
int count = 0;

void function(int* userSocket, char* buffer) 
{
    if(*userSocket != -1) {
        strcpy(buffer, "disconnect");
        send(*userSocket, buffer, strlen(buffer), 0);
        printf("\nDisconnected from server.\nCLIENT> ");
        fflush(stdout);
        close(*userSocket);
        *userSocket = -1;
    } 
    else {
        printf("\nNot connected to server.\n\rCLIENT> ");
        fflush(stdout);
    }
}

void functionHadler(int signalNumber) 
{
    function(&userSocket, buffer);
}

int main() 
{
    struct sockaddr_in serverAddr = {0};

    char serverIP[30];
    int serverPort = 0;

    while(1) {
        memset(buffer, 0, sizeof(buffer));
        signal(SIGINT, functionHadler);
        printf("CLIENT> ");
        fgets(buffer, sizeof(buffer), stdin);
        int count = 0;
        char* bufferPtr = buffer;
        while(bufferPtr[count] == 32 || bufferPtr[count] == 10) {
            ++count;
        }
        bufferPtr = buffer + count;
        if(strncmp(bufferPtr, "disconnect", 10) == 0) {
            if(userSocket != -1) {
                send(userSocket, bufferPtr, strlen(buffer), 0);
                printf("Disconnected from server.\n");
                close(userSocket);
                userSocket = -1;
            } 
            else {
                printf("Not connected to any server.\n");
            }
        } 
        else if(strncmp(bufferPtr, "shell ", 6) == 0) {
            if(userSocket != -1) {
                send(userSocket, bufferPtr, strlen(buffer), 0);
                ssize_t recivedBytes = recv(userSocket, buffer, sizeof(buffer), 0);
                if(recivedBytes > 0) {
                    buffer[recivedBytes] = '\0';
                    if(strncmp(buffer, "Error", 5) == 0) {
                        printf("Server error: %s\n", buffer);
                    } 
                    else {
                        printf("Server answer: \n%s", buffer);
                    }
                }
            } 
            else {
                printf("Not connected to any server.\n");
            }
        } 
        else if(strncmp(bufferPtr, "connect", 7) == 0) {
            if(userSocket == -1) {
                sscanf(bufferPtr + 8, "%s %d", serverIP, &serverPort);
                userSocket = socket(AF_INET, SOCK_STREAM, 0);
                if(userSocket == -1) {
                    perror("Socket");
                    return 1;
                }
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_addr.s_addr = inet_addr(serverIP);
                serverAddr.sin_port = htons(serverPort);

                if(connect(userSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
                    perror("Connect");
                    close(userSocket);
                    userSocket = -1;
                } 
                else {
                    printf("Connected to server at %s.%d:\n", serverIP, serverPort);
                }
            } 
            else {
                printf("Already connected to a server:\n");
            }
        } 
        else if(strcmp(buffer, "\n") == 0) {
            continue;
        }
        else if(strncmp(bufferPtr, "exit", 4) == 0) {
            if(userSocket != -1) {
                close(userSocket);
            } 
            break;
        }
        else {
            printf("Invalid input. Please enter 'connect', 'disconnect', 'exit' or a valid  shell command:\n");
        }
    }
    if(userSocket != -1) {
        close(userSocket);
    }
    return 0;
}
