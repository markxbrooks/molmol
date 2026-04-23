/*
************************************************************************
*
*   IsoSurface.c - iso-surface calculation
*
*   Copyright (c) 1995
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.IsoSurface.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <iso_surface.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <bool.h>
#include <mat_vec.h>

#define TET_NO 5
#define FACE_NO 4
#define EDGE_NO 6

#define STATE_DONE 256

typedef enum {
  D_0,
  D_MX,
  D_PX,
  D_MY,
  D_PY,
  D_MZ,
  D_PZ
} Dir;

typedef struct {
  int selfI;
  int otherI;
} EdgeCorr;

typedef struct {
  Dir dir;
  int ti;
  int pi0, pi1, pi2;
  EdgeCorr corrA[3];
} FaceDesc;

typedef struct {
  int pi0, pi1, pi2, pi3;
  FaceDesc faceA[FACE_NO];
} TetDesc;

typedef struct {
  TetDesc tetA[TET_NO];
} CubeDesc;

typedef struct {
  short state;
} Tet;

typedef struct {
  int pat;
  Tet tetA[TET_NO];
} Cube;

typedef struct {
  float x, y, z;
  float nx, ny, nz;
} Point;

typedef struct {
  int vi;
  int xi, yi, zi;
  Cube *cP;
  Tet *tP;
  int ti;
  int pat;
  BOOL odd;
  int piA[EDGE_NO];
  int stripIA[FACE_NO];
  int stripNo;
  int nextI;
} QueueEntry;

typedef struct {
  int *piA;
  int len, size;
  int polNo;
} Strip;

static CubeDesc CubeEven = {
  {{0, 3, 5, 6, {{D_0, 1}, {D_0,  2}, {D_0,  3}, {D_0,  4}}},
  {3, 0, 5, 1, {{D_0, 0}, {D_MZ, 4}, {D_MY, 1}, {D_PX, 2}}},
  {0, 3, 6, 2, {{D_0, 0}, {D_MZ, 3}, {D_PY, 2}, {D_MX, 1}}},
  {3, 5, 6, 7, {{D_0, 0}, {D_PX, 3}, {D_PZ, 1}, {D_PY, 4}}},
  {5, 0, 6, 4, {{D_0, 0}, {D_MY, 3}, {D_MX, 4}, {D_PZ, 2}}}}
};

static CubeDesc CubeOdd = {
  {{1, 2, 7, 4, {{D_0, 1}, {D_0,  2}, {D_0,  3}, {D_0,  4}}},
  {2, 1, 7, 3, {{D_0, 0}, {D_MZ, 3}, {D_PX, 2}, {D_PY, 1}}},
  {1, 2, 4, 0, {{D_0, 0}, {D_MZ, 4}, {D_MX, 1}, {D_MY, 2}}},
  {2, 7, 4, 6, {{D_0, 0}, {D_PY, 4}, {D_PZ, 2}, {D_MX, 3}}},
  {7, 1, 4, 5, {{D_0, 0}, {D_PX, 4}, {D_MY, 3}, {D_PZ, 1}}}}
};

static BOOL Initialized = FALSE;

static float XOffs, YOffs, ZOffs;
static float XScale, YScale, ZScale;
static BOOL CalcNorm;

static Point *PointList;
static int PointListSize, PointNo;

static Strip *StripList;
static int StripListSize, StripNo;

static QueueEntry *Queue;
static int QueueSize, QueueFirst, QueueLast, QueueFree;

static void
fillFaces(TetDesc *tetP)
{
  tetP->faceA[0].pi0 = tetP->pi0;
  tetP->faceA[0].pi1 = tetP->pi1;
  tetP->faceA[0].pi2 = tetP->pi2;

  tetP->faceA[1].pi0 = tetP->pi1;
  tetP->faceA[1].pi1 = tetP->pi0;
  tetP->faceA[1].pi2 = tetP->pi3;

  tetP->faceA[2].pi0 = tetP->pi2;
  tetP->faceA[2].pi1 = tetP->pi1;
  tetP->faceA[2].pi2 = tetP->pi3;

  tetP->faceA[3].pi0 = tetP->pi0;
  tetP->faceA[3].pi1 = tetP->pi2;
  tetP->faceA[3].pi2 = tetP->pi3;
}


static BOOL
equalPair(int i00, int i01, int i10, int i11)
{
  return (i00 == i10 && i01 == i11) || (i00 == i11 && i01 == i10);
}

static int
getEdgeI(int pi0, int pi1, TetDesc *tetP)
{
  if (equalPair(pi0, pi1, tetP->pi0, tetP->pi1))
    return 0;

  if (equalPair(pi0, pi1, tetP->pi1, tetP->pi2))
    return 1;

  if (equalPair(pi0, pi1, tetP->pi2, tetP->pi0))
    return 2;

  if (equalPair(pi0, pi1, tetP->pi0, tetP->pi3))
    return 3;

  if (equalPair(pi0, pi1, tetP->pi1, tetP->pi3))
    return 4;

  return 5;
}

static void
fillCorr(CubeDesc *thisCubeP, CubeDesc *otherCubeP)
{
  int ti, fi1, fi2;
  TetDesc *tet1P, *tet2P;
  FaceDesc *face1P;
  int offs, pi0, pi1, pi2;

  for (ti = 0; ti < TET_NO; ti++) {
    tet1P = thisCubeP->tetA + ti;
    for (fi1 = 0; fi1 < FACE_NO; fi1++) {
      face1P = tet1P->faceA + fi1;

      pi0 = face1P->pi0;
      pi1 = face1P->pi1;
      pi2 = face1P->pi2;

      face1P->corrA[0].selfI = getEdgeI(pi0, pi1, tet1P);
      face1P->corrA[1].selfI = getEdgeI(pi1, pi2, tet1P);
      face1P->corrA[2].selfI = getEdgeI(pi2, pi0, tet1P);

      if (face1P->dir == D_0)
	tet2P = thisCubeP->tetA + face1P->ti;
      else
	tet2P = otherCubeP->tetA + face1P->ti;

      for (fi2 = 0; fi2 < FACE_NO; fi2++)
	if (tet2P->faceA[fi2].ti == ti)
	  break;

      switch (face1P->dir) {
	case D_0:
	  offs = 0;
	  break;
	case D_MX:
	  offs = 1;
	  break;
	case D_PX:
	  offs = -1;
	  break;
	case D_MY:
	  offs = 2;
	  break;
	case D_PY:
	  offs = -2;
	  break;
	case D_MZ:
	  offs = 4;
	  break;
	case D_PZ:
	  offs = -4;
	  break;
      }

      pi0 += offs;
      pi1 += offs;
      pi2 += offs;

      face1P->corrA[0].otherI = getEdgeI(pi0, pi1, tet2P);
      face1P->corrA[1].otherI = getEdgeI(pi1, pi2, tet2P);
      face1P->corrA[2].otherI = getEdgeI(pi2, pi0, tet2P);
    }
  }
}

static void
initStruc(void)
{
  int i;

  if (Initialized)
    return;

  for (i = 0; i < TET_NO; i++) {
    fillFaces(CubeEven.tetA + i);
    fillFaces(CubeOdd.tetA + i);
  }

  fillCorr(&CubeEven, &CubeOdd);
  fillCorr(&CubeOdd, &CubeEven);

  Initialized = TRUE;
}

static int
addPoint(Point **pointPP)
{
  if (PointNo == PointListSize) {
    PointListSize *= 2;
    PointList = realloc(PointList, PointListSize * sizeof(*PointList));
  }

  *pointPP = PointList + PointNo;

  if (CalcNorm) {
    (*pointPP)->nx = 0.0f;
    (*pointPP)->ny = 0.0f;
    (*pointPP)->nz = 0.0f;
  }

  PointNo++;

  return PointNo - 1;
}

static void
addNorm(int *piA, int n)
{
  float vxA[4], vyA[4], vzA[4];
  float nx, ny, nz, a;
  int i1, i2;

  for (i1 = 0; i1 < n; i1++) {
    i2 = (i1 + 1) % n;
    vxA[i1] = PointList[piA[i2]].x - PointList[piA[i1]].x;
    vyA[i1] = PointList[piA[i2]].y - PointList[piA[i1]].y;
    vzA[i1] = PointList[piA[i2]].z - PointList[piA[i1]].z;
  }

  for (i2 = 0; i2 < n; i2++) {
    i1 = (i2 + n - 1) % n;

    nx = vyA[i1] * vzA[i2] - vzA[i1] * vyA[i2];
    ny = vzA[i1] * vxA[i2] - vxA[i1] * vzA[i2];
    nz = vxA[i1] * vyA[i2] - vyA[i1] * vxA[i2];

    a = sqrtf(nx * nx + ny * ny + nz * nz);
    if (a == 0.0f)
      continue;

    PointList[piA[i2]].nx += nx / a;
    PointList[piA[i2]].ny += ny / a;
    PointList[piA[i2]].nz += nz / a;
  }
}

static void
normNorm(void)
{
  float nx, ny, nz, a;
  int i;

  for (i = 0; i < PointNo; i++) {
    nx = PointList[i].nx;
    ny = PointList[i].ny;
    nz = PointList[i].nz;

    a = sqrtf(nx * nx + ny * ny + nz * nz);
    if (a == 0.0f)
      continue;

    PointList[i].nx /= a;
    PointList[i].ny /= a;
    PointList[i].nz /= a;
  }
}

static int
newStrip(void)
{
  if (StripNo == StripListSize) {
    StripListSize *= 2;
    StripList = realloc(StripList, StripListSize * sizeof(*StripList));
  }

  StripList[StripNo].size = 6;
  StripList[StripNo].piA = malloc(
      StripList[StripNo].size * sizeof(*StripList[StripNo].piA));
  StripList[StripNo].len = 0;

  StripNo++;

  return StripNo - 1;
}

static BOOL
connectPol(Strip *stripP, int *piA, int n)
{
  int siA[4], si0, si1, i, k;

  if (stripP->polNo > 1) {
    if (stripP->len % 2 == 0) {
      si0 = stripP->piA[stripP->len - 2];
      si1 = stripP->piA[stripP->len - 1];
    } else {
      si0 = stripP->piA[stripP->len - 1];
      si1 = stripP->piA[stripP->len - 2];
    }

    for (i = 0; i < n; i++)
      if (piA[i] == si0 && piA[(i + 1) % n] == si1)
	break;

    if (i == n)
      return FALSE;

    if (n == 3) {
      stripP->piA[stripP->len++] = piA[(i + 2) % n];
    } else if (stripP->len % 2 == 0) {
      stripP->piA[stripP->len++] = piA[(i + 3) % n];
      stripP->piA[stripP->len++] = piA[(i + 2) % n];
    } else {
      stripP->piA[stripP->len++] = piA[(i + 2) % n];
      stripP->piA[stripP->len++] = piA[(i + 3) % n];
    }

    return TRUE;
  }

  /* strip just consists of a single polygon that can be
     reoriented to fit */

  siA[0] = stripP->piA[0];
  siA[1] = stripP->piA[1];

  if (stripP->len == 3) {
    siA[2] = stripP->piA[2];
  } else {
    siA[2] = stripP->piA[3];
    siA[3] = stripP->piA[2];
  }

  for (i = 0; i < n; i++) {
    for (k = 0; k < stripP->len; k++)
      if (piA[i] == siA[(k + 1) % stripP->len] && piA[(i + 1) % n] == siA[k])
	break;
    
    if (k < stripP->len)
      break;
  }

  if (i == n)
    return FALSE;
    
  if (stripP->len == 3) {
    stripP->piA[0] = siA[(k + 2) % stripP->len];
    stripP->piA[1] = siA[k];
    stripP->piA[2] = siA[(k + 1) % stripP->len];
    stripP->piA[3] = piA[(i + 2) % n];

    if (n == 3) {
      stripP->len = 4;
    } else {
      stripP->piA[4] = piA[(i + 3) % n];
      stripP->len = 5;
    }
  } else {
    stripP->piA[0] = siA[(k + 2) % stripP->len];
    stripP->piA[1] = siA[(k + 3) % stripP->len];
    stripP->piA[2] = siA[(k + 1) % stripP->len];
    stripP->piA[3] = siA[k];

    if (n == 3) {
      stripP->piA[4] = piA[(i + 2) % n];
      stripP->len = 5;
    } else {
      stripP->piA[4] = piA[(i + 3) % n];
      stripP->piA[5] = piA[(i + 2) % n];
      stripP->len = 6;
    }
  }

  return TRUE;
}

