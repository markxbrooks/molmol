/*
************************************************************************
*
*   ExPaintSurf.c - PaintSurface command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExPaintSurf.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <mat_vec.h>
#include <sphere.h>
#include <grid.h>
#include <arg.h>
#include <par_hand.h>
#include <data_hand.h>
#include <data_ref.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <graph_draw.h>

#define PN_SURFACE_COLOR "SurfacColor"

typedef struct {
  GRID grid;
  float maxRad;
} GridData;

typedef struct {
  PrimObjP primP;
  GridData gridData;
  DhAtomP atomP;
  float minDist;
} AtomData;

typedef struct {
  float val;
  float r, g, b;
} MapVal;

typedef struct {
  MapVal *valA;
  int valNo;
} MapData;

typedef struct {
  MapData *mapP;
  Vec3 cent;
} CentData;

typedef struct {
  MapData *mapP;
  DhAtomP *atomPA;
  int atomNo;
} DistData;

typedef struct {
  MapData *mapP;
  GridData gridData;
  DhAtomP *atomPA;
  int atomNo;
  int hitNo;
  Vec3 v;
} AccessData;

static int SurfPaint = 0;
static float MinDist = 0.0f;
static float MaxDist = 1.4f;
static float Offs = 0.2f;
static int AccessPrec = 3;

static int SpherePointNo;
static Vec3 *SpherePoints;

static void
paintSurfOne(PrimObjP primP, void *clientData)
{
  PrimSurfaceSetColor(primP, NULL, NULL);
}

static void
prepareAtom(DhAtomP atomP, void *clientData)
{
  GridData *dataP = clientData;
  float rad;
  Vec3 x;

  if (BreakCheck(10000))
    return;

  rad = DhAtomGetVdw(atomP);
  if (rad < 0.0f)
    return;

  DhAtomGetCoord(atomP, x);
  GridPrepareAddEntry(dataP->grid, x);

  if (rad > dataP->maxRad)
    dataP->maxRad = rad;
}

static void
insertAtom(DhAtomP atomP, void *clientData)
{
  GridData *dataP = clientData;
  float rad;
  Vec3 x;

  if (BreakCheck(10000))
    return;

  rad = DhAtomGetVdw(atomP);
  if (rad < 0.0f)
    return;

  DhAtomGetCoord(atomP, x);
  GridInsertEntry(dataP->grid, x, atomP);
}

static BOOL
getClosest(void *entryP, Vec3 x, void *clientData)
{
  DhAtomP atomP = entryP;
  AtomData *dataP = clientData;
  Vec3 dx;
  float rad, dist;

  DhAtomGetCoord(atomP, dx);
  Vec3Sub(dx, x);
  rad = DhAtomGetVdw(atomP);
  dist = dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] - rad * rad;

  if (dist < dataP->minDist) {
    dataP->atomP = atomP;
    dataP->minDist = dist;
  }

  return TRUE;
}

static void
setColorAtom(Vec3 col, Vec3 x, Vec3 nv, float pot, void *clientData)
{
  AtomData *dataP = clientData;
  AttrP attrP;

  if (BreakCheck(10000))
    return;

  dataP->atomP = NULL;
  dataP->minDist = MAXFLOAT;
  GridFind(dataP->gridData.grid, x, 1, getClosest, dataP);

  if (dataP->atomP == NULL)
    /* no close atom, give it the default color */
    attrP = PrimGetAttr(dataP->primP);
  else
    attrP = DhAtomGetAttr(dataP->atomP);

  col[0] = attrP->colR;
  col[1] = attrP->colG;
  col[2] = attrP->colB;
}

static void
paintSurfAtom(PrimObjP primP, void *clientData)
{
  DhMolP molP;
  PropRefP refP;
  AtomData data;

  if (BreakInterrupted())
    return;

  molP = PrimGetMol(primP);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.primP = primP;
  data.gridData.grid = GridNew();
  data.gridData.maxRad = 0.0f;
  DhMolApplyAtom(refP, molP, prepareAtom, &data.gridData);
  if (data.gridData.maxRad > 0.0f) {
    GridInsertInit(data.gridData.grid, 1.1f * data.gridData.maxRad + MaxDist);
    DhMolApplyAtom(refP, molP, insertAtom, &data.gridData);
    PrimSurfaceSetColor(primP, setColorAtom, &data);
  } else {
    /* must be initialized, otherwise GridDestroy crashes */
    GridInsertInit(data.gridData.grid, 10.0f);
  }

  GridDestroy(data.gridData.grid);
}

