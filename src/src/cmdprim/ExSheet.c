/*
************************************************************************
*
*   ExSheet.c - AddSheet command
*
*   Copyright (c) 1994-95
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
*   Date of last modification : 01/07/29
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExSheet.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <data_hand.h>
#include <prim_hand.h>
#include <graph_draw.h>

static void
addSheet(DhMolP molP, void *clientData)
{
  PrimObjP primP;

  primP = PrimNew(PT_SHEET, molP);
  if (primP == NULL)
    return;

  PrimSetProp(PropGetRef("sheet", TRUE), primP, TRUE);
}

ErrCode
ExAddSheet(char *cmd)
{
  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), addSheet, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
