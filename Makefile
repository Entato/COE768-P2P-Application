BIN = ./bin/
SOURCE = ./src/
CC = gcc

.PHONY: all server client clean

all: $(BIN)server $(BIN)client

server: $(BIN)server

client: $(BIN)client

$(BIN)%: $(SOURCE)%.c
	$(CC) -o $@ $<

clean:
	rm -f $(BIN)server $(BIN)client
