/*
************************************************************************
*
*   PuEvent.c - Pu wrapper functions for event management
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuEvent.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_dev.h>

void
PuEventLoop(void)
{
  CurrPuDevP->eventLoop();
}

void
PuProcessEvent(void)
{
  CurrPuDevP->processEvent();
}

void
PuAddTimeOut(int interv, PuTimeOut timeOutF, void *clientData)
{
  CurrPuDevP->addTimeOut(interv, timeOutF, clientData);
}