static int
addPol(int *stripIA, int stripNo, int *piA, int pointNo)
{
  int si, i;

  if (CalcNorm)
    addNorm(piA, pointNo);

  for (i = 0; i < stripNo; i++) {
    si = stripIA[i];
    if (connectPol(StripList + si, piA, pointNo)) {
      StripList[si].polNo++;
      if (StripList[si].len + 2 > StripList[si].size) {
	StripList[si].size *= 2;
	StripList[si].piA = realloc(StripList[si].piA,
	    StripList[si].size * sizeof(*StripList[si].piA));
      }
      return si;
    }
  }

  si = newStrip();

  StripList[si].len = pointNo;

  StripList[si].piA[0] = piA[0];
  StripList[si].piA[1] = piA[1];

  if (pointNo == 3) {
    StripList[si].piA[2] = piA[2];
  } else {
    StripList[si].piA[2] = piA[3];
    StripList[si].piA[3] = piA[2];
  }

  StripList[si].polNo = 1;

  return si;
}

static int
addTri(int *stripIA, int stripNo, int pi0, int pi1, int pi2)
{
  int piA[3];

  piA[0] = pi0;
  piA[1] = pi1;
  piA[2] = pi2;

  return addPol(stripIA, stripNo, piA, 3);
}

static int
addQuad(int *stripIA, int stripNo, int pi0, int pi1, int pi2, int pi3)
{
  int piA[4];

  piA[0] = pi0;
  piA[1] = pi1;
  piA[2] = pi2;
  piA[3] = pi3;

  return addPol(stripIA, stripNo, piA, 4);
}

