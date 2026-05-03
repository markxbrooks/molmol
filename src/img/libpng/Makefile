TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(IMGDIR)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(ZLIBDIR)
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = png.o pngerror.o pngget.o pngmem.o \
      pngpread.o pngread.o pngrio.o pngrtran.o \
      pngrutil.o pngset.o pngtest.o pngtrans.o \
      pngwio.o pngwrite.o pngwtran.o pngwutil.o

SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libpng.a

$(LIBDIR)/libpng.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
