/*
************************************************************************
*
*   DataCalc.c - calculations
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/data/SCCS/s.DataCalc.c
*   SCCS identification       : 1.23
*
************************************************************************
*/

#include <data_hand.h>
#include "data_calc.h"

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <mat_vec.h>
#include <map_coord.h>
#include <strmatch.h>
#include "data_struc.h"
#include "data_res_lib.h"

#define MAX_NEIGH_NO 5

typedef struct {
  DhMolP molP;
  DhMolChangedCB changedCB;
  void *clientData;
} ChangedMolData;

typedef struct {
  Vec3 vecSum;
  int atomNo;
} CalcRefInfo;

typedef struct {
  int firstI;
  int lastI;
  Mat4 m;
} RotEntry;

static LINLIST ChangedMolList = NULL;

void
DhAddMolChangedCB(DhMolP molP, DhMolChangedCB changedCB, void *clientData)
{
  ChangedMolData entry;

  entry.molP = molP;
  entry.changedCB = changedCB;
  entry.clientData = clientData;

  if (ChangedMolList == NULL)
    ChangedMolList = ListOpen(sizeof(ChangedMolData));

  (void) ListInsertLast(ChangedMolList, &entry);
}

void
DhRemoveMolChangedCB(DhMolP molP, DhMolChangedCB changedCB, void *clientData)
{
  ChangedMolData *entryP;

  entryP = ListFirst(ChangedMolList);
  while (entryP != NULL) {
    if (entryP->molP == molP &&
	entryP->changedCB == changedCB && entryP->clientData == clientData) {
      ListRemove(ChangedMolList, entryP);
      return;
    }
    entryP = ListNext(ChangedMolList, entryP);
  }
}

void
callMolChanged(DhMolP molP)
{
  ChangedMolData *entryP, *nextEntryP;

  entryP = ListFirst(ChangedMolList);
  while (entryP != NULL) {
    /* entryP could be removed from callback */
    nextEntryP = ListNext(ChangedMolList, entryP);
    if (entryP->molP == molP)
      entryP->changedCB(molP, entryP->clientData);
    entryP = nextEntryP;
  }
}

static void
atomCalcRef(DhAtomP atomP, void *clientData)
{
  CalcRefInfo *infoP = clientData;
  Vec3 x;

  DhAtomGetCoord(atomP, x);
  Vec3Add(infoP->vecSum, x);
  infoP->atomNo++;
}

void
DhCalcCenter(Vec3 v, DhMolP molP)
{
  CalcRefInfo info;

  Vec3Zero(info.vecSum);
  info.atomNo = 0;
  DhMolApplyAtom(PropGetRef(PROP_SELECTED, FALSE), molP, atomCalcRef, &info);
  Vec3Copy(v, info.vecSum);
  if (info.atomNo != 0)
    Vec3Scale(v, 1.0f / info.atomNo);
}

void
DhMolCalcMeanStruc(DhMolP molP, DhMolP molPA[], int molNo,
    BFactorCalc bfc, BOOL setAll)
/* Assumes that all molecules have same structure, should be
   compared with DhMolEqualStruc before. */
{
  DhResP resP, *resPA;
  Vec3 coordAvg, coord, dv;
  AtomState state;
  int atomI;
  float bf, disp;
  Mat4 rotMat;
  Vec3 v;
  int i;

  resPA = malloc(molNo * sizeof(*resPA));

  resP = ListFirst(molP->resL);
  for (i = 0; i < molNo; i++)
    resPA[i] = ListFirst(molPA[i]->resL);

  while (resP != NULL) {
    for (atomI = 0; atomI < resP->defP->atomNo; atomI++) {
      Vec3Zero(coordAvg);
      state = AS_VALID;
      for (i = 0; i < molNo; i++) {
	if (resPA[i]->atomA[atomI].state == AS_VALID) {
	  DhAtomGetCoordTransf(resPA[i]->atomA + atomI, coord);
	  Vec3Add(coordAvg, coord);
	} else if (resPA[i]->atomA[atomI].state == AS_DELETED) {
	  state = AS_DELETED;
	} else if (state != AS_DELETED) {
	  state = AS_UNKNOWN_COORD;
	}
      }
	
      resP->atomA[atomI].state = state;
      if (state != AS_VALID)
	continue;

      Vec3Scale(coordAvg, 1.0f / molNo);
      Vec3Copy(resP->atomA[atomI].coord, coordAvg);

      bf = 0.0f;
      for (i = 0; i < molNo; i++) {
	if (bfc == BFC_AVG_B) {
	  bf += resPA[i]->atomA[atomI].bFactor;
	} else {
	  DhAtomGetCoordTransf(resPA[i]->atomA + atomI, dv);
	  Vec3Sub(dv, coordAvg);
	  disp = dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2];

	  if (bfc == BFC_MAX_DISP) {
	    if (disp > bf)
	      bf = disp;
	  } else if (bfc == BFC_AVG_DISP) {
	    bf += sqrtf(disp);
	  } else {
	    bf += disp;
	  }
	}
      }

      if (bfc == BFC_AVG_B) {
	bf /= molNo;
      } else if (bfc == BFC_AVG_DISP) {
	bf /= molNo;
      } else if (bfc == BFC_RMS_DISP) {
	bf = sqrtf(bf / molNo);
      } else if (bfc == BFC_XRAY) {
	bf = (8.0f / 3.0f) * (float) M_PI * (float) M_PI * (bf / molNo);
      }

      resP->atomA[atomI].bFactor = bf;
      if (setAll)
	for (i = 0; i < molNo; i++)
	  resPA[i]->atomA[atomI].bFactor = bf;
    }

    resP = ListNext(molP->resL, resP);
    for (i = 0; i < molNo; i++)
      resPA[i] = ListNext(molPA[i]->resL, resPA[i]);
  }

  free(resPA);

  /* since molecule contains transformed coordinates already,
     the mapping must be set to identity */
  Mat4Ident(rotMat);
  DhMolSetRotMat(molP, rotMat);
  Vec3Zero(v);
  DhMolSetTransVect(molP, v);
  DhMolSetRotPoint(molP, v);

  DhMolCoordsChanged(molP);
}

static void
calcBaseTransf(Mat3 m, Vec3 p1, Vec3 p2, Vec3 p3)
{
  Vec3 vx, vy, vz;
  int i;

  /* calculate new basis vx, vy, vz */
  Vec3Copy(vz, p2);
  Vec3Sub(vz, p3);
  Vec3Norm(vz);

  Vec3Copy(vx, p1);
  Vec3Sub(vx, p2);
  Vec3ScaleSub(vx, Vec3Scalar(vz, vx), vz);
  Vec3Norm(vx);

  Vec3Copy(vy, vz);
  Vec3Cross(vy, vx);

  for (i = 0; i < 3; i++) {
    m[i][0] = vx[i];
    m[i][1] = vy[i];
    m[i][2] = vz[i];
  }
}

static float
calcDihAngle(Vec3 p1, Vec3 p2, Vec3 p3, Vec3 p4)
{
  Mat3 btM;
  Vec3 v34;
  float c, s, a;

  calcBaseTransf(btM, p1, p2, p3);

  /* calculate vector from p3 to p4 */
  Vec3Copy(v34, p4);
  Vec3Sub(v34, p3);

  Mat3VecMult(v34, btM);

  c = v34[0];
  s = v34[1];
  a = sqrtf(c * c + s * s);
  c /= a;
  s /= a;

  return atan2f(s, c);
}

void
DhDistCalc(DhDistP distP)
{
  DhAtomP atom1P, atom2P;
  DhMolP mol1P, mol2P;
  Vec3 v1, v2;

  atom1P = distP->res1P->atomA + distP->atom1I;
  atom2P = distP->res2P->atomA + distP->atom2I;

  mol1P = distP->res1P->molP;
  mol2P = distP->res2P->molP;

  if (mol1P != mol2P) {
    DhAtomGetCoordTransf(atom1P, v1);
    DhAtomGetCoordTransf(atom2P, v2);
  } else {
    Vec3Copy(v1, atom1P->coord);
    Vec3Copy(v2, atom2P->coord);
  }

  distP->val = Vec3DiffAbs(v1, v2);
}

static void
calcOneDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DhDistCalc(distP);
}

