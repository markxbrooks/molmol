/*
************************************************************************
*
*   ExEditText.c - EditText command
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
*   Date of last modification : 95/07/20
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdprim/SCCS/s.ExEditText.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arg.h>
#include <prim_hand.h>
#include <graph_draw.h>

static void
getPrim(PrimObjP primP, void *clientData)
{
  PrimObjP *primPP = clientData;

  *primPP = primP;
}

static void
setText(PrimObjP primP, void *clientData)
{
  DSTR str = clientData;

  PrimSetText(primP, DStrToStr(str));
}

#define ARG_NUM 1

ErrCode
ExEditText(char *cmd)
{
  PropRefP refP;
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PrimObjP primP;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  primP = NULL;
  PrimApply(PT_TEXT, refP, getPrim, &primP);

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "String";
  if (primP != NULL)
    DStrAssignStr(arg[0].v.strVal, PrimGetText(primP));

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  PrimApply(PT_TEXT, refP, setText, arg[0].v.strVal);

  ArgCleanup(arg, ARG_NUM);

  GraphRedrawNeeded();

  return EC_OK;
}
