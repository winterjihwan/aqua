.PHONY: main

CC = clang++
CFLAGS = -std=c++11 -O3 -Wall -Wno-deprecated -pedantic
INCLUDE = -I/usr/local/include/
LIB = -L/usr/local/lib/
GL = -framework OpenGL -framework GLUT

all: main

main: main
	$(CC) $(INCLUDE) $(LIB) $(GL) $(CFLAGS) $(LIBS) main.cc -o main
