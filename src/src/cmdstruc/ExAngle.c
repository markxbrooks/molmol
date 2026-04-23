/*
************************************************************************
*
*   ExAngle.c - AddAngle and RemoveAngle commands
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
*   Date of last modification : 95/01/31
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdstruc/SCCS/s.ExAngle.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>

#define ARG_NUM 1

ErrCode
ExAddAngle(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  int selNo;
  DhAtomP atomPA[4];
  BOOL ok;

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Angle Name";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  selNo = SelAtomGet(atomPA, 4);
  if (selNo != 4) {
    ArgCleanup(arg, ARG_NUM);
    CipSetError("exactly 4 atoms must be selected");
    return EC_ERROR;
  }

  ok = DhAngleNew(atomPA[0], atomPA[1], atomPA[2], atomPA[3],
      arg[0].v.strVal);

  ArgCleanup(arg, ARG_NUM);

  if (! ok) {
    CipSetError("illegal atom selection");
    return EC_ERROR;
  }

  return EC_OK;
}

static void
removeAngle(DhAngleP angleP, void *clientData)
{
  DhAngleDestroy(angleP);
}

ErrCode
ExRemoveAngle(char *cmd)
{
  DhApplyAngle(PropGetRef(PROP_SELECTED, FALSE), removeAngle, NULL);

  return EC_OK;
}
