

INC = -L/usr/local/lib -I/usr/include/glib-2.0/  -I/usr/lib/glib-2.0/include

LIBS = -lglib-2.0 -lgthread-2.0 -lpthread -lfcgi -ldl
LIBS += -lz 

all: main

main: main.c route.c
	gcc -o ff -g main.c route.c $(LIBS) $(INC) -Wall

