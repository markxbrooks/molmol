/*
************************************************************************
*
*   DataDump.c - read/write data part of dump file
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/data/SCCS/s.DataDump.c
*   SCCS identification       : 1.22
*
************************************************************************
*/

#include <data_hand.h>

#include <stdio.h>
#include <stdlib.h>

#include <prop_tab.h>
#include <attr_mng.h>
#include "data_struc.h"
#include "data_res_lib.h"
#include "prop_stand.h"

#define NAME_LEN 100
#define NO_RES_NUM -999

typedef struct {
  GFile gf;
  BOOL ok;
} DumpData;

#define CHECK_RES(s) if (s != GF_RES_OK) {dumpP->ok = FALSE; return;}

static void
dumpBondList(LINLIST bondL, DumpData *dumpP)
{
  GFile gf = dumpP->gf;
  DhBondAddP bondAddP;

  CHECK_RES(GFileWriteINT32(gf, ListSize(bondL)));
  CHECK_RES(GFileWriteNL(gf));

  bondAddP = ListFirst(bondL);
  while (bondAddP != NULL) {
    CHECK_RES(GFileWriteINT32(gf, bondAddP->res1P->molP->num));
    CHECK_RES(GFileWriteINT32(gf, bondAddP->res2P->molP->num));
    CHECK_RES(GFileWriteINT32(gf, bondAddP->res1P->num));
    CHECK_RES(GFileWriteINT32(gf, bondAddP->res2P->num));
    CHECK_RES(GFileWriteINT32(gf, bondAddP->atom1I));
    CHECK_RES(GFileWriteINT32(gf, bondAddP->atom2I));
    CHECK_RES(GFileWriteNL(gf));

    CHECK_RES(GFileWriteINT32(gf, PropGetIndex(bondAddP->bondS.propTab)));
    CHECK_RES(GFileWriteINT32(gf, AttrGetIndex(bondAddP->bondS.attrP)));
    CHECK_RES(GFileWriteNL(gf));
    bondAddP = ListNext(bondL, bondAddP);
  }
}

static void
dumpRes(DhResP resP, DumpData *dumpP)
{
  GFile gf = dumpP->gf;
  DhResDefP defP = resP->defP;
  int i;

  if (! dumpP->ok)
    return;

  CHECK_RES(GFileWriteINT32(gf, defP->num));
  CHECK_RES(GFileWriteINT32(gf, resP->num));

  if (resP->neighLeftP != NULL && resP->neighLeftP->num < resP->num) {
    CHECK_RES(GFileWriteINT32(gf, resP->neighLeftP->num));
  } else {
    CHECK_RES(GFileWriteINT32(gf, NO_RES_NUM));
  }
  if (resP->neighRightP != NULL && resP->neighRightP->num < resP->num) {
    CHECK_RES(GFileWriteINT32(gf, resP->neighRightP->num));
  } else {
    CHECK_RES(GFileWriteINT32(gf, NO_RES_NUM));
  }

  CHECK_RES(GFileWriteNL(gf));

  for (i = 0; i < defP->atomNo; i++) {
    CHECK_RES(GFileWriteFLOAT32(gf, resP->atomA[i].coord[0]));
    CHECK_RES(GFileWriteFLOAT32(gf, resP->atomA[i].coord[1]));
    CHECK_RES(GFileWriteFLOAT32(gf, resP->atomA[i].coord[2]));
    CHECK_RES(GFileWriteINT16(gf, (INT16) resP->atomA[i].state));
#ifdef NMR
    CHECK_RES(GFileWriteINT32(gf, resP->atomA[i].shiftI));
    CHECK_RES(GFileWriteFLOAT32(gf, resP->atomA[i].shift));
#else
    CHECK_RES(GFileWriteINT32(gf, 0));
    CHECK_RES(GFileWriteFLOAT32(gf, DH_SHIFT_UNKNOWN));
#endif
    CHECK_RES(GFileWriteFLOAT32(gf, resP->atomA[i].bFactor));
    CHECK_RES(GFileWriteINT32(gf, PropGetIndex(resP->atomA[i].propTab)));
    CHECK_RES(GFileWriteINT32(gf, AttrGetIndex(resP->atomA[i].attrP)));
    CHECK_RES(GFileWriteNL(gf));
  }

  for (i = 0; i < EQUIV_NO; i++)
    CHECK_RES(GFileWriteINT32(gf, resP->equivI[i]));
  CHECK_RES(GFileWriteNL(gf));

  for (i = defP->firstBondI; i <= defP->lastBondI; i++) {
    CHECK_RES(GFileWriteINT32(gf, PropGetIndex(resP->bondA[i].propTab)));
    CHECK_RES(GFileWriteINT32(gf, AttrGetIndex(resP->bondA[i].attrP)));
    CHECK_RES(GFileWriteNL(gf));
  }

  for (i = 0; i < defP->angleNo; i++) {
    CHECK_RES(GFileWriteFLOAT32(gf, resP->angleA[i].val));
    CHECK_RES(GFileWriteINT16(gf, (INT16) resP->angleA[i].changed));
    CHECK_RES(GFileWriteFLOAT32(gf, resP->angleA[i].minVal));
    CHECK_RES(GFileWriteFLOAT32(gf, resP->angleA[i].maxVal));
    CHECK_RES(GFileWriteINT32(gf, PropGetIndex(resP->angleA[i].propTab)));
    CHECK_RES(GFileWriteNL(gf));
  }

  CHECK_RES(GFileWriteINT32(gf, PropGetIndex(resP->propTab)));
  CHECK_RES(GFileWriteNL(gf));
}

