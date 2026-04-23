TOP      = ../../..

include $(TOP)/makedef

LIBDIR   = $(SGDIR)/lib
INCLUDES = -I$(SGDIR)/include -I$(TOOLSDIR)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = XGLDev.o XGLInit.o XGLView.o XGLClear.o \
      XGLMarker.o XGLLine.o XGLPoly.o XGLText.o XGLMesh.o \
      XGLFeature.o XGLColor.o XGLTransf.o XGLLight.o
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
