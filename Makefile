CC			=	gcc
CPP			=	g++
CPPFLAGS	=	-W -Wall -g
AR	    	= 	ar
ARFLAGS 	= 	rcv
LDFLAGS 	= 	-Wl,-rpath -Wl,/usr/lib -Wl,-rpath,/usr/lib -L. -liniparser -L./socket -lcsocket

TARGETS	=	libiniparser.a inotify_mod


SRCS = iniparser/iniparser.c \
	   iniparser/dictionary.c

OBJS = $(SRCS:.c=.o)

.PHONYl:all clean distclean

all:$(TARGETS)

clean:
	rm *.o *~ $(TARGETS) $(OBJS) depend	-f

distclean:
	rm *.o	$(TARGETS) $(OBJS) libiniparser.a libiniparser.so* -f

depend:$(wildcard *.c *.h)
	$(CC) $(CPPFLAGS) -MM $^ > $@

libiniparser.a:	$(OBJS)
	@($(AR) $(ARFLAGS) libiniparser.a $(OBJS))

inotify_mod:
	$(CPP) $(CPPFLAGS)	-o $@ $^ $(LDFLAGS) 

inotify_mod: inotify_mod.o comm.o configure.o watch.o help.o 

-include depend
