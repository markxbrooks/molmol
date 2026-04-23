TOP      = ../..

include $(TOP)/makedef

LIBDIR   = $(TOP)/lib
INCLUDES = -I$(TOOLSDIR)/include -I$(TOP)/include
CPPFLAGS = $(INCLUDES) $(MCPPFLAGS)
CFLAGS   = $(CPPFLAGS) $(MCFLAGS)

OBJ = UiSwitch.o UiMenu.o UiCmdField.o UiValuator.o UiShowSel.o UiTip.o
SRC = $(OBJ:.o=.c)

default: $(LIBDIR)/libui.a

$(LIBDIR)/libui.a: $(OBJ)
	$(AR) r $@ $?
	$(RANLIB) $@

clean:
	rm -f $(OBJ)

depend: $(SRC)
include $(TOP)/makedep