static MapData *
getMapData(void)
{
  MapData *mapP;
  char *s, buf[50];
  int pos, i;

  mapP = malloc(sizeof(*mapP));
  mapP->valNo = 1;
  mapP->valA = malloc(sizeof(*mapP->valA));
  mapP->valA[0].val = 0.0f;
  mapP->valA[0].r = 0.0f;
  mapP->valA[0].g = 0.0f;
  mapP->valA[0].b = 1.0f;

  s = ParGetStrVal(PN_SURFACE_COLOR);
  pos = 0;
  while (*s != '\0') {
    for (i = 0; i < sizeof(buf) - 1; i++) {
      if (*s == '\0' || isspace(*s))
	break;
      buf[i] = *s;
      s++;
    }
    buf[i] = '\0';

    if (pos == 0)
      mapP->valA[mapP->valNo - 1].val = (float) atof(buf);
    else if (pos == 1)
      mapP->valA[mapP->valNo - 1].r = (float) atof(buf);
    else if (pos == 2)
      mapP->valA[mapP->valNo - 1].g = (float) atof(buf);
    else
      mapP->valA[mapP->valNo - 1].b = (float) atof(buf);
    
    pos++;

    while (isspace(*s))
      s++;
    
    if (*s == '\0')
      break;

    if (pos == 4) {
      mapP->valNo++;
      mapP->valA = realloc(mapP->valA,
	  mapP->valNo * sizeof(*mapP->valA));
      pos = 0;
    }
  }

  return mapP;
}

static void
freeMapData(MapData *mapP)
{
  free(mapP->valA);
  free(mapP);
}

static void
lookupMap(MapData *mapP, float val, Vec3 col)
{
  int valI;
  float par;

  valI = -1;
  while (valI < mapP->valNo - 1 && val > mapP->valA[valI + 1].val)
    valI++;
  
  if (valI == -1) {
    col[0] = mapP->valA[0].r;
    col[1] = mapP->valA[0].g;
    col[2] = mapP->valA[0].b;
  } else if (valI == mapP->valNo - 1) {
    col[0] = mapP->valA[valI].r;
    col[1] = mapP->valA[valI].g;
    col[2] = mapP->valA[valI].b;
  } else {
    par = (val - mapP->valA[valI].val) /
	(mapP->valA[valI + 1].val - mapP->valA[valI].val);
    col[0] = (1.0f - par) * mapP->valA[valI].r +
	par * mapP->valA[valI + 1].r;
    col[1] = (1.0f - par) * mapP->valA[valI].g +
	par * mapP->valA[valI + 1].g;
    col[2] = (1.0f - par) * mapP->valA[valI].b +
	par * mapP->valA[valI + 1].b;
  }
}

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
fillAtoms(DhAtomP atomP, void *clientData)
{
  DistData *dataP = clientData;

  dataP->atomPA[dataP->atomNo++] = atomP;
}

static void
setColorDist(Vec3 col, Vec3 x, Vec3 nv, float pot, void *clientData)
{
  DistData *dataP = clientData;
  float dMin, d;
  int atomI;
  Vec3 v, c;

  if (BreakCheck(100000 / dataP->atomNo))
    return;

  dMin = MAXFLOAT;
  for (atomI = 0; atomI < dataP->atomNo; atomI++) {
    DhAtomGetCoord(dataP->atomPA[atomI], c);
    Vec3Copy(v, x);
    Vec3Sub(v, c);
    d = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if (d < dMin)
      dMin = d;
  }

  lookupMap(dataP->mapP, sqrtf(dMin), col);
}

