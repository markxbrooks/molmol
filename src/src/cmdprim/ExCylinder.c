/*
************************************************************************
*
*   ExCylinder.c - AddCylinder command
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
*   Date of last modification : 97/09/26
*   Pathname of SCCS file     : /files/kor/molmol/src/cmdprim/SCCS/s.ExCylinder.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>

#include <fit_cylinder.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

static int CurrFit = 0;

typedef struct {
  Vec3 *pA;
  int atomInd;
} AtomData;

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
fillData(DhAtomP atomP, void *clientData)
{
  AtomData *dataP = clientData;

  DhAtomGetCoord(atomP, dataP->pA[dataP->atomInd]);
  dataP->atomInd++;
}

#define ARG_NUM 1
#define FIT_NUM 2

ErrCode
ExAddCylinder(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[FIT_NUM];
  ErrCode errCode;
  AtomData data;
  DhMolP *molPA;
  int molNo, molI, atomNo;
  PropRefP refP;
  PrimObjP primP;
  Vec3 x1, x2;
  AttrP attrP;
  struct AttrS attr;

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "least_square";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "spacing";
  enumEntry[1].onOff = FALSE;

  enumEntry[CurrFit].onOff = TRUE;

  arg[0].prompt = "Fit";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = FIT_NUM;
  arg[0].v.intVal = CurrFit;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrFit = arg[0].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0) {
    CipSetError("no molecule selected");
    return EC_WARNING;
  }

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  for (molI = 0; molI < molNo; molI++) {
    atomNo = 0;
    DhMolApplyAtom(refP, molPA[molI], countAtoms, &atomNo);
    if (atomNo == 0)
      continue;

    if (CurrFit == 0) {
      if (atomNo < 5) {
	CipSetError("at least 5 atoms must be selected");
	free(molPA);
	return EC_ERROR;
      }
    } else {
      if (atomNo < 4) {
	CipSetError("at least 4 atoms must be selected");
	free(molPA);
	return EC_ERROR;
      }
    }

    data.pA = malloc(atomNo * sizeof(Vec3));
    data.atomInd = 0;
    DhMolApplyAtom(refP, molPA[molI], fillData, &data);

    primP = PrimNew(PT_CYLINDER, molPA[molI]);

    attrP = PrimGetAttr(primP);
    AttrCopy(&attr, attrP);
    AttrReturn(attrP);

    if (CurrFit == 0)
      FitCylinderLeastSq(data.pA, atomNo, x1, x2, &attr.radius);
    else
      FitCylinderSpacing(data.pA, atomNo, x1, x2, &attr.radius);

    free(data.pA);

    PrimSetPos(primP, x1);
    Vec3Sub(x2, x1);
    PrimSetVec(primP, x2);
    PrimSetAttr(primP, AttrGet(&attr));

    PrimSetProp(PropGetRef("cylinder", TRUE), primP, TRUE);
  }

  free(molPA);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
