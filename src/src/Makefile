TOP      = ..

include $(TOP)/makedef

SUBDIRS  = os fileio attr data expr prim calc \
	   pudev $(PUDEVDIR) iodev $(IODEVDIR) graph ui \
	   cmdio cmddial cmdmisc cmdattr cmdrep cmdprop cmdprim \
	   cmdanim cmdstruc cmdcalc cmdfig cip main
TARGETS  = default clean depend

$(TARGETS):
	$(MAKE) $(SUBDIRS) TARGET=$(TARGET)

$(SUBDIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:
