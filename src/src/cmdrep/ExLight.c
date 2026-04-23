/*
************************************************************************
*
*   ExLight.c - Light command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExLight.c
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
ExLight(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[3];
  ErrCode errCode;
  int lightModel;
  float lightPos[3];
  int i;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  lightModel = ParGetIntVal(PN_LIGHT);

  arg[0].prompt = "Lighting Model";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 3;
  arg[0].v.intVal = lightModel;

  enumEntry[0].str = "off";
  enumEntry[1].str = "point";
  enumEntry[2].str = "infinite";

  for (i = 0; i < 3; i++)
    enumEntry[i].onOff = (lightModel == i);

  arg[1].prompt = "Position x";
  arg[2].prompt = "Position y";
  arg[3].prompt = "Position z";

  for (i = 0; i < 3; i++)
    arg[1 + i].v.doubleVal = ParGetDoubleArrVal(PN_LIGHT_POS, i);

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  lightModel = arg[0].v.intVal;
  ParSetIntVal(PN_LIGHT, lightModel);

  for (i = 0; i < 3; i++) {
    lightPos[i] = (float) arg[1 + i].v.doubleVal;
    ParSetDoubleArrVal(PN_LIGHT_POS, i, lightPos[i]);
  }

  ArgCleanup(arg, ARG_NUM);

  SgSetLightPosition(lightPos);
  SgSetLight(lightModel);

  GraphRedrawNeeded();

  return EC_OK;
}
