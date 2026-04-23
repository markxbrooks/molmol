TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOOLSDIR)/lib
INCLUDES = -I$(TOOLSDIR)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = Break.o RandNum.o \
      MatVec.o TrackBall.o EllInteg.o \
      Callback.o Tree.o Hashtab.o Linlist.o RectSet.o Grid.o \
      StrTab.o DStr.o StrMatch.o FileName.o LeastSqr.o CubicSpline.o \
      Sphere.o IsoSurface.o SurfaceDots.o SurfaceSolid.o \
      SurfCheck.o Potential.o \
      FitCylinder.o MapCoord.o PrincipAxis.o \
      SingVal.o \
      Cluster.o SimulAnneal.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libtools.a

$(LIBDIR)/libtools.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
