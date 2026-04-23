TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) -DNMR
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = DataHand.o DataResLib.o DataInit.o DataWriteRes.o DataCalc.o \
      DataSel.o DataRef.o DataDist.o DataDump.o PropTab.o PropStand.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libdata.a

$(LIBDIR)/libdata.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@
	$(WAIT)

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
