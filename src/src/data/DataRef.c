/*
************************************************************************
*
*   DataRef.c - manage reference atom(s)
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
*   Pathname of SCCS file     : /sgiext/molmol/src/data/SCCS/s.DataRef.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <data_ref.h>

#include <stdio.h>
#include <values.h>

#include <mat_vec.h>
#include <linlist.h>

typedef struct {
  DhAtomP atomP;
} RefListEntry;

static LINLIST RefList = NULL;

static void
atomInvalidCB(DhAtomP firstAtomP, DhAtomP lastAtomP, void *clientData)
{
  RefListEntry *entryP, *nextEntryP;

  entryP = ListFirst(RefList);
  while (entryP != NULL) {
    nextEntryP = ListNext(RefList, entryP);
    if (entryP->atomP >= firstAtomP && entryP->atomP <= lastAtomP)
      ListRemove(RefList, entryP);
    entryP = nextEntryP;
  }
}

void
RefInit(void)
{
  if (RefList == NULL)
    DhAddAtomInvalidCB(atomInvalidCB, NULL);
  else
    ListClose(RefList);
  
  RefList = ListOpen(sizeof(RefListEntry));
}

void
RefAddAtom(DhAtomP atomP)
{
  RefListEntry entry;

  entry.atomP = atomP;
  (void) ListInsertLast(RefList, &entry);
}

float
RefGetDist(Vec3 x)
{
  float dMin, d;
  RefListEntry *entryP;
  Vec3 coord;

  dMin = MAXFLOAT;

  entryP = ListFirst(RefList);
  while (entryP != NULL) {
    DhAtomGetCoordTransf(entryP->atomP, coord);
    d = Vec3DiffAbs(x, coord);
    if (d < dMin)
      dMin = d;
    entryP = ListNext(RefList, entryP);
  }

  return dMin;
}
