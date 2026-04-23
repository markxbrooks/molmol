/*
************************************************************************
*
*   SgBuff.c - Sg double buffering
*
*   Copyright (c) 1994-2001
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgBuff.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <sg_dev.h>

static BOOL DoubleBuffOn = FALSE;

void
SgSetDoubleBuffer(BOOL onOff)
{
  DoubleBuffOn = onOff;
}

BOOL
SgGetDoubleBuffer(void)
{
  return DoubleBuffOn;
}

void
SgSetBuffer(SgBuffer buf)
{
  CurrSgDevP->setBuffer(buf);
}

void
SgFlushFrame(void)
{
  CurrSgDevP->flushFrame();
}

void
SgEndFrame(void)
{
  CurrSgDevP->calcHidden();
  CurrSgDevP->endFrame();
}

BOOL
SgRefresh(void)
{
  return CurrSgDevP->refresh();
}
