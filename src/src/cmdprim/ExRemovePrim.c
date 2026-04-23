/*
************************************************************************
*
*   ExRemovePrim.c - RemovePrim command
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
*   Date of last modification : 95/04/07
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdprim/SCCS/s.ExRemovePrim.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <prim_hand.h>
#include <graph_draw.h>

static void
removePrim(PrimObjP primP, void *clientData)
{
  PrimDestroy(primP);
}

ErrCode
ExRemovePrim(char *cmd)
{
  PrimApply(PT_ALL, PropGetRef(PROP_SELECTED, FALSE), removePrim, NULL);

  GraphMolChanged(PROP_ALL);
  GraphRedrawNeeded();

  return EC_OK;
}
