/*
************************************************************************
*
*   ExPlates.c - AddPlates command
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 03/01/20
*   Pathname of SCCS file     : /home/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExPlates.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <data_hand.h>
#include <prim_hand.h>
#include <graph_draw.h>

#define MAX_RING_LEN 8

typedef struct {
  PropRefP selRefP, allRefP;
  DhResP resP;
  int atomIA[MAX_RING_LEN];
  int len;
} TravData;

static void
addRing(DhResP resP, int *atomIA, int len)
{
  DhAtomP atomPA[MAX_RING_LEN];
  PrimObjP primP;
  int i;

  for (i = 0; i < len; i++)
    atomPA[i] = DhAtomFindNumber(resP, atomIA[i], FALSE);
  
  primP = PrimNewPlate(atomPA, len);
  if (primP != NULL)
    PrimSetProp(PropGetRef("plate", TRUE), primP, TRUE);
}

static void
travAtom(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  TravData *dataP = clientData;
  int atomI, prevI;
  DhAtomP atomP;
  int prevLen;
  int i;

  prevI = DhAtomGetNumber(atom1P);
  if (prevI == dataP->atomIA[dataP->len - 1]) {
    if (DhAtomGetRes(atom2P) != dataP->resP)
      return;
    atomI = DhAtomGetNumber(atom2P);
    atomP = atom2P;
  } else {
    if (DhAtomGetRes(atom1P) != dataP->resP)
      return;
    atomI = prevI;
    prevI = DhAtomGetNumber(atom2P);
    atomP = atom1P;
  }

  if (! DhAtomGetProp(dataP->selRefP, atomP))
    return;

  /* make sure that start atom has the smallest index so that we
     get each ring only once */
  if (atomI < dataP->atomIA[0])
    return;

  /* avoid going back to the atom we came from */
  if (dataP->len >= 2 && dataP->atomIA[dataP->len - 2] == atomI)
    return;

  if (atomI == dataP->atomIA[0]) {
    if (dataP->atomIA[1] < dataP->atomIA[dataP->len - 1])
      /* take ring only in one direction */
      addRing(dataP->resP, dataP->atomIA, dataP->len);
    return;
  }

  for (i = 1; i < dataP->len; i++)
    if (atomI == dataP->atomIA[i])
      /* part of the path is closed, but it is not a ring with
         this start atom */
      return;

  prevLen = dataP->len;

  dataP->atomIA[dataP->len] = atomI;
  dataP->len++;

  if (dataP->len < MAX_RING_LEN)
    DhAtomApplyBond(dataP->allRefP, atomP, travAtom, dataP);

  dataP->len = prevLen;
}

static void
startAtom(DhAtomP atomP, void *clientData)
{
  TravData *dataP = clientData;

  dataP->resP = DhAtomGetRes(atomP);
  dataP->atomIA[0] = DhAtomGetNumber(atomP);
  dataP->len = 1;

  DhAtomApplyBond(dataP->allRefP, atomP, travAtom, dataP);
}

ErrCode
ExAddPlates(char *cmd)
{
  TravData data;

  data.selRefP = PropGetRef(PROP_SELECTED, FALSE);
  data.allRefP = PropGetRef(PROP_ALL, FALSE);
  DhApplyAtom(data.selRefP, startAtom, &data);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
