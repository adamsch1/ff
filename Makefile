# Generated automatically from Makefile.in by configure.
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

prefix = /usr/local
exec_prefix = ${prefix}

SHAREDOPT = -shared
LIBDIR = $(prefix)/lib
INCDIR = $(prefix)/include
MANDIR	= $(prefix)/man/man3
SHELL = /bin/sh
EXTRA_LIBS = -lfcgi -ldl -lz

INCS =  -Isrc
FLAGS = -Wall -fpic

OBJS = *.o

.c.o: $(CC) $(FLAGS) -c $<

INC = -L/usr/local/lib -L/usr/lib -I/usr/include/glib-2.0/  -I/usr/lib/glib-2.0/include

all: main blog


blog: blog.c
	gcc -shared -o controllers/blog.so -c blog.c $(EXTRA_LIBS) $(INC) -Wall

main: main.c route.c util.c
	gcc -o ff -g main.c route.c util.c $(EXTRA_LIBS) $(INC) -Wall


all: $(OBJS) src/libcgi.so

	@echo ""
	@echo ""
	@echo ""
	@echo ""


shared: src/libcgi.so
	cp src/libcgi.so $(LIBDIR)

src/libcgi.a: $(OBJS)
	$(AR) rc src/libcgi.a $(OBJS)

src/libcgi.so: src/libcgi.a
	$(CC) $(SHAREDOPT) -o src/libcgi.so $(OBJS) $(EXTRA_LIBS)

install:
	cp src/libcgi.a $(LIBDIR)
	cp src/libcgi.so $(LIBDIR)
	cp src/cgi.h $(INCDIR)
	cp src/session.h $(INCDIR)


src/error.o: src/error.c src/error.h
src/cgi.o: src/cgi.c src/cgi.h
src/session.o: src/session.c src/session.h
src/base64.o: src/base64.c
src/md5.o: src/md5.c
src/string.o: src/string.c
src/cookie.o: src/cookie.c
src/general.o: src/general.c
src/list.o: src/list.c

clean:
	find ./ -name '*.*o' -exec rm -f {} \;
	find ./ -name '*.a' -exec rm -f {} \;

uninstall: clean
	rm -f $(LIBDIR)/libcgi.*
	rm -f $(INCDIR)/cgi.h
	rm -f $(INCDIR)/session.h
	rm -f $(MANDIR)/libcgi*

install_man:
	cp doc/man/man3/libcgi_base64.3 $(MANDIR)
	cp doc/man/man3/libcgi_cgi.3 $(MANDIR)
	cp doc/man/man3/libcgi_general.3 $(MANDIR)
	cp doc/man/man3/libcgi_string.3 $(MANDIR)
	cp doc/man/man3/libcgi_session.3 $(MANDIR)
	cp doc/man/man3/libcgi_cookie.3 $(MANDIR)

