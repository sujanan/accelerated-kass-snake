CXX=g++ 
CXX_FLAGS=-fvisibility=hidden
CXX_LIBS=-lDIP -llapack

CC=gcc
CC_LIBS=-lglut

all: main

main: main.o snake.o
	$(CXX) -o main main.o snake.o $(CXX_LIBS) $(CC_LIBS)

snake: snake.o
	$(CXX) -o snake snake.o $(CXX_LIBS)

snake.o: snake.cpp
	$(CXX) $(CXX_FLAGS) -c snake.cpp

clean:
	rm -rf *.o *.gch snake main

