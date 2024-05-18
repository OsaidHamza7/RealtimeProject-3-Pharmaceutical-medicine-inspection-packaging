CC = gcc
CFLAGS = -g -Wall 
ARGS= arguments.txt
UILIBS = -lglut -lGLU -lGL -lm  -lrt
LIBS = -lpthread
NAMES = parent 

all: parent

parent: parent.c functions.c ipcs.c 
	$(CC) $(CFLAGS) -o parent parent.c functions.c ipcs.c  $(LIBS)

run: parent
	./parent $(ARGS) 

clean:
	rm -f $(NAMES)