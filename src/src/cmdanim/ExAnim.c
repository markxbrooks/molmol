/*
************************************************************************
*
*   ExAnim.c - StartAnim and StopAnim commands
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
*   Date of last modification : 99/10/23
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdanim/SCCS/s.ExAnim.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_anim.h>

#include <stdio.h>
#include <stdlib.h>

#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

#define ARG_NUM 2
#define ENUM_SIZE 2

static BOOL Cycle = TRUE;
static int Delay = 50;
static BOOL Running = FALSE;
static int MolNo, MolInd;
static DhMolP *MolPA;

static void invalidMol(DhMolP, void *);

static void
stopAnim(void)
{
  PropRefP refP;
  int i;

  if (Running) {
    Running = FALSE;

    DhActivateGroups(TRUE);

    refP = PropGetRef(PROP_DISPLAYED, FALSE);
    for (i = 0; i < MolNo; i++)
      DhMolSetProp(refP, MolPA[i], TRUE);

    DhActivateGroups(FALSE);

    free(MolPA);

    DhRemoveMolInvalidCB(invalidMol, NULL);
    GraphRedrawNeeded();
  }
}

static void
invalidMol(DhMolP molP, void *clientData)
{
  stopAnim();
}

static void
showAnim(void *clientData)
{
  PropRefP refP;
  int i;

  if (MolInd == MolNo) {
    if (Cycle) {
      MolInd = 0;
    } else {
      stopAnim();
    }
  }

  if (! Running)
    return;
  
  DhActivateGroups(TRUE);

  refP = PropGetRef(PROP_DISPLAYED, FALSE);

  for (i = 0; i < MolNo; i++)
    DhMolSetProp(refP, MolPA[i], FALSE);
  
  DhMolSetProp(refP, MolPA[MolInd], TRUE);
  MolInd++;

  DhActivateGroups(FALSE);

  PuAddTimeOut(Delay, showAnim, NULL);

  GraphRedraw();
}

ErrCode
ExStartAnim(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[ENUM_SIZE];
  ErrCode errCode;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "once";
  enumEntry[0].onOff = ! Cycle;
  enumEntry[1].str = "cycle";
  enumEntry[1].onOff = Cycle;

  arg[0].prompt = "Animation";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = ENUM_SIZE;
  if (Cycle)
    arg[0].v.intVal = 1;
  else
    arg[0].v.intVal = 0;

  arg[1].prompt = "delay (ms)";
  arg[1].v.intVal = Delay;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  Cycle = (arg[0].v.intVal == 1);
  Delay = arg[1].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  MolInd = 0;

  if (Running)
    return EC_OK;

  DhActivateGroups(TRUE);

  MolNo = SelMolGet(NULL, 0);
  if (MolNo < 2) {
    CipSetError("at least 2 molecules must be selected");
    return EC_ERROR;
  }

  MolPA = malloc(MolNo * sizeof(DhMolP));
  (void) SelMolGet(MolPA, MolNo);

  DhActivateGroups(FALSE);

  Running = TRUE;

  DhAddMolInvalidCB(invalidMol, NULL);
  PuAddTimeOut(Delay, showAnim, NULL);

  return EC_OK;
}

ErrCode
ExStopAnim(char *cmd)
{
  stopAnim();

  return EC_OK;
}
