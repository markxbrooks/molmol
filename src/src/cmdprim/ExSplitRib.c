/*
************************************************************************
*
*   ExSplitRib.c - SplitRibbon command
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
*   Date of last modification : 95/05/23
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdprim/SCCS/s.ExSplitRib.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <prim_hand.h>
#include <graph_draw.h>

static void
splitRibbon(PrimObjP primP, void *clientData)
{
  RibbonSplit(primP);
}

ErrCode
ExSplitRibbon(char *cmd)
{
  PrimApply(PT_RIBBON, PropGetRef(PROP_SELECTED, FALSE), splitRibbon, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
