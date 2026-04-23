/*
************************************************************************
*
*   DrawobjCmd.c - commands for draw object manipulation
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
*   Date of last modification : 95/06/16
*   Pathname of SCCS file     : /sgiext/molmol/src/cip/SCCS/s.DrawobjCmd.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "drawobj_cmd.h"

#include <stdio.h>

#include <graph_dobj.h>
#include <cip.h>

static void
modifyDrawobj(int ind, float x, float y)
{
  char buf[100];

  (void) sprintf(buf, "MoveDrawobj %d %.5f %.5f 0.0", ind, x, y);
  CipExecCmd(buf);
}

void
DrawobjCmdInit(void)
{
  GraphDrawobjSetModifyFunc(modifyDrawobj);
}
