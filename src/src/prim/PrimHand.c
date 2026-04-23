/*
************************************************************************
*
*   PrimHand.c - manage data for primitives
*
*   Copyright (c) 1994-99
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
*   Date of last modification : 01/07/28
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.PrimHand.c
*   SCCS identification       : 1.29
*
************************************************************************
*/

#include <prim_hand.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linlist.h>
#include <prop_tab.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <data_hand.h>
#include "prim_struc.h"
#include "plate_calc.h"
#include "solid_calc.h"
#include "ribbon_build.h"

#define MIN_LEN 0.5f

typedef struct {
  DhAtomP *atomA;
  int atomNo;
} AtomList;

typedef struct {
  PrimType type;
  PropRefP refP;
  PrimPropCB propCB;
  void *clientData;
} PropCBData;

static LINLIST RibbonList = NULL;
static LINLIST PrimList = NULL;
static LINLIST PropCBList = NULL;

static void
ribbonMolChangedCB(DhMolP molP, void *clientData)
{
  Ribbon *ribbonP = clientData;

  ribbonP->splineValid = FALSE;
}

static void
ribbonAtomInvalidCB(DhAtomP firstAtomP, DhAtomP lastAtomP, void *clientData)
{
  Ribbon *ribbonP = clientData;
  DhAtomP atomP;
  PrimObjP primP, nextPrimP;
  int i;

  for (i = 0; i < ribbonP->atomNo; i++) {
    atomP = ribbonP->atomA[i];
    if (atomP >= firstAtomP && atomP <= lastAtomP) {
      primP = ListFirst(PrimList);
      while (primP != NULL) {
	nextPrimP = ListNext(PrimList, primP);
	if (primP->type == PT_RIBBON && primP->u.ribbon.ribbonP == ribbonP)
	  PrimDestroy(primP);
	primP = nextPrimP;
      }
      return;
    }
  }
}

void
RibbonAddInvalidCB(Ribbon *ribbonP)
{
  DhMolP molP;

  if (ribbonP->atomNo > 0) {
    molP = DhResGetMol(DhAtomGetRes(ribbonP->atomA[0]));
    DhAddMolChangedCB(molP, ribbonMolChangedCB, ribbonP);
  }

  DhAddAtomInvalidCB(ribbonAtomInvalidCB, ribbonP);
}

static void
primMolChangedCB(DhMolP molP, void *clientData)
{
  PrimObjP primP = clientData;

  if (primP->type == PT_PLATE)
    primP->u.plate.pointValid = FALSE;
  else if (primP->type == PT_SOLID)
    primP->u.solid.geomValid = FALSE;
  else if (primP->type == PT_SHEET)
    primP->u.sheet.pointValid = FALSE;
}

static void
primMolInvalidCB(DhMolP molP, void *clientData)
{
  PrimObjP primP = clientData;

  if (primP->molP == molP)
    PrimDestroy(primP);
}

static void
atomsInvalid(DhAtomP firstAtomP, DhAtomP lastAtomP,
    PrimObjP primP, DhAtomP *atomA, int atomNo)
{
  int i;

  for (i = 0; i < atomNo; i++) {
    if (atomA[i] >= firstAtomP && atomA[i] <= lastAtomP) {
      PrimDestroy(primP);
      return;
    }
  }
}

static void
trajecAtomInvalidCB(DhAtomP firstAtomP, DhAtomP lastAtomP, void *clientData)
{
  PrimObjP primP = clientData;

  atomsInvalid(firstAtomP, lastAtomP,
      primP, primP->u.trajec.atomA, primP->u.trajec.atomNo);
}

static void
plateAtomInvalidCB(DhAtomP firstAtomP, DhAtomP lastAtomP, void *clientData)
{
  PrimObjP primP = clientData;

  atomsInvalid(firstAtomP, lastAtomP,
      primP, primP->u.plate.atomA, primP->u.plate.atomNo);
}

static void
solidAtomInvalidCB(DhAtomP firstAtomP, DhAtomP lastAtomP, void *clientData)
{
  PrimObjP primP = clientData;

  atomsInvalid(firstAtomP, lastAtomP,
      primP, primP->u.solid.atomA, primP->u.solid.atomNo);
}

static void
sheetAtomInvalidCB(DhAtomP firstAtomP, DhAtomP lastAtomP, void *clientData)
{
  PrimObjP primP = clientData;

  atomsInvalid(firstAtomP, lastAtomP,
      primP, primP->u.sheet.atomA, primP->u.sheet.atomNo);
}

static void
surfaceAtomInvalidCB(DhAtomP firstAtomP, DhAtomP lastAtomP, void *clientData)
{
  PrimObjP primP = clientData;
  DhAtomP atomP;
  int i;

  for (i = 0; i < primP->u.dotSurface.size; i++) {
    atomP = primP->u.dotSurface.p[i].atomP;
    if (atomP >= firstAtomP && atomP <= lastAtomP) {
      PrimDestroy(primP);
      return;
    }
  }
}

void
PrimAddInvalidCB(PrimObjP primP)
{
  if (primP->molP != NULL) {
    DhAddMolChangedCB(primP->molP, primMolChangedCB, primP);
    DhAddMolInvalidCB(primMolInvalidCB, primP);
  }

  if (primP->type == PT_TRAJEC)
    DhAddAtomInvalidCB(trajecAtomInvalidCB, primP);
  else if (primP->type == PT_PLATE)
    DhAddAtomInvalidCB(plateAtomInvalidCB, primP);
  else if (primP->type == PT_SOLID)
    DhAddAtomInvalidCB(solidAtomInvalidCB, primP);
  else if (primP->type == PT_SHEET)
    DhAddAtomInvalidCB(sheetAtomInvalidCB, primP);
  else if (primP->type == PT_DOT_SURFACE)
    DhAddAtomInvalidCB(surfaceAtomInvalidCB, primP);
}

void
PrimDestroyAll(void)
{
  ListClose(PrimList);
  PrimList = NULL;
}

static void
freeRibbon(void *p, void *clientData)
{
  Ribbon *ribbonP = p;

  if (ribbonP->atomA != NULL)
    free(ribbonP->atomA);

  if (ribbonP->radA != NULL)
    free(ribbonP->radA);

  if (ribbonP->coeffA != NULL)
    free(ribbonP->coeffA);
}

static void
removeRibbon(Ribbon *ribbonP)
{
  DhMolP molP;
  int num;

  if (ribbonP->atomNo > 0) {
    molP = DhResGetMol(DhAtomGetRes(ribbonP->atomA[0]));
    DhRemoveMolChangedCB(molP, ribbonMolChangedCB, ribbonP);
  }

  DhRemoveAtomInvalidCB(ribbonAtomInvalidCB, ribbonP);
  ListRemove(RibbonList, ribbonP);

  ribbonP = ListFirst(RibbonList);
  num = 0;
  while (ribbonP != NULL) {
    ribbonP->num = num;
    ribbonP = ListNext(RibbonList, ribbonP);
    num++;
  }
}

static void
freePrim(void *p, void *clientData)
{
  PrimObjP primP = p;
  Ribbon *ribbonP;
  int i;

  if (primP->molP != NULL) {
    DhRemoveMolChangedCB(primP->molP, primMolChangedCB, primP);
    DhRemoveMolInvalidCB(primMolInvalidCB, primP);
  }

  AttrReturn(primP->attrP);
  PropFreeTab(primP->propTab);

  switch (primP->type) {
    case PT_CIRCLE:
      break;
    case PT_MAP:
      for (i = 0; i < primP->u.map.polylineNo; i++)
        free(primP->u.map.polylineA[i].xA);
      if (primP->u.map.polylineA != NULL)
        free(primP->u.map.polylineA);
      break;
    case PT_TRAJEC:
      DhRemoveAtomInvalidCB(trajecAtomInvalidCB, primP);
      free(primP->u.trajec.atomA);
      if (primP->u.trajec.xA != NULL)
	free(primP->u.trajec.xA);
      break;
    case PT_PLATE:
      DhRemoveAtomInvalidCB(plateAtomInvalidCB, primP);
      free(primP->u.plate.atomA);
      if (primP->u.plate.triA != NULL)
	free(primP->u.plate.triA);
      if (primP->u.plate.quadA != NULL)
	free(primP->u.plate.quadA);
      break;
    case PT_CYLINDER:
      break;
    case PT_SOLID:
      DhRemoveAtomInvalidCB(solidAtomInvalidCB, primP);
      free(primP->u.solid.atomA);
      for (i = 0; i < primP->u.solid.meshNo; i++) {
	free(primP->u.solid.meshA[i].xA);
	free(primP->u.solid.meshA[i].nvA);
      }
      if (primP->u.solid.meshA != NULL)
	free(primP->u.solid.meshA);
      for (i = 0; i < primP->u.solid.polyNo; i++)
	free(primP->u.solid.polyA[i].xA);
      if (primP->u.solid.polyA != NULL)
	free(primP->u.solid.polyA);
      break;
    case PT_TEXT:
      if (primP->u.text.str != NULL)
	free(primP->u.text.str);
      break;
    case PT_DOT_SURFACE:
      DhRemoveAtomInvalidCB(surfaceAtomInvalidCB, primP);
      for (i = 0; i < primP->u.dotSurface.size; i++)
	free(primP->u.dotSurface.p[i].p);
      if (primP->u.dotSurface.p != NULL)
	free(primP->u.dotSurface.p);
      break;
    case PT_SURFACE:
      for (i = 0; i < primP->u.surface.meshNo; i++)
	free(primP->u.surface.meshA[i].indA);
      if (primP->u.surface.meshA != NULL)
	free(primP->u.surface.meshA);
      if (primP->u.surface.xA != NULL) {
	free(primP->u.surface.xA);
	free(primP->u.surface.nvA);
      }
      if (primP->u.surface.colA != NULL)
	free(primP->u.surface.colA);
      if (primP->u.surface.potA != NULL)
	free(primP->u.surface.potA);
      break;
    case PT_RIBBON:
      for (i = 0; i < primP->u.ribbon.traceNo; i++) {
	free(primP->u.ribbon.traceA[i].xA);
	if (primP->u.ribbon.traceA[i].nvA != NULL)
	  free(primP->u.ribbon.traceA[i].nvA);
      }
      if (primP->u.ribbon.traceA != NULL)
	free(primP->u.ribbon.traceA);
      for (i = 0; i < primP->u.ribbon.stripNo; i++)
	free(primP->u.ribbon.stripA[i].parA);
      if (primP->u.ribbon.stripA != NULL)
	free(primP->u.ribbon.stripA);
      for (i = 0; i < primP->u.ribbon.polyNo; i++)
	free(primP->u.ribbon.polyA[i].xA);
      if (primP->u.ribbon.polyA != NULL)
	free(primP->u.ribbon.polyA);

      ribbonP = primP->u.ribbon.ribbonP;
      ribbonP->primNo--;
      if (ribbonP->primNo == 0)
	removeRibbon(ribbonP);
      break;
    case PT_SHEET:
      DhRemoveAtomInvalidCB(sheetAtomInvalidCB, primP);
      free(primP->u.sheet.atomA);
      if (primP->u.sheet.xA1 != NULL) {
	free(primP->u.sheet.xA1);
	free(primP->u.sheet.xA2);
	free(primP->u.sheet.nvA1);
	free(primP->u.sheet.nvA2);
      }
      break;
    case PT_DRAWOBJ:
      if (primP->u.drawobj.xA != NULL) {
	free(primP->u.drawobj.xA);
	free(primP->u.drawobj.dxA);
      }
      break;
  }
}

static void
countAtom(DhAtomP atomP, void *clientData)
{
  AtomList *listP = clientData;

  listP->atomNo++;
}

static void
fillAtom(DhAtomP atomP, void *clientData)
{
  AtomList *listP = clientData;

  listP->atomA[listP->atomNo++] = atomP;
}

static PrimObjP
primNew(PrimType type, DhMolP molP, BOOL getAtoms)
{
  struct PrimObjS primS;
  PrimObjP primP;
  PropRefP refP;
  AtomList atomList;
  AttrP attrP;
  struct AttrS attr;

  if (PrimList == NULL) {
    PrimList = ListOpen(sizeof(struct PrimObjS));
    ListAddDestroyCB(PrimList, freePrim, NULL, NULL);
    primS.num = 0;
  } else if (ListSize(PrimList) == 0) {
    primS.num = 0;
  } else {
    primP = ListLast(PrimList);
    primS.num = primP->num + 1;
  }

  primS.type = type;
  primS.molP = molP;

  if (type == PT_MAP) {
    primS.u.map.style = PMS_LINE;
    primS.u.map.polylineA = NULL;
    primS.u.map.polylineNo = 0;
  } else if (type == PT_TRAJEC) {
    primS.u.trajec.style = PTS_LINE;
    primS.u.trajec.xA = NULL;
  } else if (type == PT_PLATE) {
    primS.u.plate.thick = 0.4f;
    primS.u.plate.pointValid = FALSE;
    primS.u.plate.triA = NULL;
    primS.u.plate.triNo = 0;
    primS.u.plate.quadA = NULL;
    primS.u.plate.quadNo = 0;
  } else if (type == PT_CYLINDER) {
    primS.u.cylinder.style = PCS_NORMAL;
    Vec3Zero(primS.u.cylinder.x1);
    Vec3Zero(primS.u.cylinder.x2);
  } else if (type == PT_SOLID) {
    if (getAtoms) {
      refP = PropGetRef(PROP_SELECTED, FALSE);
      atomList.atomNo = 0;
      DhMolApplyAtom(refP, molP, countAtom, &atomList);
      if (atomList.atomNo == 0)
	return NULL;
      atomList.atomA = malloc(atomList.atomNo * sizeof(DhAtomP));
      atomList.atomNo = 0;
      DhMolApplyAtom(refP, molP, fillAtom, &atomList);

      primS.u.solid.atomNo = atomList.atomNo;
      primS.u.solid.atomA = atomList.atomA;
    }

    primS.u.solid.kind = PSK_SPHERE;
    primS.u.solid.cover = 1.0f;
    primS.u.solid.geomValid = FALSE;
    primS.u.solid.pointValid = FALSE;
    primS.u.solid.meshA = NULL;
    primS.u.solid.meshNo = 0;
    primS.u.solid.polyA = NULL;
    primS.u.solid.polyNo = 0;
  } else if (type == PT_TEXT) {
    primS.u.text.str = NULL;
  } else if (type == PT_DOT_SURFACE) {
    primS.u.dotSurface.p = NULL;
    primS.u.dotSurface.size = 0;
  } else if (type == PT_SURFACE) {
    primS.u.surface.xA = NULL;
    primS.u.surface.nvA = NULL;
    primS.u.surface.colA = NULL;
    primS.u.surface.potA = NULL;
    primS.u.surface.pointNo = 0;
    primS.u.surface.meshA = NULL;
    primS.u.surface.meshNo = 0;
    primS.u.surface.maxSize = 0;
  } else if (type == PT_RIBBON) {
    primS.u.ribbon.pointValid = FALSE;
    primS.u.ribbon.traceA = NULL;
    primS.u.ribbon.traceNo = 0;
    primS.u.ribbon.stripA = NULL;
    primS.u.ribbon.stripNo = 0;
    primS.u.ribbon.polyA = NULL;
    primS.u.ribbon.polyNo = 0;
  } else if (type == PT_SHEET) {
    if (getAtoms) {
      refP = PropGetRef(PROP_SELECTED, FALSE);
      atomList.atomNo = 0;
      DhMolApplyAtom(refP, molP, countAtom, &atomList);
      if (atomList.atomNo == 0)
	return NULL;
      atomList.atomA = malloc(atomList.atomNo * sizeof(DhAtomP));
      atomList.atomNo = 0;
      DhMolApplyAtom(refP, molP, fillAtom, &atomList);

      primS.u.sheet.atomNo = atomList.atomNo;
      primS.u.sheet.atomA = atomList.atomA;
    }

    primS.u.sheet.pointValid = FALSE;
    primS.u.sheet.xA1 = NULL;
    primS.u.sheet.xA2 = NULL;
    primS.u.sheet.nvA1 = NULL;
    primS.u.sheet.nvA2 = NULL;
  } else if (type == PT_DRAWOBJ) {
    primS.u.drawobj.type = PDT_LINE;
    primS.u.drawobj.style = PDS_LINE;
    primS.u.drawobj.xA = NULL;
    primS.u.drawobj.dxA = NULL;
    primS.u.drawobj.pointNo = 0;
  }

  attrP = AttrGetInit();
  if (type == PT_PLATE || type == PT_CYLINDER ||
      type == PT_SOLID || type == PT_SURFACE ||
      type == PT_RIBBON || type == PT_SHEET) {
    AttrCopy(&attr, attrP);
    AttrReturn(attrP);
    attr.shadeModel = SHADE_DEFAULT;
    attrP = AttrGet(&attr);
  }
  primS.attrP = attrP;

  primS.propTab = PropNewTab(TRUE);

  primP = ListInsertLast(PrimList, &primS);
  PrimAddInvalidCB(primP);

  return primP;
}

