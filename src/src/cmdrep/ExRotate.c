/*
************************************************************************
*
*   ExRotate.c - Rotate* commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExRotate.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_rep.h>

#include <string.h>

#include <mat_vec.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_transf.h>
#include <graph_draw.h>

static float RotAngle = 10.0f;

static void
molInit(DhMolP molP, void *clientData)
{
  Mat4 m;

  Mat4Ident(m);
  DhMolSetRotMat(molP, m);
}

ErrCode
ExRotateInit(char *cmd)
{
  DhApplyMol(PropGetRef(PROP_MOVABLE, FALSE), molInit, NULL);

  GraphRedrawNeeded();

  return EC_OK;
}

#define ARG_NUM 1

ErrCode
ExRotate(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Angle";
  arg[0].v.doubleVal = RotAngle;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  RotAngle = (float) arg[0].v.doubleVal;
  ArgCleanup(arg, ARG_NUM);

  if (strcmp(cmd, "RotateX") == 0)
    GraphRotateX(GraphDegToRad(RotAngle));
  else if (strcmp(cmd, "RotateY") == 0)
    GraphRotateY(GraphDegToRad(RotAngle));
  else
    GraphRotateZ(GraphDegToRad(RotAngle));

  GraphRedrawNeeded();

  return EC_OK;
}