static void
initQueue(void)
{
  int i;

  QueueSize = 10;
  Queue = malloc(QueueSize * sizeof(*Queue));

  for (i = 0; i < QueueSize - 1; i++)
    Queue[i].nextI = i + 1;
  Queue[QueueSize - 1].nextI = -1;

  QueueFirst = -1;
  QueueLast = -1;
  QueueFree = 0;
}

static int
putQueue(QueueEntry **entryPP, int pat)
{
  QueueEntry *entryP;
  int oldSize, entryI, i;

  if (QueueFree < 0) {
    oldSize = QueueSize;
    QueueSize *= 2;
    Queue = realloc(Queue, QueueSize * sizeof(*Queue));

    for (i = oldSize; i < QueueSize - 1; i++)
      Queue[i].nextI = i + 1;
    Queue[QueueSize - 1].nextI = -1;
    QueueFree = oldSize;
  }

  entryI = QueueFree;
  entryP = Queue + entryI;
  QueueFree = entryP->nextI;

  if (QueueFirst < 0)
    QueueFirst = entryI;
  else
    Queue[QueueLast].nextI = entryI;

  QueueLast = entryI;
  entryP->nextI = -1;

  entryP->pat = pat;
  for (i = 0; i < EDGE_NO; i++)
    entryP->piA[i] = -1;
 
  entryP->stripNo = 0;

  *entryPP = entryP;

  return entryI;
}

