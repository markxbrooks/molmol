/*
************************************************************************
*
*   PrimDump.c - read/write prim part of dump file
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.PrimDump.c
*   SCCS identification       : 1.21
*
************************************************************************
*/

#include <prim_hand.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <prop_tab.h>
#include <attr_mng.h>
#include <data_hand.h>
#include "prim_struc.h"
#include "solid_calc.h"

#define NO_RES -999

#define CHECK_RES(s) if (s != GF_RES_OK) return FALSE

BOOL
PrimDump(GFile gf)
/* may only be called AFTER DhDump() */
{
  LINLIST ribbonL;
  Ribbon *ribbonP;
  DhMolP molP;
  DhResP resP;
  LINLIST primL;
  PrimObjP primP;
  PrimMapPolyline *polylineP;
  PrimDotSurfacePatch *patchP;
  BOOL hasColors, hasPot;
  PrimSurfaceMesh *meshP;
  int polyI, patchI, i, k;

  ribbonL = RibbonListGet();

  CHECK_RES(GFileWriteINT32(gf, ListSize(ribbonL)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  ribbonP = ListFirst(ribbonL);
  while (ribbonP != NULL) {
    resP = DhAtomGetRes(ribbonP->atomA[0]);
    molP = DhResGetMol(resP);
    CHECK_RES(GFileWriteINT32(gf, DhMolGetNumber(molP)));

    CHECK_RES(GFileWriteINT32(gf, ribbonP->primNo));
    CHECK_RES(GFileWriteINT32(gf, ribbonP->atomNo));
    CHECK_RES(GFileWriteNL(gf));

    for (i = 0; i < ribbonP->atomNo; i++) {
      resP = DhAtomGetRes(ribbonP->atomA[i]);
      CHECK_RES(GFileWriteINT32(gf, DhResGetNumber(resP)));
      CHECK_RES(GFileWriteINT32(gf, DhAtomGetNumber(ribbonP->atomA[i])));
      CHECK_RES(GFileWriteNL(gf));
    }

    ribbonP = ListNext(ribbonL, ribbonP);
  }

  primL = PrimListGet();

  CHECK_RES(GFileWriteINT32(gf, ListSize(primL)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  primP = ListFirst(primL);
  while (primP != NULL) {
    CHECK_RES(GFileWriteINT16(gf, (INT16) primP->type));
    CHECK_RES(GFileWriteNL(gf));

    if (primP->molP == NULL) {
      CHECK_RES(GFileWriteINT32(gf, -1));
    } else {
      CHECK_RES(GFileWriteINT32(gf, DhMolGetNumber(primP->molP)));
    }

    CHECK_RES(GFileWriteNL(gf));

    switch (primP->type) {
      case PT_CIRCLE:
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.circle.x[i]));
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.circle.v[i]));
	CHECK_RES(GFileWriteNL(gf));
	break;
      case PT_MAP:
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.map.style));
	CHECK_RES(GFileWriteINT32(gf, primP->u.map.polylineNo));
	CHECK_RES(GFileWriteNL(gf));
	for (polyI = 0; polyI < primP->u.map.polylineNo; polyI++) {
	  polylineP = primP->u.map.polylineA + polyI;
	  CHECK_RES(GFileWriteINT32(gf, polylineP->pointNo));
	  CHECK_RES(GFileWriteNL(gf));
	  for (i = 0; i < polylineP->pointNo; i++) {
	    for (k = 0; k < 3; k++)
	      CHECK_RES(GFileWriteFLOAT32(gf, polylineP->xA[i][k]));
	    CHECK_RES(GFileWriteNL(gf));
	  }
	}
	break;
      case PT_TRAJEC:
	CHECK_RES(GFileWriteINT32(gf, primP->u.trajec.atomNo));
	CHECK_RES(GFileWriteNL(gf));
	for (i = 0; i < primP->u.trajec.atomNo; i++) {
	  resP = DhAtomGetRes(primP->u.trajec.atomA[i]);
	  CHECK_RES(GFileWriteINT32(gf, DhMolGetNumber(DhResGetMol(resP))));
	  CHECK_RES(GFileWriteINT32(gf, DhResGetNumber(resP)));
	  CHECK_RES(GFileWriteINT32(gf,
	      DhAtomGetNumber(primP->u.trajec.atomA[i])));
	  CHECK_RES(GFileWriteNL(gf));
	}
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.trajec.style));
	break;
      case PT_PLATE:
	CHECK_RES(GFileWriteINT32(gf, primP->u.plate.atomNo));
	CHECK_RES(GFileWriteNL(gf));
	for (i = 0; i < primP->u.plate.atomNo; i++) {
	  resP = DhAtomGetRes(primP->u.plate.atomA[i]);
	  CHECK_RES(GFileWriteINT32(gf, DhResGetNumber(resP)));
	  CHECK_RES(GFileWriteINT32(gf,
	      DhAtomGetNumber(primP->u.plate.atomA[i])));
	  CHECK_RES(GFileWriteNL(gf));
	}
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.plate.thick));
	break;
      case PT_CYLINDER:
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.cylinder.style));
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.cylinder.x1[i]));
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.cylinder.x2[i]));
	CHECK_RES(GFileWriteNL(gf));
	break;
      case PT_SOLID:
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.solid.kind));
	CHECK_RES(GFileWriteINT32(gf, primP->u.solid.atomNo));
	CHECK_RES(GFileWriteNL(gf));
	for (i = 0; i < primP->u.solid.atomNo; i++) {
	  resP = DhAtomGetRes(primP->u.solid.atomA[i]);
	  CHECK_RES(GFileWriteINT32(gf, DhResGetNumber(resP)));
	  CHECK_RES(GFileWriteINT32(gf,
	      DhAtomGetNumber(primP->u.solid.atomA[i])));
	  CHECK_RES(GFileWriteNL(gf));
	}
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.solid.cover));
	SolidCalcGeom(primP);
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.solid.cent[i]));
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.solid.axis1[i]));
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.solid.axis2[i]));
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.solid.axis3[i]));
	CHECK_RES(GFileWriteNL(gf));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.solid.size1));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.solid.size2));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.solid.size3));
	CHECK_RES(GFileWriteNL(gf));
	break;
      case PT_TEXT:
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.text.x[i]));
	for (i = 0; i < 3; i++)
	  CHECK_RES(GFileWriteFLOAT32(gf, primP->u.text.dx[i]));
	CHECK_RES(GFileWriteINT32(gf, strlen(primP->u.text.str)));
	CHECK_RES(GFileWriteQuotedStr(gf, primP->u.text.str));
	CHECK_RES(GFileWriteNL(gf));
	break;
      case PT_DOT_SURFACE:
	CHECK_RES(GFileWriteINT32(gf, primP->u.dotSurface.size));
	CHECK_RES(GFileWriteNL(gf));
	for (patchI = 0; patchI < primP->u.dotSurface.size; patchI++) {
	  patchP = primP->u.dotSurface.p + patchI;
	  CHECK_RES(GFileWriteINT32(gf, patchP->size));
	  CHECK_RES(GFileWriteNL(gf));
	  for (i = 0; i < patchP->size; i++) {
	    for (k = 0; k < 3; k++)
	      CHECK_RES(GFileWriteFLOAT32(gf, patchP->p[i][k]));
	    CHECK_RES(GFileWriteNL(gf));
	  }
	  if (patchP->atomP == NULL) {
	    CHECK_RES(GFileWriteINT32(gf, NO_RES));
	  } else {
	    resP = DhAtomGetRes(patchP->atomP);
	    CHECK_RES(GFileWriteINT32(gf, DhResGetNumber(resP)));
	    CHECK_RES(GFileWriteINT32(gf, DhAtomGetNumber(patchP->atomP)));
	    CHECK_RES(GFileWriteNL(gf));
	  }
	}
	CHECK_RES(GFileWriteNL(gf));
	break;
      case PT_SURFACE:
	CHECK_RES(GFileWriteINT32(gf, primP->u.surface.pointNo));
	hasColors = (primP->u.surface.colA != NULL);
	hasPot = (primP->u.surface.potA != NULL);
	CHECK_RES(GFileWriteINT16(gf, (INT16) hasColors));
	CHECK_RES(GFileWriteINT16(gf, (INT16) hasPot));
	CHECK_RES(GFileWriteNL(gf));
	for (i = 0; i < primP->u.surface.pointNo; i++) {
	  for (k = 0; k < 3; k++)
	    CHECK_RES(GFileWriteFLOAT32(gf, primP->u.surface.xA[i][k]));
	  for (k = 0; k < 3; k++)
	    CHECK_RES(GFileWriteFLOAT32(gf, primP->u.surface.nvA[i][k]));
	  if (hasColors)
	    for (k = 0; k < 3; k++)
	      CHECK_RES(GFileWriteFLOAT32(gf, primP->u.surface.colA[i][k]));
	  if (hasPot)
	    CHECK_RES(GFileWriteFLOAT32(gf, primP->u.surface.potA[i]));
	  CHECK_RES(GFileWriteNL(gf));
	}
	CHECK_RES(GFileWriteINT32(gf, primP->u.surface.meshNo));
	CHECK_RES(GFileWriteNL(gf));
	for (i = 0; i < primP->u.surface.meshNo; i++) {
	  meshP = primP->u.surface.meshA + i;
	  CHECK_RES(GFileWriteINT32(gf, meshP->size));
	  for (k = 0; k < meshP->size; k++)
	    CHECK_RES(GFileWriteINT32(gf, meshP->indA[k]));
	  CHECK_RES(GFileWriteNL(gf));
	}
	CHECK_RES(GFileWriteINT32(gf, primP->u.surface.maxSize));
	CHECK_RES(GFileWriteNL(gf));
	break;
      case PT_RIBBON:
	CHECK_RES(GFileWriteINT32(gf, primP->u.ribbon.ribbonP->num));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.ribbon.startPar));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.ribbon.endPar));
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.ribbon.shape));
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.ribbon.orient));
	CHECK_RES(GFileWriteNL(gf));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.ribbon.width));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.ribbon.thick));
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.ribbon.style));
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.ribbon.startStyle));
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.ribbon.endStyle));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.ribbon.startLen));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.ribbon.endLen));
	CHECK_RES(GFileWriteFLOAT32(gf, primP->u.ribbon.arrowWidth));
	CHECK_RES(GFileWriteNL(gf));
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.ribbon.paint));
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.ribbon.radius));
	CHECK_RES(GFileWriteNL(gf));
	break;
      case PT_SHEET:
	CHECK_RES(GFileWriteINT32(gf, primP->u.sheet.atomNo));
	CHECK_RES(GFileWriteNL(gf));
	for (i = 0; i < primP->u.sheet.atomNo; i++) {
	  resP = DhAtomGetRes(primP->u.sheet.atomA[i]);
	  CHECK_RES(GFileWriteINT32(gf, DhResGetNumber(resP)));
	  CHECK_RES(GFileWriteINT32(gf,
	      DhAtomGetNumber(primP->u.sheet.atomA[i])));
	  CHECK_RES(GFileWriteNL(gf));
	}
	break;
      case PT_DRAWOBJ:
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.drawobj.type));
	CHECK_RES(GFileWriteINT16(gf, (INT16) primP->u.drawobj.style));
	CHECK_RES(GFileWriteINT32(gf, primP->u.drawobj.pointNo));
	CHECK_RES(GFileWriteNL(gf));
	for (i = 0; i < primP->u.drawobj.pointNo; i++) {
	  for (k = 0; k < 3; k++)
	    CHECK_RES(GFileWriteFLOAT32(gf, primP->u.drawobj.xA[i][k]));
	  for (k = 0; k < 3; k++)
	    CHECK_RES(GFileWriteFLOAT32(gf, primP->u.drawobj.dxA[i][k]));
	  CHECK_RES(GFileWriteNL(gf));
	}
	CHECK_RES(GFileWriteNL(gf));
	break;
    }

    CHECK_RES(GFileWriteINT32(gf, PropGetIndex(primP->propTab)));
    CHECK_RES(GFileWriteINT32(gf, AttrGetIndex(primP->attrP)));
    CHECK_RES(GFileWriteNL(gf));

    primP = ListNext(primL, primP);
  }

  return TRUE;
}

