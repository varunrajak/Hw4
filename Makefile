CC = gcc
CFLAGS = -Wall -pthread
SOURCES = scheduler.c queue.c
EXECS = scheduler

all: $(EXECS)

scheduler: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(EXECS) *.out *.err
