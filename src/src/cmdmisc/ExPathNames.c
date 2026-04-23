/*
************************************************************************
*
*   ExPathNames.c - PathNames command
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
*   Date of last modification : 96/04/16
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdmisc/SCCS/s.ExPathNames.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_misc.h>

#include <stdio.h>

#include <dstr.h>
#include <par_names.h>
#include <par_hand.h>
#include <arg.h>

#define PAR_NUM 9

static char *ParNames[] = {
  PN_ATOM_RADIUS,
  PN_ATOM_CHARGE,
  PN_FLIP_ATOMS,
  PN_RING_SHIFT,
  PN_PDB_ATOMS,
  PN_PROP_DEF,
  PN_RES_LIB,
  PN_MENU_DIR,
  PN_COLOR_LIST
};

ErrCode
ExPathNames(char *cmd)
{
  ArgDescr arg[PAR_NUM];
  ErrCode errCode;
  char *s;
  int i;

  for (i = 0; i < PAR_NUM; i++)
    arg[i].type = AT_STR;
  
  ArgInit(arg, PAR_NUM);

  for (i = 0; i < PAR_NUM; i++) {
    arg[i].prompt = ParNames[i];
    if (ParDefined(ParNames[i]))
      DStrAssignStr(arg[i].v.strVal, ParGetStrVal(ParNames[i]));
  }

  errCode = ArgGet(arg, PAR_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, PAR_NUM);
    return errCode;
  }

  for (i = 0; i < PAR_NUM; i++) {
    s = DStrToStr(arg[i].v.strVal);
    if (s[0] == '\0')
      ParUndefine(ParNames[i]);
    else
      ParSetStrVal(ParNames[i], s);
  }

  ArgCleanup(arg, PAR_NUM);

  return errCode;
}
