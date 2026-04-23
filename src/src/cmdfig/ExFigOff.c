/*
************************************************************************
*
*   ExFigOff.c - FigOff command
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/12/24
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdfig/SCCS/s.ExFigOff.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_fig.h>

#include <stdio.h>

#include "fig_util.h"

ErrCode
ExFigOff(char *cmd)
{
  FigOff();

  return EC_OK;
}