static void
dumpIntraDist(DhDistP distP, DumpData *dumpP)
{
  GFile gf = dumpP->gf;

  if (! dumpP->ok)
    return;

  CHECK_RES(GFileWriteINT32(gf, distP->res1P->num));
  CHECK_RES(GFileWriteINT32(gf, distP->res2P->num));
  CHECK_RES(GFileWriteINT32(gf, distP->atom1I));
  CHECK_RES(GFileWriteINT32(gf, distP->atom2I));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteINT16(gf, (INT16) distP->kind));
  CHECK_RES(GFileWriteFLOAT32(gf, distP->limit));
  CHECK_RES(GFileWriteFLOAT32(gf, distP->val));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteINT32(gf, PropGetIndex(distP->propTab)));
  CHECK_RES(GFileWriteINT32(gf, AttrGetIndex(distP->attrP)));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteNL(gf));
}

static void
dumpMol(DhMolP molP, DumpData *dumpP)
{
  GFile gf = dumpP->gf;
  DhResP resP;
  DhDistP distP;
  int i, k;

  if (! dumpP->ok)
    return;

  CHECK_RES(GFileWriteQuotedStr(gf, DStrToStr(molP->name)));

  if (molP->groupP == NULL || molP->groupP->prevP == NULL) {
    CHECK_RES(GFileWriteINT32(gf, -1));
  } else {
    CHECK_RES(GFileWriteINT32(gf, molP->groupP->prevP->num));
  }

  CHECK_RES(GFileWriteINT32(gf, ListSize(molP->resL)));
  CHECK_RES(GFileWriteNL(gf));

  resP = ListFirst(molP->resL);
  while (resP != NULL) {
    dumpRes(resP, dumpP);
    resP = ListNext(molP->resL, resP);
  }
  CHECK_RES(GFileWriteNL(gf));

  dumpBondList(molP->bondL, dumpP);
  if (! dumpP->ok)
    return;
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteINT32(gf, ListSize(molP->distL)));
  distP = ListFirst(molP->distL);
  while (distP != NULL) {
    dumpIntraDist(distP, dumpP);
    distP = ListNext(molP->distL, distP);
  }

  for (i = 0; i < 4; i++)
    for (k = 0; k < 4; k++)
      CHECK_RES(GFileWriteFLOAT32(gf, molP->rotMat[i][k]));
  CHECK_RES(GFileWriteNL(gf));

  for (i = 0; i < 3; i++)
    CHECK_RES(GFileWriteFLOAT32(gf, molP->rotPoint[i]));
  CHECK_RES(GFileWriteNL(gf));

  for (i = 0; i < 3; i++)
    CHECK_RES(GFileWriteFLOAT32(gf, molP->transVect[i]));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteINT32(gf, PropGetIndex(molP->propTab)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));
}

