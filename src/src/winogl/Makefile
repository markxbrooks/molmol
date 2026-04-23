TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = $(IMGINCL) -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) $(WINDEF) $(IMGDEF)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = WinOGLDev.o WinOGL.o WinOGLDump.o
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