static void
calcMolDist(DhMolP molP)
{
  PropRefP refP;

  refP = PropGetRef(PROP_ALL, FALSE);
  DhMolApplyDist(refP, molP, calcOneDist, NULL);
  DhApplyInterDist(refP, calcOneDist, NULL);
}

void
DhCalcPseudo(DhResP resP)
{
  int *avgNoA;
  int i1, i2;

  avgNoA = malloc(resP->defP->atomNo * sizeof(*avgNoA));

  for (i1 = 0; i1 < resP->defP->atomNo; i1++) {
    avgNoA[i1] = -1;

    if (resP->defP->atomA[i1].typeP->kind != AK_PSEUDO)
      continue;
    
    if (resP->atomA[i1].state == AS_DELETED)
      continue;

    avgNoA[i1] = 0;
  }

  for (i2 = 0; i2 < resP->defP->atomNo; i2++) {
    if (resP->defP->atomA[i2].typeP->kind == AK_PSEUDO)
      continue;
    
    if (resP->atomA[i2].state != AS_VALID)
      continue;

    i1 = i2;
    for (;;) {
      i1 = resP->defP->atomA[i1].pseudoI;
      if (i1 < 0)
	break;
    
      if (avgNoA[i1] == -1)
	continue;

      if (avgNoA[i1] == 0)
	Vec3Zero(resP->atomA[i1].coord);

      avgNoA[i1]++;
      Vec3Add(resP->atomA[i1].coord, resP->atomA[i2].coord);
    }
  }

  for (i1 = 0; i1 < resP->defP->atomNo; i1++)
    if (avgNoA[i1] > 0) {
      Vec3Scale(resP->atomA[i1].coord, 1.0f / avgNoA[i1]);
      resP->atomA[i1].state = AS_VALID;
    }

  free(avgNoA);
}

void
DhMolCoordsChanged(DhMolP molP)
{
  DhResP resP;
  DhAngleDefP defP;
  DhAtomP atom1P, atom2P, atom3P, atom4P;
  int i;

  resP = ListFirst(molP->resL);
  while (resP != NULL) {
    for (i = 0; i < resP->defP->angleNo; i++) {
      defP = resP->defP->angleA + i;

      atom1P = resP->atomA + defP->atom1I;
      atom2P = resP->atomA + defP->atom2I;
      atom3P = resP->atomA + defP->atom3I;
      atom4P = resP->atomA + defP->atom4I;

      if (atom1P->state == AS_VALID && atom2P->state == AS_VALID &&
	  atom3P->state == AS_VALID && atom4P->state == AS_VALID)
	resP->angleA[i].val = calcDihAngle(
	    atom1P->coord, atom2P->coord, atom3P->coord, atom4P->coord);

      resP->angleA[i].changed = FALSE;
    }

    DhCalcPseudo(resP);

    resP = ListNext(molP->resL, resP);
  }

  calcMolDist(molP);
  callMolChanged(molP);
}

void
DhResStandGeom(DhResP resP)
{
  DhResDefP resDefP;
  DhAngleDefP angleDefP;
  int i;

  resDefP = resP->defP;
  for (i = 0; i < resDefP->atomNo; i++) {
    Vec3Copy(resP->atomA[i].coord, resDefP->atomA[i].coord);
    resP->atomA[i].state = AS_VALID;
  }

  for (i = 0; i < resDefP->angleNo; i++) {
    angleDefP = resP->defP->angleA + i;
    resP->angleA[i].val = calcDihAngle(
	resP->atomA[angleDefP->atom1I].coord,
	resP->atomA[angleDefP->atom2I].coord,
	resP->atomA[angleDefP->atom3I].coord,
	resP->atomA[angleDefP->atom4I].coord);
  }
}

