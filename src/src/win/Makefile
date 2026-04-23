TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) $(WINDEF)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = WinDev.o WinInit.o WinEvent.o WinExtIn.o \
      WinText.o WinMainW.o WinMenu.o \
      WinMsg.o WinHelp.o WinTextW.o WinDial.o WinPlace.o
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
