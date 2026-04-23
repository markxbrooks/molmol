/*
************************************************************************
*
*   ExDrawPrec.c - DrawPrec command
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 98/08/18
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExDrawPrec.c
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

#define MAX_PREC 5

#define ARG_NUM 2

ErrCode
ExDrawPrec(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  int drawPrec, movePrec;

  arg[0].type = AT_INT;
  arg[1].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Drawing Precision";
  arg[0].v.intVal = ParGetIntVal(PN_DRAW_PREC);

  arg[1].prompt = "Moving Precision";
  arg[1].optional = TRUE;
  arg[1].v.intVal = ParGetIntVal(PN_MOVE_PREC);

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  drawPrec = arg[0].v.intVal;
  movePrec = arg[1].v.intVal;
  ArgCleanup(arg, ARG_NUM);

  if (drawPrec < 0)
    drawPrec = 0;
  else if (drawPrec > MAX_PREC)
    drawPrec = MAX_PREC;

  if (movePrec < 0)
    movePrec = 0;
  else if (movePrec > drawPrec)
    movePrec = drawPrec;

  ParSetIntVal(PN_DRAW_PREC, drawPrec);
  ParSetIntVal(PN_MOVE_PREC, movePrec);
  SgSetPrecision(drawPrec);

  GraphRedrawNeeded();

  return EC_OK;
}
