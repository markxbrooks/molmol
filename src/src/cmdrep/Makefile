TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = ExFullscreen.o ExDrawSize.o ExStereo.o ExDrawDelay.o ExDrawPrec.o \
      ExGroup.o ExZoom.o ExRotate.o ExMove.o ExAutoScale.o \
      ExBondMode.o ExLenBond.o ExCenter.o ExFit.o \
      ExBackCol.o ExClipPlanes.o ExViewAngle.o \
      ExLight.o ExFog.o ExProj.o ExRend.o
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
