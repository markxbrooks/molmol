/*
************************************************************************
*
*   ExSetAngle.c - SetAngle command
*
*   Copyright (c) 1994
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExSetAngle.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_struc.h>

#include <string.h>

#include <arg.h>
#include <data_hand.h>
#include <graph_draw.h>

static float CurrAng = 0.0f;

static void
setAng(DhAngleP angleP, void *clientData)
{
  DhAngleSetVal(angleP, CurrAng);
}

static void
recalcCoord(DhMolP molP, void *clientData)
{
  DhMolAnglesChanged(molP);
}

#define ARG_NUM 1

ErrCode
ExSetAngle(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PropRefP refP;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Angle";
  arg[0].v.doubleVal = CurrAng;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrAng = (float) arg[0].v.doubleVal;
  ArgCleanup(arg, ARG_NUM);

  refP = PropGetRef(PROP_SELECTED, FALSE);
  DhApplyAngle(refP, setAng, NULL);
  DhApplyMol(refP, recalcCoord, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
