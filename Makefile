CC		=	gcc
CPP		=	g++
CFLAGS	=	-g
AR	    = ar
ARFLAGS = rcv
LDFLAGS = -Wl,-rpath -Wl,/usr/lib -Wl,-rpath,/usr/lib -L. -liniparser -L./socket -lcsocket

TARGETS	=	libiniparser.a inotify_mod


SRCS = iniparser/iniparser.c \
	   iniparser/dictionary.c

OBJS = $(SRCS:.c=.o)

.PHONYl:all clean distclean

all:$(TARGETS)

clean:
	rm *.o *~ $(TARGETS) $(OBJS)	-f

distclean:
	rm *.o	$(TARGETS) $(OBJS) libiniparser.a libiniparser.so* -f


libiniparser.a:	$(OBJS)
	@($(AR) $(ARFLAGS) libiniparser.a $(OBJS))

inotify_mod		:	inotify_mod.o
	$(CPP) $(CFLAGS)	-o $@ $^ $(LDFLAGS) 
