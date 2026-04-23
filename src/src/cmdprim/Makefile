TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = ExCircles.o ExTrajec.o ExPlates.o ExCylinder.o ExDipole.o ExSolid.o \
      ExRibbon.o ExSplitRib.o ExSheet.o ExSurface.o ExIso.o \
      ExText.o ExDrawobj.o \
      ExRemovePrim.o ExMovePrim.o ExLenPrim.o ExSizePlate.o ExEditText.o \
      ExTypeRib.o ExStyleRib.o ExSizeRib.o ExPaintRib.o \
      ExPaintSurf.o ExTrimSurf.o ExStyleCyl.o ExStyleMap.o ExStyleDobj.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libcmd.a

$(LIBDIR)/libcmd.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@
	$(WAIT)

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
