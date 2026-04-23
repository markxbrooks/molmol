TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = OsSleep.o OsSeed.o OsDir.o OsSystem.o OsMail.o OsBrowse.o \
      GFile.o CurrDir.o HomeDir.o ProgDir.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libos.a

$(LIBDIR)/libos.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
