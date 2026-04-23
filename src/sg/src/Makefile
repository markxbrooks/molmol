TOP      = ../..

include $(TOP)/makedef

SUBDIRS  = sgdev def pick range $(SGDEVDIR)
TARGETS  = default clean depend

$(TARGETS):
	$(MAKE) $(SUBDIRS) TARGET=$(TARGET)

$(SUBDIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:
