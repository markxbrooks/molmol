/*
************************************************************************
*
*   ExRend.c - Rendering command
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
*   Date of last modification : 96/03/05
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdrep/SCCS/s.ExRend.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_rep.h>

#include <sg.h>
#include <par_names.h>
#include <par_hand.h>
#include <arg.h>
#include <graph_draw.h>

ErrCode
ExRendering(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[SG_FEATURE_NO];
  ErrCode errCode;
  int i;

  arg.type = AT_MULT_ENUM;

  ArgInit(&arg, 1);

  arg.prompt = "Rendering Attributes";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.n = SG_FEATURE_NO;

  enumEntry[0].str = "Anti Aliasing";
  enumEntry[1].str = "Transparency";
  enumEntry[2].str = "Fast Spheres Drawing";
  enumEntry[3].str = "Backface Elimination";
  enumEntry[4].str = "Shading";
  enumEntry[5].str = "Two-sided Lighting";
  enumEntry[6].str = "Depth Sort";
  enumEntry[7].str = "Hidden Surface Elimination";

  for (i = 0; i < SG_FEATURE_NO; i++)
    enumEntry[i].onOff = ParGetIntArrVal(PN_RENDERING, i);

  errCode = ArgGet(&arg, 1);
  if (errCode != EC_OK) {
    ArgCleanup(&arg, 1);
    return errCode;
  }

  for (i = 0; i < SG_FEATURE_NO; i++) {
    ParSetIntArrVal(PN_RENDERING, i, enumEntry[i].onOff);
    SgSetFeature(i, enumEntry[i].onOff);
  }

  ArgCleanup(&arg, 1);

  GraphRedrawNeeded();

  return EC_OK;
}
