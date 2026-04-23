/*
************************************************************************
*
*   ExFit.c - Fit command
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExFit.c
*   SCCS identification       : 1.21
*
************************************************************************
*/

#include <cmd_rep.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <break.h>
#include <least_sqr.h>
#include <mat_vec.h>
#include <map_coord.h>
#include <princip_axis.h>
#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

#define MAX_STEPS 100

#define PAR_NO  3
#define COOR_NO 3

typedef struct {
  Vec3 coord;
  int atomNo;
} CenterData;

typedef struct {
  Vec3 *coordA;
  int atomI;
} MapData;

typedef struct {
  float **c;
  float *d;
  int molNo;
  int atomNo;
  int molI;
  int atomI;
} LSData;

static int FitAlgo = 1;

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
sumAtoms(DhAtomP atomP, void *clientData)
{
  CenterData *centP = clientData;
  Vec3 x;

  DhAtomGetCoord(atomP, x);
  Vec3Add(centP->coord, x);
  centP->atomNo++;
}

static void
fillMap(DhAtomP atomP, void *clientData)
{
  MapData *mapDataP = clientData;

  DhAtomGetCoordTransf(atomP, mapDataP->coordA[mapDataP->atomI]);
  mapDataP->atomI++;
}

static void
fillDataCirc(DhAtomP atomP, void *clientData)
{
  LSData *lsDataP = clientData;
  Vec3 x;
  int ai, an, mi0, mi1;
  float *equP;

  DhAtomGetCoordTransf(atomP, x);
  ai = lsDataP->atomI;
  an = lsDataP->atomNo;
  mi1 = lsDataP->molI;
  mi0 = mi1 - 1;

  if (mi1 > 0) {
    equP = lsDataP->c[COOR_NO * (mi0 * an + ai)] + PAR_NO * mi0;
    equP[1] = x[2];
    equP[2] = x[1];
    equP = lsDataP->c[COOR_NO * (mi1 * an + ai)] + PAR_NO * mi0;
    equP[1] = - x[2];
    equP[2] = - x[1];

    equP = lsDataP->c[COOR_NO * (mi0 * an + ai) + 1] + PAR_NO * mi0;
    equP[0] = x[2];
    equP[2] = - x[0];
    equP = lsDataP->c[COOR_NO * (mi1 * an + ai) + 1] + PAR_NO * mi0;
    equP[0] = - x[2];
    equP[2] = x[0];

    equP = lsDataP->c[COOR_NO * (mi0 * an + ai) + 2] + PAR_NO * mi0;
    equP[0] = - x[1];
    equP[1] = - x[0];
    equP = lsDataP->c[COOR_NO * (mi1 * an + ai) + 2] + PAR_NO * mi0;
    equP[0] = x[1];
    equP[1] = x[0];
  }

  lsDataP->d[COOR_NO * (mi1 * an + ai)] += x[0];
  lsDataP->d[COOR_NO * (mi1 * an + ai) + 1] += x[1];
  lsDataP->d[COOR_NO * (mi1 * an + ai) + 2] += x[2];

  if (mi1 == 0)
    mi0 = lsDataP->molNo - 1;

  lsDataP->d[COOR_NO * (mi0 * an + ai)] -= x[0];
  lsDataP->d[COOR_NO * (mi0 * an + ai) + 1] -= x[1];
  lsDataP->d[COOR_NO * (mi0 * an + ai) + 2] -= x[2];

  lsDataP->atomI++;
}

static void
fillDataMean(DhAtomP atomP, void *clientData)
{
  LSData *lsDataP = clientData;
  Vec3 x;
  int ai, an, mi;
  float *equP;
  float nInv;
  int i;

  DhAtomGetCoordTransf(atomP, x);
  ai = lsDataP->atomI;
  an = lsDataP->atomNo;
  mi = lsDataP->molI;

  nInv = 1.0f / (float) lsDataP->molNo;

  if (mi > 0) {
    equP = lsDataP->c[COOR_NO * (mi * an + ai)] + PAR_NO * (mi - 1);
    equP[1] += x[2];
    equP[2] += x[1];

    equP = lsDataP->c[COOR_NO * (mi * an + ai) + 1] + PAR_NO * (mi - 1);
    equP[0] += x[2];
    equP[2] += - x[0];

    equP = lsDataP->c[COOR_NO * (mi * an + ai) + 2] + PAR_NO * (mi - 1);
    equP[0] += - x[1];
    equP[1] += - x[0];

    for (i = 0; i < lsDataP->molNo; i++) {
      equP = lsDataP->c[COOR_NO * (i * an + ai)] + PAR_NO * (mi - 1);
      equP[1] -= nInv * x[2];
      equP[2] -= nInv * x[1];

      equP = lsDataP->c[COOR_NO * (i * an + ai) + 1] + PAR_NO * (mi - 1);
      equP[0] -= nInv * x[2];
      equP[2] -= nInv * - x[0];

      equP = lsDataP->c[COOR_NO * (i * an + ai) + 2] + PAR_NO * (mi - 1);
      equP[0] -= nInv * - x[1];
      equP[1] -= nInv * - x[0];
    }
  }

  lsDataP->d[COOR_NO * (mi * an + ai)] -= x[0];
  lsDataP->d[COOR_NO * (mi * an + ai) + 1] -= x[1];
  lsDataP->d[COOR_NO * (mi * an + ai) + 2] -= x[2];

  for (i = 0; i < lsDataP->molNo; i++) {
    lsDataP->d[COOR_NO * (i * an + ai)] += nInv * x[0];
    lsDataP->d[COOR_NO * (i * an + ai) + 1] += nInv * x[1];
    lsDataP->d[COOR_NO * (i * an + ai) + 2] += nInv * x[2];
  }

  lsDataP->atomI++;
}

