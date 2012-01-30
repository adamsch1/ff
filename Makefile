# Generated automatically from Makefile.in by configure.
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

prefix = /usr/local
exec_prefix = ${prefix}

SHAREDOPT = -shared
LIBDIR = $(prefix)/lib .
INCDIR = $(prefix)/include
MANDIR	= $(prefix)/man/man3
SHELL = /bin/sh
EXTRA_LIBS = -lfcgi -ldl -lz

INC = -I .
FLAGS = -Wall -fpic -O3 -g
CC = gcc
OBJS = *.o

#.c.o: $(CC) -g $(FLAGS) -c -g $<

INC = -L/usr/local/lib -L/usr/lib -I/usr/include/glib-2.0/  -I/usr/lib/glib-2.0/include -Llib
 
all: libff.a session main blog 

session: session.c
	gcc -shared -g session.c -lff $(FLAGS) -o lib/libsession.so -L . $(EXTRA_LIBS) $(INC) -Wall

blog: blog.c
	gcc -shared -g blog.c -lff $(FLAGS) -o controllers/blog.so -L .

main: main.c route.c util.c  ccgi.c array.c
	gcc -o ff -g main.c route.c util.c ccgi.c array.c $(EXTRA_LIBS) $(INC) -Wall -Llib -lsession

libff.a: ccgi.o template.o array.o util.o form.o array.o $(CRYPT)
	ar r libff.a ccgi.o template.o array.o util.o form.o array.o $(CRYPT)
	ranlib libff.a

ccgi.o: ccgi.c ccgi.h

prefork.o: prefork.c

crypt.o: crypt.c ccgi.h

test: libccgi.a
	cd t; make

clean:
	find ./ -name '*.*o' -exec rm -f {} \;
	find ./ -name '*.a' -exec rm -f {} \;

