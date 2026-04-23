/*
************************************************************************
*
*   ExRock.c - StartRock and StopRock commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdanim/SCCS/s.ExRock.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_anim.h>

#include <stdio.h>

#include <pu.h>
#include <arg.h>
#include <graph_transf.h>
#include <graph_draw.h>

static BOOL Cycle = TRUE;
static int Delay = 50;
static float XAng1 = 0.0f, YAng1 = 10.0f, ZAng1 = 0.0f;
static int Steps1 = 18;
static float XAng2 = 0.0f, YAng2 = -10.0f, ZAng2 = 0.0f;
static int Steps2 = 18;
static int Step = 0;
static BOOL Running = FALSE;

static void
doRock(void *clientData)
{
  float xAng, yAng, zAng;

  if (Step == Steps1 + Steps2) {
    if (Cycle) {
      Step = 0;
    } else {
      Running = FALSE;
    }
  }

  if (! Running)
    return;
  
  if (Step < Steps1) {
    xAng = XAng1;
    yAng = YAng1;
    zAng = ZAng1;
  } else {
    xAng = XAng2;
    yAng = YAng2;
    zAng = ZAng2;
  }

  Step++;

  if (xAng != 0.0f)
    GraphRotateX(GraphDegToRad(xAng));
  if (yAng != 0.0f)
    GraphRotateY(GraphDegToRad(yAng));
  if (zAng != 0.0f)
    GraphRotateZ(GraphDegToRad(zAng));

  PuAddTimeOut(Delay, doRock, NULL);

  GraphRedraw();
}

#define ARG_NUM 10
#define ENUM_SIZE 2

ErrCode
ExStartRock(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[ENUM_SIZE];
  ErrCode errCode;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_INT;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;
  arg[4].type = AT_DOUBLE;
  arg[5].type = AT_INT;
  arg[6].type = AT_DOUBLE;
  arg[7].type = AT_DOUBLE;
  arg[8].type = AT_DOUBLE;
  arg[9].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "once";
  enumEntry[0].onOff = ! Cycle;
  enumEntry[1].str = "cycle";
  enumEntry[1].onOff = Cycle;

  arg[0].prompt = "Rock";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = ENUM_SIZE;
  if (Cycle)
    arg[0].v.intVal = 1;
  else
    arg[0].v.intVal = 0;

  arg[1].prompt = "delay (ms)";
  arg[1].v.intVal = Delay;

  arg[2].prompt = "Phase 1, X rotation";
  arg[2].v.doubleVal = XAng1;
  arg[3].prompt = "Phase 1, Y rotation";
  arg[3].v.doubleVal = YAng1;
  arg[4].prompt = "Phase 1, Z rotation";
  arg[4].v.doubleVal = ZAng1;
  arg[5].prompt = "Phase 1, steps";
  arg[5].v.intVal = Steps1;

  arg[6].prompt = "Phase 2, X rotation";
  arg[6].v.doubleVal = XAng2;
  arg[7].prompt = "Phase 2, Y rotation";
  arg[7].v.doubleVal = YAng2;
  arg[8].prompt = "Phase 2, Z rotation";
  arg[8].v.doubleVal = ZAng2;
  arg[9].prompt = "Phase 2, steps";
  arg[9].v.intVal = Steps2;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  Cycle = (arg[0].v.intVal == 1);
  Delay = arg[1].v.intVal;
  XAng1 = (float) arg[2].v.doubleVal;
  YAng1 = (float) arg[3].v.doubleVal;
  ZAng1 = (float) arg[4].v.doubleVal;
  Steps1 = arg[5].v.intVal;
  XAng2 = (float) arg[6].v.doubleVal;
  YAng2 = (float) arg[7].v.doubleVal;
  ZAng2 = (float) arg[8].v.doubleVal;
  Steps2 = arg[9].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  Step = 0;

  if (! Running) {
    Running = TRUE;
    PuAddTimeOut(Delay, doRock, NULL);
  }

  return EC_OK;
}

ErrCode
ExStopRock(char *cmd)
{
  Running = FALSE;

  return EC_OK;
}
