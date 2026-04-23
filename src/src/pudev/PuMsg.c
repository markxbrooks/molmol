/*
************************************************************************
*
*   PuMsg.c - Pu wrapper function for message dialogs
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
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuMsg.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <pu_dev.h>

void
PuShowMessage(PuMessageType type, char *title, char *msg)
{
  CurrPuDevP->showMessage(type, title, msg);
}

void
PuBeep(void)
{
  CurrPuDevP->beep();
}