static void
paintSurfDist(PrimObjP primP, void *clientData)
{
  MapData *mapP = clientData;
  DhMolP molP;
  PropRefP refP;
  DistData data;

  if (BreakInterrupted())
    return;

  molP = PrimGetMol(primP);
  if (molP == NULL)
    return;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.mapP = mapP;
  data.atomNo = 0;
  DhMolApplyAtom(refP, molP, countAtoms, &data.atomNo);
  if (data.atomNo == 0) {
    PrimSurfaceSetColor(primP, NULL, NULL);
    return;
  }

  data.atomPA = malloc(data.atomNo * sizeof(*data.atomPA));
  data.atomNo = 0;
  DhMolApplyAtom(refP, molP, fillAtoms, &data);

  PrimSurfaceSetColor(primP, setColorDist, &data);

  free(data.atomPA);
}

static void
setColorCent(Vec3 col, Vec3 x, Vec3 nv, float pot, void *clientData)
{
  CentData *dataP = clientData;

  if (BreakCheck(100000))
    return;

  lookupMap(dataP->mapP, Vec3DiffAbs(x, dataP->cent), col);
}

static void
paintSurfCent(PrimObjP primP, void *clientData)
{
  MapData *mapP = clientData;
  DhMolP molP;
  CentData data;

  if (BreakInterrupted())
    return;

  molP = PrimGetMol(primP);
  if (molP == NULL)
    return;

  data.mapP = mapP;
  DhCalcCenter(data.cent, molP);

  PrimSurfaceSetColor(primP, setColorCent, &data);
}

static BOOL
getCloseAtoms(void *entryP, Vec3 x, void *clientData)
{
  DhAtomP atomP = entryP;
  AccessData *dataP = clientData;
  Vec3 v;
  float r;

  DhAtomGetCoord(atomP, v);
  Vec3Sub(v, x);
  r = DhAtomGetVdw(atomP);

  if (v[0] * v[0] + v[1] * v[1] + v[2] * v[2] >
      (MaxDist + r) * (MaxDist + r))
    return TRUE;

  dataP->atomPA[dataP->atomNo++] = atomP;

  return TRUE;
}

static void
setColorAccess(Vec3 col, Vec3 x, Vec3 nv, float pot, void *clientData)
{
  AccessData *dataP = clientData;
  DhAtomP atomP;
  Vec3 cent;
  int hitNo;
  Vec3 c;
  float d, r, s, t;
  int pointI, atomI;

  if (BreakCheck(100000 / SpherePointNo))
    return;

  Vec3Copy(cent, x);
  Vec3ScaleAdd(cent, Offs, nv);

  dataP->atomNo = 0;
  GridFind(dataP->gridData.grid, cent, 1, getCloseAtoms, dataP);

  hitNo = 0;
  for (pointI = 0; pointI < SpherePointNo; pointI++) {
    for (atomI = 0; atomI < dataP->atomNo; atomI++) {
      atomP = dataP->atomPA[atomI];

      DhAtomGetCoord(atomP, c);
      Vec3Sub(c, cent);

      s = Vec3Scalar(c, SpherePoints[pointI]);
      if (s < MinDist)  /* behind or too close */
	continue;

      r = DhAtomGetVdw(atomP);
      if (s - r > MaxDist)  /* too far */
	continue;

      d = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
      t = s * s - d + r * r;
      if (t < 0.0f)  /* missed */
	continue;

      if ((s - MinDist) * (s - MinDist) < t ||
	  (s > MaxDist && (s - MaxDist) * (s - MaxDist) > t))
	continue;

      hitNo++;
      break;
    }
  }

  lookupMap(dataP->mapP, 1.0f - (float) hitNo / SpherePointNo, col);
}

static void
paintSurfAccess(PrimObjP primP, void *clientData)
{
  MapData *mapP = clientData;
  DhMolP molP;
  PropRefP refP;
  int atomNo;
  AccessData data;

  if (BreakInterrupted())
    return;

  molP = PrimGetMol(primP);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  atomNo = 0;
  DhMolApplyAtom(refP, molP, countAtoms, &atomNo);
  if (atomNo == 0) {
    PrimSurfaceSetColor(primP, NULL, NULL);
    return;
  }

  data.mapP = mapP;
  data.atomPA = malloc(atomNo * sizeof(*data.atomPA));

  data.gridData.grid = GridNew();
  data.gridData.maxRad = 0.0f;
  DhMolApplyAtom(refP, molP, prepareAtom, &data.gridData);
  if (data.gridData.maxRad > 0.0f) {
    GridInsertInit(data.gridData.grid, MaxDist + data.gridData.maxRad);
    DhMolApplyAtom(refP, molP, insertAtom, &data.gridData);
    PrimSurfaceSetColor(primP, setColorAccess, &data);
  } else {
    /* must be initialized, otherwise GridDestroy crashes */
    GridInsertInit(data.gridData.grid, 10.0f);
  }

  free(data.atomPA);
  GridDestroy(data.gridData.grid);
}

