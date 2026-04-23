/*
************************************************************************
*
*   ExRemoveRes.c - RemoveRes command
*
*   Copyright (c) 1996
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
*   Date of last modification : 96/06/16
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdstruc/SCCS/s.ExRemoveRes.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>
#include <stdlib.h>

#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

ErrCode
ExRemoveRes(char *cmd)
{
  DhResP *resPA;
  int resNo, resI;
  BOOL allDel;

  resNo = SelResGet(NULL, 0);
  if (resNo == 0)
    return EC_OK;

  resPA = malloc(resNo * sizeof(*resPA));
  (void) SelResGet(resPA, resNo);

  /* try deleting residues from start */
  for (resI = 0; resI < resNo; resI++)
    if (DhResDestroy(resPA[resI]))
      resPA[resI] = NULL;

  /* try deleting residues from end */
  allDel = TRUE;
  for (resI = resNo - 1; resI >= 0; resI--) {
    if (resPA[resI] == NULL)
      continue;
    if (! DhResDestroy(resPA[resI]))
      allDel = FALSE;
  }

  free(resPA);

  GraphRedrawNeeded();

  if (! allDel) {
    CipSetError("can only remove residues at start/end");
    return EC_WARNING;
  }

  return EC_OK;
}