void
DhRotMatCalc(Mat4 mRes, Mat4 mResI,
    Vec3 p1, Vec3 p2, Vec3 p3, Vec3 p4, float angle)
{
  Vec3 v1;
  Mat3 btM, btIM, rotM;
  float c1, s1, c2, s2, c, s, a;
  Mat4 m;
  int i, k;

  calcBaseTransf(btM, p1, p2, p3);

  /* inverse (= transposed) of base tranformation */
  for (i = 0; i < 3; i++)
    for (k = 0; k < 3; k++)
      btIM[i][k] = btM[k][i];

  /* calculate vector from p3 to p4 in new coordinate space */
  Vec3Copy(v1, p4);
  Vec3Sub(v1, p3);
  Mat3VecMult(v1, btM);

  /* cosine and sine of original dihedral angle */
  c1 = v1[0];
  s1 = v1[1];
  a = sqrtf(c1 * c1 + s1 * s1);
  c1 /= a;
  s1 /= a;

  /* cosine and sine of new dihedral angle */
  c2 = cosf(angle);
  s2 = sinf(angle);

  /* cosine and sine of rotation angle */
  c = c1 * c2 + s1 * s2;
  s = c1 * s2 - c2 * s1;

  /* rotation matrix */
  Mat3Ident(rotM);
  rotM[0][0] = c;
  rotM[0][1] = s;
  rotM[1][0] = -s;
  rotM[1][1] = c;

  /* combine matrices */
  Mat4Ident(mRes);
  for (i = 0; i < 3; i++)
    mRes[3][i] = p2[i];  /* translation */
  Mat3To4(m, btIM);
  Mat4Mult(mRes, m);     /* inverse base transformation */
  Mat3To4(m, rotM);
  Mat4Mult(mRes, m);     /* rotation */
  Mat3To4(m, btM);
  Mat4Mult(mRes, m);     /* base tranformation */
  Mat4Ident(m);
  for (i = 0; i < 3; i++)
    m[3][i] = - p2[i];
  Mat4Mult(mRes, m);     /* translation */

  if (mResI != NULL) {  /* also calculate inverse transformation */
    rotM[0][1] = -s;
    rotM[1][0] = s;

    Mat4Ident(mResI);
    for (i = 0; i < 3; i++)
      mResI[3][i] = p2[i];
    Mat3To4(m, btIM);
    Mat4Mult(mResI, m);
    Mat3To4(m, rotM);
    Mat4Mult(mResI, m);
    Mat3To4(m, btM);
    Mat4Mult(mResI, m);
    Mat4Ident(m);
    for (i = 0; i < 3; i++)
      m[3][i] = - p2[i];
    Mat4Mult(mResI, m);
  }
}

static void
resAnglesChanged(DhResP resP, Mat4 bbMat)
{
  LINLIST rotList;
  RotEntry rotEntry, *rotEntryP;
  DhResDefP resDefP;
  DhAngleP angleP;
  DhAngleDefP angleDefP;
  DhAtomP atomP;
  Mat4 m;
  Vec4 v;
  int i;

  resDefP = resP->defP;

  rotList = ListOpen(sizeof(RotEntry));
  rotEntry.firstI = 0;
  rotEntry.lastI = resDefP->atomNo - 1;
  Mat4Copy(rotEntry.m, bbMat);
  (void) ListInsertFirst(rotList, &rotEntry);

  for (i = 0; i < resDefP->angleNo; i++) {
    angleP = resP->angleA + i;
    angleDefP = resDefP->angleA + i;

    if (! angleP->changed)
      continue;

    DhRotMatCalc(m, NULL,
	resP->atomA[angleDefP->atom1I].coord,
	resP->atomA[angleDefP->atom2I].coord,
	resP->atomA[angleDefP->atom3I].coord,
	resP->atomA[angleDefP->atom4I].coord,
	angleP->val);

    rotEntry.firstI = angleDefP->atom3I + 1;
    if (angleDefP->lastAtomI == -1) {
      rotEntry.lastI = resDefP->atomNo - 1;
      Mat4Mult(bbMat, m);
      Mat4Ortho(bbMat);
      Mat4Copy(rotEntry.m, bbMat);
    } else {
      rotEntry.lastI = angleDefP->lastAtomI;
      rotEntryP = ListFirst(rotList);
      while (rotEntry.firstI > rotEntryP->lastI)
	rotEntryP = ListNext(rotList, rotEntryP);
      Mat4Copy(rotEntry.m, rotEntryP->m);
      Mat4Mult(rotEntry.m, m);
    }
    (void) ListInsertFirst(rotList, &rotEntry);

    angleP->changed = FALSE;
  }

  for (i = 0; i < resDefP->atomNo; i++) {
    atomP = resP->atomA + i;

    if (atomP->state != AS_VALID)
      continue;

    rotEntryP = ListFirst(rotList);
    while (i < rotEntryP->firstI || i > rotEntryP->lastI)
      rotEntryP = ListNext(rotList, rotEntryP);

    Vec3To4(v, atomP->coord);
    Mat4VecMult(v, rotEntryP->m);
    Vec4To3(atomP->coord, v);
  }

  ListClose(rotList);
}

