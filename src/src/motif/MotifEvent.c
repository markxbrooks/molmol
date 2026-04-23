/*
************************************************************************
*
*   MotifEvent.c - handle events
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
*   Date of last modification : 95/07/20
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifEvent.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "motif_event.h"

#include <stdlib.h>

#include <X11/Intrinsic.h>

#include <pu_types.h>
#include "motif_cb.h"
#include "motif_access.h"

typedef struct {
  PuTimeOut timeOutF;
  void *clientData;
} TimeOutData;

static BOOL CallbackInitDone = FALSE;

static void
processOneEvent(XtAppContext app)
{
  XtAppProcessEvent(app, XtIMAll);
}

void
PuMotifEventLoop(void)
{
  XtAppContext app = MotifGetAppContext();

  if (! CallbackInitDone) {
    MotifInitCallbacks();
    CallbackInitDone = TRUE;
  }

  for (;;)
    processOneEvent(app);
}

void
PuMotifProcessEvent(void)
{
  processOneEvent(MotifGetAppContext());
}

static void
timeOutCB(XtPointer clientData, XtIntervalId *idP)
{
  TimeOutData *dataP = clientData;

  dataP->timeOutF(dataP->clientData);

  free(dataP);
}

void
PuMotifAddTimeOut(int interv, PuTimeOut timeOutF, void *clientData)
{
  XtAppContext app = MotifGetAppContext();
  TimeOutData *dataP;

  /* process pending events first to avoid lockup */
  while (XtAppPending(app) & XtIMXEvent)
    XtAppProcessEvent(app, XtIMXEvent);
    
  dataP = malloc(sizeof(*dataP));

  dataP->timeOutF = timeOutF;
  dataP->clientData = clientData;

  XtAppAddTimeOut(app, interv, timeOutCB, dataP);
}
