/*
************************************************************************
*
*   PuTextW.c - Pu wrapper functions for text windows
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
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuTextW.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <pu_dev.h>

#include <string.h>
#include <stdlib.h>

#define DEFAULT_EXT ".txt"

PuTextWindow
PuCreateTextWindow(char *title)
{
  char *full;
  PuTextWindow textW;
  int len, i;

  /* add default extension */
  len = strlen(title);
  for (i = 0; i < len; i++)
    if (title[i] == '.')
      return CurrPuDevP->createTextWindow(title);

  full = malloc(len + strlen(DEFAULT_EXT) + 1);
  (void) strcpy(full, title);
  (void) strcpy(full + len, DEFAULT_EXT);

  textW = CurrPuDevP->createTextWindow(full);

  free(full);

  return textW;
}

void
PuWriteStr(PuTextWindow textW, char *str)
{
  CurrPuDevP->writeStr(textW, str);
}
