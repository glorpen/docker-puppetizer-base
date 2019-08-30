SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:%.c=%.o)
CC      := gcc -std=c99 -Os -g2
CFLAGS  := -Wall -pedantic -I.
LDFLAGS := -Wall -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

init: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	rm -f $(OBJECTS) init

init.static: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -static -o $@
