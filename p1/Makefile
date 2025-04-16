CC=gcc
CFLAGS=-I./src
LDFLAGS=

SRC_DIR=src
BUILD_DIR=build
BIN_DIR=bin

SERVER_SRCS = $(SRC_DIR)/server.c $(SRC_DIR)/base_connection.c
CLIENT_SRCS = $(SRC_DIR)/client.c $(SRC_DIR)/base_connection.c

SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CLIENT_SRCS))

SERVER_EXEC = $(BIN_DIR)/server
CLIENT_EXEC = $(BIN_DIR)/client

.PHONY: all clean directories

all: directories $(SERVER_EXEC) $(CLIENT_EXEC)

directories:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(SERVER_OBJS) -o $@ $(LDFLAGS)

$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
