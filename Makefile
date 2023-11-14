BIN = ./bin/
SOURCE = ./src/
CC = gcc

.PHONY: all index peer clean

all: $(BIN)index $(BIN)peer

$(BIN)%: $(SOURCE)%.c | $(BIN)
	$(CC) -o $@ $<

$(BIN):
	mkdir $@

clean:
	rm -f $(BIN)index $(BIN)peer