PrimObjP
PrimNew(PrimType type, DhMolP molP)
{
  return primNew(type, molP, TRUE);
}

PrimObjP
PrimNewEmpty(PrimType type, DhMolP molP)
{
  return primNew(type, molP, FALSE);
}

PrimObjP
PrimNewTrajec(DhAtomP atomA[], int atomNo)
{
  PrimObjP primP;
  int i;

  primP = PrimNew(PT_TRAJEC, DhResGetMol(DhAtomGetRes(atomA[0])));

  primP->u.plate.atomA = malloc(atomNo * sizeof(DhAtomP));
  for (i = 0; i < atomNo; i++)
    primP->u.plate.atomA[i] = atomA[i];
  primP->u.plate.atomNo = atomNo;

  return primP;
}

PrimObjP
PrimNewPlate(DhAtomP atomA[], int atomNo)
{
  PrimObjP primP;
  int i;

  if (! PlateCheckAtoms(atomA, atomNo))
    return NULL;

  primP = PrimNew(PT_PLATE, DhResGetMol(DhAtomGetRes(atomA[0])));

  primP->u.plate.atomA = malloc(atomNo * sizeof(DhAtomP));
  for (i = 0; i < atomNo; i++)
    primP->u.plate.atomA[i] = atomA[i];
  primP->u.plate.atomNo = atomNo;

  return primP;
}

void
PrimDestroy(PrimObjP primP)
{
  PrimObjP prevP, nextP;
  float midPar;
  BOOL closeGap;
  int num;

  closeGap = FALSE;

  if (primP->type == PT_RIBBON) {
    prevP = ListPrev(PrimList, primP);
    if (prevP != NULL && (prevP->type != PT_RIBBON ||
	prevP->u.ribbon.ribbonP != primP->u.ribbon.ribbonP))
      prevP = NULL;

    nextP = ListNext(PrimList, primP);
    if (nextP != NULL && (nextP->type != PT_RIBBON ||
	nextP->u.ribbon.ribbonP != primP->u.ribbon.ribbonP))
      nextP = NULL;

    if (prevP != NULL && nextP != NULL) {
      midPar = 0.5f * (primP->u.ribbon.startPar + primP->u.ribbon.endPar);
      closeGap = TRUE;

      if ((primP->u.ribbon.shape == RS_SMOOTH) !=
	  (prevP->u.ribbon.shape == RS_SMOOTH) ||
	  (primP->u.ribbon.shape == RS_SMOOTH) !=
	  (nextP->u.ribbon.shape == RS_SMOOTH))
	primP->u.ribbon.ribbonP->splineValid = FALSE;
    }
  }

  ListRemove(PrimList, primP);

  primP = ListFirst(PrimList);
  num = 0;
  while (primP != NULL) {
    primP->num = num;
    primP = ListNext(PrimList, primP);
    num++;
  }

  if (closeGap)
    RibbonSetEndPar(prevP, REC_END, midPar);
}

void
PrimMapAddPolyline(PrimObjP primP, Vec3 *p, int size)
/* doesn't copy p, must be malloced */
{
  PrimMap *mapP;
  PrimMapPolyline *polylineP;

  mapP = &primP->u.map;

  if (mapP->polylineNo == 0)
    mapP->polylineA = malloc(sizeof(*mapP->polylineA));
  else
    mapP->polylineA = realloc(mapP->polylineA,
        (mapP->polylineNo + 1) * sizeof(*mapP->polylineA));

  polylineP = mapP->polylineA + mapP->polylineNo;

  polylineP->xA = p;
  polylineP->pointNo = size;

  mapP->polylineNo++;
}

void
PrimDotSurfaceAddPatch(PrimObjP primP, DhAtomP atomP, Vec3 *p, int size)
/* doesn't copy p, must be malloced */
{
  PrimDotSurface *surfaceP;
  PrimDotSurfacePatch *patchP;

  surfaceP = &primP->u.dotSurface;

  if (surfaceP->size == 0)
    surfaceP->p = malloc(sizeof(*surfaceP->p));
  else
    surfaceP->p = realloc(surfaceP->p,
	(surfaceP->size + 1) * sizeof(*surfaceP->p));
  
  patchP = surfaceP->p + surfaceP->size;

  patchP->p = p;
  patchP->size = size;
  patchP->atomP = atomP;

  surfaceP->size++;
}

