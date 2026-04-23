TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = ExSetAngle.o ExRotAngle.o ExBuild.o ExMeanMol.o ExFirstMol.o \
      ExNameMol.o ExRemoveMol.o ExRemoveRes.o ExAtom.o ExBond.o ExDist.o \
      ExFlipAtom.o ExSetNeigh.o ExAngle.o ExAco.o
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