static void
mapMol(DhMolP molP, float *x)
{
  Mat3 newMat3;
  Mat4 newMat4, rotMat;

  Mat3Ident(newMat3);
  newMat3[0][1] = x[2];
  newMat3[0][2] = x[1];
  newMat3[1][0] = - x[2];
  newMat3[1][2] = x[0];
  newMat3[2][0] = - x[1];
  newMat3[2][1] = - x[0];
  Mat3Ortho(newMat3);
  Mat3To4(newMat4, newMat3);

  DhMolGetRotMat(molP, rotMat);
  Mat4Mult(newMat4, rotMat);
  Mat4Ortho(newMat4);
  DhMolSetRotMat(molP, newMat4);
}

static BOOL
isIdent(float *x)
{
  float a;
  int i;

  a = 0.0f;
  for (i = 0; i < PAR_NO; i++)
    a += x[i] * x[i];
  
  return a < (float) 1.0E-10;
}

#define ARG_NUM 2
#define ENUM_SIZE 4

ErrCode
ExFit(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[ENUM_SIZE];
  ErrCode errCode;
  MapData mapData0, mapData1;
  LSData lsData;
  DhMolP *molPA;
  char *str, propName[100];
  PropRefP *refPA;
  int refNo, refI, nameLen;
  int molNo, switchNo, atomNo, atomNo1;
  int parNo, equNo;
  float *m;
  float *t, *x;
  int step;
  BOOL converged, done;
  CenterData cent;
  Vec3 transV, rotPoint, restTransV;
  Vec4 v4;
  Mat4 rotMat, mapM;
  float avgRmsd, rmsd;
  DSTR statStr;
  char buf[20];
  Vec3 v;
  int molI, atomI, i;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "to_axes";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "to_first";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "circular";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "to_mean";
  enumEntry[3].onOff = FALSE;

  enumEntry[FitAlgo].onOff = TRUE;

  arg[0].prompt = "Fitting Algorithm";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = ENUM_SIZE;
  arg[0].u.enumD.lineNo = 2;
  arg[0].v.intVal = FitAlgo;

  arg[1].prompt = "Properties";
  DStrAssignStr(arg[1].v.strVal, PROP_SELECTED);
  arg[1].optional = TRUE;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  FitAlgo = arg[0].v.intVal;

  str = DStrToStr(arg[1].v.strVal);
  refNo = 0;
  for (;;) {
    while (*str == ' ')
      str++;
    if (*str == '\0')
      break;
    
    nameLen = 0;
    while (*str != ' ' && *str != '\0') {
      if (nameLen < sizeof(propName) - 1)
	propName[nameLen++] = *str;
      str++;
    }
    propName[nameLen] = '\0';

    if (refNo == 0)
      refPA = malloc(sizeof(*refPA));
    else
      refPA = realloc(refPA, (refNo + 1) * sizeof(*refPA));

    refPA[refNo++] = PropGetRef(propName, FALSE);
  }

  ArgCleanup(arg, ARG_NUM);

  if (refNo == 0) {
    CipSetError("property list may not be empty");
    return EC_ERROR;
  }

  DhActivateGroups(TRUE);

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0) {
    free(refPA);
    DhActivateGroups(FALSE);
    CipSetError("at least 1 molecule must be selected");
    return EC_ERROR;
  }

  molPA = malloc(molNo * sizeof(DhMolP));
  (void) SelMolGet(molPA, molNo);

  /* ignore molecules with no atoms selected */
  switchNo = 0;
  for (molI = 0; molI < molNo; molI++) {
    atomNo = 0;
    for (refI = 0; refI < refNo; refI++)
      DhMolApplyAtom(refPA[refI], molPA[molI], countAtoms, &atomNo);
    if (atomNo > 0)
      molPA[switchNo++] = molPA[molI];
  }
  molNo = switchNo;

  if (FitAlgo == 0 && molNo == 0) {
    free(refPA);
    free(molPA);
    DhActivateGroups(FALSE);
    CipSetError("at least 1 molecule must be selected");
    return EC_ERROR;
  } else if (FitAlgo != 0 && molNo < 2) {
    free(refPA);
    free(molPA);
    DhActivateGroups(FALSE);
    CipSetError("at least 2 molecules must be selected");
    return EC_ERROR;
  }

  atomNo = 0;
  for (refI = 0; refI < refNo; refI++)
    DhMolApplyAtom(refPA[refI], molPA[0], countAtoms, &atomNo);
  if (atomNo < PAR_NO) {
    free(refPA);
    free(molPA);
    DhActivateGroups(FALSE);
    CipSetError("at least 3 atoms must be selected");
    return EC_ERROR;
  }

  for (molI = 1; molI < molNo; molI++) {
    atomNo1 = 0;
    for (refI = 0; refI < refNo; refI++)
      DhMolApplyAtom(refPA[refI], molPA[molI], countAtoms, &atomNo1);
    if (FitAlgo == 0) {
      if (atomNo1 > atomNo) {
	atomNo = atomNo1;
      } else if (atomNo1 < PAR_NO) {
	free(refPA);
	free(molPA);
	DhActivateGroups(FALSE);
	CipSetError("at least 3 atoms must be selected");
	return EC_ERROR;
      }
    } else {
      if (atomNo1 != atomNo) {
	free(refPA);
	free(molPA);
	DhActivateGroups(FALSE);
	CipSetError("number of selected atoms must be equal");
	return EC_ERROR;
      }
    }
  }

  /* rotation point is at center of mass */
  for (molI = 0; molI < molNo; molI++) {
    Vec3Zero(cent.coord);
    cent.atomNo = 0;
    for (refI = 0; refI < refNo; refI++)
      DhMolApplyAtom(refPA[refI], molPA[molI], sumAtoms, &cent);
    Vec3Scale(cent.coord, 1.0f / cent.atomNo);

    if (molI == 0) {
      /* restore translation of first structure after fit */
      Vec3Copy(restTransV, cent.coord);
      DhMolGetRotPoint(molPA[0], rotPoint);
      Vec3Sub(restTransV, rotPoint);
      Vec3To4(v4, restTransV);
      DhMolGetRotMat(molPA[0], rotMat);
      Mat4VecMult(v4, rotMat);
      Vec4To3(restTransV, v4);
      DhMolGetTransVect(molPA[0], transV);
      Vec3Add(restTransV, transV);
    } 

    DhMolSetRotPoint(molPA[molI], cent.coord);
    Vec3Zero(transV);
    DhMolSetTransVect(molPA[molI], transV);
  }

  converged = TRUE;
  BreakActivate(TRUE);

  if (FitAlgo == 0) {
    mapData0.coordA = malloc(atomNo * sizeof(Vec3));

    for (molI = 0; molI < molNo; molI++) {
      if (BreakCheck(1))
	break;

      mapData0.atomI = 0;
      for (refI = 0; refI < refNo; refI++)
	DhMolApplyAtom(refPA[refI], molPA[molI], fillMap, &mapData0);

      PrincipAxis(mapData0.coordA, mapData0.atomI, molI > 0, mapM, NULL);

      DhMolGetRotMat(molPA[molI], rotMat);
      Mat4Mult(mapM, rotMat);
      Mat4Ortho(mapM);
      DhMolSetRotMat(molPA[molI], mapM);
    }

    free(mapData0.coordA);
  } else if (FitAlgo == 1) {
    mapData0.coordA = malloc(atomNo * sizeof(Vec3));
    mapData1.coordA = malloc(atomNo * sizeof(Vec3));

    mapData0.atomI = 0;
    for (refI = 0; refI < refNo; refI++)
      DhMolApplyAtom(refPA[refI], molPA[0], fillMap, &mapData0);

    for (molI = 1; molI < molNo; molI++) {
      if (BreakCheck(1))
	break;

      mapData1.atomI = 0;
      for (refI = 0; refI < refNo; refI++)
	DhMolApplyAtom(refPA[refI], molPA[molI], fillMap, &mapData1);

      MapCoord(mapData0.coordA, atomNo, mapData1.coordA, FALSE, mapM);

      DhMolGetRotMat(molPA[molI], rotMat);
      Mat4Mult(mapM, rotMat);
      Mat4Ortho(mapM);
      DhMolSetRotMat(molPA[molI], mapM);
    }

    free(mapData0.coordA);
    free(mapData1.coordA);
  } else {
    parNo = PAR_NO * (molNo - 1);
    equNo = COOR_NO * atomNo * molNo;

    m = malloc(parNo * equNo * sizeof(float));
    lsData.c = malloc(equNo * sizeof(float *));
    for (i = 0; i < equNo; i++)
      lsData.c[i] = m + parNo * i;
    lsData.d = malloc(equNo * sizeof(float));
    t = malloc(parNo * sizeof(float));
    x = malloc(parNo * sizeof(float));

    lsData.molNo = molNo;
    lsData.atomNo = atomNo;

    for (step = 0; step < MAX_STEPS; step++) {  /* iteration */
      if (BreakCheck(1))
	break;

      for (i = 0; i < parNo * equNo; i++)
	m[i] = 0.0f;

      for (i = 0; i < equNo; i++)
	lsData.d[i] = 0.0f;

      for (molI = 0; molI < molNo; molI++) {
	if (BreakCheck(1))
	  break;

	lsData.molI = molI;
	lsData.atomI = 0;
	if (FitAlgo == 2)
	  for (refI = 0; refI < refNo; refI++)
	    DhMolApplyAtom(refPA[refI], molPA[molI], fillDataCirc, &lsData);
	else
	  for (refI = 0; refI < refNo; refI++)
	    DhMolApplyAtom(refPA[refI], molPA[molI], fillDataMean, &lsData);
      }

      LeastSqrMatTransf(lsData.c, parNo, equNo, t);
      LeastSqrCalcSol(lsData.c, t, lsData.d, parNo, equNo, x, NULL);

      done = TRUE;
      for (molI = 1; molI < molNo; molI++) {
	if (BreakCheck(1))
	  break;

	mapMol(molPA[molI], x + (molI - 1) * PAR_NO);
	if (! isIdent(x + (molI - 1) * PAR_NO))
	  done = FALSE;
      }

      if (done)
	break;
    }

    if (step == MAX_STEPS)
      converged = FALSE;

    free(m);
    free(t);
    free(x);
    free(lsData.c);
    free(lsData.d);
  }

  BreakActivate(FALSE);

  for (molI = 0; molI < molNo; molI++)
    DhMolSetTransVect(molPA[molI], restTransV);

  if (FitAlgo > 0) {
    mapData0.coordA = malloc(atomNo * sizeof(Vec3));
    mapData1.coordA = malloc(atomNo * sizeof(Vec3));

    /* calculate mean structure */

    for (atomI = 0; atomI < atomNo; atomI++)
      Vec3Zero(mapData0.coordA[atomI]);

    for (molI = 0; molI < molNo; molI++) {
      mapData1.atomI = 0;
      for (refI = 0; refI < refNo; refI++)
	DhMolApplyAtom(refPA[refI], molPA[molI], fillMap, &mapData1);

      for (atomI = 0; atomI < atomNo; atomI++)
	Vec3Add(mapData0.coordA[atomI], mapData1.coordA[atomI]);
    }

    for (atomI = 0; atomI < atomNo; atomI++)
      Vec3Scale(mapData0.coordA[atomI], 1.0f / molNo);

    /* calculate RMSD to mean */

    avgRmsd = 0.0f;

    for (molI = 0; molI < molNo; molI++) {
      mapData1.atomI = 0;
      for (refI = 0; refI < refNo; refI++)
	DhMolApplyAtom(refPA[refI], molPA[molI], fillMap, &mapData1);

      rmsd = 0.0f;

      for (atomI = 0; atomI < atomNo; atomI++) {
	Vec3Copy(v, mapData1.coordA[atomI]);
	Vec3Sub(v, mapData0.coordA[atomI]);
	rmsd += v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
      }

      rmsd = sqrtf(rmsd / atomNo);
      avgRmsd += rmsd;
    }

    avgRmsd /= molNo;

    free(mapData0.coordA);
    free(mapData1.coordA);

    statStr = DStrNew();
    if (molNo == 2) {
      DStrAssignStr(statStr, "RMSD:");
      (void) sprintf(buf, "%6.3f", 2.0f * avgRmsd);
    } else {
      DStrAssignStr(statStr, "average RMSD to mean:");
      (void) sprintf(buf, "%6.3f", avgRmsd);
    }
    DStrAppStr(statStr, buf);
    PuSetTextField(PU_TF_STATUS, DStrToStr(statStr));
  }

  free(refPA);
  free(molPA);

  DhActivateGroups(FALSE);
  GraphRedrawNeeded();

  if (! converged) {
    CipSetError("iteration not succeeded");
    return EC_WARNING;
  }

  return EC_OK;
}
