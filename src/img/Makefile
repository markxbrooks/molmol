# zlib must build before libpng; libtiff may use zlib/jpeg after those exist.
SRCDIRS  = zlib libjpeg libpng libtiff
SUBDIRS  = $(SRCDIRS) lib
TARGETS  = default depend

$(TARGETS):
	$(MAKE) $(SRCDIRS) TARGET=$(TARGET)

clean:
	$(MAKE) $(SUBDIRS) TARGET=$@

$(SUBDIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:
