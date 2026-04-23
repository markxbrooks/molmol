TOP      = ../../..

include $(TOP)/makedef

LIBDIR   = $(SGDIR)/lib
INCLUDES = -I$(SGDIR)/include -I$(TOOLSDIR)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = FMDev.o FMInit.o FMFrame.o FMLine.o FMPoly.o FMCircle.o FMText.o FMColor.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libsg.a

$(LIBDIR)/libsg.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@
	$(WAIT)

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
