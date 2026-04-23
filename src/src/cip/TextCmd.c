/*
************************************************************************
*
*   TextCmd.c - commands for text manipulation
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
*   Date of last modification : 95/04/13
*   Pathname of SCCS file     : /sgiext/molmol/src/cip/SCCS/s.TextCmd.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "text_cmd.h"

#include <stdio.h>

#include <graph_text.h>
#include <cip.h>

static void
moveText(float x, float y)
{
  char buf[100];

  (void) sprintf(buf, "MovePrim %.5f %.5f 0.0", x, y);
  CipExecCmd(buf);
}

static void
resizeText(float s)
{
  char buf[100];

  (void) sprintf(buf, "SizeText rel %.5f", s);
  CipExecCmd(buf);
}

void
TextCmdInit(void)
{
  GraphTextSetMoveFunc(moveText);
  GraphTextSetResizeFunc(resizeText);
}