void
DhResAnglesChanged(DhResP resP)
{
  Mat4 bbMat;  /* backbone rotation */

  Mat4Ident(bbMat);
  resAnglesChanged(resP, bbMat);
}

static void
resDock(DhResP resP, Vec3 p11, Vec3 p12, Vec3 p13,
    Vec3 p21, Vec3 p22, Vec3 p23, BOOL forward)
{
  Vec3 d1, x1, y1, z1;
  Vec3 d2, x2, y2, z2;
  Mat3 m1, m2;
  int i;

  Vec3Copy(x1, p11);
  Vec3Copy(d1, p12);
  Vec3Copy(y1, p13);

  Vec3Copy(x2, p21);
  Vec3Copy(d2, p22);
  Vec3Copy(y2, p23);

  Vec3Sub(x1, d1);
  Vec3Sub(y1, d1);

  Vec3Sub(x2, d2);
  Vec3Sub(y2, d2);

  Vec3Copy(z1, x1);
  Vec3Cross(z1, y1);
  Vec3Copy(y1, z1);
  Vec3Cross(y1, x1);
  Vec3Norm(x1);
  Vec3Norm(y1);
  Vec3Norm(z1);

  Vec3Copy(z2, x2);
  Vec3Cross(z2, y2);
  Vec3Copy(y2, z2);
  Vec3Cross(y2, x2);
  Vec3Norm(x2);
  Vec3Norm(y2);
  Vec3Norm(z2);

  for (i = 0; i < 3; i++) {
    m1[i][0] = x1[i];
    m1[i][1] = y1[i];
    m1[i][2] = z1[i];
    m2[0][i] = x2[i];
    m2[1][i] = y2[i];
    m2[2][i] = z2[i];
  }

  Mat3Mult(m2, m1);

  while (resP != NULL) {
    for (i = 0; i < resP->defP->atomNo; i++) {
      Vec3Sub(resP->atomA[i].coord, d1);
      Mat3VecMult(resP->atomA[i].coord, m2);
      Vec3Add(resP->atomA[i].coord, d2);
    }
    if (forward)
      resP = ListNext(resP->molP->resL, resP);
    else
      resP = ListPrev(resP->molP->resL, resP);
  }
}

void
DhResDockPrev(DhResP resP)
/* transform coordinates so that first three atoms of residue
   overlap last three atoms of previous residue */
{
  DhResP prevResP;
  int i;

  for (i = 0; i < EQUIV_NO; i++)
    if (resP->equivI[i] < 0)
      return;

  prevResP = ListPrev(resP->molP->resL, resP);

  resDock(resP,
      resP->atomA[0].coord,
      resP->atomA[1].coord,
      resP->atomA[2].coord,
      prevResP->atomA[resP->equivI[0]].coord,
      prevResP->atomA[resP->equivI[1]].coord,
      prevResP->atomA[resP->equivI[2]].coord,
      TRUE);
}

void
DhResDockNext(DhResP resP)
/* transform coordinates so that last three atoms of residue
   overlap first three atoms of next residue */
{
  DhResP nextResP;
  int i;

  nextResP = ListNext(resP->molP->resL, resP);
  if (nextResP == NULL)
    return;

  for (i = 0; i < EQUIV_NO; i++)
    if (nextResP->equivI[i] < 0)
      return;

  resDock(resP,
      resP->atomA[nextResP->equivI[0]].coord,
      resP->atomA[nextResP->equivI[1]].coord,
      resP->atomA[nextResP->equivI[2]].coord,
      nextResP->atomA[0].coord,
      nextResP->atomA[1].coord,
      nextResP->atomA[2].coord,
      FALSE);
}

void
DhMolAnglesChanged(DhMolP molP)
{
  Mat4 bbMat;  /* backbone rotation */
  DhResP resP;

  Mat4Ident(bbMat);

  resP = ListFirst(molP->resL);
  while (resP != NULL) {
    resAnglesChanged(resP, bbMat);
    resP = ListNext(molP->resL, resP);
  }

  calcMolDist(molP);
  callMolChanged(molP);
}

