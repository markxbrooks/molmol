TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) $(PUDEVDEF)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = PuDev.o PuInit.o PuEvent.o PuExtIn.o PuCB.o \
      PuMainW.o PuMenu.o PuCmdField.o PuVal.o \
      PuBusy.o PuMsg.o PuHelp.o PuTextW.o PuDial.o
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
