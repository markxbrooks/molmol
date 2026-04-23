/*
************************************************************************
*
*   ExMove.c - Move* commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExMove.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_rep.h>

#include <string.h>

#include <arg.h>
#include <graph_transf.h>
#include <graph_draw.h>

static float MoveDist = 1.0f;

static void
molInit(DhMolP molP, void *clientData)
{
  Vec3 v;

  Vec3Zero(v);
  DhMolSetRotPoint(molP, v);
  DhMolSetTransVect(molP, v);
}

ErrCode
ExMoveInit(char *cmd)
{
  DhApplyMol(PropGetRef(PROP_MOVABLE, FALSE), molInit, NULL);

  GraphRedrawNeeded();

  return EC_OK;
}

#define ARG_NUM 1

ErrCode
ExMove(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  Vec3 moveV;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Distance";
  arg[0].v.doubleVal = MoveDist;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  MoveDist = (float) arg[0].v.doubleVal;
  ArgCleanup(arg, ARG_NUM);

  Vec3Zero(moveV);

  if (strcmp(cmd, "MoveX") == 0)
    moveV[0] = MoveDist;
  else if (strcmp(cmd, "MoveY") == 0)
    moveV[1] = MoveDist;
  else
    moveV[2] = MoveDist;

  GraphMove(moveV);

  GraphRedrawNeeded();

  return EC_OK;
}
