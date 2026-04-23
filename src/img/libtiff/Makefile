TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(IMGDIR)/lib
INCLUDES = -I$(TOOLSDIR)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = mkg3states.o \
      tif_aux.o tif_close.o tif_codec.o tif_color.o \
      tif_compress.o tif_dir.o tif_dirinfo.o tif_dirread.o \
      tif_dirwrite.o tif_dumpmode.o tif_error.o \
      tif_extension.o tif_fax3.o tif_fax3sm.o tif_flush.o \
      tif_getimage.o tif_jbig.o tif_jpeg.o tif_jpeg_12.o \
      tif_luv.o tif_lzma.o tif_lzw.o tif_next.o \
      tif_ojpeg.o tif_open.o tif_packbits.o tif_pixarlog.o \
      tif_predict.o tif_print.o tif_read.o tif_strip.o \
      tif_swab.o tif_thunder.o tif_tile.o tif_unix.o \
      tif_version.o tif_warning.o tif_write.o \
      tif_zip.o

SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libtiff.a

$(LIBDIR)/libtiff.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
