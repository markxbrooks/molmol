TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(IMGDIR)/lib
INCLUDES = -I$(TOOLSDIR)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = adler32.o compress.o crc32.o deflate.o \
      gzclose.o gzlib.o gzread.o gzwrite.o \
      infback.o inffast.o inflate.o inftrees.o \
      trees.o uncompr.o zutil.o

SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/zlib.a

$(LIBDIR)/zlib.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
