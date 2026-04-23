/*
************************************************************************
*
*   PuVal.c - Pu wrapper functions for valuator box
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
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuVal.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_dev.h>

PuValuatorBox
PuCreateValuatorBox(char *title)
{
  return CurrPuDevP->createValuatorBox(title);
}

PuValuator
PuCreateValuator(PuValuatorBox box, char *name,
    float minVal, float maxVal, int digits, float initVal,
    PuValuatorCB valuatorCB, void *clientData, PuFreeCB freeCB)
{
  return CurrPuDevP->createValuator(box, name,
      minVal, maxVal, digits, initVal,
      valuatorCB, clientData, freeCB);
}

void
PuSwitchValuatorBox(PuValuatorBox box, BOOL onOff)
{
  CurrPuDevP->switchValuatorBox(box, onOff);
}
