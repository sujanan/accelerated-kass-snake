CXX=g++ 
CXX_FLAGS=-fvisibility=hidden
CXX_LIBS=-lopencv_core -lopencv_imgproc
CC_LIBS=-lraylib -lm -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lGLU

acseq: acseq.o snake.o
	$(CXX) -o acseq acseq.o snake.o $(CC_LIBS) $(CXX_LIBS)

acpar: acpar.o snake.o
	$(CXX) -o acpar acpar.o snake.o $(CC_LIBS) $(CXX_LIBS)

snake.o: snake.h

clean:
	rm -rf *.o *.gch acpar acseq