static void
dumpAngleAdd(DhAngleAddP angleP, DumpData *dumpP)
{
  GFile gf = dumpP->gf;

  if (! dumpP->ok)
    return;

  CHECK_RES(GFileWriteQuotedStr(gf, DStrToStr(angleP->name)));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteINT32(gf, angleP->res1P->molP->num));
  CHECK_RES(GFileWriteINT32(gf, angleP->res1P->num));
  CHECK_RES(GFileWriteINT32(gf, angleP->res2P->num));
  CHECK_RES(GFileWriteINT32(gf, angleP->res3P->num));
  CHECK_RES(GFileWriteINT32(gf, angleP->res4P->num));
  CHECK_RES(GFileWriteINT32(gf, angleP->atom1I));
  CHECK_RES(GFileWriteINT32(gf, angleP->atom2I));
  CHECK_RES(GFileWriteINT32(gf, angleP->atom3I));
  CHECK_RES(GFileWriteINT32(gf, angleP->atom4I));
  CHECK_RES(GFileWriteNL(gf));
}

static void
dumpInterDist(DhDistP distP, DumpData *dumpP)
{
  GFile gf = dumpP->gf;

  if (! dumpP->ok)
    return;

  CHECK_RES(GFileWriteINT32(gf, distP->res1P->molP->num));
  CHECK_RES(GFileWriteINT32(gf, distP->res2P->molP->num));
  CHECK_RES(GFileWriteINT32(gf, distP->res1P->num));
  CHECK_RES(GFileWriteINT32(gf, distP->res2P->num));
  CHECK_RES(GFileWriteINT32(gf, distP->atom1I));
  CHECK_RES(GFileWriteINT32(gf, distP->atom2I));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteINT16(gf, (INT16) distP->kind));
  CHECK_RES(GFileWriteFLOAT32(gf, distP->limit));
  CHECK_RES(GFileWriteFLOAT32(gf, distP->val));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteINT32(gf, PropGetIndex(distP->propTab)));
  CHECK_RES(GFileWriteINT32(gf, AttrGetIndex(distP->attrP)));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteNL(gf));
}

#undef CHECK_RES
#define CHECK_RES(readStat) if (readStat != GF_RES_OK) return FALSE

