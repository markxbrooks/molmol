/*
************************************************************************
*
*   MolInit.c - program initialization
*
*   Copyright (c) 1994-2000
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/main/SCCS/s.MolInit.c
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include "mol_init.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <strmatch.h>
#include <io.h>
#include <pu.h>
#include <sg.h>
#include <os_dir.h>
#include <g_file.h>
#include <prog_vers.h>
#include <prog_dir.h>
#include <curr_dir.h>
#include <user_file.h>
#include <setup_file.h>
#include <par_names.h>
#include <par_hand.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <graph_draw.h>
#include <graph_input.h>
#include <graph_text.h>
#include <graph_dobj.h>
#include <ui_menu.h>
#include <ui_cmd_field.h>
#include <ui_valuator.h>
#include <ui_show_sel.h>
#include <ui_switch.h>
#include <ui_tip.h>
#include <cip.h>
#include <arg.h>

static BOOL X11Features[SG_FEATURE_NO] = {0, 0, 1, 1, 0, 0, 0, 0};
static BOOL XGLFeatures[SG_FEATURE_NO] = {0, 0, 0, 1, 1, 1, 0, 1};
static BOOL GLFeatures[SG_FEATURE_NO] =  {0, 0, 0, 0, 1, 0, 0, 1};

static void
printMsg(void)
{
  (void) fprintf(stderr, "\n%s %s.%d", PROG_NAME, VERS_MAJOR, VERS_MINOR);
#if VERS_REV > 0
  (void) fprintf(stderr, ".%d", VERS_REV);
#endif
  (void) fprintf(stderr, " %s\n", VERS_STATE);
  (void) fprintf(stderr, "\n%s\n", PROG_COPYRIGHT);
}

static void
setFeatures(char *dev)
{
  BOOL *features;
  int i;

  if (ParDefined(PN_GRAPH_DEV) &&
      ParGetArrSize(PN_RENDERING) == SG_FEATURE_NO &&
      strcmp(ParGetStrVal(PN_GRAPH_DEV), dev) == 0) {
    /* same device, use values from dump file */
    for (i = 0; i < SG_FEATURE_NO; i++)
      SgSetFeature(i, ParGetIntArrVal(PN_RENDERING, i));
  } else {
    /* different device, use default values */
    if (strcmp(dev, "X11") == 0)
      features = X11Features;
    else if (strcmp(dev, "XGL") == 0)
      features = XGLFeatures;
    else
      features = GLFeatures;

    for (i = 0; i < SG_FEATURE_NO; i++) {
      ParSetIntArrVal(PN_RENDERING, i, features[i]);
      SgSetFeature(i, features[i]);
    }
  }
}

static void
dropCB(PuWindow win, void *clientData, PuDropCBStruc *callP)
{
  int maxLen, len;
  int fileI, i;
  DSTR cmd;
  char *name;

  maxLen = 0;
  for (fileI = 0; fileI < callP->fileNo; fileI++) {
    len = strlen(callP->fileNameA[fileI]);
    if (len > maxLen)
      maxLen = len;
  }

  cmd = DStrNew();
  name = malloc(maxLen + 1);

  for (fileI = 0; fileI < callP->fileNo; fileI++) {
    (void) strcpy(name, callP->fileNameA[fileI]);
    len = strlen(name);
    for (i = 0; i < len; i++)
      if (isupper(name[i]))
	name[i] = tolower(name[i]);

    if (StrMatch(name, "*.pdb"))
      DStrAssignStr(cmd, "ReadPdb");
    else if (StrMatch(name, "*.ent"))
      DStrAssignStr(cmd, "ReadPdb");
    else if (StrMatch(name, "*.cor"))
      DStrAssignStr(cmd, "ReadDg");
    else if (StrMatch(name, "*.dg"))
      DStrAssignStr(cmd, "ReadDg");
    else if (StrMatch(name, "*.mol2"))
      DStrAssignStr(cmd, "ReadSybyl");
    else if (StrMatch(name, "*.ml2"))
      DStrAssignStr(cmd, "ReadSybyl");
    else if (StrMatch(name, "*.xyz"))
      DStrAssignStr(cmd, "ReadXyz");
    else if (StrMatch(name, "*.ang"))
      DStrAssignStr(cmd, "ReadAng");
    else if (StrMatch(name, "*.seq"))
      DStrAssignStr(cmd, "ReadSeq");
    else
      DStrAssignStr(cmd, "ReadDump");

    DStrAppChar(cmd, ' ');
    DStrAppStr(cmd, name);

    (void) CipExecCmd(DStrToStr(cmd));
  }

  DStrFree(cmd);
  free(name);
}