void
PrimSurfaceSetPoints(PrimObjP primP, Vec3 xA[], Vec3 nvA[], int pointNo)
/* xA and nvA are not copied, must be malloced */
{
  if (primP->u.surface.xA != NULL) {
    free(primP->u.surface.xA);
    free(primP->u.surface.nvA);
  }

  if (primP->u.surface.colA != NULL) {
    free(primP->u.surface.colA);
    primP->u.surface.colA = NULL;
  }

  if (primP->u.surface.potA != NULL) {
    free(primP->u.surface.potA);
    primP->u.surface.potA = NULL;
  }

  primP->u.surface.xA = xA;
  primP->u.surface.nvA = nvA;
  primP->u.surface.pointNo = pointNo;
}

void
PrimSurfaceAddMesh(PrimObjP primP, int indA[], int size)
/* indA is not copied, must be malloced */
{
  PrimSurface *surfaceP;
  PrimSurfaceMesh *meshP;

  surfaceP = &primP->u.surface;

  if (surfaceP->meshNo == 0)
    surfaceP->meshA = malloc(sizeof(*surfaceP->meshA));
  else
    surfaceP->meshA = realloc(surfaceP->meshA,
	(surfaceP->meshNo + 1) * sizeof(*surfaceP->meshA));
  
  meshP = surfaceP->meshA + surfaceP->meshNo;

  meshP->indA = indA;
  meshP->size = size;

  if (size > surfaceP->maxSize)
    surfaceP->maxSize = size;

  surfaceP->meshNo++;
}

Ribbon *
RibbonAdd(void)
{
  Ribbon *ribbonP, ribbon;

  if (RibbonList == NULL) {
    RibbonList = ListOpen(sizeof(Ribbon));
    ListAddDestroyCB(RibbonList, freeRibbon, NULL, NULL);
    ribbon.num = 0;
  } else if (ListSize(RibbonList) == 0) {
    ribbon.num = 0;
  } else {
    ribbonP = ListLast(RibbonList);
    ribbon.num = ribbonP->num + 1;
  }
  
  ribbon.primNo = 0;
  ribbon.atomNo = 0;
  ribbon.atomA = NULL;
  ribbon.radA = NULL;
  ribbon.splineValid = FALSE;
  ribbon.coeffA = NULL;

  ribbonP = ListInsertLast(RibbonList, &ribbon);

  return ribbonP;
}

void
RibbonNew(DhMolP molP)
{
  Ribbon *ribbonP;

  ribbonP = RibbonAdd();
  RibbonBuild(molP, ribbonP);
  RibbonAddInvalidCB(ribbonP);

  if (ribbonP->atomNo < 2)
    removeRibbon(ribbonP);
}

void
RibbonSplit(PrimObjP oldPrimP)
{
  struct PrimObjS primS;
  float midPar;
  PrimObjP newPrimP, primP;
  int num;

  primS = *oldPrimP;

  midPar = 0.5f * (oldPrimP->u.ribbon.startPar + oldPrimP->u.ribbon.endPar);
  primS.u.ribbon.startPar = midPar;

  primS.u.ribbon.pointValid = FALSE;
  primS.u.ribbon.traceA = NULL;
  primS.u.ribbon.traceNo = 0;
  primS.u.ribbon.stripA = NULL;
  primS.u.ribbon.stripNo = 0;
  primS.u.ribbon.polyA = NULL;
  primS.u.ribbon.polyNo = 0;

  newPrimP = ListInsertAfter(PrimList, oldPrimP, &primS);
  PrimAddInvalidCB(newPrimP);

  newPrimP->u.ribbon.ribbonP->primNo++;

  newPrimP->propTab = PropGet(oldPrimP->propTab);
  newPrimP->attrP = AttrGet(oldPrimP->attrP);

  oldPrimP->u.ribbon.endPar = midPar;
  oldPrimP->u.ribbon.pointValid = FALSE;

  primP = newPrimP;
  num = oldPrimP->num + 1;
  while (primP != NULL) {
    primP->num = num;
    primP = ListNext(PrimList, primP);
    num++;
  }
}

void
PrimApply(PrimType type, PropRefP refP,
    PrimApplyFunc applyF, void *clientData)
{
  PrimObjP primP, nextPrimP;

  primP = ListFirst(PrimList);
  while (primP != NULL) {
    nextPrimP = ListNext(PrimList, primP);
    if ((type == PT_ALL || type == primP->type) &&
	(primP->propTab[refP->index] & refP->mask))
      applyF(primP, clientData);
    primP = nextPrimP;
  }
}

void
PrimMolApply(PrimType type, PropRefP refP, DhMolP molP,
    PrimApplyFunc applyF, void *clientData)
{
  PrimObjP primP, nextPrimP;

  primP = ListFirst(PrimList);
  while (primP != NULL) {
    nextPrimP = ListNext(PrimList, primP);
    if ((type == PT_ALL || type == primP->type) && molP == primP->molP &&
	(primP->propTab[refP->index] & refP->mask))
      applyF(primP, clientData);
    primP = nextPrimP;
  }
}

PrimObjP
PrimFindNumber(int num)
{
  return ListPos(PrimList, num - 1);
}

