CC = gcc -std=c11
CFLAGS = -Wall -Wextra -pthread
SRC_DIR = src
BIN_DIR = .

CLIENT_SRC = $(wildcard $(SRC_DIR)/client.c)
SERVER_SRC = $(wildcard $(SRC_DIR)/server.c)

CLIENT_OBJ = $(CLIENT_SRC:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)
SERVER_OBJ = $(SERVER_SRC:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

all: client server

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) -o $(BIN_DIR)/client

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(SERVER_OBJ) -o $(BIN_DIR)/server

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)  # Create the directory if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN_DIR)/client $(BIN_DIR)/server $(CLIENT_OBJ) $(SERVER_OBJ)

