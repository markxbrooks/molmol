TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(SGDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = ExPathNames.o ExXMac.o ExPlot.o ExInitAll.o ExUndo.o \
      ExHelpViewer.o ExHelpHtml.o ExHelpTip.o \
      ExHelpAbout.o ExHelpAprop.o ExHelpMouse.o ExHelpButton.o ExHelpProp.o \
      ExQuit.o ExSystem.o ExRegister.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libcmd.a

$(LIBDIR)/libcmd.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@
	$(WAIT)

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