static void
setColorPot(Vec3 col, Vec3 x, Vec3 nv, float pot, void *clientData)
{
  MapData *mapP = clientData;

  if (BreakCheck(100000))
    return;

  lookupMap(mapP, pot, col);
}

static void
paintSurfPot(PrimObjP primP, void *clientData)
{
  if (BreakInterrupted())
    return;

  PrimSurfaceSetColor(primP, setColorPot, clientData);
}

#define ARG_NUM 6
#define PAINT_NUM 6

ErrCode
ExPaintSurface(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[PAINT_NUM];
  ErrCode errCode;
  MapData *mapP;
  PropRefP refP;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;
  arg[4].type = AT_INT;
  arg[5].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "one_color";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "atom";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "dist";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "center";
  enumEntry[3].onOff = FALSE;
  enumEntry[4].str = "access";
  enumEntry[4].onOff = FALSE;
  enumEntry[5].str = "pot";
  enumEntry[5].onOff = FALSE;

  enumEntry[SurfPaint].onOff = TRUE;

  arg[0].prompt = "Paint";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = PAINT_NUM;
  arg[0].u.enumD.lineNo = 2;
  arg[0].v.intVal = SurfPaint;

  arg[1].prompt = "Min. Dist";
  arg[1].v.doubleVal = MinDist;

  arg[2].prompt = "Max. Dist";
  arg[2].v.doubleVal = MaxDist;

  arg[3].prompt = "Offset";
  arg[3].v.doubleVal = Offs;

  arg[4].prompt = "Precision";
  arg[4].v.intVal = AccessPrec;

  arg[5].prompt = "Color Map";
  if (ParDefined(PN_SURFACE_COLOR))
    DStrAssignStr(arg[5].v.strVal, ParGetStrVal(PN_SURFACE_COLOR));
  else
    DStrAssignStr(arg[5].v.strVal, "-0.5 1 0 0 0.0 1 1 1 0.5 0 0 1");

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  SurfPaint = arg[0].v.intVal;
  MinDist = (float) arg[1].v.doubleVal;
  MaxDist = (float) arg[2].v.doubleVal;
  Offs = (float) arg[3].v.doubleVal;
  AccessPrec = arg[4].v.intVal;
  ParSetStrVal(PN_SURFACE_COLOR, DStrToStr(arg[5].v.strVal));

  ArgCleanup(arg, ARG_NUM);

  BreakActivate(TRUE);

  refP = PropGetRef(PROP_SELECTED, FALSE);
  if (SurfPaint == 0) {
    PrimApply(PT_SURFACE, refP, paintSurfOne, NULL);
  } else if (SurfPaint == 1) {
    PrimApply(PT_SURFACE, refP, paintSurfAtom, NULL);
  } else if (SurfPaint == 2) {
    mapP = getMapData();
    PrimApply(PT_SURFACE, refP, paintSurfDist, mapP);
    freeMapData(mapP);
  } else if (SurfPaint == 3) {
    mapP = getMapData();
    PrimApply(PT_SURFACE, refP, paintSurfCent, mapP);
    freeMapData(mapP);
  } else if (SurfPaint == 4) {
    mapP = getMapData();
    SphereCalcPoints(AccessPrec, &SpherePoints, &SpherePointNo);
    PrimApply(PT_SURFACE, refP, paintSurfAccess, mapP);
    freeMapData(mapP);
    free(SpherePoints);
  } else {
    mapP = getMapData();
    PrimApply(PT_SURFACE, refP, paintSurfPot, mapP);
    freeMapData(mapP);
  }

  BreakActivate(FALSE);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
