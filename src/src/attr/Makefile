TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = AttrMng.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libattr.a

$(LIBDIR)/libattr.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
