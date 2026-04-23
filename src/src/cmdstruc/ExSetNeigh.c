/*
************************************************************************
*
*   ExSetNeigh.c - SetNeigh command
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
*   Date of last modification : 95/05/23
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdstruc/SCCS/s.ExSetNeigh.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>

#include <arg.h>
#include <data_hand.h>

typedef struct {
  int start1, end1, start2;
  BOOL parallel;
} NeighData;

static void
setNeigh(DhMolP molP, void *clientData)
{
  NeighData *dataP = clientData;
  int i1, i2;
  DhResP res1P, res2P;

  i2 = dataP->start2;
  for (i1 = dataP->start1; i1 <= dataP->end1; i1++) {
    res1P = DhResFind(molP, i1);
    res2P = DhResFind(molP, i2);

    if (res1P != NULL && res2P != NULL)
      DhResSetNeigh(res1P, NC_RIGHT, res2P);

    if (dataP->parallel)
      i2++;
    else
      i2--;
  }
}

#define ARG_NUM 4

ErrCode
ExSetNeigh(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  NeighData data;
  int end2, resNo1, resNo2;

  arg[0].type = AT_INT;
  arg[1].type = AT_INT;
  arg[2].type = AT_INT;
  arg[3].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Left Start Res";
  arg[1].prompt = "Left End Res";
  arg[2].prompt = "Right Start Res";
  arg[3].prompt = "Right End Res";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.start1 = arg[0].v.intVal;
  data.end1 = arg[1].v.intVal;
  data.start2 = arg[2].v.intVal;
  end2 = arg[3].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  if (data.end1 < data.start1) {
    CipSetError("end of left strand must not be less than start");
    return EC_ERROR;
  }

  if ((data.start2 >= data.start1 || end2 >= data.start1) &&
      (data.start2 <= data.end1 || end2 <= data.end1)) {
    CipSetError("strands must not overlap");
    return EC_ERROR;
  }

  resNo1 = data.end1 - data.start1 + 1;
  if (end2 >= data.start2) {
    resNo2 = end2 - data.start2 + 1;
    data.parallel = TRUE;
  } else {
    resNo2 = data.start2 - end2 + 1;
    data.parallel = FALSE;
  }

  if (resNo1 != resNo2) {
    CipSetError("strands must have equal size");
    return EC_ERROR;
  }

  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), setNeigh, &data);

  return EC_OK;
}
