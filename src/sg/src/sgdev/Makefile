TOP      = ../../..

include $(TOP)/makedef

LIBDIR   = $(SGDIR)/lib
INCLUDES = -I$(SGDIR)/include -I$(TOOLSDIR)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) $(SGDEVDEF)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = SgDev.o SgName.o SgPlot.o SgInit.o SgError.o SgView.o SgObj.o \
      SgClear.o SgDraw.o SgText.o SgPick.o SgRange.o \
      SgBuff.o SgSet.o SgTransf.o
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
