CC=gcc
CFLAG=-g

LIBS= -lpthread

all: build

build: build-test-shutdown build-test-heavy

build-test-shutdown: tests/test-shutdown.o pool.o
	$(CC) -o tests/test-shutdown tests/test-shutdown.o pool.o $(LIBS)

build-test-heavy: tests/test-heavy.o pool.o
	$(CC) -o tests/test-heavy tests/test-heavy.o pool.o $(LIBS)

check: build test-shutdown test-heavy

test-shutdown: build-test-shutdown
	./tests/test-shutdown

test-heavy:
	./tests/test-heavy

pool.o: pool.h

clean:
	rm -f *.o tests/*.o
	rm -f tests/test-shutdown tests/test-heavy