void
PrimAddPropCB(PrimType type, PropRefP refP,
    PrimPropCB propCB, void *clientData)
{
  PropCBData cbData;

  if (PropCBList == NULL)
    PropCBList = ListOpen(sizeof(PropCBData));
  
  cbData.type = type;
  cbData.refP = refP;
  cbData.propCB = propCB;
  cbData.clientData = clientData;

  (void) ListInsertLast(PropCBList, &cbData);
}

void
PrimSetProp(PropRefP refP, PrimObjP primP, BOOL onOff)
{
  PropCBData *cbDataP;

  if (onOff == PrimGetProp(refP, primP))
    return;

  primP->propTab = PropChange(primP->propTab, refP, onOff);

  cbDataP = ListFirst(PropCBList);
  while (cbDataP != NULL) {
    if (cbDataP->refP == refP &&
        (cbDataP->type == PT_ALL || cbDataP->type == primP->type))
      cbDataP->propCB(primP, refP, onOff, cbDataP->clientData);
    cbDataP = ListNext(PropCBList, cbDataP);
  }
}

BOOL
PrimGetProp(PropRefP refP, PrimObjP primP)
{
  return (primP->propTab[refP->index] & refP->mask) != 0;
}

unsigned *
PrimGetPropTab(PrimObjP primP)
{
  return primP->propTab;
}

void
PrimSetAttr(PrimObjP primP, AttrP attrP)
{
  primP->attrP = attrP;
}

void
PrimSetPos(PrimObjP primP, Vec3 x)
{
  Vec3 v;

  if (primP->type == PT_CIRCLE) {
    Vec3Copy(primP->u.circle.x, x);
  } else if (primP->type == PT_CYLINDER) {
    Vec3Copy(v, x);
    Vec3Sub(v, primP->u.cylinder.x1);
    Vec3Copy(primP->u.cylinder.x1, x);
    Vec3Add(primP->u.cylinder.x2, v);
  } else if (primP->type == PT_SOLID) {
    Vec3Copy(primP->u.solid.cent, x);
    primP->u.solid.pointValid = FALSE;
  } else {
    Vec3Copy(primP->u.text.x, x);
  }
}

void
PrimSetVec(PrimObjP primP, Vec3 v)
{
  if (primP->type == PT_CIRCLE) {
    Vec3Copy(primP->u.circle.v, v);
  } else if (primP->type == PT_CYLINDER) {
    Vec3Copy(primP->u.cylinder.x2, primP->u.cylinder.x1);
    Vec3Add(primP->u.cylinder.x2, v);
  } else {
    Vec3Copy(primP->u.text.dx, v);
  }
}

void
PrimSetCylinderStyle(PrimObjP primP, PrimCylinderStyle style)
{
  primP->u.cylinder.style = style;
}

void
PrimSetMapStyle(PrimObjP primP, PrimMapStyle style)
{
  primP->u.map.style = style;
}

void
PrimSetTrajecStyle(PrimObjP primP, PrimTrajecStyle style)
{
  primP->u.trajec.style = style;
}

void
PrimSetThick(PrimObjP primP, float thick)
{
  if (primP->type == PT_PLATE) {
    if (primP->u.plate.thick == thick)
      return;
    primP->u.plate.thick = thick;
    primP->u.plate.pointValid = FALSE;
  } else {
    if (primP->u.ribbon.thick == thick)
      return;
    primP->u.ribbon.thick = thick;
    primP->u.ribbon.pointValid = FALSE;
  }
}

void
PrimSetSolidKind(PrimObjP primP, PrimSolidKind kind)
{
  if (primP->u.solid.kind == kind)
    return;
  
  primP->u.solid.kind = kind;
  primP->u.solid.geomValid = FALSE;
}

void
PrimSetCover(PrimObjP primP, float cover)
{
  if (primP->u.solid.cover == cover)
    return;
  
  primP->u.solid.cover = cover;
  primP->u.solid.geomValid = FALSE;
}

void
PrimSetText(PrimObjP primP, char *str)
/* str is copied, must be freed by caller if it was malloced */
{
  if (primP->u.text.str != NULL)
    free(primP->u.text.str);
  
  primP->u.text.str = malloc(strlen(str) + 1);
  (void) strcpy(primP->u.text.str, str);
}

void
PrimSetDrawobjType(PrimObjP primP, PrimDrawobjType type)
{
  primP->u.drawobj.type = type;
}

void
PrimSetDrawobjStyle(PrimObjP primP, PrimDrawobjStyle style)
{
  primP->u.drawobj.style = style;
}

void
PrimSetPoint(PrimObjP primP, int ind, Vec3 x, Vec3 dx)
{
  if (primP->u.drawobj.xA == NULL) {
    primP->u.drawobj.xA = malloc((ind + 1) * sizeof(*primP->u.drawobj.xA));
    primP->u.drawobj.dxA = malloc((ind + 1) * sizeof(*primP->u.drawobj.dxA));
    primP->u.drawobj.pointNo = ind + 1;
  } else if (ind >= primP->u.drawobj.pointNo) {
    primP->u.drawobj.xA = realloc(primP->u.drawobj.xA,
	(ind + 1) * sizeof(*primP->u.drawobj.xA));
    primP->u.drawobj.dxA = realloc(primP->u.drawobj.dxA,
	(ind + 1) * sizeof(*primP->u.drawobj.dxA));
    primP->u.drawobj.pointNo = ind + 1;
  }

  Vec3Copy(primP->u.drawobj.xA[ind], x);
  Vec3Copy(primP->u.drawobj.dxA[ind], dx);
}

