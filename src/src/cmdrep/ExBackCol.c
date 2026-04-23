/*
************************************************************************
*
*   ExBackCol.c - BackCol command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExBackCol.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_rep.h>

#include <stdio.h>

#include <sg.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

static void
drawNothing(void *clientData)
{
}

static BOOL
verifyCol(ArgDescrP argP)
{
  float r, g, b;

  if (argP->prompt[0] == 'R') {
    r = (float) argP->v.doubleVal;
    ParSetDoubleArrVal(PN_BACK_COLOR, 0, r);
    g = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 1);
    b = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 2);
  } else if (argP->prompt[0] == 'G') {
    r = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 0);
    g = (float) argP->v.doubleVal;
    ParSetDoubleArrVal(PN_BACK_COLOR, 1, g);
    b = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 2);
  } else {
    r = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 0);
    g = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 1);
    b = (float) argP->v.doubleVal;
    ParSetDoubleArrVal(PN_BACK_COLOR, 2, b);
  }

  SgSetBackgroundColor(r, g, b);
  GraphRedraw();

  return TRUE;
}

ErrCode
ExBackColor(char *cmd)
{
  ArgDescr arg[3];
  float oldCol[3];
  ErrCode errCode;
  int i;

  GraphShowAlt(drawNothing, NULL);

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;

  ArgInit(arg, 3);

  arg[0].prompt = "Red";
  arg[1].prompt = "Green";
  arg[2].prompt = "Blue";

  for (i = 0; i < 3; i++) {
    arg[i].verifyFunc = verifyCol;
    oldCol[i] = (float) ParGetDoubleArrVal(PN_BACK_COLOR, i);
    arg[i].v.doubleVal = oldCol[i];

    arg[i].useSlider = TRUE;
    arg[i].u.sliderD.minVal = 0.0f;
    arg[i].u.sliderD.maxVal = 1.0f;
    arg[i].u.sliderD.digits = 2;
  }

  errCode = ArgGet(arg, 3);
  ArgCleanup(arg, 3);

  if (errCode != EC_OK) {
    for (i = 0; i < 3; i++)
      ParSetDoubleArrVal(PN_BACK_COLOR, i, oldCol[i]);
    SgSetBackgroundColor(oldCol[0], oldCol[1], oldCol[2]);
  }

  GraphShowAlt(NULL, NULL);
  GraphRedrawNeeded();

  return errCode;
}
