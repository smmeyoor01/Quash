CC = gcc
CFLAGS = -Wall -Werror -g
OBJS = shell.o jobs.o pipe.o
TARGET = quash

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

shell.o: shell.c jobs.h pipe.h
	$(CC) $(CFLAGS) -c shell.c

jobs.o: jobs.c jobs.h
	$(CC) $(CFLAGS) -c jobs.c

pipe.o: pipe.c pipe.h
	$(CC) $(CFLAGS) -c pipe.c

clean:
	rm -f $(TARGET) $(OBJS)

run: all
	./$(TARGET)

.PHONY: all clean run