BOOL
PrimUndump(GFile gf, int vers)
/* may only be called AFTER DhUndump() */
{
  int ribbonNo, primNo, idx;
  Ribbon *ribbonP;
  LINLIST ribbonL;
  PrimType type;
  DhMolP molP;
  PrimObjP primP;
  PrimMapPolyline *polylineP;
  PrimDotSurfacePatch *patchP;
  BOOL hasColors, hasPot;
  PrimSurfaceMesh *meshP;
  DhResP resP;
  INT32 i32;
  INT16 i16;
  FLOAT32 f32;
  int polyI, patchI, i, k;

  /* PrimDestroyAll() must be called before! */

  CHECK_RES(GFileReadINT32(gf, &i32));
  ribbonNo = i32;

  for (idx = 0; idx < ribbonNo; idx++) {
    ribbonP = RibbonAdd();

    ribbonP->num = idx;

    CHECK_RES(GFileReadINT32(gf, &i32));
    molP = DhMolFindNumber(i32);

    CHECK_RES(GFileReadINT32(gf, &i32));
    ribbonP->primNo = i32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    ribbonP->atomNo = i32;

    ribbonP->atomA = malloc(ribbonP->atomNo * sizeof(*ribbonP->atomA));

    for (i = 0; i < ribbonP->atomNo; i++) {
      CHECK_RES(GFileReadINT32(gf, &i32));
      resP = DhResFind(molP, i32);
      CHECK_RES(GFileReadINT32(gf, &i32));
      ribbonP->atomA[i] = DhAtomFindNumber(resP, i32, TRUE);
    }

    ribbonP->splineValid = FALSE;

    RibbonAddInvalidCB(ribbonP);
  }

  ribbonL = RibbonListGet();

  CHECK_RES(GFileReadINT32(gf, &i32));
  primNo = i32;

  for (idx = 0; idx < primNo; idx++) {
    CHECK_RES(GFileReadINT16(gf, &i16));
    type = i16;
    if (vers < 2 && type > PT_RIBBON)  /* inserted PT_SHEET in version 2 */
      type++;

    CHECK_RES(GFileReadINT32(gf, &i32));
    if (i32 == -1)
      molP = NULL;
    else
      molP = DhMolFindNumber(i32);

    primP = PrimNewEmpty(type, molP);
    primP->num = idx;

    switch (type) {
      case PT_CIRCLE:
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.circle.x[i] = f32;
	}
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.circle.v[i] = f32;
	}
	break;
      case PT_MAP:
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.map.style = i16;
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.map.polylineNo = i32;
	primP->u.map.polylineA = malloc(
	    primP->u.map.polylineNo * sizeof(PrimMapPolyline));
	for (polyI = 0; polyI < primP->u.map.polylineNo; polyI++) {
	  polylineP = primP->u.map.polylineA + polyI;
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  polylineP->pointNo = i32;
	  polylineP->xA = malloc(polylineP->pointNo * sizeof(Vec3));
	  for (i = 0; i < polylineP->pointNo; i++) {
	    for (k = 0; k < 3; k++) {
	      CHECK_RES(GFileReadFLOAT32(gf, &f32));
	      polylineP->xA[i][k] = f32;
	    }
	  }
	}
	break;
      case PT_TRAJEC:
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.trajec.atomNo = i32;
	primP->u.trajec.atomA = malloc(
	    primP->u.trajec.atomNo * sizeof(DhAtomP));
	for (i = 0; i < primP->u.trajec.atomNo; i++) {
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  molP = DhMolFindNumber(i32);
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  resP = DhResFind(molP, i32);
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  primP->u.trajec.atomA[i] = DhAtomFindNumber(resP, i32, TRUE);
	}
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.trajec.style = i16;
	break;
      case PT_PLATE:
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.plate.atomNo = i32;
	primP->u.plate.atomA = malloc(primP->u.plate.atomNo * sizeof(DhAtomP));
	for (i = 0; i < primP->u.plate.atomNo; i++) {
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  resP = DhResFind(molP, i32);
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  primP->u.plate.atomA[i] = DhAtomFindNumber(resP, i32, TRUE);
	}
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.plate.thick = f32;
	break;
      case PT_CYLINDER:
	if (vers < 15) {
	  primP->u.cylinder.style = PCS_NORMAL;
	} else {
	  CHECK_RES(GFileReadINT16(gf, &i16));
	  primP->u.cylinder.style = i16;
	}
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.cylinder.x1[i] = f32;
	}
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.cylinder.x2[i] = f32;
	}
	break;
      case PT_SOLID:
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.solid.kind = i16;
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.solid.atomNo = i32;
	primP->u.solid.atomA = malloc(primP->u.solid.atomNo * sizeof(DhAtomP));
	for (i = 0; i < primP->u.solid.atomNo; i++) {
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  resP = DhResFind(molP, i32);
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  primP->u.solid.atomA[i] = DhAtomFindNumber(resP, i32, TRUE);
	}
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.solid.cover = f32;
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.solid.cent[i] = f32;
	}
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.solid.axis1[i] = f32;
	}
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.solid.axis2[i] = f32;
	}
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.solid.axis3[i] = f32;
	}
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.solid.size1 = f32;
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.solid.size2 = f32;
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.solid.size3 = f32;
	primP->u.solid.geomValid = TRUE;
	break;
      case PT_TEXT:
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.text.x[i] = f32;
	}
	for (i = 0; i < 3; i++) {
	  CHECK_RES(GFileReadFLOAT32(gf, &f32));
	  primP->u.text.dx[i] = f32;
	}
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.text.str = malloc(i32 + 1);
	CHECK_RES(GFileReadQuotedStr(gf, primP->u.text.str, i32 + 1));
	break;
      case PT_DOT_SURFACE:
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.dotSurface.size = i32;
	primP->u.dotSurface.p =
	    malloc(primP->u.dotSurface.size * sizeof(PrimDotSurfacePatch));
	for (patchI = 0; patchI < primP->u.dotSurface.size; patchI++) {
	  patchP = primP->u.dotSurface.p + patchI;
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  patchP->size = i32;
	  patchP->p = malloc(patchP->size * sizeof(Vec3));
	  for (i = 0; i < patchP->size; i++) {
	    for (k = 0; k < 3; k++) {
	      CHECK_RES(GFileReadFLOAT32(gf, &f32));
	      patchP->p[i][k] = f32;
	    }
	  }
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  if (i32 == NO_RES) {
	    patchP->atomP = NULL;
	  } else {
	    resP = DhResFind(primP->molP, i32);
	    CHECK_RES(GFileReadINT32(gf, &i32));
	    patchP->atomP = DhAtomFindNumber(resP, i32, TRUE);
	  }
	}
	break;
      case PT_SURFACE:
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.surface.pointNo = i32;
	if (vers < 7) {
	  hasColors = FALSE;
	  hasPot = FALSE;
	} else {
	  CHECK_RES(GFileReadINT16(gf, &i16));
	  hasColors = i16;
	  CHECK_RES(GFileReadINT16(gf, &i16));
	  hasPot = i16;
	}
	primP->u.surface.xA =
	    malloc(primP->u.surface.pointNo * sizeof(Vec3));
	primP->u.surface.nvA =
	    malloc(primP->u.surface.pointNo * sizeof(Vec3));
	if (hasColors)
	  primP->u.surface.colA =
	      malloc(primP->u.surface.pointNo * sizeof(Vec3));
	else
	  primP->u.surface.colA = NULL;
	if (hasPot)
	  primP->u.surface.potA =
	      malloc(primP->u.surface.pointNo * sizeof(float));
	else
	  primP->u.surface.potA = NULL;
	for (i = 0; i < primP->u.surface.pointNo; i++) {
	  for (k = 0; k < 3; k++) {
	    CHECK_RES(GFileReadFLOAT32(gf, &f32));
	    primP->u.surface.xA[i][k] = f32;
	  }
	  for (k = 0; k < 3; k++) {
	    CHECK_RES(GFileReadFLOAT32(gf, &f32));
	    primP->u.surface.nvA[i][k] = f32;
	  }
	  if (hasColors)
	    for (k = 0; k < 3; k++) {
	      CHECK_RES(GFileReadFLOAT32(gf, &f32));
	      primP->u.surface.colA[i][k] = f32;
	    }
	  if (hasPot) {
	    CHECK_RES(GFileReadFLOAT32(gf, &f32));
	    primP->u.surface.potA[i] = f32;
	  }
	}
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.surface.meshNo = i32;
	primP->u.surface.meshA =
	    malloc(primP->u.surface.meshNo * sizeof(PrimSurfaceMesh));
	for (i = 0; i < primP->u.surface.meshNo; i++) {
	  meshP = primP->u.surface.meshA + i;
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  meshP->size = i32;
	  meshP->indA = malloc(meshP->size * sizeof(*meshP->indA));
	  for (k = 0; k < meshP->size; k++) {
	    CHECK_RES(GFileReadINT32(gf, &i32));
	    meshP->indA[k] = i32;
	  }
	}
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.surface.maxSize = i32;
	break;
      case PT_RIBBON:
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.ribbon.ribbonP = ListPos(ribbonL, i32);
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.ribbon.startPar = f32;
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.ribbon.endPar = f32;
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.ribbon.shape = i16;
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.ribbon.orient = i16;
	if (vers < 8 && primP->u.ribbon.orient > RO_CYLIND)
	  /* inserted RO_SPACING in version 8 */
	  primP->u.ribbon.orient++;
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.ribbon.width = f32;
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.ribbon.thick = f32;
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.ribbon.style = i16;
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.ribbon.startStyle = i16;
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.ribbon.endStyle = i16;
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.ribbon.startLen = f32;
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.ribbon.endLen = f32;
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	primP->u.ribbon.arrowWidth = f32;
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.ribbon.paint = i16;
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.ribbon.radius = i16;
	break;
      case PT_SHEET:
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.sheet.atomNo = i32;
	primP->u.sheet.atomA = malloc(primP->u.sheet.atomNo * sizeof(DhAtomP));
	for (i = 0; i < primP->u.sheet.atomNo; i++) {
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  resP = DhResFind(molP, i32);
	  CHECK_RES(GFileReadINT32(gf, &i32));
	  primP->u.sheet.atomA[i] = DhAtomFindNumber(resP, i32, TRUE);
	}
	break;
      case PT_DRAWOBJ:
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.drawobj.type = i16;
	CHECK_RES(GFileReadINT16(gf, &i16));
	primP->u.drawobj.style = i16;
	CHECK_RES(GFileReadINT32(gf, &i32));
	primP->u.drawobj.pointNo = i32;
	primP->u.drawobj.xA = malloc(primP->u.drawobj.pointNo *
	    sizeof(*primP->u.drawobj.xA));
	primP->u.drawobj.dxA = malloc(primP->u.drawobj.pointNo *
	    sizeof(*primP->u.drawobj.dxA));
	for (i = 0; i < primP->u.drawobj.pointNo; i++) {
	  for (k = 0; k < 3; k++) {
	    CHECK_RES(GFileReadFLOAT32(gf, &f32));
	    primP->u.drawobj.xA[i][k] = f32;
	  }
	  for (k = 0; k < 3; k++) {
	    CHECK_RES(GFileReadFLOAT32(gf, &f32));
	    primP->u.drawobj.dxA[i][k] = f32;
	  }
	}
	break;
    }

    PropFreeTab(primP->propTab);  /* created by PrimNew() */
    CHECK_RES(GFileReadINT32(gf, &i32));
    primP->propTab = PropGet(PropFindIndex(i32));

    AttrReturn(primP->attrP);  /* created by PrimNew() */
    CHECK_RES(GFileReadINT32(gf, &i32));
    primP->attrP = AttrGet(AttrFindIndex(i32));
  }

  PropUndumpEnd();
  AttrUndumpEnd();

  return TRUE;
}
