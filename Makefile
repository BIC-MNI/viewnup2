PROGS = viewnup
HEADERS =  callbacks.h \
geometry.h \
gtk_gl.h \
interface.h \
minc_io.h \
trackball.h \
viewnup.h \
pixmaps.h \
lookup_table.h

EXT_HEADERS = config.h globals.h

OBJS = $(HEADERS:.h=.o)

CC = gcc

WARNINGS = -Wall -Wunused -Wmissing-prototypes -Wmissing-declarations

# OPTIONS = -O3 $(WARNINGS) -DGTK_DISABLE_DEPRECATED=1
OPTIONS = -O3 $(WARNINGS)

INCLUDES = -I/usr/local/mni/include -I/usr/include \
	`pkg-config gtkglext-1.0 gtk+-2.0 glib-2.0 --cflags`
CFLAGS = $(OPTIONS) $(INCLUDES)

LDINCLUDES = -L/usr/local/mni/lib \
	`pkg-config gtkglext-1.0 gtk+-2.0 glib-2.0 --libs`
LDLIBS = -lpopt -lGL -lGLU -lvolume_io -lminc -lnetcdf -lm

LDOPTS = $(LDINCLUDES) $(LDLIBS)


all: $(PROGS) 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGS): $(OBJS)
	$(CC) $(OBJS) -o $@ $(OPTIONS) $(LDOPTS)
        
clean:
	rm -f *.o *~ $(PROGS)