static void
molInitCB(DhMolP molP, void *clientData)
{
  PropRefP newRefP;
  GFile gf;
  char *macName;

  newRefP = PropGetRef("new", TRUE);
  DhMolSetProp(newRefP, molP, TRUE);

  gf = SetupOpen("", "newmol.mac", FALSE);
  if (gf != NULL) {
    GFileClose(gf);
    macName = SetupGetName("", "newmol.mac", FALSE);
    (void) CipExecFile(macName);
    free(macName);
  }

  gf = UserFileOpenRead("newmol.mac");
  if (gf != NULL) {
    GFileClose(gf);
    macName = UserFileGetName("newmol.mac");
    (void) CipExecFile(macName);
    free(macName);
  }

  DhMolSetProp(newRefP, molP, FALSE);
}

static void
quitCB(void *clientData)
{
  GFile gf;
  BOOL ok;

  gf = UserFileOpenWrite("par", GF_FORMAT_ASCII);
  if (gf != NULL) {
    (void) ParDump(gf);
    GFileClose(gf);
  }

  if (ParDefined(PN_SAVE_STATE) && ParGetIntVal(PN_SAVE_STATE) == 0)
    return;

  (void) fprintf(stderr, "\nsaving program state\n");

  gf = UserFileOpenWrite("dump", GF_FORMAT_BINARY);
  /* This code is common with the WriteDump command. It would be nice to
     have it only once, but it doesn't fit into any existing module,
     and adding a new module just for these few lines would be overkill. */
  if (gf != NULL) {
    ok = (GFileWriteINT16(gf, VERS_DUMP) == GF_RES_OK);
    /* also dump parameters to make file compatible with dumps
       generated by WriteDump command */
    ok = (ok && ParDump(gf) == GF_RES_OK);
    ok = ok && DhDump(gf);
    ok = ok && PrimDump(gf);
    GFileClose(gf);

    if (! ok)
      (void) fprintf(stderr, "failed, please remove ~/molmol/dump\n");
  }
}

static void
handleFileError(char *fileName, char *errMsg)
{
  DSTR errStr;

  errStr = DStrNew();
  DStrAppStr(errStr, fileName);
  DStrAppStr(errStr, ": ");
  DStrAppStr(errStr, errMsg);
  CipSetError(DStrToStr(errStr));
  DStrFree(errStr);
}

static void
handleIOError(char *errMsg)
{
  CipSetError(errMsg);
}

static void
addMol(DhMolP molP, void *clientData)
{
  GraphMolAdd(molP);
}

static void
execStartupMac(void)
{
  char *progName, *userName, *optName;
  char *progDir, *userDir;
  char **progA, **userA;
  int progNo, userNo, i;

  progName = SetupGetName("", "startup.mac", FALSE);
  progDir = SetupGetName("", "startup", FALSE);
  OsDirGet(progDir, "*.mac", &progA, &progNo);

  userName = UserFileGetName("startup.mac");
  userDir = UserFileGetName("startup");
  OsDirGet(userDir, "*.mac", &userA, &userNo);

  optName = UserFileGetName("options.mac");

  CipCmdSeqBegin();

  (void) CipExecFile(progName);
  for (i = 0; i < progNo; i++)
    (void) CipExecFile(progA[i]);

  (void) CipExecFile(userName);
  for (i = 0; i < userNo; i++)
    (void) CipExecFile(userA[i]);

  (void) CipExecFile(optName);

  CipCmdSeqEnd();

  free(progName);
  free(progDir);
  OsDirFree(progA, progNo);

  free(userName);
  free(userDir);
  OsDirFree(userA, userNo);

  free(optName);
}

