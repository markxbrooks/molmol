/*
************************************************************************
*
*   RibbonBuild.c - build ribbon
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.RibbonBuild.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include "ribbon_build.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <prop_def.h>
#include <prop_second.h>
#include <data_hand.h>
#include "prim_struc.h"

#define MAX_SEC_NO 1000

typedef enum {
  SK_HELIX,
  SK_SHEET,
  SK_COIL
} SecKind;

static void
countAtom(DhAtomP atomP, void *clientData)
{
  Ribbon *ribbonP = clientData;

  ribbonP->atomNo++;
}

static void
fillAtom(DhAtomP atomP, void *clientData)
{
  Ribbon *ribbonP = clientData;

  ribbonP->atomA[ribbonP->atomNo++] = atomP;
}

void
RibbonBuild(DhMolP molP, Ribbon *ribbonP)
{
  PropRefP refP;
  PropRefP ribbonRefP, helixRefP, sheetRefP, coilRefP;
  PropRefP secRefP, sheetIRefP;
  SecKind kind, lastKind;
  DhResP resP;
  PrimObjP primP;
  BOOL newPrim;
  char propName[30];
  int atomI, helixI, sheetI, strandI;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  ribbonP->atomNo = 0;
  DhMolApplyAtom(refP, molP, countAtom, ribbonP);
  if (ribbonP->atomNo < 2) {
    ribbonP->atomNo = 0;
    return;
  }
  
  ribbonP->atomA = malloc(ribbonP->atomNo * sizeof(*ribbonP->atomA));

  ribbonP->atomNo = 0;
  DhMolApplyAtom(refP, molP, fillAtom, ribbonP);

  ribbonRefP = PropGetRef("ribbon", TRUE);
  helixRefP = PropGetRef(PROP_HELIX, FALSE);
  sheetRefP = PropGetRef(PROP_SHEET, FALSE);
  coilRefP = PropGetRef(PROP_COIL, FALSE);

  ribbonP->primNo = 0;

  for (atomI = 0; atomI < ribbonP->atomNo; atomI++) {
    resP = DhAtomGetRes(ribbonP->atomA[atomI]);

    if (DhResGetProp(helixRefP, resP))
      kind = SK_HELIX;
    else if (DhResGetProp(sheetRefP, resP))
      kind = SK_SHEET;
    else
      kind = SK_COIL;

    newPrim = FALSE;
    if (atomI == 0) {
      newPrim = TRUE;
    } else if (kind == SK_COIL) {
      if (lastKind != kind)
	newPrim = TRUE;
    } else if (kind != lastKind) {
      newPrim = TRUE;
    } else {
      if (! DhResGetProp(secRefP, resP))
	newPrim = TRUE;
    }

    if (newPrim && kind == SK_HELIX) {
      for (helixI = 1; helixI < MAX_SEC_NO; helixI++) {
	(void) sprintf(propName, "%s%d", PROP_HELIX, helixI);
	secRefP = PropGetRef(propName, FALSE);
	if (DhResGetProp(secRefP, resP))
	  break;
      }

      if (helixI == MAX_SEC_NO)
	secRefP = helixRefP;
    } else if (newPrim && kind == SK_SHEET) {
      for (sheetI = 1; sheetI < MAX_SEC_NO; sheetI++) {
	(void) sprintf(propName, "%s%d", PROP_SHEET, sheetI);
	sheetIRefP = PropGetRef(propName, FALSE);
	if (DhResGetProp(sheetIRefP, resP))
	  break;
      }

      if (sheetI == MAX_SEC_NO) {
	sheetIRefP = sheetRefP;
	secRefP = sheetRefP;
      } else {
	for (strandI = 1; strandI < MAX_SEC_NO; strandI++) {
	  (void) sprintf(propName, "%s%d_%d", PROP_STRAND, sheetI, strandI);
	  secRefP = PropGetRef(propName, FALSE);
	  if (DhResGetProp(secRefP, resP))
	    break;
	}

	if (strandI == MAX_SEC_NO)
	  secRefP = sheetRefP;
      }
    }

    if (newPrim) {
      if (atomI != 0)
	primP->u.ribbon.endPar = (float) atomI - 0.5f;

      primP = PrimNew(PT_RIBBON, molP);
      primP->u.ribbon.ribbonP = ribbonP;

      if (atomI == 0)
	primP->u.ribbon.startPar = 0.0f;
      else
	primP->u.ribbon.startPar = (float) atomI - 0.5f;

      switch (kind) {
	case SK_HELIX:
	  primP->u.ribbon.shape = RS_INTERPOL;
	  primP->u.ribbon.orient = RO_CYLIND;
	  primP->u.ribbon.style = RS_ELLIP;
	  primP->u.ribbon.startStyle = RE_SOFT;
	  primP->u.ribbon.endStyle = RE_SOFT;
	  primP->u.ribbon.paint = RP_TWO;

	  PrimSetProp(helixRefP, primP, TRUE);
	  PrimSetProp(secRefP, primP, TRUE);
	  break;
	case SK_SHEET:
	  primP->u.ribbon.shape = RS_SMOOTH;
	  primP->u.ribbon.orient = RO_NEIGH;
	  primP->u.ribbon.style = RS_ELLIP;
	  primP->u.ribbon.startStyle = RE_SHARP;
	  primP->u.ribbon.endStyle = RE_ARROW;
	  primP->u.ribbon.paint = RP_ONE;

	  PrimSetProp(sheetRefP, primP, TRUE);
	  PrimSetProp(sheetIRefP, primP, TRUE);
	  PrimSetProp(secRefP, primP, TRUE);
	  break;
	case SK_COIL:
	  primP->u.ribbon.shape = RS_INTERPOL;
	  primP->u.ribbon.orient = RO_FREE;
	  primP->u.ribbon.style = RS_ROUND;
	  primP->u.ribbon.paint = RP_ONE;
	  if (atomI == 0)
	    primP->u.ribbon.startStyle = RE_SHARP;
	  else
	    primP->u.ribbon.startStyle = RE_OPEN;
	  primP->u.ribbon.endStyle = RE_OPEN;

	  PrimSetProp(coilRefP, primP, TRUE);
	  break;
      }

      primP->u.ribbon.width = 2.5f;
      primP->u.ribbon.thick = 0.4f;
      primP->u.ribbon.startLen = 2.0f;
      primP->u.ribbon.endLen = 2.0f;
      primP->u.ribbon.arrowWidth = 1.4f;
      primP->u.ribbon.radius = RR_CONST;

      PrimSetProp(ribbonRefP, primP, TRUE);

      ribbonP->primNo++;
      lastKind = kind;
    }
  }

  primP->u.ribbon.endPar = (float) (ribbonP->atomNo - 1);
  if (primP->u.ribbon.endStyle == RE_OPEN)
    primP->u.ribbon.endStyle = RE_SHARP;
}
