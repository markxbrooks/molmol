TOP      = ../../..

include $(TOP)/makedef

LIBDIR   = $(SGDIR)/lib
INCLUDES = -I$(SGDIR)/include -I$(TOOLSDIR)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = VRML1Dev.o VRML1Init.o VRML1Frame.o \
      VRML1Marker.o VRML1Line.o VRML1Poly.o VRML1Text.o VRML1Shape.o \
      VRML1Color.o VRML1Light.o VRML1Util.o
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
