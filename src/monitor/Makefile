CC := gcc
INC := -I../lib
CFLAGS := -std=gnu99 -Wall -g $(INC)
LIBS := -lpthread
EXEC := monitor
UTILS := $(wildcard ../lib/*.c)

SRC=$(wildcard *.c)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(UTILS) $^ $(LIBS)
	mv $(EXEC) ../../build/

clean:
	rm -rf *.o

.PHONY: $(EXEC)