static void
getQueue(QueueEntry **entryPP)
{
  if (QueueFirst < 0)
    *entryPP = NULL;
  else
    *entryPP = Queue + QueueFirst;
}

static void
takeQueue(QueueEntry **entryPP)
{
  QueueEntry *entryP;
  int entryI;

  if (QueueFirst < 0) {
    *entryPP = NULL;
    return;
  }

  entryI = QueueFirst;
  entryP = Queue + entryI;

  QueueFirst = entryP->nextI;
  entryP->nextI = QueueFree;
  QueueFree = entryI;

  *entryPP = entryP;
}

static float
getVal(int pi, float *valP, int xys, int xs)
{
  switch (pi) {
    case 0:
      return valP[- xys - xs - 1];
    case 1:
      return valP[- xys - xs];
    case 2:
      return valP[- xys - 1];
    case 3:
      return valP[- xys];
    case 4:
      return valP[- xs - 1];
    case 5:
      return valP[- xs];
    case 6:
      return valP[- 1];
    default:
      return valP[0];
  }
}

static int
calcPoint(int pi0, int pi1, int xi, int yi, int zi,
    float *valP, int xys, int xs, float level)
{
  float v0, v1, t;
  Point *p;
  int pi;

  if (pi0 > pi1) {
    pi = pi0;
    pi0 = pi1;
    pi1 = pi;
  }

  v0 = getVal(pi0, valP, xys, xs);
  v1 = getVal(pi1, valP, xys, xs);
  t = (level - v0) / (v1 - v0);

  pi = addPoint(&p);

  switch(8 * pi0 + pi1) {
    case 1:
      p->x = xi + t;
      p->y = (float) yi;
      p->z = (float) zi;
      break;
    case 2:
      p->x = (float) xi;
      p->y = yi + t;
      p->z = (float) zi;
      break;
    case 3:
      p->x = xi + t;
      p->y = yi + t;
      p->z = (float) zi;
      break;
    case 4:
      p->x = (float) xi;
      p->y = (float) yi;
      p->z = zi + t;
      break;
    case 5:
      p->x = xi + t;
      p->y = (float) yi;
      p->z = zi + t;
      break;
    case 6:
      p->x = (float) xi;
      p->y = yi + t;
      p->z = zi + t;
      break;
    case 10:
      p->x = (xi + 1) - t;
      p->y = yi + t;
      p->z = (float) zi;
      break;
    case 11:
      p->x = (float) (xi + 1);
      p->y = yi + t;
      p->z = (float) zi;
      break;
    case 12:
      p->x = (xi + 1) - t;
      p->y = (float) yi;
      p->z = zi + t;
      break;
    case 13:
      p->x = (float) (xi + 1);
      p->y = (float) yi;
      p->z = zi + t;
      break;
    case 15:
      p->x = (float) (xi + 1);
      p->y = yi + t;
      p->z = zi + t;
      break;
    case 19:
      p->x = xi + t;
      p->y = (float) (yi + 1);
      p->z = (float) zi;
      break;
    case 20:
      p->x = (float) xi;
      p->y = (yi + 1) - t;
      p->z = zi + t;
      break;
    case 22:
      p->x = (float) xi;
      p->y = (float) (yi + 1);
      p->z = zi + t;
      break;
    case 23:
      p->x = xi + t;
      p->y = (float) (yi + 1);
      p->z = zi + t;
      break;
    case 29:
      p->x = (float) (xi + 1);
      p->y = (yi + 1) - t;
      p->z = zi + t;
      break;
    case 30:
      p->x = (xi + 1) - t;
      p->y = (float) (yi + 1);
      p->z = zi + t;
      break;
    case 31:
      p->x = (float) (xi + 1);
      p->y = (float) (yi + 1);
      p->z = zi + t;
      break;
    case 37:
      p->x = xi + t;
      p->y = (float) yi;
      p->z = (float) (zi + 1);
      break;
    case 38:
      p->x = (float) xi;
      p->y = yi + t;
      p->z = (float) (zi + 1);
      break;
    case 39:
      p->x = xi + t;
      p->y = yi + t;
      p->z = (float) (zi + 1);
      break;
    case 46:
      p->x = (xi + 1) - t;
      p->y = yi + t;
      p->z = (float) (zi + 1);
      break;
    case 47:
      p->x = (float) (xi + 1);
      p->y = yi + t;
      p->z = (float) (zi + 1);
      break;
    case 55:
      p->x = xi + t;
      p->y = (float) (yi + 1);
      p->z = (float) (zi + 1);
      break;
  }

  p->x = XOffs + XScale * p->x;
  p->y = YOffs + YScale * p->y;
  p->z = ZOffs + ZScale * p->z;

  return pi;
}

