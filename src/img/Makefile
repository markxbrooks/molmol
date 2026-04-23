SRCDIRS  = libjpeg libpng libtiff zlib
SUBDIRS  = $(SRCDIRS) lib
TARGETS  = default depend

$(TARGETS):
	$(MAKE) $(SRCDIRS) TARGET=$(TARGET)

clean:
	$(MAKE) $(SUBDIRS) TARGET=$@

$(SUBDIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:
