/*
************************************************************************
*
*   ExHelpTip.c - HelpTip command
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
*   Date of last modification : 98/07/20
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/cmdmisc/SCCS/s.ExHelpTip.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_misc.h>

#include <ui_tip.h>

ErrCode
ExHelpTip(char *cmd)
{
  UiTipDisplay();

  return EC_OK;
}
