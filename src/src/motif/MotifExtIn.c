/*
************************************************************************
*
*   MotifExtIn.c - Motif functions for external input
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
*   Date of last modification : 94/12/19
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifExtIn.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "motif_ext_in.h"

#include <X11/Intrinsic.h>

#include <linlist.h>
#include <pu_types.h>
#include "motif_access.h"

typedef struct {
  int fd;
  PuExtInput extInpF;
  void *clientData;
  PuFreeCB freeCB;
  XtInputId id;
} ExtInpData;

static LINLIST ExtInpList = NULL;

static void
extInpCB(XtPointer clientData, int *fdP, XtInputId *idP)
{
  ExtInpData *dataP = clientData;

  dataP->extInpF(*fdP, dataP->clientData);
}

void
PuMotifAddExtInput(int fd,
    PuExtInput extInpF, void *clientData, PuFreeCB freeCB)
{
  XtAppContext app = MotifGetAppContext();
  ExtInpData data, *dataP;

  if (ExtInpList == NULL)
    ExtInpList = ListOpen(sizeof(ExtInpData));

  data.fd = fd;
  data.extInpF = extInpF;
  data.clientData = clientData;
  data.freeCB = freeCB;

  dataP = ListInsertLast(ExtInpList, &data);

  dataP->id = XtAppAddInput(app, fd, (XtPointer) XtInputReadMask,
      extInpCB, dataP);
}

void
PuMotifRemoveExtInput(int fd, PuExtInput extInpF, void *clientData)
{
  ExtInpData *dataP;

  dataP = ListFirst(ExtInpList);
  while (dataP != NULL) {
    if (dataP->fd == fd && dataP->extInpF == extInpF &&
	dataP->clientData == clientData) {
      XtRemoveInput(dataP->id);

      if (dataP->freeCB != NULL)
	dataP->freeCB(dataP->clientData);
      ListRemove(ExtInpList, dataP);
      break;
    }
    dataP = ListNext(ExtInpList, dataP);
  }
}
