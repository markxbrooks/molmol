/*
************************************************************************
*
*   ExRibbon.c - AddRibbon command
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
*   Date of last modification : 95/04/07
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdprim/SCCS/s.ExRibbon.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <data_hand.h>
#include <prim_hand.h>
#include <graph_draw.h>

static void
addRibbon(DhMolP molP, void *clientData)
{
  RibbonNew(molP);
}

ErrCode
ExAddRibbon(char *cmd)
{
  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), addRibbon, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
