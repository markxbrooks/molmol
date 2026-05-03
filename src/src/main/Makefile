TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
# IODEVDEF so MolMol.c can pick a default IO device matching the linked libio.a
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS) $(IODEVDEF)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

LIBS     = $(LIBDIR)/libcip.a $(LIBDIR)/libcmd.a \
	   $(LIBDIR)/libui.a \
	   $(LIBDIR)/libgraph.a $(LIBDIR)/libio.a $(LIBDIR)/libpu.a \
	   $(LIBDIR)/libcalc.a $(LIBDIR)/libprim.a $(LIBDIR)/libdata.a \
	   $(LIBDIR)/libattr.a $(LIBDIR)/libfileio.a $(LIBDIR)/libos.a
SGLIB    = $(SGDIR)/lib/libsg.a
TOOLSLIB = $(TOOLSDIR)/lib/libtools.a $(IMGLIB)

PROGRAM = molmol
OBJ = MolMol.o MolInit.o
SRC = $(OBJ:.o=.c)

default: $(PROGRAM)

$(PROGRAM): $(OBJ) $(LIBS) $(SGLIB) $(TOOLSLIB)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LIBS) $(SGLIB) $(TOOLSLIB) $(SYSLIB)

clean:
	rm -f $(OBJ) $(PROGRAM) core *.ps

depend: $(SRC)
include $(TOP)/makedep
