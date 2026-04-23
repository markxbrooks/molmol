SRCDIRS  = tools sg img src
SUBDIRS  = $(SRCDIRS) lib
TARGETS  = default depend

default:

$(TARGETS):
	$(MAKE) $(SRCDIRS) TARGET=$@

clean:
	$(MAKE) $(SUBDIRS) TARGET=$@
	find */* -name makedep -exec cp /dev/null {} \;
	rm -f dump par history

$(SUBDIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:
