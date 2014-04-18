# Makefile for cs171 HW 0
# Just type "make" to compile; if it works right, you should have an
# executable called hw0.

CXX		= g++
# add -g for debugging info
CXXFLAGS	= -Wall -g -DMACOSX

# use this for mac:
#LDFLAGS	= -L/usr/X11R6/lib -framework GLUT -framework OpenGl -lpng

# use this for linux:
LDFLAGS	= -L/usr/X11R6/lib -lglut -lGL -lGLU -lXi -lXmu -lpng

SRCS	= *.cpp *.h
OBJS	= main.o neighbors.o data.o matrix.o vec3.o readpng.o
PROG	= depixeler

all: $PROG

$PROG: $(OBJS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o $(PROG) $^

clean:
	rm *.o $(PROG)

main.o: main.cpp data.h
	$(CXX) $(CXXFLAGS) -c main.cpp

neighbors.o: neighbors.cpp neighbors.h
	$(CXX) $(CXXFLAGS) -c neighbors.cpp

matrix.o: matrix.cpp matrix.h
	$(CXX) $(CXXFLAGS) -c matrix.cpp

readpng.o: readpng.cpp readpng.h
	$(CXX) $(CXXFLAGS) -c readpng.cpp

vec3.o: vec3.cpp vec3.h
	$(CXX) $(CXXFLAGS) -c vec3.cpp
    
.cpp.o:
	g++ -c $(CXXFLAGS) -o $@ $<

.PHONY: all clean
