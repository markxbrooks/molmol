/*
************************************************************************
*
*   ExText.c - AddTitle and AddText commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExText.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_input.h>
#include <graph_draw.h>

static float CurrSize = 0.05f;

#define ARG_NUM 4

ErrCode
ExAddTitle(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PrimObjP primP;
  float x0, y0;
  Vec3 x, dx;
  AttrP attrP;
  struct AttrS attr;

  GraphInputGetClick(&x0, &y0);

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Pos x";
  arg[0].v.doubleVal = x0;

  arg[1].prompt = "Pos y";
  arg[1].v.doubleVal = y0;

  arg[2].prompt = "Size";
  arg[2].v.doubleVal = CurrSize;

  arg[3].prompt = "String";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrSize = (float) arg[2].v.doubleVal;

  primP = PrimNew(PT_TEXT, NULL);

  x[0] = (float) arg[0].v.doubleVal;
  x[1] = (float) arg[1].v.doubleVal;
  x[2] = 0.0f;
  PrimSetPos(primP, x);
  Vec3Zero(dx);
  PrimSetVec(primP, dx);
  PrimSetText(primP, DStrToStr(arg[3].v.strVal));

  ArgCleanup(arg, ARG_NUM);

  attrP = PrimGetAttr(primP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.fontSize = CurrSize;
  PrimSetAttr(primP, AttrGet(&attr));

  PrimSetProp(PropGetRef("title", TRUE), primP, TRUE);
  PrimSetProp(PropGetRef("text", TRUE), primP, TRUE);
  
  GraphRedrawNeeded();

  return EC_OK;
}

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

#undef ARG_NUM
#define ARG_NUM 2

ErrCode
ExAddText(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  int molNo, molI, atomNo;
  DhMolP *molPA;
  PropRefP refP;
  PrimObjP primP;
  Vec3 x, dx;
  AttrP attrP;
  struct AttrS attr;

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Size";
  arg[0].v.doubleVal = CurrSize;

  arg[1].prompt = "String";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrSize = (float) arg[0].v.doubleVal;

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0) {
    CipSetError("no molecule selected");
    ArgCleanup(arg, ARG_NUM);
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

    primP = PrimNew(PT_TEXT, molPA[molI]);

    DhCalcCenter(x, molPA[molI]);
    PrimSetPos(primP, x);
    Vec3Zero(dx);
    PrimSetVec(primP, dx);

    PrimSetText(primP, DStrToStr(arg[1].v.strVal));

    attrP = PrimGetAttr(primP);
    AttrCopy(&attr, attrP);
    AttrReturn(attrP);
    attr.fontSize = CurrSize;
    PrimSetAttr(primP, AttrGet(&attr));
    
    PrimSetProp(PropGetRef("text", TRUE), primP, TRUE);
  }

  free(molPA);
  ArgCleanup(arg, ARG_NUM);

  GraphRedrawNeeded();

  return EC_OK;
}
