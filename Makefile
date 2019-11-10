# Makefile for hdaps-gl

LIBDIR		= -L/usr/X11R6/lib
CFLAGS		= -O2 -W -Wall -Wshadow -Waggregate-return -Wbad-function-cast \
		  -Wpointer-arith -Wmissing-prototypes -Wmissing-declarations \
		  -Wcast-align -Wdisabled-optimization -Wstrict-prototypes \
		  -Wcast-qual -Wwrite-strings -Wredundant-decls
LIBRARIES	= -lglut -lGL -lGLU -lm

all: hdaps-gl

hdaps-gl: hdaps-gl.c
	$(CC) $(CFLAGS) $(LIBDIR) $(LIBRARIES) -o hdaps-gl hdaps-gl.c

clean:
	rm -f hdaps-gl *.o
