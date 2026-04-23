/*
************************************************************************
*
*   ExNameMol.c - NameMol command
*
*   Copyright (c) 1998
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
*   Date of last modification : 98/03/19
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExNameMol.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>
#include <stdlib.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>

#define ARG_NUM 3

ErrCode
ExNameMol(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  int molNo, molI;
  DhMolP molP, *molPA;
  int startI, digits;
  DSTR name;
  char form[10], buf[50];

  arg[0].type = AT_STR;
  arg[1].type = AT_INT;
  arg[2].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  molNo = SelMolGet(&molP, 1);

  arg[0].prompt = "Molecule Name";
  if (molNo > 0)
    DStrAssignDStr(arg[0].v.strVal, DhMolGetName(molP));

  arg[1].prompt = "Start Number";
  arg[1].v.intVal = 1;

  arg[2].prompt = "Digits";
  if (molNo > 1)
    arg[2].v.intVal = 3;
  else
    arg[2].v.intVal = 0;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  startI = arg[1].v.intVal;
  digits = arg[2].v.intVal;

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0) {
    CipSetError("no molecule selected");
    return EC_WARNING;
  }

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  name = DStrNew();
  (void) sprintf(form, "%%0%dd", digits);

  for (molI = 0; molI < molNo; molI++) {
    DStrAssignDStr(name, arg[0].v.strVal);
    if (digits > 0) {
      (void) sprintf(buf, form, startI + molI);
      DStrAppStr(name, buf);
    }
    DhMolSetName(molPA[molI], name);
  }

  DStrFree(name);
  free(molPA);

  ArgCleanup(arg, ARG_NUM);

  return EC_OK;
}
