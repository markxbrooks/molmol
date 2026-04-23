/*
************************************************************************
*
*   MotifInit.c - init Motif device
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
*   Date of last modification : 95/07/18
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifInit.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "motif_init.h"

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Protocols.h>

#include <pu_cb.h>
#include "motif_access.h"

#define PROP_FONT "-*-helvetica-bold-r-normal-*-*-120-*-*-*-*-*-*"
#define FIXED_FONT "-*-fixed-bold-r-normal-*-*-*-*-*-*-90-*-*"

String FallbackResources[] = {
  "*geometry: 800x600",
  "*background: grey",
  "*fontList: " PROP_FONT,
  "*font: " PROP_FONT,
  "*TextWin*Text.fontList: " FIXED_FONT,
  "*HelpWin*Text.fontList: " FIXED_FONT,
  NULL
};

XtAppContext AppContext;

static void
quitCB(Widget w, XtPointer clientData, XtPointer callData)
{
  PuCallQuitCB();
}

PuRes
PuMotifInit(char *appName, int *argcP, char *argv[])
{
  Widget topW;
  Atom wmDelete;

  topW = XtAppInitialize(&AppContext, appName, NULL, 0,
      (void *) argcP, argv, FallbackResources, NULL, 0);
  if (topW == NULL)
    return PU_RES_ERR;

  XtVaSetValues(topW,
      XmNiconName, appName,
      NULL);

  wmDelete = XmInternAtom(XtDisplay(topW), "WM_DELETE_WINDOW", False);
  XmAddWMProtocolCallback(topW, wmDelete, quitCB, NULL);

  MotifSetTopW(topW);

  /* force creation of main window */
  (void) MotifGetMainW();

  XtRealizeWidget(topW);

  return PU_RES_OK;
}


XtAppContext
MotifGetAppContext(void)
{
  return AppContext;
}


PuRes
PuMotifCleanup(void)
{
  XtDestroyWidget(MotifGetTopW());
  return PU_RES_OK;
}