int
MolInit(char *defaultDev, int argc, char *argv[], char *macroName)
{
  char *dev;
  GFile gf;
  INT16 vers;
  BOOL ok;
  DSTR helpDir;

  printMsg();

  dev = getenv("MOLMOLDEV");
  if (dev == NULL)
    dev = defaultDev;
  (void) fprintf(stderr, "\nusing %s\n", dev);
  if (IOSetDevice(dev) != IO_RES_OK) {
    (void) fprintf(stderr, "unknown IO device\n");
    exit(1);
  }

  SgSetDoubleBuffer(TRUE);

  ProgDirSet("MOLMOLHOME", argv[0]);
  SetupSetDir("setup");
  CurrDirSet(ProgDirGet());
  UserFileSetDir(CurrDirGet(), "molmol");
  GFileSetErrorHandler(handleFileError);

  if (IOInit(PROG_NAME, &argc, argv) != IO_RES_OK) {
    (void) fprintf(stderr, "initialization of IO failed\n");
    exit(1);
  }

  IOSetErrorHandler(handleIOError);

  PuSetTextField(PU_TF_TITLE,
      "MOLMOL - MOLecule analysis and MOLecule display - JCU V1.0.6");

  gf = UserFileOpenRead("par");
  if (gf != NULL) {
    (void) ParUndump(gf);
    GFileClose(gf);
  }

  if (macroName == NULL)
    gf = UserFileOpenRead("dump");
  else
    gf = GFileOpenRead(macroName);

  if (gf != NULL) {
    ok = (GFileReadINT16(gf, &vers) == GF_RES_OK);
    if (ok && vers < VERS_DUMP_MIN) {
      (void) fprintf(stderr, "\ntoo old version of dump file, not read\n");
    } else if (ok && vers > VERS_DUMP) {
      (void) fprintf(stderr,
	  "\ntoo new version of dump file (corrupted?), not read\n");
    } else {
      ok = (ok && ParUndump(gf) == GF_RES_OK);
      ok = ok && DhUndump(gf, vers);
      ok = ok && PrimUndump(gf, vers);
      if (! ok) {
	(void) fprintf(stderr, "reading of dump file failed\n");
	(void) fprintf(stderr, "remove ~/molmol/dump and restart program\n");
      }
    }
    GFileClose(gf);
  }

  GraphDrawInit();
  GraphInputInit();
  GraphTextInit();
  GraphDrawobjInit();
  GraphRedrawEnable(FALSE);
  DhApplyMol(PropGetRef(PROP_ALL, FALSE), addMol, NULL);

  while (*dev != '/')
    dev++;
  dev++;
  setFeatures(dev);
  ParSetStrVal(PN_GRAPH_DEV, dev);

  UiMenuCreateBar();
  UiMenuCreatePopup();
  UiCmdFieldsCreate();
  UiValuatorCreate();
  UiShowSelInit();
  UiSwitch();

  if (ParDefined(PN_SWITCH_TIP)) {
    if (ParGetIntVal(PN_SWITCH_TIP))
      UiTipDisplay();
  } else {
    ParSetIntVal(PN_SWITCH_TIP, TRUE);
    UiTipDisplay();
  }

  helpDir = DStrNew();
  DStrAssignStr(helpDir, ProgDirGet());
  DStrAppStr(helpDir, "/help");
  CipInit(DStrToStr(helpDir));
  DStrFree(helpDir);

  CipAddUpdateCB(DhCallMolListCB, NULL);
  PuAddDropCB(dropCB, NULL, NULL);
  DhAddMolInitCB(molInitCB, NULL);

  if (! ParDefined(PN_SAVE_STATE))
    ParSetIntVal(PN_SAVE_STATE, 1);
  PuAddQuitCB(quitCB, NULL, NULL);

  GraphRedrawEnable(TRUE);

  execStartupMac();

  PuEventLoop();

  return 0;
}
