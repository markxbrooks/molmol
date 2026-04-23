/*
************************************************************************
*
*   ExTrajec.c - AddTrajec and StyleTrajec commands
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
*   Date of last modification : 99/10/23
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExTrajec.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

typedef struct {
  DhAtomP *atomPA;
  int atomNo;
} AtomList;

static int CurrStyle = 1;
static PropRefP VisibleRefP;

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
fillAtoms(DhAtomP atomP, void *clientData)
{
  AtomList *listP = clientData;

  listP->atomPA[listP->atomNo++] = atomP;
}

ErrCode
ExAddTrajec(char *cmd)
{
  int molNo, switchNo, molI;
  DhMolP *molPA;
  int atomNo, atomNo1, atomI;
  AtomList *listA;
  DhAtomP *atomPA;
  PropRefP refP;
  PrimObjP primP;

  molPA = NULL;
  molNo = SelMolGet(NULL, 0);
  if (molNo >= 2) {
    molPA = malloc(molNo * sizeof(DhMolP));
    (void) SelMolGet(molPA, molNo);

    refP = PropGetRef(PROP_SELECTED, FALSE);

    /* ignore molecules with no atoms selected */
    switchNo = 0;
    for (molI = 0; molI < molNo; molI++) {
      atomNo1 = 0;
      DhMolApplyAtom(refP, molPA[molI], countAtoms, &atomNo1);

      if (atomNo1 > 0) {
	molPA[switchNo] = molPA[molI];
	switchNo++;

	if (switchNo == 1) {
	  atomNo = atomNo1;
	} else if (atomNo1 != atomNo) {
          free(molPA);
          CipSetError("number of selected atoms must be equal");
          return EC_ERROR;
	}
      }
    }

    molNo = switchNo;
  }

  if (molNo < 2) {
    CipSetError("at least 2 molecules must be selected");
    if (molPA != NULL)
      free(molPA);
    return EC_ERROR;
  }

  listA = malloc(molNo * sizeof(*listA));

  for (molI = 0; molI < molNo; molI++) {
    listA[molI].atomNo = 0;
    listA[molI].atomPA = malloc(atomNo * sizeof(DhAtomP));
    DhMolApplyAtom(refP, molPA[molI], fillAtoms, listA + molI);
  }

  atomPA = malloc(molNo * sizeof(*atomPA));

  for (atomI = 0; atomI < atomNo; atomI++) {
    for (molI = 0; molI < molNo; molI++)
      atomPA[molI] = listA[molI].atomPA[atomI];
    primP = PrimNewTrajec(atomPA, molNo);
    PrimSetProp(PropGetRef("trajec", TRUE), primP, TRUE);
  }

  for (molI = 0; molI < molNo; molI++)
    free(listA[molI].atomPA);

  free(molPA);
  free(listA);
  free(atomPA);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

static void
setStyle(PrimObjP primP, void *clientData)
{
  AttrP attrP;
  struct AttrS attr;

  PrimSetTrajecStyle(primP, CurrStyle);

  attrP = PrimGetAttr(primP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  if (CurrStyle == PTS_NEON || CurrStyle == PTS_ELLIPSOID)
    attr.shadeModel = SHADE_DEFAULT;
  else
    attr.shadeModel = -1;
  PrimSetAttr(primP, AttrGet(&attr));

  PrimSetProp(VisibleRefP, primP, CurrStyle != PTS_INVISIBLE);
}

#define ARG_NUM 1
#define STYLE_NUM 4

ErrCode
ExStyleTrajec(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[STYLE_NUM];
  ErrCode errCode;

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "invisible";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "line";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "neon";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "ellipsoid";
  enumEntry[3].onOff = FALSE;

  enumEntry[CurrStyle].onOff = TRUE;

  arg[0].prompt = "Style";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = STYLE_NUM;
  arg[0].u.enumD.lineNo = 2;
  arg[0].v.intVal = CurrStyle;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrStyle = arg[0].v.intVal;
  ArgCleanup(arg, ARG_NUM);

  VisibleRefP = PropGetRef(PROP_VISIBLE, FALSE);
  PrimApply(PT_TRAJEC, PropGetRef(PROP_SELECTED, FALSE), setStyle, NULL);

  GraphRedrawNeeded();

  return EC_OK;
}
