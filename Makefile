CXX=g++ 
CXX_FLAGS=-fvisibility=hidden
CXX_LIBS=-lDIP -llapack

CC=gcc
CC_INCS=-I./deps/include
CC_LIBS=-lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lGLEW -lGLU

VPATH=deps

all: main

main: main.o snake.o glad.o
	$(CXX) -o main main.o snake.o glad.o $(CXX_LIBS) $(CC_LIBS)

main.o: main.c snake.h
	$(CC) -c $(CC_INCS) main.c

snake: snake.o
	$(CXX) -o snake snake.o $(CXX_LIBS)

snake.o: snake.h
	$(CXX) $(CXX_FLAGS) -c snake.cpp

glad.o: ./deps/glad.c
	$(CC) -c $(CC_INCS) ./deps/glad.c 

clean:
	rm -rf *.o *.gch snake main

