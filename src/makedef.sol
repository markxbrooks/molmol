# definitons for Sun Solaris 2.8

IMGDIR = $(TOP)/../imglib/sol

IMGDEF  = -DTIFF_SUPPORT -DJPEG_SUPPORT -DPNG_SUPPORT
IMGINCL = -I$(IMGDIR)/include
IMGLIB  = $(IMGDIR)/lib/libtiff.a $(IMGDIR)/lib/libjpeg.a \
          $(IMGDIR)/lib/libpng.a $(IMGDIR)/lib/libz.a

OPENWINHOME = /usr/openwin
MOTIFHOME   = /usr/dt
MISSFUNC = -Dsqrtf=sqrt -Dexpf=exp -Dlogf=log -Dpowf=pow \
           -Dsinf=sin -Dcosf=cos -Dtanf=tan \
           -Dasinf=asin -Dacosf=acos -Datanf=atan -Datan2f=atan2 \
           -Dfabsf=fabs -Dceilf=ceil

MCPPFLAGS = $(MISSFUNC) -D_SYSTYPE_SVR4 -I$(OPENWINHOME)/include -I$(MOTIFHOME)/include
MCFLAGS  = -xO2 -Xa

AR       = /usr/ccs/bin/ar
RANLIB   = /bin/true
WAIT     = /usr/bin/sleep 2
SHELL    = /bin/sh
CPP      = /usr/ccs/lib/cpp
CC       = /opt/SUNWspro/bin/cc

SGDEVDIR = ogl x11 no pov rib vrml1 vrml2 ps fm
SGDEVDEF = -DSG_DEV_OGL -DSG_DEV_X11 -DSG_DEV_NO -DSG_DEV_POV -DSG_DEV_RIB -DSG_DEV_VRML1 -DSG_DEV_VRML2 -DSG_DEV_PS -DSG_DEV_CPS -DSG_DEV_FM3 -DSG_DEV_FM4

PUDEVDIR = motif tty
PUDEVDEF = -DPU_DEV_MOTIF -DPU_DEV_TTY

IODEVDIR = motogl motx11 ttyno
IODEVDEF = -DIO_DEV_MOTIF_X11 -DIO_DEV_MOTIF_OGL -DIO_DEV_TTY_NO

MOTIFDEF = -DFUNCPROTO

SYSLIB   = -L$(OPENWINHOME)/lib -L$(MOTIFHOME)/lib -R $(OPENWINHOME)/lib:$(MOTIFHOME)/lib -lGLw -lXm -lXmu -lXt -lGLU -lGL -lX11 -ldga -lm

TOOLSDIR = $(TOP)/tools
SGDIR    = $(TOP)/sg
