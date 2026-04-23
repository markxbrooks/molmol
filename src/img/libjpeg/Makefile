TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(IMGDIR)/lib
INCLUDES = -I$(TOOLSDIR)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = cdjpeg.o cjpeg.o ckconfig.o djpeg.o jaricom.o \
      jcapimin.o jcapistd.o jcarith.o jccoefct.o jccolor.o \
      jcdctmgr.o jchuff.o jcinit.o jcmainct.o jcmarker.o \
      jcmaster.o jcomapi.o jcparam.o jcprepct.o jcsample.o \
      jctrans.o jdapimin.o jdapistd.o jdarith.o jdatadst.o \
      jdatasrc.o jdcoefct.o jdcolor.o jddctmgr.o jdhuff.o \
      jdinput.o jdmainct.o jdmarker.o jdmaster.o jdmerge.o \
      jdpostct.o jdsample.o jdtrans.o jerror.o jfdctflt.o \
      jfdctfst.o jfdctint.o jidctflt.o jidctfst.o jidctint.o \
      jmemmgr.o jmemname.o jmemnobs.o jpegtran.o \
      jquant1.o jquant2.o jutils.o rdbmp.o rdcolmap.o \
      rdgif.o rdjpgcom.o rdppm.o rdrle.o rdswitch.o \
      rdtarga.o transupp.o wrbmp.o wrgif.o wrjpgcom.o \
      wrppm.o wrrle.o wrtarga.o

SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libjpeg.a

$(LIBDIR)/libjpeg.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
