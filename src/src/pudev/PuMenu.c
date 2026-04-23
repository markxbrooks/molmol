/*
************************************************************************
*
*   PuMenu.c - Pu wrapper functions for menu creation
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
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuMenu.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_dev.h>

PuMenubar
PuCreateMenubar(void)
{
  return CurrPuDevP->createMenubar();
}

PuMenu
PuCreatePulldown(PuMenubar menubar, char *name, char mnemonic)
{
  return CurrPuDevP->createPulldown(menubar, name, mnemonic);
}

PuMenu
PuCreatePopup(char *name)
{
  return CurrPuDevP->createPopup(name);
}

PuMenu
PuCreatePullright(PuMenu menu, char *name, char mnemonic)
{
  return CurrPuDevP->createPullright(menu, name, mnemonic);
}

PuMenuEntry
PuCreateMenuEntry(PuMenu menu, char *name,
    char mnemonic, char *acc, char *accText,
    PuMenuCB menuCB, void *clientData, PuFreeCB freeCB)
{
  return CurrPuDevP->createMenuEntry(menu, name,
      mnemonic, acc, accText,
      menuCB, clientData, freeCB);
}

void
PuSwitchMenubar(PuMenubar menubar, BOOL onOff)
{
  CurrPuDevP->switchMenubar(menubar, onOff);
}

void
PuSwitchPopup(PuMenu popup, BOOL onOff)
{
  CurrPuDevP->switchPopup(popup, onOff);
}
