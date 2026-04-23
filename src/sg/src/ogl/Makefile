TOP      = ../../..

include $(TOP)/makedef

LIBDIR   = $(SGDIR)/lib
INCLUDES = -I$(SGDIR)/include -I$(TOOLSDIR)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) $(OPENGLDEF)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = OGLDev.o OGLInit.o OGLView.o OGLObj.o OGLClear.o \
      OGLMarker.o OGLLine.o OGLPoly.o OGLText.o OGLMesh.o \
      OGLFeature.o OGLColor.o OGLTransf.o OGLStereo.o OGLLight.o OGLUtil.o
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