void
RibbonSetEndPar(PrimObjP primP, RibbonEndChoice endChoice, float par)
{
  PrimObjP neighPrimP;
  BOOL smooth;

  if (endChoice == REC_START)
    neighPrimP = ListPrev(PrimList, primP);
  else
    neighPrimP = ListNext(PrimList, primP);

  if (neighPrimP != NULL && (neighPrimP->type != PT_RIBBON ||
      neighPrimP->u.ribbon.ribbonP != primP->u.ribbon.ribbonP))
    neighPrimP = NULL;

  smooth = (primP->u.ribbon.shape == RS_SMOOTH);
  if (neighPrimP != NULL)
    smooth |= (neighPrimP->u.ribbon.shape == RS_SMOOTH);

  if (endChoice == REC_START) {
    if (neighPrimP != NULL) {
      if (par < neighPrimP->u.ribbon.startPar + MIN_LEN)
	par = neighPrimP->u.ribbon.startPar + MIN_LEN;

      neighPrimP->u.ribbon.endPar = par;
      neighPrimP->u.ribbon.pointValid = FALSE;
    }

    if (smooth && (int) par != (int) primP->u.ribbon.startPar)
      primP->u.ribbon.ribbonP->splineValid = FALSE;

    primP->u.ribbon.startPar = par;
  } else {
    if (neighPrimP != NULL) {
      if (par > neighPrimP->u.ribbon.endPar - MIN_LEN)
	par = neighPrimP->u.ribbon.endPar - MIN_LEN;
      
      neighPrimP->u.ribbon.startPar = par;
      neighPrimP->u.ribbon.pointValid = FALSE;
    }

    if (smooth && (int) par != (int) primP->u.ribbon.endPar)
      primP->u.ribbon.ribbonP->splineValid = FALSE;

    primP->u.ribbon.endPar = par;
  }

  primP->u.ribbon.pointValid = FALSE;
}

void
RibbonSetShape(PrimObjP primP, RibbonShape shape)
{
  if (primP->u.ribbon.shape == shape)
    return;

  primP->u.ribbon.shape = shape;
  primP->u.ribbon.pointValid = FALSE;
  primP->u.ribbon.ribbonP->splineValid = FALSE;
}

void
RibbonSetOrient(PrimObjP primP, RibbonOrient orient)
{
  if (primP->u.ribbon.orient == orient)
    return;

  primP->u.ribbon.orient = orient;
  primP->u.ribbon.pointValid = FALSE;
}

void
RibbonSetWidth(PrimObjP primP, float width)
{
  if (primP->u.ribbon.width == width)
    return;

  primP->u.ribbon.width = width;
  primP->u.ribbon.pointValid = FALSE;
}

void
RibbonSetStyle(PrimObjP primP, RibbonStyle style)
{
  if (primP->u.ribbon.style == style)
    return;

  primP->u.ribbon.style = style;
  primP->u.ribbon.pointValid = FALSE;
}

void
RibbonSetEndStyle(PrimObjP primP, RibbonEndChoice endChoice, RibbonEnd style)
{
  if (endChoice == REC_START) {
    if (primP->u.ribbon.startStyle == style)
      return;

    primP->u.ribbon.startStyle = style;
    primP->u.ribbon.pointValid = FALSE;
  } else {
    if (primP->u.ribbon.endStyle == style)
      return;

    primP->u.ribbon.endStyle = style;
    primP->u.ribbon.pointValid = FALSE;
  }
}

void
RibbonSetEndLen(PrimObjP primP, RibbonEndChoice endChoice, float len)
{
  if (endChoice == REC_START) {
    if (primP->u.ribbon.startLen == len)
      return;

    primP->u.ribbon.startLen = len;
    primP->u.ribbon.pointValid = FALSE;
  } else {
    if (primP->u.ribbon.endLen == len)
      return;

    primP->u.ribbon.endLen = len;
    primP->u.ribbon.pointValid = FALSE;
  }
}

void
RibbonSetArrowWidth(PrimObjP primP, float width)
{
  if (primP->u.ribbon.arrowWidth == width)
    return;

  primP->u.ribbon.arrowWidth = width;
  if (primP->u.ribbon.endStyle == RE_ARROW)
    primP->u.ribbon.pointValid = FALSE;
}

void
RibbonSetPaint(PrimObjP primP, RibbonPaint paint)
{
  if (primP->u.ribbon.paint == paint)
    return;

  primP->u.ribbon.paint = paint;
  primP->u.ribbon.pointValid = FALSE;
}

void
RibbonSetRadius(PrimObjP primP, RibbonRadius radius)
{
  if (primP->u.ribbon.radius == radius)
    return;

  primP->u.ribbon.radius = radius;
  primP->u.ribbon.pointValid = FALSE;
}

void
PrimSurfaceSetColor(PrimObjP primP,
    PrimSurfaceSetColorFunc setF, void *clientData)
{
  float pot;
  int i;

  if (setF == NULL) {
    if (primP->u.surface.colA != NULL) {
      free(primP->u.surface.colA);
      primP->u.surface.colA = NULL;
    }
    return;
  }

  if (primP->u.surface.colA == NULL)
    primP->u.surface.colA = malloc(primP->u.surface.pointNo * sizeof(Vec3));

  pot = 0.0f;
  for (i = 0; i < primP->u.surface.pointNo; i++) {
    if (primP->u.surface.potA != NULL)
      pot = primP->u.surface.potA[i];
    setF(primP->u.surface.colA[i],
        primP->u.surface.xA[i], primP->u.surface.nvA[i], pot, clientData);
  }
}

