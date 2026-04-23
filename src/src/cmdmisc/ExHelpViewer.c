/*
************************************************************************
*
*   ExHelpViewer.c - HelpViewer commands
*
*   Copyright (c) 1999
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
*   Date of last modification : 99/11/06
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdmisc/SCCS/s.ExHelpViewer.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_misc.h>

#include <stdlib.h>
#include <string.h>

#include <dstr.h>
#include <os_browse.h>
#include <par_names.h>
#include <par_hand.h>
#include <arg.h>

#define ARG_NUM 4

ErrCode
ExHelpViewer(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  char *viewer;
  int viewI;
  EnumEntryDescr viewEntry[2];
  char *currStartCmd, *currSpec;
  int browseNo, currBrowseI, browseI;
  EnumEntryDescr *browseEntry;
  char *name, *startCmd, *spec;
  ErrCode errCode;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_ENUM;
  arg[2].type = AT_STR;
  arg[3].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  if (ParDefined(PN_HELP_VIEWER)) {
    viewer = ParGetStrVal(PN_HELP_VIEWER);
    if (viewer[0] == '\0' || strcmp(viewer, "builtin") == 0)
      viewI = 0;
    else
      viewI = 1;
  } else {
    viewI = 0;
  }

  viewEntry[0].str = "builtin";
  viewEntry[0].onOff = FALSE;
  viewEntry[1].str = "browser";
  viewEntry[1].onOff = FALSE;

  viewEntry[viewI].onOff = TRUE;

  arg[0].prompt = "Help Viewer";
  arg[0].u.enumD.entryP = viewEntry;
  arg[0].u.enumD.n = 2;
  arg[0].v.intVal = viewI;

  if (ParDefined(PN_BROWSE_CMD))
    currStartCmd = ParGetStrVal(PN_BROWSE_CMD);
  else
    currStartCmd = "";

  if (ParDefined(PN_BROWSE_SPEC))
    currSpec = ParGetStrVal(PN_BROWSE_SPEC);
  else
    currSpec = "";

  browseNo = OsBrowseGetListSize();
  browseEntry = malloc((browseNo + 1) * sizeof(*browseEntry));
  currBrowseI = browseNo;
  for (browseI = 0; browseI < browseNo; browseI++) {
    OsBrowseGetListEntry(browseI, &name, &startCmd, &spec);
    browseEntry[browseI].str = name;
    browseEntry[browseI].onOff = FALSE;
    if (strcmp(currStartCmd, startCmd) == 0 &&
        strcmp(currSpec, spec) == 0)
      currBrowseI = browseI;
  }
  browseEntry[browseNo].str = "custom";
  browseEntry[browseNo].onOff = FALSE;
  browseEntry[currBrowseI].onOff = TRUE;

  arg[1].prompt = "Browser";
  arg[1].u.enumD.entryP = browseEntry;
  arg[1].u.enumD.n = browseNo + 1;
  arg[1].v.intVal = currBrowseI;

  arg[2].prompt = "Start Command";
  if (ParDefined(PN_BROWSE_CMD))
    DStrAssignStr(arg[2].v.strVal, ParGetStrVal(PN_BROWSE_CMD));

  arg[3].prompt = OsBrowseGetSpecPrompt();
  if (ParDefined(PN_BROWSE_SPEC))
    DStrAssignStr(arg[3].v.strVal, ParGetStrVal(PN_BROWSE_SPEC));

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  if (arg[0].v.intVal == 0)
    ParSetStrVal(PN_HELP_VIEWER, "builtin");
  else
    ParSetStrVal(PN_HELP_VIEWER, "browser");

  browseI = arg[1].v.intVal;
  if (browseI == browseNo) {
    ParSetStrVal(PN_BROWSE_CMD, DStrToStr(arg[2].v.strVal));
    ParSetStrVal(PN_BROWSE_SPEC, DStrToStr(arg[3].v.strVal));
  } else {
    OsBrowseGetListEntry(browseI, &name, &startCmd, &spec);
    ParSetStrVal(PN_BROWSE_CMD, startCmd);
    ParSetStrVal(PN_BROWSE_SPEC, spec);
  }

  ArgCleanup(arg, ARG_NUM);

  return EC_OK;
}
