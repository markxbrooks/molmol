TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = CalcRmsd.o CalcPair.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libcalc.a

$(LIBDIR)/libcalc.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
