#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h> // read(), write(), close()
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr()

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void func(int sockfd) {
    char buff[MAX];
    int n;
    
    while(1) {
        memset(buff, 0, sizeof(buff));
        printf("Enter the string : ");
        fflush(stdout);
        if (fgets(buff, sizeof(buff), stdin) == NULL) {
            perror("Error reading input");
            exit(1);
        }
        buff[strcspn(buff, "\n")] = '\0'; // Remove the newline character

        if(strcmp(buff, "run shell") == 0){
            write(sockfd, buff, sizeof(buff)); // Send the message to the server    
            printf("Shell run in the server...\n");
            // fflush(stdout);
            while(1) {
                memset(buff, 0, sizeof(buff));  // Clear the buffer
                read(sockfd, buff, sizeof(buff)); // Read the message from the server
                printf("%s", buff);
                fflush(stdout); 
                memset(buff, 0, sizeof(buff));
                // printf("Enter the command : ");
                if (fgets(buff, sizeof(buff), stdin) == NULL) {
                    perror("Error reading input");
                    exit(1);
                }
                // read(sockfd, buff, sizeof(buff)); // Read the message from the server   
                buff[strcspn(buff, "\n")] = '\0';
                write(sockfd, buff, sizeof(buff)); // Send the message to the server
                if(strcmp(buff, "exit") == 0) {
                    break;
                }
            }	
        }

        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            write(sockfd, buff, sizeof(buff)); 
            break;
        }
        write(sockfd, buff, sizeof(buff));

        memset(buff, 0, sizeof(buff)); 
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
    }
}

int main() {

	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP connection
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	} else {
		printf("Socket successfully created..\n");
	}
	memset(&servaddr, 0, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost
	servaddr.sin_port = htons(PORT);// Server port 8080

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	} else {
		printf("connected to the server..\n");
	}

	// function for chat
	func(sockfd);

	// close the socket
	close(sockfd);
}