static void
addNeigh(FaceDesc *fDescP, int qi,
    int xys, int xs, int cxys, int cxs, int si)
{
  Cube *cP;
  Tet *tP;
  QueueEntry *qP, *qNeighP;

  qP = Queue + qi;

  switch (fDescP->dir) {
    case D_0:
      cP = qP->cP;
      break;
    case D_MX:
      cP = qP->cP - 1;
      break;
    case D_PX:
      cP = qP->cP + 1;
      break;
    case D_MY:
      cP = qP->cP - cxs;
      break;
    case D_PY:
      cP = qP->cP + cxs;
      break;
    case D_MZ:
      cP = qP->cP - cxys;
      break;
    case D_PZ:
      cP = qP->cP + cxys;
      break;
  }

  tP = cP->tetA + fDescP->ti;

  if (tP->state == STATE_DONE)
    return;

  if (tP->state > 0) {
    tP->state = - putQueue(&qNeighP, tP->state);

    /* Queue can be realloced in putQueue! */
    qP = Queue + qi;

    switch (fDescP->dir) {
      case D_0:
	qNeighP->xi = qP->xi;
	qNeighP->yi = qP->yi;
	qNeighP->zi = qP->zi;
	qNeighP->vi = qP->vi;
	break;
      case D_MX:
	qNeighP->xi = qP->xi - 1;
	qNeighP->yi = qP->yi;
	qNeighP->zi = qP->zi;
	qNeighP->vi = qP->vi - 1;
	break;
      case D_PX:
	qNeighP->xi = qP->xi + 1;
	qNeighP->yi = qP->yi;
	qNeighP->zi = qP->zi;
	qNeighP->vi = qP->vi + 1;
	break;
      case D_MY:
	qNeighP->xi = qP->xi;
	qNeighP->yi = qP->yi - 1;
	qNeighP->zi = qP->zi;
	qNeighP->vi = qP->vi - xs;
	break;
      case D_PY:
	qNeighP->xi = qP->xi;
	qNeighP->yi = qP->yi + 1;
	qNeighP->zi = qP->zi;
	qNeighP->vi = qP->vi + xs;
	break;
      case D_MZ:
	qNeighP->xi = qP->xi;
	qNeighP->yi = qP->yi;
	qNeighP->zi = qP->zi - 1;
	qNeighP->vi = qP->vi - xys;
	break;
      case D_PZ:
	qNeighP->xi = qP->xi;
	qNeighP->yi = qP->yi;
	qNeighP->zi = qP->zi + 1;
	qNeighP->vi = qP->vi + xys;
	break;
    }

    qNeighP->cP = cP;
    qNeighP->tP = tP;
    qNeighP->ti = fDescP->ti;

    if (fDescP->dir == D_0)
      qNeighP->odd = qP->odd;
    else
      qNeighP->odd = ! qP->odd;
  } else {
    qNeighP = Queue - tP->state; 
  }

  qNeighP->piA[fDescP->corrA[0].otherI] = qP->piA[fDescP->corrA[0].selfI];
  qNeighP->piA[fDescP->corrA[1].otherI] = qP->piA[fDescP->corrA[1].selfI];
  qNeighP->piA[fDescP->corrA[2].otherI] = qP->piA[fDescP->corrA[2].selfI];

  qNeighP->stripIA[qNeighP->stripNo++] = si;
}

static void
fillSurf(IsoSurface *surfP)
{
  int i;

  surfP->pointNo = PointNo;
  surfP->xA = malloc(PointNo * sizeof(*surfP->xA));
  surfP->nvA = malloc(PointNo * sizeof(*surfP->nvA));

  for (i = 0; i < PointNo; i++) {
    surfP->xA[i][0] = PointList[i].x;
    surfP->xA[i][1] = PointList[i].y;
    surfP->xA[i][2] = PointList[i].z;

    surfP->nvA[i][0] = PointList[i].nx;
    surfP->nvA[i][1] = PointList[i].ny;
    surfP->nvA[i][2] = PointList[i].nz;
  }

  surfP->meshNo = StripNo;
  surfP->meshA = malloc(StripNo * sizeof(*surfP->meshA));

  for (i = 0; i < StripNo; i++) {
    surfP->meshA[i].indA = StripList[i].piA;
    surfP->meshA[i].pointNo = StripList[i].len;
  }
}

