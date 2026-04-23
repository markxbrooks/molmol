/*
************************************************************************
*
*   Def.c - dummy functions for Sg device
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/def/SCCS/s.Def.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <sg_def.h>

void
SgDefClear(void)
{
}

void
SgDefSetBuffer(SgBuffer buf)
{
}

void
SgDefFlushFrame(void)
{
}

void
SgDefEndFrame(void)
{
}

BOOL
SgDefRefresh(void)
{
  return FALSE;  /* cannot refresh */
}

void
SgDefSetFeature(SgFeature feature, BOOL onOff)
{
}

void
SgDefSetColor(float r, float g, float b, float a, BOOL fg)
{
}

void
SgDefSetTexture(char *name)
{
}
