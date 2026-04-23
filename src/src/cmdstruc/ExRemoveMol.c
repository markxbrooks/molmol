/*
************************************************************************
*
*   ExRemoveMol.c - RemoveMol command
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
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdstruc/SCCS/s.ExRemoveMol.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>

#include <data_hand.h>
#include <graph_draw.h>

static void
removeMol(DhMolP molP, void *clientData)
{
  DhMolDestroy(molP);
}

ErrCode
ExRemoveMol(char *cmd)
{
  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), removeMol, NULL);
  GraphRedrawNeeded();

  return EC_OK;
}
