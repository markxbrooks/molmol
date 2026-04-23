/*
************************************************************************
*
*   ExReadLib.c - ReadLib command
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
*   Date of last modification : 96/12/06
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdio/SCCS/s.ExReadLib.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>

#include <pu.h>
#include <data_lib.h>

static PuTextWindow TextW;

static void
writeStr(char *str)
{
  PuWriteStr(TextW, str);
}

ErrCode
ExReadLib(char *cmd)
{
  TextW = PuCreateTextWindow(cmd);

  DhResLibRead(writeStr);

  return EC_OK;
}