void
IsoSurfaceCalc(float *valA, int xs, int ys, int zs,
    float xMin, float yMin, float zMin,
    float xRange, float yRange, float zRange,
    float level, BOOL calcNorm,
    IsoSurface **surfAP, int *surfNoP)
{
  IsoSurface *surfA;
  int surfNo;
  int cxs, cys, czs;
  int xys, cxys;
  int cubeNo;
  Cube *cubeA;
  CubeDesc *cDescP;
  TetDesc *tDescP;
  int cPat, tPat;
  BOOL odd;
  Cube *cP;
  Tet *tP;
  QueueEntry *qP;
  int qi, tStartI;
  int xi, yi, zi, ci, ti, si;

  CalcNorm = calcNorm;

  surfA = NULL;
  surfNo = 0;

  initStruc();

  cxs = xs + 1;
  cys = ys + 1;
  czs = zs + 1;

  xys = xs * ys;
  cxys = cxs * cys;

  cubeNo = cxys * czs;
  cubeA = malloc(cubeNo * sizeof(*cubeA));
  /* this can be huge, check return value */
  if (cubeA == NULL)
    return;

  for (ci = 0; ci < cubeNo; ci++)
    cubeA[ci].pat = 0;

  for (zi = 0; zi < zs; zi++)
    for (yi = 0; yi < ys; yi++)
      for (xi = 0; xi < xs; xi++) {
	ci = zi * cxys + yi * cxs + xi;

	if (valA[zi * xys + yi * xs + xi] > level) {
	  cubeA[ci + cxys + cxs + 1].pat |= 1;
	  cubeA[ci + cxys + cxs    ].pat |= 2;
	  cubeA[ci + cxys +       1].pat |= 4;
	  cubeA[ci + cxys          ].pat |= 8;
	  cubeA[ci +        cxs + 1].pat |= 16;
	  cubeA[ci +        cxs    ].pat |= 32;
	  cubeA[ci +              1].pat |= 64;
	  cubeA[ci                 ].pat |= 128;
	}
      }
  
  for (yi = 0; yi < cys; yi++)
    for (xi = 0; xi < cxs; xi++) {
      cubeA[yi * cxs + xi].pat = STATE_DONE;
      cubeA[(czs - 1) * cxys + yi * cxs + xi].pat = STATE_DONE;
    }

  for (zi = 0; zi < czs; zi++)
    for (xi = 0; xi < cxs; xi++) {
      cubeA[zi * cxys + xi].pat = STATE_DONE;
      cubeA[zi * cxys + (cys - 1) * cxs + xi].pat = STATE_DONE;
    }

  for (zi = 0; zi < czs; zi++)
    for (yi = 0; yi < cys; yi++) {
      cubeA[zi * cxys + yi * cxs].pat = STATE_DONE;
      cubeA[zi * cxys + yi * cxs + cxs - 1].pat = STATE_DONE;
    }

  for (zi = 0; zi < czs; zi++)
    for (yi = 0; yi < cys; yi++) {
      odd = ((yi + zi) % 2 == 1);

      for (xi = 0; xi < cxs; xi++) {
	ci = zi * cxys + yi * cxs + xi;

	cPat = cubeA[ci].pat;

	if (cPat == 0 || cPat == 255 || cPat == STATE_DONE) {
	  for (ti = 0; ti < TET_NO; ti++)
	    cubeA[ci].tetA[ti].state = STATE_DONE;
	  
	  cubeA[ci].pat = STATE_DONE;
	  odd = ! odd;

	  continue;
	}

	if (odd)
	  cDescP = &CubeOdd;
	else
	  cDescP = &CubeEven;

	for (ti = 0; ti < TET_NO; ti++) {
	  tDescP = cDescP->tetA + ti;
	  tPat = 0;

	  if ((cPat & (1 << tDescP->pi0)) != 0)
	    tPat |= 1;
	  if ((cPat & (1 << tDescP->pi1)) != 0)
	    tPat |= 2;
	  if ((cPat & (1 << tDescP->pi2)) != 0)
	    tPat |= 4;
	  if ((cPat & (1 << tDescP->pi3)) != 0)
	    tPat |= 8;

	  if (tPat == 0 || tPat == 15)
	    cubeA[ci].tetA[ti].state = STATE_DONE;
	  else
	    cubeA[ci].tetA[ti].state = tPat;
	}

	odd = ! odd;
      }
    }

  XScale = xRange / (xs - 1);
  YScale = yRange / (ys - 1);
  ZScale = zRange / (zs - 1);

  XOffs = xMin - XScale;
  YOffs = yMin - YScale;
  ZOffs = zMin - ZScale;

  PointListSize = 10;
  PointList = malloc(PointListSize * sizeof(*PointList));
  StripListSize = 10;
  StripList = malloc(StripListSize * sizeof(*StripList));

  initQueue();

  for (zi = 1; zi < czs - 1; zi++)
    for (yi = 1; yi < cys - 1; yi++) {
      odd = ((yi + zi) % 2 == 0);

      for (xi = 1; xi < cxs - 1; xi++) {
	cP = cubeA + zi * cxys + yi * cxs + xi;

	if (cP->pat == STATE_DONE) {
	  odd = ! odd;
	  continue;
	}

	for (tStartI = 0; tStartI < TET_NO; tStartI++) {
	  tP = cP->tetA + tStartI;
	  if (tP->state == STATE_DONE)
	    continue;

	  /* start new surface */
	  PointNo = 0;
	  StripNo = 0;

	  tP->state = - putQueue(&qP, tP->state);
	  qP->xi = xi;
	  qP->yi = yi;
	  qP->zi = zi;
	  qP->vi = zi * xys + yi * xs + xi;
	  qP->cP = cP;
	  qP->tP = tP;
	  qP->ti = tStartI;
	  qP->odd = odd;

	  for (;;) {
	    getQueue(&qP);
	    if (qP == NULL)
	      break;

	    if (qP->odd)
	      cDescP = &CubeOdd;
	    else
	      cDescP = &CubeEven;

	    tDescP = cDescP->tetA + qP->ti;

	    /* use index for addNeigh because Queue can be realloced */
	    qi = qP - Queue;

	    switch (qP->pat) {
	      case 1:
		if (qP->piA[2] < 0)
		  qP->piA[2] = calcPoint(tDescP->pi0, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[0] < 0)
		  qP->piA[0] = calcPoint(tDescP->pi0, tDescP->pi1,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[3] < 0)
		  qP->piA[3] = calcPoint(tDescP->pi0, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addTri(qP->stripIA, qP->stripNo,
		    qP->piA[2], qP->piA[0], qP->piA[3]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		break;
	      case 2:
		if (qP->piA[0] < 0)
		  qP->piA[0] = calcPoint(tDescP->pi0, tDescP->pi1,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[1] < 0)
		  qP->piA[1] = calcPoint(tDescP->pi1, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[4] < 0)
		  qP->piA[4] = calcPoint(tDescP->pi1, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addTri(qP->stripIA, qP->stripNo,
		    qP->piA[0], qP->piA[1], qP->piA[4]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		break;
	      case 3:
		if (qP->piA[2] < 0)
		  qP->piA[2] = calcPoint(tDescP->pi0, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[1] < 0)
		  qP->piA[1] = calcPoint(tDescP->pi1, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[4] < 0)
		  qP->piA[4] = calcPoint(tDescP->pi1, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[3] < 0)
		  qP->piA[3] = calcPoint(tDescP->pi0, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addQuad(qP->stripIA, qP->stripNo,
		    qP->piA[2], qP->piA[1], qP->piA[4], qP->piA[3]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		break;
	      case 4:
		if (qP->piA[1] < 0)
		  qP->piA[1] = calcPoint(tDescP->pi1, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[2] < 0)
		  qP->piA[2] = calcPoint(tDescP->pi0, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[5] < 0)
		  qP->piA[5] = calcPoint(tDescP->pi2, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addTri(qP->stripIA, qP->stripNo,
		    qP->piA[1], qP->piA[2], qP->piA[5]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		break;
	      case 5:
		if (qP->piA[1] < 0)
		  qP->piA[1] = calcPoint(tDescP->pi1, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[0] < 0)
		  qP->piA[0] = calcPoint(tDescP->pi0, tDescP->pi1,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[3] < 0)
		  qP->piA[3] = calcPoint(tDescP->pi0, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[5] < 0)
		  qP->piA[5] = calcPoint(tDescP->pi2, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addQuad(qP->stripIA, qP->stripNo,
		    qP->piA[1], qP->piA[0], qP->piA[3], qP->piA[5]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		break;
	      case 6:
		if (qP->piA[0] < 0)
		  qP->piA[0] = calcPoint(tDescP->pi0, tDescP->pi1,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[2] < 0)
		  qP->piA[2] = calcPoint(tDescP->pi0, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[5] < 0)
		  qP->piA[5] = calcPoint(tDescP->pi2, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[4] < 0)
		  qP->piA[4] = calcPoint(tDescP->pi1, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addQuad(qP->stripIA, qP->stripNo,
		    qP->piA[0], qP->piA[2], qP->piA[5], qP->piA[4]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		break;
	      case 7:
		if (qP->piA[4] < 0)
		  qP->piA[4] = calcPoint(tDescP->pi1, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[3] < 0)
		  qP->piA[3] = calcPoint(tDescP->pi0, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[5] < 0)
		  qP->piA[5] = calcPoint(tDescP->pi2, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addTri(qP->stripIA, qP->stripNo,
		    qP->piA[4], qP->piA[3], qP->piA[5]);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		break;
	      case 8:
		if (qP->piA[3] < 0)
		  qP->piA[3] = calcPoint(tDescP->pi0, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[4] < 0)
		  qP->piA[4] = calcPoint(tDescP->pi1, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[5] < 0)
		  qP->piA[5] = calcPoint(tDescP->pi2, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addTri(qP->stripIA, qP->stripNo,
		    qP->piA[3], qP->piA[4], qP->piA[5]);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		break;
	      case 9:
		if (qP->piA[2] < 0)
		  qP->piA[2] = calcPoint(tDescP->pi0, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[0] < 0)
		  qP->piA[0] = calcPoint(tDescP->pi0, tDescP->pi1,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[4] < 0)
		  qP->piA[4] = calcPoint(tDescP->pi1, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[5] < 0)
		  qP->piA[5] = calcPoint(tDescP->pi2, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addQuad(qP->stripIA, qP->stripNo,
		    qP->piA[2], qP->piA[0], qP->piA[4], qP->piA[5]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		break;
	      case 10:
		if (qP->piA[0] < 0)
		  qP->piA[0] = calcPoint(tDescP->pi0, tDescP->pi1,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[1] < 0)
		  qP->piA[1] = calcPoint(tDescP->pi1, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[5] < 0)
		  qP->piA[5] = calcPoint(tDescP->pi2, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[3] < 0)
		  qP->piA[3] = calcPoint(tDescP->pi0, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addQuad(qP->stripIA, qP->stripNo,
		    qP->piA[0], qP->piA[1], qP->piA[5], qP->piA[3]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		break;
	      case 11:
		if (qP->piA[2] < 0)
		  qP->piA[2] = calcPoint(tDescP->pi0, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[1] < 0)
		  qP->piA[1] = calcPoint(tDescP->pi1, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[5] < 0)
		  qP->piA[5] = calcPoint(tDescP->pi2, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addTri(qP->stripIA, qP->stripNo,
		    qP->piA[2], qP->piA[1], qP->piA[5]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		break;
	      case 12:
		if (qP->piA[1] < 0)
		  qP->piA[1] = calcPoint(tDescP->pi1, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[2] < 0)
		  qP->piA[2] = calcPoint(tDescP->pi0, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[3] < 0)
		  qP->piA[3] = calcPoint(tDescP->pi0, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[4] < 0)
		  qP->piA[4] = calcPoint(tDescP->pi1, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addQuad(qP->stripIA, qP->stripNo,
		    qP->piA[1], qP->piA[2], qP->piA[3], qP->piA[4]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		break;
	      case 13:
		if (qP->piA[1] < 0)
		  qP->piA[1] = calcPoint(tDescP->pi1, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[0] < 0)
		  qP->piA[0] = calcPoint(tDescP->pi0, tDescP->pi1,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[4] < 0)
		  qP->piA[4] = calcPoint(tDescP->pi1, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addTri(qP->stripIA, qP->stripNo,
		    qP->piA[1], qP->piA[0], qP->piA[4]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 2, qi, xys, xs, cxys, cxs, si);
		break;
	      case 14:
		if (qP->piA[0] < 0)
		  qP->piA[0] = calcPoint(tDescP->pi0, tDescP->pi1,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[2] < 0)
		  qP->piA[2] = calcPoint(tDescP->pi0, tDescP->pi2,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		if (qP->piA[3] < 0)
		  qP->piA[3] = calcPoint(tDescP->pi0, tDescP->pi3,
		      qP->xi, qP->yi, qP->zi, valA + qP->vi, xys, xs, level);
		si = addTri(qP->stripIA, qP->stripNo,
		    qP->piA[0], qP->piA[2], qP->piA[3]);
		addNeigh(tDescP->faceA + 0, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 3, qi, xys, xs, cxys, cxs, si);
		addNeigh(tDescP->faceA + 1, qi, xys, xs, cxys, cxs, si);
		break;
	    }

	    qP = Queue + qi;

	    qP->tP->state = STATE_DONE;
	    takeQueue(&qP);
	  }

	  if (CalcNorm)
	    normNorm();

	  if (surfNo == 0)
	    surfA = malloc(sizeof(*surfA));
	  else
	    surfA = realloc(surfA, (surfNo + 1) * sizeof(*surfA));

	  fillSurf(surfA + surfNo);
	  surfNo++;
	}

	odd = ! odd;
      }
    }

  free(Queue);
  free(cubeA);

  free(PointList);
  free(StripList);

  *surfAP = surfA;
  *surfNoP = surfNo;
}
