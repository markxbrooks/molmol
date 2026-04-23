/*
************************************************************************
*
*   ExProj.c - Projection command
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdrep/SCCS/s.ExProj.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_rep.h>

#include <sg.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

ErrCode
ExProjection(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;
  int proj;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  proj = ParGetIntVal(PN_PROJECTION);

  arg.prompt = "Projection";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.n = 2;
  arg.v.intVal = proj;

  enumEntry[0].str = "orthogonal";
  enumEntry[0].onOff = (proj == 0);
  enumEntry[1].str = "perspective";
  enumEntry[1].onOff = (proj == 1);

  errCode = ArgGet(&arg, 1);
  if (errCode != EC_OK) {
    ArgCleanup(&arg, 1);
    return errCode;
  }

  proj = arg.v.intVal;

  ArgCleanup(&arg, 1);

  ParSetIntVal(PN_PROJECTION, proj);
  SgSetProjection(proj);

  SgUpdateView();
  GraphRedrawNeeded();

  return EC_OK;
}
