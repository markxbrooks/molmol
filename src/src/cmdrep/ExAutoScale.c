/*
************************************************************************
*
*   ExAutoScale.c - AutoScale command
*
*   Copyright (c) 1994-99
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
*   Date of last modification : 99/10/09
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExAutoScale.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_rep.h>

#include <string.h>

#include <graph_transf.h>
#include <graph_draw.h>

ErrCode
ExAutoScale(char *cmd)
{
  GraphAutoScale();
  GraphRedrawNeeded();

  return EC_OK;
}
