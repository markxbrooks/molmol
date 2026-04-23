/*
************************************************************************
*
*   ExBondMode.c - BondMode command
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
*   Date of last modification : 95/07/06
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdrep/SCCS/s.ExBondMode.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_rep.h>

#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

#define ENUM_SIZE 2

ErrCode
ExBondMode(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[ENUM_SIZE];
  ErrCode errCode;
  int mode;
  int i;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  if (ParDefined(PN_BOND_MODE))
    mode = ParGetIntVal(PN_BOND_MODE);
  else
    mode = 0;

  arg.prompt = "Bond Display Mode";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.n = ENUM_SIZE;

  enumEntry[0].str = "normal";
  enumEntry[1].str = "pseudo";

  for (i = 0; i < ENUM_SIZE; i++)
    enumEntry[i].onOff = FALSE;
  
  enumEntry[mode].onOff = TRUE;
  arg.v.intVal = mode;

  errCode = ArgGet(&arg, 1);
  if (errCode != EC_OK) {
    ArgCleanup(&arg, 1);
    return errCode;
  }

  mode = arg.v.intVal;

  ArgCleanup(&arg, 1);

  ParSetIntVal(PN_BOND_MODE, mode);
  DhSetPseudoMode(mode == 1);

  GraphMolChanged(PROP_ALL);
  GraphRedrawNeeded();

  return EC_OK;
}
