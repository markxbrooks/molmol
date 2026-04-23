TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = ExCalcAxis.o ExCalcHelix.o ExCalcRmsd.o ExCalcSurf.o ExCalcShift.o \
      ExCalcBond.o ExCalcDist.o ExCalcVdw.o ExCalcHbond.o \
      ExCalcMatch.o ExCalcClust.o \
      ExCalcAngle.o ExCalcAtom.o ExCalcSecond.o ExCheckLimit.o ExCheckAco.o \
      ExCalcPot.o
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
