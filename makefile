CC = gcc
CFLAGS = -g -Wall 
ARGS= arguments.txt
UILIBS = -lglut -lGLU -lGL -lm  -lrt
LIBS = -lpthread
NAMES = parent liquid_production_line

all: parent liquid_production_line

parent: parent.c functions.c ipcs.c 
	$(CC) $(CFLAGS) -o parent parent.c functions.c ipcs.c  $(LIBS)

liquid_production_line: liquid_production_line.c functions.c ipcs.c 
	$(CC) $(CFLAGS) -o liquid_production_line liquid_production_line.c functions.c ipcs.c  $(LIBS)

run: parent
	./parent $(ARGS) 

clean:
	rm -f $(NAMES)