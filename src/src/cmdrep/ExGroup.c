/*
************************************************************************
*
*   ExGroup.c - Group command
*
*   Copyright (c) 1996
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
*   Date of last modification : 96/05/23
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdrep/SCCS/s.ExGroup.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_rep.h>

#include <stdio.h>
#include <stdlib.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>

#define ARG_NUM 1

ErrCode
ExGroup(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  int molNo, groupSize, startI;
  DhMolP *molPA;

  arg[0].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Group Size";
  arg[0].v.intVal = SelMolGet(NULL, 0);

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  groupSize = arg[0].v.intVal;
  if (groupSize <= 0)
    groupSize = 1;
  ArgCleanup(arg, ARG_NUM);

  molNo = SelMolGet(NULL, 0);
  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  startI = 0;
  while (startI < molNo) {
    if (startI + groupSize > molNo)
      groupSize = molNo - startI;
    
    DhMakeGroup(molPA + startI, groupSize);

    startI += groupSize;
  }

  free(molPA);

  return EC_OK;
}
