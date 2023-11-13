BIN = ./bin/
SOURCE = ./src/
CC = gcc

.PHONY: all server client

all: $(BIN)server $(BIN)client

server: $(BIN)server

client: $(BIN)client

$(BIN)%: $(SOURCE)%.c
	$(CC) -o $@ $<
