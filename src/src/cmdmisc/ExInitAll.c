/*
************************************************************************
*
*   ExInitAll.c - InitAll command
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
*   Date of last modification : 95/06/29
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdmisc/SCCS/s.ExInitAll.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_misc.h>

#include <arg.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <graph_draw.h>

#define ENUM_SIZE 2

ErrCode
ExInitAll(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[ENUM_SIZE];
  ErrCode errCode;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  enumEntry[0].str = "yes";
  enumEntry[0].onOff = TRUE;
  enumEntry[1].str = "no";
  enumEntry[1].onOff = FALSE;

  arg.prompt = "Delete Everything?";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.n = ENUM_SIZE;
  arg.v.intVal = 0;

  errCode = ArgGet(&arg, 1);
  if (errCode != EC_OK || arg.v.intVal == 1) {
    ArgCleanup(&arg, 1);
    return errCode;
  }

  ArgCleanup(&arg, 1);

  DhDestroyAll();
  PrimDestroyAll();

  GraphRedrawNeeded();

  return EC_OK;
}
