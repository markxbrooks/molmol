/*
************************************************************************
*
*   ExBond.c - AddBond and RemoveBond commands
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 98/07/21
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExBond.c
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>

#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

ErrCode
ExAddBond(char *cmd)
{
  int selNo;
  DhAtomP atomPA[2];
  DhBondP bondP;

  selNo = SelAtomGet(atomPA, 2);
  if (selNo != 2) {
    CipSetError("exactly 2 atoms must be selected");
    return EC_ERROR;
  }

  bondP = DhBondNew(atomPA[0], atomPA[1]);
  if (bondP == NULL)  /* already exists, don't give error */
    return EC_OK;

  DhBondInit(bondP);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

static void
removeBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DhBondDestroy(bondP);
}

ErrCode
ExRemoveBond(char *cmd)
{
  DhApplyBond(PropGetRef(PROP_SELECTED, FALSE), removeBond, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
