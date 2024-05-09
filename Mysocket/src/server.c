#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h> 

#define BUFFER_SIZE 2048
#define MAX_COMMAND_LENGTH 256
#define MAX_ARGUMENTS 64
#define MAX_JOBS 64

typedef struct {
    int status;     // Status of the job (1 for running, 0 for stopped)
    pid_t pid;       // Process ID of the job
    char cmd[MAX_COMMAND_LENGTH]; // The command associated with the job
    int jobNumber;  // Unique job number
} Job;

Job jobs[MAX_JOBS]; // Array to store background jobs
int jobCount = 0;
int nextJobNumber = 1; // Keeps track of the next available job number
int runInBackground = 0;
volatile int ctrlC_pressed = 0;

void sigint_handler() {
    if (ctrlC_pressed == 1) {
        // The program has already received Ctrl+C once, exit the program
        printf("\n");
        exit(0);
    } else {
        // The program has not yet received Ctrl+C, stop the current command
        printf("\n");
        ctrlC_pressed = 1;
    }
}

void welcomeShell() {
    printf("**************************************\n");
    printf("**************************************\n\n");
    printf("Hello My dear, this is my simple shell\n");
    printf("\n\n**************************************\n");
    printf("**************************************\n");
}

char* executeHelp() {
    return("\n***WELCOME TO MY SHELL HELP***"
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit - to close program"
        "\n>Ctrl+C to exit program"
        "\n>all other general commands available in UNIX shell"
        "\n>jobs - list background jobs"
        "\n>fg <job number> - bring a background job to the foreground"
        "\n>bg <job number> - restart a stopped background job");
}

void addJob(pid_t pid, const char* cmd) {
    if (jobCount < MAX_JOBS) {
        jobs[jobCount].status = 1; // Running
        jobs[jobCount].pid = pid;
        strncpy(jobs[jobCount].cmd, cmd, MAX_COMMAND_LENGTH);
        jobs[jobCount].jobNumber = nextJobNumber++;
        jobCount++;
    } else {
        printf("Maximum number of background jobs reached.\n");
    }
}

