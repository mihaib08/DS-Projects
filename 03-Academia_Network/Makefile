CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -m32
PUBL=publications

.PHONY: build clean

build: $(PUBL).o

$(PUBL).o: $(PUBL).c $(PUBL).h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o *.h.gch
