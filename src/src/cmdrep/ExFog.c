/*
************************************************************************
*
*   ExFog.c - Fog command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExFog.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_rep.h>

#include <sg.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

#define ARG_NUM 4

ErrCode
ExFog(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[4];
  ErrCode errCode;
  int fogSwitch;
  float fogDensity, fogStart, fogEnd;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  fogSwitch = ParGetIntVal(PN_FOG);
  fogDensity = (float) ParGetDoubleVal(PN_FOG_DENSITY);
  fogStart = (float) ParGetDoubleVal(PN_FOG_START);
  fogEnd = (float) ParGetDoubleVal(PN_FOG_END);

  arg[0].prompt = "Fog";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 4;
  arg[0].v.intVal = fogSwitch;

  enumEntry[0].str = "off";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "exp";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "exp2";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "linear";
  enumEntry[3].onOff = FALSE;
  enumEntry[fogSwitch].onOff = TRUE;

  arg[1].prompt = "Density";
  arg[1].v.doubleVal = fogDensity;

  arg[2].prompt = "Start";
  arg[2].v.doubleVal = fogStart;

  arg[3].prompt = "End";
  arg[3].v.doubleVal = fogEnd;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  fogSwitch = arg[0].v.intVal;

  if (arg[1].v.doubleVal > 0.0)
    fogDensity = (float) arg[1].v.doubleVal;

  if (arg[2].v.doubleVal > 0.0)
    fogStart = (float) arg[2].v.doubleVal;

  if (arg[3].v.doubleVal > 0.0)
    fogEnd = (float) arg[3].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  ParSetIntVal(PN_FOG, fogSwitch);
  ParSetDoubleVal(PN_FOG_DENSITY, fogDensity);
  ParSetDoubleVal(PN_FOG_START, fogStart);
  ParSetDoubleVal(PN_FOG_END, fogEnd);

  SgSetFogMode(fogSwitch);
  SgSetFogPar(SG_FOG_DENSITY, fogDensity);
  SgSetFogPar(SG_FOG_START, fogStart);
  SgSetFogPar(SG_FOG_END, fogEnd);

  GraphRedrawNeeded();

  return EC_OK;
}
