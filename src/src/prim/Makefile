TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = PrimHand.o PrimDump.o PrimSel.o \
      PlateCalc.o SolidCalc.o RibbonBuild.o RibbonCalc.o SheetCalc.o \
      SurfaceTrim.o PrimDraw.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libprim.a

$(LIBDIR)/libprim.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
