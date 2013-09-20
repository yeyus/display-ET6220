CC=gcc
CFLAGS=-c -Wall
BIN=./bin/
SOURCE=./

PROG=demo_animation test_et6220 writeto_et6220 parser_test
LIST=$(addprefix $(BIN)/, $(PROG))

all: $(LIST)

$(BIN)/%:  $(SOURCE)%.c
	$(CC) $< $(CFLAGS) -o $@

clean:
	rm -f $(BIN)/*