BOOL
DhDump(GFile gf)
{
  DumpData dumpS;
  LINLIST molL, angleL, distL, altL;
  DhMolP molP;
  DhAngleAddP angleP;
  DhDistP distP;
  DhAltCoordP altP;

  if (PropDump(gf) == FALSE)
    return FALSE;

  if (AttrDump(gf) == FALSE)
    return FALSE;

  if (DhResLibDump(gf) == FALSE)
    return FALSE;

  molL = DhMolListGet();
  CHECK_RES(GFileWriteINT32(gf, ListSize(molL)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  dumpS.gf = gf;
  dumpS.ok = TRUE;

  molP = ListFirst(molL);
  while (molP != NULL) {
    dumpMol(molP, &dumpS);
    molP = ListNext(molL, molP);
  }
  if (! dumpS.ok)
    return FALSE;
  
  dumpBondList(DhBondListGet(), &dumpS);
  if (! dumpS.ok)
    return FALSE;
  CHECK_RES(GFileWriteNL(gf));

  angleL = DhAngleListGet();
  CHECK_RES(GFileWriteINT32(gf, ListSize(angleL)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  angleP = ListFirst(angleL);
  while (angleP != NULL) {
    dumpAngleAdd(angleP, &dumpS);
    angleP = ListNext(angleL, angleP);
  }
  if (! dumpS.ok)
    return FALSE;

  distL = DhDistListGet();
  CHECK_RES(GFileWriteINT32(gf, ListSize(distL)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  distP = ListFirst(distL);
  while (distP != NULL) {
    dumpInterDist(distP, &dumpS);
    distP = ListNext(distL, distP);
  }
  if (! dumpS.ok)
    return FALSE;

  altL = DhAltCoordListGet();
  CHECK_RES(GFileWriteINT32(gf, ListSize(altL)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  altP = ListFirst(altL);
  while (altP != NULL) {
    CHECK_RES(GFileWriteINT32(gf, altP->atomP->resP->molP->num));
    CHECK_RES(GFileWriteINT32(gf, altP->atomP->resP->num));
    CHECK_RES(GFileWriteINT32(gf, altP->atomP - altP->atomP->resP->atomA));
    CHECK_RES(GFileWriteFLOAT32(gf, altP->coord[0]));
    CHECK_RES(GFileWriteFLOAT32(gf, altP->coord[1]));
    CHECK_RES(GFileWriteFLOAT32(gf, altP->coord[2]));
    CHECK_RES(GFileWriteNL(gf));
    altP = ListNext(altL, altP);
  }

  /* must be dumped after molecules because it contains a
     list of molecules */
  if (PropStandDump(gf) == FALSE)
    return FALSE;

  return TRUE;
}

static BOOL
undumpBondList(GFile gf)
{
  int bondNo, bondIdx;
  LINLIST molL;
  DhMolP mol1P, mol2P;
  DhResP res1P, res2P;
  DhAtomP atom1P, atom2P;
  DhBondP bondP;
  INT32 i32;

  CHECK_RES(GFileReadINT32(gf, &i32));
  bondNo = i32;
  molL = DhMolListGet();

  for (bondIdx = 0; bondIdx < bondNo; bondIdx++) {
    CHECK_RES(GFileReadINT32(gf, &i32));
    mol1P = ListPos(molL, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    mol2P = ListPos(molL, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    res1P = DhResFind(mol1P, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    res2P = DhResFind(mol2P, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    atom1P = res1P->atomA + i32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    atom2P = res2P->atomA + i32;

    bondP = DhBondNew(atom1P, atom2P);

    PropFreeTab(bondP->propTab);  /* created by DhBondNew() */
    CHECK_RES(GFileReadINT32(gf, &i32));
    bondP->propTab = PropGet(PropFindIndex(i32));

    AttrReturn(bondP->attrP);  /* created by DhBondNew() */
    CHECK_RES(GFileReadINT32(gf, &i32));
    bondP->attrP = AttrGet(AttrFindIndex(i32));
  }

  return TRUE;
}

static BOOL
undumpRes(GFile gf, DhResP resP)
{
  DhResDefP defP;
  DhResP neighResP;
  FLOAT32 f32;
  INT16 i16;
  INT32 i32;
  int i;

  CHECK_RES(GFileReadINT32(gf, &i32));
  defP = DhResDefFindIndex(i32);
  resP->defP = defP;
  CHECK_RES(GFileReadINT32(gf, &i32));
  resP->num = i32;

  CHECK_RES(GFileReadINT32(gf, &i32));
  if (i32 == NO_RES_NUM) {
    resP->neighLeftP = NULL;
  } else {
    neighResP = DhResFind(resP->molP, i32);
    resP->neighLeftP = neighResP;
    neighResP->neighRightP = resP;
  }
  CHECK_RES(GFileReadINT32(gf, &i32));
  if (i32 == NO_RES_NUM) {
    resP->neighRightP = NULL;
  } else {
    neighResP = DhResFind(resP->molP, i32);
    resP->neighRightP = neighResP;
    neighResP->neighLeftP = resP;
  }

  resP->atomA = malloc(defP->atomNo * sizeof(*resP->atomA));
  for (i = 0; i < defP->atomNo; i++) {
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    resP->atomA[i].coord[0] = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    resP->atomA[i].coord[1] = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    resP->atomA[i].coord[2] = f32;
    CHECK_RES(GFileReadINT16(gf, &i16));
    resP->atomA[i].state = i16;
#ifdef NMR
    CHECK_RES(GFileReadINT32(gf, &i32));
    resP->atomA[i].shiftI = i32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    resP->atomA[i].shift = f32;
#else
    CHECK_RES(GFileReadINT32(gf, &i32));
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
#endif
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    resP->atomA[i].bFactor = f32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    resP->atomA[i].propTab = PropGet(PropFindIndex(i32));
    CHECK_RES(GFileReadINT32(gf, &i32));
    resP->atomA[i].attrP = AttrGet(AttrFindIndex(i32));
    resP->atomA[i].resP = resP;
  }

  for (i = 0; i < EQUIV_NO; i++) {
    CHECK_RES(GFileReadINT32(gf, &i32));
    resP->equivI[i] = i32;
  }

  resP->bondA = malloc(defP->bondNo * sizeof(*resP->bondA));
  for (i = 0; i < defP->bondNo; i++)
    resP->bondA[i].resP = resP;
  for (i = defP->firstBondI; i <= defP->lastBondI; i++) {
    CHECK_RES(GFileReadINT32(gf, &i32));
    resP->bondA[i].propTab = PropGet(PropFindIndex(i32));
    CHECK_RES(GFileReadINT32(gf, &i32));
    resP->bondA[i].attrP = AttrGet(AttrFindIndex(i32));
  }

  resP->angleA = malloc(defP->angleNo * sizeof(*resP->angleA));
  for (i = 0; i < defP->angleNo; i++) {
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    resP->angleA[i].val = f32;
    CHECK_RES(GFileReadINT16(gf, &i16));
    resP->angleA[i].changed = i16;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    resP->angleA[i].minVal = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    resP->angleA[i].maxVal = f32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    resP->angleA[i].propTab = PropGet(PropFindIndex(i32));
    resP->angleA[i].resP = resP;
  }

  CHECK_RES(GFileReadINT32(gf, &i32));
  resP->propTab = PropGet(PropFindIndex(i32));

  return TRUE;
}

static BOOL
undumpIntraDist(GFile gf, DhDistP distP, DhMolP molP)
{
  INT16 i16;
  INT32 i32;
  FLOAT32 f32;

  CHECK_RES(GFileReadINT32(gf, &i32));
  distP->res1P = DhResFind(molP, i32);
  CHECK_RES(GFileReadINT32(gf, &i32));
  distP->res2P = DhResFind(molP, i32);
  CHECK_RES(GFileReadINT32(gf, &i32));
  distP->atom1I = i32;
  CHECK_RES(GFileReadINT32(gf, &i32));
  distP->atom2I = i32;

  CHECK_RES(GFileReadINT16(gf, &i16));
  distP->kind = i16;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  distP->limit = f32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  distP->val = f32;

  CHECK_RES(GFileReadINT32(gf, &i32));
  distP->propTab = PropGet(PropFindIndex(i32));

  CHECK_RES(GFileReadINT32(gf, &i32));
  distP->attrP = AttrGet(AttrFindIndex(i32));

  return TRUE;
}

BOOL
DhUndump(GFile gf, int vers)
{
  int molNo, resNo, angleNo, distNo, altNo;
  int molIdx, resIdx, angleIdx, distIdx, altIdx;
  LINLIST molL;
  DhMolP molP, mol1P, mol2P;
  struct DhResS resS;
  DhResP resP, res1P, res2P, res3P, res4P;
  struct DhDistS distS;
  DhAtomP atomP, atom1P, atom2P, atom3P, atom4P;
  DhDistP distP;
  Vec3 coord;
  INT16 i16;
  INT32 i32;
  FLOAT32 f32;
  char name[NAME_LEN];
  DSTR nameStr;
  int i, k;

  /* DhDestroyAll() must be called before! */

  if (PropUndumpStart(gf, vers) == FALSE)
    return FALSE;

  if (AttrUndumpStart(gf, vers) == FALSE)
    return FALSE;

  if (DhResLibUndump(gf, vers) == FALSE)
    return FALSE;

  CHECK_RES(GFileReadINT32(gf, &i32));
  molNo = i32;

  for (molIdx = 0; molIdx < molNo; molIdx++) {
    molP = DhMolNew();
    molP->num = molIdx;

    CHECK_RES(GFileReadQuotedStr(gf, name, sizeof(name)));
    DStrAssignStr(molP->name, name);

    if (vers >= 9) {
      /* groups introduced in version 9 */
      CHECK_RES(GFileReadINT32(gf, &i32));
      if (i32 >= 0) {
	molP->groupP = malloc(sizeof(*molP->groupP));
	molP->groupP->prevP = DhMolFindNumber(i32);
	molP->groupP->nextP = NULL;
	if (molP->groupP->prevP->groupP == NULL) {
	  molP->groupP->prevP->groupP = malloc(sizeof(*molP->groupP));
	  molP->groupP->prevP->groupP->prevP = NULL;
	}
	molP->groupP->prevP->groupP->nextP = molP;
      }
    }

    CHECK_RES(GFileReadINT32(gf, &i32));
    resNo = i32;

    for (resIdx = 0; resIdx < resNo; resIdx++) {
      resP = ListInsertLast(molP->resL, &resS);
      resP->molP = molP;
      if (! undumpRes(gf, resP))
	return FALSE;
    }

    if (! undumpBondList(gf))
      return FALSE;

    CHECK_RES(GFileReadINT32(gf, &i32));
    distNo = i32;

    for (distIdx = 0; distIdx < distNo; distIdx++) {
      distP = ListInsertLast(molP->distL, &distS);
      if (! undumpIntraDist(gf, distP, molP))
	return FALSE;
    }

    for (i = 0; i < 4; i++)
      for (k = 0; k < 4; k++) {
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	molP->rotMat[i][k] = f32;
      }

    for (i = 0; i < 3; i++) {
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      molP->rotPoint[i] = f32;
    }

    for (i = 0; i < 3; i++) {
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      molP->transVect[i] = f32;
    }

    PropFreeTab(molP->propTab);
    CHECK_RES(GFileReadINT32(gf, &i32));
    molP->propTab = PropGet(PropFindIndex(i32));
  }

  if (! undumpBondList(gf))
    return FALSE;

  molL = DhMolListGet();

  CHECK_RES(GFileReadINT32(gf, &i32));
  angleNo = i32;
  nameStr = DStrNew();

  for (angleIdx = 0; angleIdx < angleNo; angleIdx++) {
    CHECK_RES(GFileReadQuotedStr(gf, name, sizeof(name)));
    DStrAssignStr(nameStr, name);

    CHECK_RES(GFileReadINT32(gf, &i32));
    molP = ListPos(molL, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    res1P = DhResFind(molP, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    res2P = DhResFind(molP, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    res3P = DhResFind(molP, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    res4P = DhResFind(molP, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    atom1P = res1P->atomA + i32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    atom2P = res2P->atomA + i32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    atom3P = res3P->atomA + i32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    atom4P = res4P->atomA + i32;

    (void) DhAngleNew(atom1P, atom2P, atom3P, atom4P, nameStr);
  }

  DStrFree(nameStr);

  CHECK_RES(GFileReadINT32(gf, &i32));
  distNo = i32;

  for (distIdx = 0; distIdx < distNo; distIdx++) {
    CHECK_RES(GFileReadINT32(gf, &i32));
    mol1P = ListPos(molL, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    mol2P = ListPos(molL, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    res1P = DhResFind(mol1P, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    res2P = DhResFind(mol2P, i32);
    CHECK_RES(GFileReadINT32(gf, &i32));
    atom1P = res1P->atomA + i32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    atom2P = res2P->atomA + i32;

    distP = DhDistNew(atom1P, atom2P);

    CHECK_RES(GFileReadINT16(gf, &i16));
    distP->kind = i16;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    distP->limit = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    distP->val = f32;

    PropFreeTab(distP->propTab);  /* created in DhDistNew() */
    CHECK_RES(GFileReadINT32(gf, &i32));
    distP->propTab = PropGet(PropFindIndex(i32));

    AttrReturn(distP->attrP);  /* created in DhDistNew() */
    CHECK_RES(GFileReadINT32(gf, &i32));
    distP->attrP = AttrGet(AttrFindIndex(i32));
  }

  if (vers >= 13) {
    /* alternative coordinates introduced in version 13 */
    CHECK_RES(GFileReadINT32(gf, &i32));
    altNo = i32;

    for (altIdx = 0; altIdx < altNo; altIdx++) {
      CHECK_RES(GFileReadINT32(gf, &i32));
      molP = ListPos(molL, i32);
      CHECK_RES(GFileReadINT32(gf, &i32));
      resP = DhResFind(molP, i32);
      CHECK_RES(GFileReadINT32(gf, &i32));
      atomP = resP->atomA + i32;
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      coord[0] = f32;
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      coord[1] = f32;
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      coord[2] = f32;
      DhAtomSetAltCoord(atomP, coord);
    }
  }

  if (PropStandUndump(gf, vers) == FALSE)
    return FALSE;

  return TRUE;
}
