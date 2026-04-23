/*
************************************************************************
*
*   ExStereo.c - Stereo command
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
*   Date of last modification : 95/11/24
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdrep/SCCS/s.ExStereo.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_rep.h>

#include <sg.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

#define ENUM_SIZE 6

ErrCode
ExStereo(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[ENUM_SIZE];
  ErrCode errCode;
  StereoMode mode;
  int i;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  mode = ParGetIntVal(PN_STEREO_MODE);

  arg.prompt = "Stereo Display";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.lineNo = 2;
  arg.u.enumD.n = ENUM_SIZE;

  enumEntry[0].str = "off";
  enumEntry[1].str = "left";
  enumEntry[2].str = "right";
  enumEntry[3].str = "side_by_side";
  enumEntry[4].str = "cross_eye";
  enumEntry[5].str = "hardware";

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

  ParSetIntVal(PN_STEREO_MODE, mode);
  SgSetStereo(mode == SM_HARDWARE);

  GraphRedrawNeeded();

  return EC_OK;
}
