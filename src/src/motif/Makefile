TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) $(MOTIFDEF)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = MotifDev.o MotifInit.o MotifEvent.o MotifExtIn.o MotifCB.o \
      MotifPlace.o MotifText.o MotifMainW.o MotifMenu.o MotifVal.o \
      MotifBusy.o MotifMsg.o MotifHelp.o MotifTextW.o MotifDial.o
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
