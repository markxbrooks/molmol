/*
************************************************************************
*
*   ExHelpHtml.c - HelpHtml command
*
*   Copyright (c) 1995-99
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
*   Date of last modification : 00/03/18
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdmisc/SCCS/s.ExHelpHtml.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_misc.h>

#include <stdio.h>
#include <string.h>

#include <dstr.h>
#include <os_browse.h>
#include <prog_dir.h>
#include <par_names.h>
#include <par_hand.h>

ErrCode
ExHelpHtml(char *cmd)
{
  char *fileName;
  DSTR fullName;
  OsBrowseRes browseRes;

  if (! ParDefined(PN_BROWSE_CMD) || ! ParDefined(PN_BROWSE_SPEC)) {
    CipSetError("web browser undefined, set with HelpViewer command");
    return EC_ERROR;
  }

  if (strcmp(cmd, "HelpIndex") == 0)
    fileName = "help/html/index.html";
  else if (strcmp(cmd, "HelpManual") == 0)
    fileName = "man/index.html";
  else
    fileName = "man/tutorial_idx.html";

  fullName = DStrNew();
  DStrAssignStr(fullName, ProgDirGet());
  DStrAppChar(fullName, '/');
  DStrAppStr(fullName, fileName);

  browseRes = OsBrowseShow(
      ParGetStrVal(PN_BROWSE_CMD),
      ParGetStrVal(PN_BROWSE_SPEC),
      DStrToStr(fullName));

  DStrFree(fullName);

  if (browseRes != OBR_OK) {
    CipSetError("web browser display failed");
    return EC_ERROR;
  }

  return EC_OK;
}
