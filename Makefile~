CC = gcc
CFLAGS = 
CPPFLAGS =

TARGETS = tsl
MANPAGE = tsl.1.gz

OBJS = tsl.o
LDFLAGS = -g -lcurl #-D_DEBUG
CPPFLAGS =

all: ${TARGETS} rmo

tsl: ${OBJS}
	${CC} ${LDFLAGS} $^ -o $@

rmo: ${OBJS}
	@rm -rf ${OBJS}

clean:
	@rm -f *.o
	@rm -f ${TARGETS}

install:
	cp -f ${TARGETS} /usr/bin/
	cp -f ${MANPAGE} /usr/share/man/man1/

uninstall:
	rm -rf /usr/bin/${TARGETS}
	rm -rf /usr/share/man/man1/${MANPAGE}