void
PrimSurfaceSetPot(PrimObjP primP,
    PrimSurfaceSetPotFunc setF, void *clientData)
{
  int i;

  if (setF == NULL) {
    if (primP->u.surface.potA != NULL) {
      free(primP->u.surface.potA);
      primP->u.surface.potA = NULL;
    }
    return;
  }

  if (primP->u.surface.potA == NULL)
    primP->u.surface.potA = malloc(primP->u.surface.pointNo * sizeof(float));

  for (i = 0; i < primP->u.surface.pointNo; i++)
    setF(primP->u.surface.potA + i, primP->u.surface.xA[i], clientData);
}

int
PrimGetNumber(PrimObjP primP)
{
  return primP->num + 1;  /* return numbers starting at 1 */
}

PrimType
PrimGetType(PrimObjP primP)
{
  return primP->type;
}

DhMolP
PrimGetMol(PrimObjP primP)
{
  return primP->molP;
}

AttrP
PrimGetAttr(PrimObjP primP)
{
  return primP->attrP;
}

void
PrimGetPos(PrimObjP primP, Vec3 x)
{
  if (primP->type == PT_CIRCLE) {
    Vec3Copy(x, primP->u.circle.x);
  } else if (primP->type == PT_CYLINDER) {
    Vec3Copy(x, primP->u.cylinder.x1);
  } else if (primP->type == PT_SOLID) {
    SolidCalcGeom(primP);
    Vec3Copy(x, primP->u.solid.cent);
  } else {
    Vec3Copy(x, primP->u.text.x);
  }
}

void
PrimGetVec(PrimObjP primP, Vec3 v)
{
  if (primP->type == PT_CIRCLE) {
    Vec3Copy(v, primP->u.circle.v);
  } else if (primP->type == PT_CYLINDER) {
    Vec3Copy(v, primP->u.cylinder.x2);
    Vec3Sub(v, primP->u.cylinder.x1);
  } else {
    Vec3Copy(v, primP->u.text.dx);
  }
}

PrimCylinderStyle
PrimGetCylinderStyle(PrimObjP primP)
{
  return primP->u.cylinder.style;
}

PrimMapStyle
PrimGetMapStyle(PrimObjP primP)
{
  return primP->u.map.style;
}

PrimTrajecStyle
PrimGetTrajecStyle(PrimObjP primP)
{
  return primP->u.trajec.style;
}

float
PrimGetThick(PrimObjP primP)
{
  if (primP->type == PT_PLATE)
    return primP->u.plate.thick;
  else
    return primP->u.ribbon.thick;
}

PrimSolidKind
PrimGetSolidKind(PrimObjP primP)
{
  return primP->u.solid.kind;
}

float
PrimGetCover(PrimObjP primP)
{
  return primP->u.solid.cover;
}

char *
PrimGetText(PrimObjP primP)
/* returns a pointer to internal memory */
{
  return primP->u.text.str;
}

PrimDrawobjType
PrimGetDrawobjType(PrimObjP primP)
{
  return primP->u.drawobj.type;
}

PrimDrawobjStyle
PrimGetDrawobjStyle(PrimObjP primP)
{
  return primP->u.drawobj.style;
}

void
PrimGetPoint(PrimObjP primP, int ind, Vec3 x, Vec3 dx)
{
  Vec3Copy(x, primP->u.drawobj.xA[ind]);
  Vec3Copy(dx, primP->u.drawobj.dxA[ind]);
}

int
PrimGetPointNo(PrimObjP primP)
{
  return primP->u.drawobj.pointNo;
}

float
RibbonGetEndPar(PrimObjP primP, RibbonEndChoice endChoice)
{
  if (endChoice == REC_START)
    return primP->u.ribbon.startPar;
  else
    return primP->u.ribbon.endPar;
}

RibbonShape
RibbonGetShape(PrimObjP primP)
{
  return primP->u.ribbon.shape;
}

RibbonOrient
RibbonGetOrient(PrimObjP primP)
{
  return primP->u.ribbon.orient;
}

float
RibbonGetWidth(PrimObjP primP)
{
  return primP->u.ribbon.width;
}

RibbonStyle
RibbonGetStyle(PrimObjP primP)
{
  return primP->u.ribbon.style;
}

RibbonEnd
RibbonGetEndStyle(PrimObjP primP, RibbonEndChoice endChoice)
{
  if (endChoice == REC_START)
    return primP->u.ribbon.startStyle;
  else
    return primP->u.ribbon.endStyle;
}

float
RibbonGetEndLen(PrimObjP primP, RibbonEndChoice endChoice)
{
  if (endChoice == REC_START)
    return primP->u.ribbon.startLen;
  else
    return primP->u.ribbon.endLen;
}

float
RibbonGetArrowWidth(PrimObjP primP)
{
  return primP->u.ribbon.arrowWidth;
}

RibbonPaint
RibbonGetPaint(PrimObjP primP)
{
  return primP->u.ribbon.paint;
}

RibbonRadius
RibbonGetRadius(PrimObjP primP)
{
  return primP->u.ribbon.radius;
}

LINLIST
RibbonListGet(void)
{
  return RibbonList;
}

LINLIST
PrimListGet(void)
{
  return PrimList;
}
