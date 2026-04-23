/*
************************************************************************
*
*   IODev.c - management of I/O devices
*
*   Copyright (c) 1994
*
*   ETH Zuerich
*   Institut fuer Molekularbiologie und Biophysik
*   ETH-Hoenggerberg
*   CH-8093 Zuerich
*
*   SPECTROSPIN AG
*   Industriestr. 26
*   CH-8117 Faellanden
*
*   All Rights Reserved
*
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/iodev/SCCS/s.IODev.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include <stdio.h>
#include <string.h>

#include <io_dev.h>

#include <sg.h>
#include <pu_p.h>
#include <io_error.h>

#ifdef IO_DEV_TTY_NO
extern void IOTTYNOSetDev(void);
#endif
#ifdef IO_DEV_MOTIF_X11
extern void IOMotifX11SetDev(void);
#endif
#ifdef IO_DEV_MOTIF_GL
extern void IOMotifGLSetDev(void);
extern void IOMotifGLDSetDev(void);
#endif
#ifdef IO_DEV_MOTIF_OGL
extern void IOMotifOGLSetDev(void);
extern void IOMotifOGLDSetDev(void);
#endif
#ifdef IO_DEV_MOTIF_XGL
extern void IOMotifXGLSetDev(void);
#endif
#ifdef IO_DEV_WIN_OGL
extern void IOWinOGLSetDev(void);
extern void IOWinOGLDSetDev(void);
#endif

IODev *CurrIODevP;

typedef void (*DevSetF) (void);

typedef struct {
  char *name;
  DevSetF set;
} DevListEntry;

static DevListEntry DevList[] = {
#ifdef IO_DEV_TTY_NO
  {"TTY/NO", IOTTYNOSetDev},
#endif
#ifdef IO_DEV_MOTIF_X11
  {"Motif/X11", IOMotifX11SetDev},
#endif
#ifdef IO_DEV_MOTIF_GL
  {"Motif/GL", IOMotifGLSetDev},
  {"Motif/GLD", IOMotifGLDSetDev},
#endif
#ifdef IO_DEV_MOTIF_OGL
  {"Motif/OpenGL", IOMotifOGLSetDev},
  {"Motif/OpenGLD", IOMotifOGLDSetDev},
#endif
#ifdef IO_DEV_MOTIF_XGL
  {"Motif/XGL", IOMotifXGLSetDev},
#endif
#ifdef IO_DEV_WIN_OGL
  {"Windows/OpenGL", IOWinOGLSetDev},
  {"Windows/OpenGLD", IOWinOGLDSetDev},
#endif
};

IORes
IOSetDevice(char *devName)
{
  int listSize, i;

  listSize = sizeof(DevList) / sizeof(DevList[0]);

  for (i = 0; i < listSize; i++)
    if (strcmp(DevList[i].name, devName) == 0) {
      DevList[i].set();
      PuSetDevice(CurrIODevP->puDevName);
      SgSetDevice(CurrIODevP->sgDevName);
      return IO_RES_OK;
    }
  
  return IO_RES_ERR;
}

IORes
IOInit(char *appName, int *argcP, char *argv[])
{
  return CurrIODevP->init(appName, argcP, argv);
}

IORes
IOCleanup(void)
{
  return CurrIODevP->cleanup();
}

IORes
IOSetWindow(PuWindow win)
{
  PuSetWindow(win);
  return CurrIODevP->setWindow(win);
}

void
IOEndFrame(void)
{
  CurrIODevP->endFrame();
}

IORes
IOStartDump(char *format, char *fileName,
    SgPlotOrient orient, int w, int h, int quality, float gamma)
{
  if (CurrIODevP->startDump == NULL) {
    IORaiseError("dumping of image not supported by IO device");
    return IO_RES_ERR;
  }

  return CurrIODevP->startDump(format, fileName, orient, w, h, quality, gamma);
}

IORes
IOEndDump(void)
{
  return CurrIODevP->endDump();
}
