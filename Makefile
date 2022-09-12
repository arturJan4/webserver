CC = gcc -g

DEBUGFLAGS = -fsanitize=address -fsanitize=leak -fsanitize=undefined -static-libasan

CFLAGS = -O2 -std=gnu17 -Wall -Wextra -Werror
#CFLAGS = -O2 -std=gnu17 $(DEBUGFLAGS)

OBJS = main.o input.o helpers.o request.o response.o server.o

all: webserver

webserver: $(OBJS)
	$(CC) $(CFLAGS) -o webserver $(OBJS)

input.o: input.c input.h
helpers.o: helpers.c helpers.h
request.o: request.c request.h
response.o: response.c response.h
server.o: server.c server.h

SRC_C = $(wildcard *.c)
SRC_H = $(wildcard *.h)

clean:
	rm -f *.o

distclean:
	rm -f *.o webserver

format:
	clang-format --style=Google -i $(SRC_C) $(SRC_H)