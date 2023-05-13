CC = gcc `pkg-config --cflags gtk+-3.0 glib-2.0 xcb` -I ./include -c -rdynamic
LD_FLAGS = `pkg-config --libs gtk+-3.0 glib-2.0 xcb` -rdynamic


SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)


all: code resources


%.o: %.c
	$(CC) $< -o $@

code: $(OBJECTS)
	gcc $^ -o main $(LD_FLAGS)

resources:
	$(MAKE) -C res -B all

clean:
	rm *.o main -rf
