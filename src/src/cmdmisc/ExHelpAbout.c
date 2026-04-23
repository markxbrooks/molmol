/*
************************************************************************
*
*   ExHelpAbout.c - HelpAbout command
*
*   Copyright (c) 1997-2000
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
*   Date of last modification : 00/03/11
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdmisc/SCCS/s.ExHelpAbout.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_misc.h>

#include <stdio.h>

#include <dstr.h>
#include <sg.h>
#include <pu.h>
#include <prog_vers.h>

ErrCode
ExHelpAbout(char *cmd)
{
  DSTR msgStr;
  char buf[50];

  msgStr = DStrNew();

  DStrAppStr(msgStr, PROG_NAME);
  (void) sprintf(buf, " %s.%d", VERS_MAJOR, VERS_MINOR);
  DStrAppStr(msgStr, buf);
#if VERS_REV > 0
  (void) sprintf(buf, ".%d", VERS_REV);
  DStrAppStr(msgStr, buf);
#endif
  DStrAppStr(msgStr, " ");
  DStrAppStr(msgStr, VERS_STATE);
  DStrAppStr(msgStr, "\n\n");
  DStrAppStr(msgStr, PROG_COPYRIGHT);
  DStrAppStr(msgStr, "\n\nGraphics Device: ");
  DStrAppStr(msgStr, SgGetDeviceName());

  PuShowMessage(PU_MT_INFO, cmd, DStrToStr(msgStr));

  DStrFree(msgStr);

  return EC_OK;
}