static float
random1(void)
/* random number between -1.0 and 1.0 */
{
  return 2.0f * (float) rand() / (((unsigned) 1 << 15) - 1) - 1.0f;
}

static void
randomOrtho(Vec3 v1, Vec3 v2)
/* construct a random vector v2 normal to v1 */
{
  if (v1[0] != 0.0f) {
    v2[1] = random1();
    v2[2] = random1();
    v2[0] = - (v1[1] * v2[1] + v1[2] * v2[2]) / v1[0];
  } else if (v1[1] != 0.0f) {
    v2[0] = random1();
    v2[2] = random1();
    v2[1] = - (v1[0] * v2[0] + v1[2] * v2[2]) / v1[1];
  } else {
    v2[0] = random1();
    v2[1] = random1();
    v2[2] = - (v1[0] * v2[0] + v1[1] * v2[1]) / v1[2];
  }
}

void
DhResCalcAtom(DhResP resP, DSTR name)
{
  DhResDefP defP = resP->defP;
  int atomI, bondI, i;
  int neighA[MAX_NEIGH_NO + 1], neighNo, oldNeighNo, searchI, neighI;
  BOOL oneNeigh;
  Vec3 sCoord[MAX_NEIGH_NO], aCoord[MAX_NEIGH_NO];
  Mat4 mapM;
  Vec3 v3;
  Vec4 v4;

  for (atomI = 0; atomI < defP->atomNo; atomI++) {
    if (resP->atomA[atomI].state != AS_UNKNOWN_COORD)
      continue;

    if (! StrMatch(DStrToStr(defP->atomA[atomI].name), DStrToStr(name)))
      continue;

    /* breadth first search until at least 3 neighbours found,
       take all of one level until MAX_NEIGH_NO is reached */
    oneNeigh = FALSE;
    neighNo = 1;
    neighA[0] = atomI;
    searchI = 0;

    for (;;) {
      oldNeighNo = neighNo;

      for (bondI = 0; bondI < defP->bondNo; bondI++) {
	if (defP->bondA[bondI].atom1I == neighA[searchI])
	  neighI = defP->bondA[bondI].atom2I;
	else if (defP->bondA[bondI].atom2I == neighA[searchI])
	  neighI = defP->bondA[bondI].atom1I;
	else
	  continue;

	if (resP->atomA[neighI].state != AS_VALID)
	  continue;
	
	for (i = 0; i < neighNo; i++)
	  if (neighA[i] == neighI)
	    break;

	if (i < neighNo)
	  continue;
	
	neighA[neighNo] = neighI;
	neighNo++;

	if (neighNo == MAX_NEIGH_NO + 1)
	  break;
      }

      if (neighNo == oldNeighNo)
	break;

      if (neighNo == 2)
	/* has exactly one direct neighbour, overlap this one exactly
	   to maintain bond length of standard geometry */
	oneNeigh = TRUE;

      if (neighNo >= 4)
	break;

      searchI++;
    }

    if (neighNo >= 4) {
      if (oneNeigh) {
	for (i = 0; i < neighNo - 2; i++) {
	  Vec3Copy(sCoord[i], defP->atomA[neighA[i + 2]].coord);
	  Vec3Sub(sCoord[i], defP->atomA[neighA[1]].coord);
	  Vec3Copy(aCoord[i], resP->atomA[neighA[i + 2]].coord);
	  Vec3Sub(aCoord[i], resP->atomA[neighA[1]].coord);
	}

	MapCoord(aCoord, neighNo - 2, sCoord, FALSE, mapM);
	Vec3Copy(v3, defP->atomA[atomI].coord);
	Vec3Sub(v3, defP->atomA[neighA[1]].coord);
	Vec3To4(v4, v3);
	Mat4VecMult(v4, mapM);
	Vec4To3(v3, v4);
	Vec3Add(v3, resP->atomA[neighA[1]].coord);
	Vec3Copy(resP->atomA[atomI].coord, v3);
      } else {
	for (i = 0; i < neighNo - 1; i++) {
	  Vec3Copy(sCoord[i], defP->atomA[neighA[i + 1]].coord);
	  Vec3Copy(aCoord[i], resP->atomA[neighA[i + 1]].coord);
	}

	MapCoord(aCoord, neighNo - 1, sCoord, TRUE, mapM);
	Vec3To4(v4, defP->atomA[atomI].coord);
	Mat4VecMult(v4, mapM);
	Vec4To3(resP->atomA[atomI].coord, v4);
      }

      resP->atomA[atomI].state = AS_VALID;
    } else if (neighNo == 3) {
      if (oneNeigh) {
	Vec3Copy(sCoord[0], defP->atomA[neighA[2]].coord);
	Vec3Sub(sCoord[0], defP->atomA[neighA[1]].coord);
	Vec3Copy(aCoord[0], resP->atomA[neighA[2]].coord);
	Vec3Sub(aCoord[0], resP->atomA[neighA[1]].coord);

	/* any vector orthogonal to sCoord[0] */
	sCoord[1][0] = sCoord[0][1];
	sCoord[1][1] = - sCoord[0][0];
	sCoord[1][2] = 0.0f;
	Vec3Norm(sCoord[1]);

	randomOrtho(aCoord[0], aCoord[1]);
	Vec3Norm(aCoord[1]);

	MapCoord(aCoord, 2, sCoord, FALSE, mapM);
	Vec3Copy(v3, defP->atomA[atomI].coord);
	Vec3Sub(v3, defP->atomA[neighA[1]].coord);
	Vec3To4(v4, v3);
	Mat4VecMult(v4, mapM);
	Vec4To3(v3, v4);
	Vec3Add(v3, resP->atomA[neighA[1]].coord);
	Vec3Copy(resP->atomA[atomI].coord, v3);
      } else {
	Vec3Copy(sCoord[0], defP->atomA[neighA[1]].coord);
	Vec3Copy(aCoord[0], resP->atomA[neighA[1]].coord);
	Vec3Copy(sCoord[1], defP->atomA[neighA[2]].coord);
	Vec3Copy(aCoord[1], resP->atomA[neighA[2]].coord);

	/* any vector orthogonal to sCoord[1] - sCoord[0] */
	sCoord[2][0] = sCoord[1][1] - sCoord[0][1];
	sCoord[2][1] = sCoord[0][0] - sCoord[1][0];
	sCoord[2][2] = 0.0f;
	Vec3Norm(sCoord[2]);
	Vec3Add(sCoord[2], sCoord[0]);

	Vec3Copy(v3, aCoord[1]);
	Vec3Sub(v3, aCoord[0]);
	randomOrtho(v3, aCoord[2]);
	Vec3Norm(aCoord[2]);
	Vec3Add(aCoord[2], aCoord[0]);

	MapCoord(aCoord, 3, sCoord, TRUE, mapM);
	Vec3To4(v4, defP->atomA[atomI].coord);
	Mat4VecMult(v4, mapM);
	Vec4To3(resP->atomA[atomI].coord, v4);
      }

      resP->atomA[atomI].state = AS_VALID;
    } else if (neighNo == 2) {
      /* any pair of orthogonal vectors */
      sCoord[0][0] = 1.0f;
      sCoord[0][1] = 0.0f;
      sCoord[0][2] = 0.0f;
      sCoord[1][0] = 0.0f;
      sCoord[1][1] = 1.0f;
      sCoord[1][2] = 0.0f;

      /* random pair of orthogonal vectors */
      aCoord[0][0] = random1();
      aCoord[0][1] = random1();
      aCoord[0][2] = random1();
      Vec3Norm(aCoord[0]);
      randomOrtho(aCoord[0], aCoord[1]);
      Vec3Norm(aCoord[1]);

      MapCoord(aCoord, 2, sCoord, FALSE, mapM);
      Vec3Copy(v3, defP->atomA[atomI].coord);
      Vec3Sub(v3, defP->atomA[neighA[1]].coord);
      Vec3To4(v4, v3);
      Mat4VecMult(v4, mapM);
      Vec4To3(v3, v4);
      Vec3Add(v3, resP->atomA[neighA[1]].coord);
      Vec3Copy(resP->atomA[atomI].coord, v3);

      resP->atomA[atomI].state = AS_VALID;
    }
  }

  DhResInit(resP);
}