int findJob(pid_t pid) {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

void removeJob(pid_t pid) {
    int index = findJob(pid);
    if (index >= 0) {
        for (int i = index; i < jobCount - 1; i++) {
            jobs[i] = jobs[i + 1];
        }
        jobCount--;
    }
}

void printJobs() {
    printf("Jobs:\n");
    for (int i = 0; i < jobCount; i++) {
        printf("[%d] %s %s\n", jobs[i].jobNumber, jobs[i].status ? "Running" : "Stopped", jobs[i].cmd);
    }
}

void executeCd(char* path) {
    if (chdir(path) == -1) {
        perror("chdir");
    }
}


// char* scanCmd() {
//     char inputCommand[MAX_COMMAND_LENGTH];
//     fgets(inputCommand, sizeof(inputCommand), stdin);
//     inputCommand[strcspn(inputCommand, "\n")] = '\0';
//     return strdup(inputCommand);
// }

int parseCommand(char* input, char** arguments) {
    char* token = strtok(input, " ");
    int argCount = 0;
    while (token != NULL) {
        arguments[argCount++] = strdup(token);
        token = strtok(NULL, " ");
    }
    arguments[argCount] = NULL;
    return argCount;
}

void freeArguments(char** arguments) {
    for (int i = 0; arguments[i] != NULL; ++i) {
        free(arguments[i]);
    }
}

void redirectionOut(char** arguments, int index) {
    if (arguments[index + 1] != NULL) {
        freopen(arguments[index + 1], "w", stdout);
    } else {
        printf("Error: Missing output file after '>'\n");
        exit(EXIT_FAILURE);
    }
    arguments[index] = NULL; // Remove '>' and output file from arguments
}

void redirectionIn(char** arguments, int index) {
    if (arguments[index + 1] != NULL) {
        freopen(arguments[index + 1], "r", stdin);
    } else {
        printf("Error: Missing input file after '<'\n");
        exit(EXIT_FAILURE);
    }
    arguments[index] = NULL; // Remove '<' and input file from arguments
}

void redirectionErr(char** arguments, int index) {
    if (arguments[index + 1] != NULL) {
        freopen(arguments[index + 1], "w", stderr);
    } else {
        printf("Error: Missing output file after '2>'\n");
        exit(EXIT_FAILURE);
    }
    arguments[index] = NULL; // Remove '2>' and output file from arguments
}

void append(char** arguments, int index) {
    if (arguments[index + 1] != NULL) {
        freopen(arguments[index + 1], "a", stdout);
    } else {
        printf("Error: Missing output file after '>>'\n");
        exit(EXIT_FAILURE);
    }
    arguments[index] = NULL; // Remove '>>' and output file from arguments
}

void execPipe (char** arguments, int index) {
        //Handle the pipe operation
    char** left_command = arguments;
    char** right_command = &arguments[index + 1];
    arguments[index] = NULL; // Terminate the left command
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid_t pipe_child_pid;
    if ((pipe_child_pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pipe_child_pid == 0) {
        // Child process for the left command
        close(pipe_fd[0]); // Close the read end of the pipe
        dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
        close(pipe_fd[1]); // Close the write end of the pipe
        // Execute the left command
        execvp(left_command[0], left_command);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process for the left command
        close(pipe_fd[1]); // Close the write end of the pipe
        dup2(pipe_fd[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe
        close(pipe_fd[0]); // Close the read end of the pipe
        // Execute the right command
        execvp(right_command[0], right_command);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}

void execArgs(char** arguments, int clientSocket) {
    pid_t childPid;
    int pipe_fd[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pipe_fd) < 0) {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    childPid = fork();
    if (childPid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (childPid == 0) {
        // Child process (command execution)
        close(pipe_fd[0]); // Close the read end of the pipe
        dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
        close(pipe_fd[1]); // Close the write end of the pipe

        // Execute the command
        int execResult = execvp(arguments[0], arguments);
        if (execResult == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }

    } else {
        // Parent process (server)
        close(pipe_fd[1]); // Close the write end of the pipe
        char buffer[BUFFER_SIZE];
        int n;

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);

            // Read the output from the pipe
            n = read(pipe_fd[0], buffer, sizeof(buffer) - 1);
            if (n <= 0) {
                break; // No more data
            }

            // Send the output to the client
            write(clientSocket, buffer, n);
        }

        // Close the read end of the pipe
        close(pipe_fd[0]);

        int status;
        waitpid(childPid, &status, 0);
    }

    freeArguments(arguments);
}


void execFg(char** arguments, int argCount) {
        // Bring a background job to the foreground
    signal(SIGINT, sigint_handler);
    
    if (argCount > 1) {
        int jobNumber = atoi(arguments[1]);
        int jobIndex = -1;

        // Find the job with the specified job number
        for (int i = 0; i < jobCount; i++) {
            if (jobs[i].jobNumber == jobNumber) {
                jobIndex = i;
                break;
            }
        }
        if (jobIndex >= 0) {
            int status = 0;
            waitpid(jobs[jobIndex].pid, &status, WUNTRACED);
            if (WIFSTOPPED(status)) {
                jobs[jobIndex].status = 0; // Stopped
            } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                removeJob(jobs[jobIndex].pid);
            }
        } else {
            printf("Job number not found.\n");
        }
    } else {
        printf("Usage: fg <job number>\n");
    }
}

void execBg(char** arguments, int argCount) {
    // Restart a stopped background job
    if (argCount > 1) {
        int jobNumber = atoi(arguments[1]);
        int jobIndex = -1;
        
        // Find the job with the specified job number

        for (int i = 0; i < jobCount; i++) {
            if (jobs[i].jobNumber == jobNumber) {
                jobIndex = i;
                break;
            }
        }

        if (jobIndex >= 0 && !jobs[jobIndex].status) {
            kill(jobs[jobIndex].pid, SIGCONT);
            jobs[jobIndex].status = 1; // Running
        } else {
            printf("Job number not found or job is not stopped.\n");
        }
    } else {
        printf("Usage: bg <job number>\n");
    }
}

int executeShellCommand(char* command, int clientSocket) {
    char* arguments[MAX_ARGUMENTS];
    int argCount = parseCommand(command, arguments);
    if (argCount == 0) {
        return 1;
    }

    if (strcmp(arguments[0], "exit") == 0) {
        return 0;
    } else if (strcmp(arguments[0], "cd") == 0) {
        if (argCount > 1) {
            executeCd(arguments[1]);
        } else {
            printf("cd: missing argument\n");
        }
    } else if (strcmp(arguments[0], "help") == 0) {
        executeHelp();
    } else if (strcmp(arguments[0], "jobs") == 0) {
        printJobs();
    } else if (strcmp(arguments[0], "fg") == 0) {
        execFg(arguments, argCount);
    } else if (strcmp(arguments[0], "bg") == 0) {
        execBg(arguments, argCount);
    } else {
        if (argCount > 1 && strcmp(arguments[argCount - 1], "&") == 0) {
            runInBackground = 1;
            arguments[argCount - 1] = NULL;
        } else {
            runInBackground = 0;
        }
        execArgs(arguments, clientSocket);
    }
    return 1;
}



void handleError(const char* message) {
    perror(message);
    exit(1);
}

int createServerSocket() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP connection
    if (serverSocket < 0) {
        handleError("Error opening socket");
    }
    return serverSocket;
}

// Bind the server socket to the server address
void bindServerSocket(int serverSocket, const struct sockaddr_in* serverAddress) {
    if (bind(serverSocket, (const struct sockaddr*)serverAddress, sizeof(*serverAddress)) < 0) {
        handleError("Error on binding");
    }
}

// Accept a client connection
int acceptClientSocket(int serverSocket, struct sockaddr_in* clientAddress, socklen_t* clientLength) {
    int clientSocket = accept(serverSocket, (struct sockaddr*)clientAddress, clientLength);
    if (clientSocket < 0) {
        handleError("Error on accept");
    }
    return clientSocket;
}

bool handleClientConnection(int clientSocket) {
    char buffer[BUFFER_SIZE];
    int n; // Number of characters read or written
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        // Read the message until a newline character is encountered
        n = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (n < 0) {
            handleError("Error reading from socket");
            break;
        }

        // Print the received message
        printf("Here is the message: %s \n", buffer);
        // fflush(stdout);
        // printf(buffer);
        if(strcmp(buffer, "run shell") == 0) {
            fflush(stdout);
            printf("Shell run in the server...\n");
            write(clientSocket, "Shell is running\n", strlen("Shell is running\n"));
            do {
                signal(SIGINT, sigint_handler);
                memset(buffer, 0, BUFFER_SIZE);
                write(clientSocket,":) ", strlen(":) "));
                memset(buffer, 0, BUFFER_SIZE);
                // fflush(stdout);
                read(clientSocket, buffer, sizeof(buffer));
                executeShellCommand(buffer, clientSocket);
                buffer[strcspn(buffer, "\n")] = '\0'; // Remove the newline character
                write(clientSocket, buffer, strlen(buffer));
                if(strcmp(buffer, "exit") == 0) {
                    break;
                }
                memset(buffer, 0, BUFFER_SIZE);
            
            } while (1);
            return false;
        }

        // Check for the "exit" command
        if (strcmp(buffer, "exit") == 0) {
            write(clientSocket, "exit\n", strlen("exit\n"));
            return false;
        }

        // Send a response to the client
        n = write(clientSocket, "I got your message\n", strlen("I got your message\n"));
        if (n < 0) {
            handleError("Error writing to socket");
            break;
        }
    }

    // Close the client socket outside the loop
    close(clientSocket);
    return true;
}


int main() {
    int serverSocket = createServerSocket();

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET; // IPv4
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Accept connections from any address, including localhost
    serverAddress.sin_port = htons(8080); // port number

    bindServerSocket(serverSocket, &serverAddress);

    listen(serverSocket, 5); // Maximum number of client connections

    struct sockaddr_in clientAddress;
    socklen_t clientLength = sizeof(clientAddress);

    bool running = true;
    while (running) {
        int clientSocket = acceptClientSocket(serverSocket, &clientAddress, &clientLength);
        running = handleClientConnection(clientSocket);
        close(clientSocket);
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
