CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=
BIN=./bin

all: demo_animation test_et6220 writeto_et6220 parser_test clean.o

demo_animation: demo_animation.o et6220.o
	$(CC) $(LDFLAGS) demo_animation.o et6220.o -o $(BIN)/demo_animation

test_et6220: test_et6220.o et6220.o
	$(CC) $(LDFLAGS) test_et6220.o et6220.o -o $(BIN)/test_et6220

writeto_et6220: writeto_et6220.o segment_parser.o et6220.o
	$(CC) $(LDFLAGS) writeto_et6220.o segment_parser.o et6220.o -o $(BIN)/writeto_et6220	

parser_test: parser_test.o segment_parser.o et6220.o
	$(CC) $(LDFLAGS) parser_test.o segment_parser.o et6220.o -o $(BIN)/parser_test

demo_animation.o:
	$(CC) $(CFLAGS) demo_animation.c

test_et6220.o:
	$(CC) $(CFLAGS) test_et6220.c	

writeto_et6220.o:
	$(CC) $(CFLAGS) writeto_et6220.c		

et6220.o:
	$(CC) $(CFLAGS) lib/et6220.c

segment_parser.o:
	$(CC) $(CFLAGS) lib/segment_parser.c	

parser_test.o:
	$(CC) $(CFLAGS) parser_test.c

clean: clean.o
	rm -f $(BIN)/*

clean.o:
	rm -f *.o
