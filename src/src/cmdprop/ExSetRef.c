/*
************************************************************************
*
*   ExSetRef.c - SetRef command
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
*   Date of last modification : 95/05/25
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdprop/SCCS/s.ExSetRef.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_prop.h>

#include <stdio.h>

#include <data_hand.h>
#include <data_ref.h>
#include <graph_draw.h>

static void
addAtom(DhAtomP atomP, void *clientData)
{
  RefAddAtom(atomP);
}

ErrCode
ExSetRef(char *cmd)
{
  RefInit();

  DhApplyAtom(PropGetRef(PROP_SELECTED, FALSE), addAtom, NULL);

  return EC_OK;
}
