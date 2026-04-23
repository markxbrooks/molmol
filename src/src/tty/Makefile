TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) $(MOTIFDEF)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = TTYDev.o TTY.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libpu.a

$(LIBDIR)/libpu.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@
	$(WAIT)

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
