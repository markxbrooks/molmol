TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = $(IMGINCL) -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) $(MOTIFDEF) $(IMGDEF)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = MotOGLDev.o MotOGL.o MotOGLDump.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libio.a

$(LIBDIR)/libio.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@
	$(WAIT)

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
