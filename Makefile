CC = gcc
CFALGS = -std=c99 -Wall -Wextra -fsanitize=address -g
LIBS = -lasan -lm -lpthread -lpulse-simple -lpulse -lfftw3

OBJS = main.o db.o mfcc.o

all: main

main: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o main $(LIBS)

main.o: main.c
	$(CC) $(CFALGS) -c main.c -o main.o

db.o: db.c db.h
	$(CC) $(CFALGS) -c db.c -o db.o

mfcc.o: mfcc.c mfcc.h
	$(CC) $(CFALGS) -c mfcc.c -o mfcc.o

clean:
	rm -f $(OBJS) main