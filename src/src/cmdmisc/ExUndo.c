/*
************************************************************************
*
*   ExUndo.c - SetUndo and Undo commands
*
*   Copyright (c) 1995
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
*   Date of last modification : 96/12/11
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdmisc/SCCS/s.ExUndo.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <undo.h>
#include <cmd_misc.h>

#include <stdio.h>
#include <string.h>

#include <linlist.h>
#include <sg.h>
#include <prog_vers.h>
#include <par_names.h>
#include <par_hand.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <arg.h>
#include <graph_draw.h>

#define DEFAULT_LEVEL 1

#define ARG_NUM 1

typedef struct {
  UndoState state;
  int fileInd;
} StateDescr;

static int StateNo = 0;
static LINLIST StateList = NULL;

static void
getFileName(char *fileName, int fileInd)
{
  /* the user will only see this name in case of an error, it should
     be one that he/she is not likely to ever choose for a real file */
  
  (void) sprintf(fileName, "(undo-dump).%d", fileInd);
}

ErrCode
ExSetUndo(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  int level;
  StateDescr *descrP;
  char fileName[20];
  GFile gf;

  arg[0].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Level";
  if (ParDefined(PN_UNDO_LEVEL))
    arg[0].v.intVal = ParGetIntVal(PN_UNDO_LEVEL);
  else
    arg[0].v.intVal = DEFAULT_LEVEL;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  level = arg[0].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  if (level < 0)
    level = 0;

  ParSetIntVal(PN_UNDO_LEVEL, level);

  while (StateNo > level) {
    /* free memory of unused "files" */
    descrP = ListFirst(StateList);
    getFileName(fileName, descrP->fileInd);
    gf = GFileOpenNew(fileName, GF_FORMAT_MEMORY);
    if (gf != NULL)
      GFileClose(gf);
    StateNo--;
    ListRemove(StateList, descrP);
  }

  return EC_OK;
}

void
UndoSave(UndoState state)
{
  int level;
  StateDescr descr, *lastP;
  char fileName[20];
  GFile gf;
  BOOL ok;

  if (state == US_LEAVE)
    return;

  if (ParDefined(PN_UNDO_LEVEL))
    level = ParGetIntVal(PN_UNDO_LEVEL);
  else
    level = DEFAULT_LEVEL;

  if (level > 0)
    StateNo++;

  while (StateNo > level) {
    StateNo--;
    ListRemove(StateList, ListFirst(StateList));
  }

  if (level == 0)
    return;
  
  descr.state = state;

  lastP = ListLast(StateList);
  if (lastP == NULL)
    descr.fileInd = 0;
  else
    descr.fileInd = (lastP->fileInd + 1) % level;

  getFileName(fileName, descr.fileInd);
  gf = GFileOpenNew(fileName, GF_FORMAT_MEMORY);
  if (gf == NULL)
    return;

  ok = TRUE;

  if (state != US_NONE)
    ok = ok && ParDump(gf) == GF_RES_OK;

  if (state == US_ALL) {
    ok = ok && DhDump(gf);
    ok = ok && PrimDump(gf);
  }

  GFileClose(gf);

  if (! ok)
    return;

  if (StateList == NULL)
    StateList = ListOpen(sizeof(StateDescr));
  (void) ListInsertLast(StateList, &descr);
}

static void
addMol(DhMolP molP, void *clientData)
{
  GraphMolAdd(molP);
}

ErrCode
ExUndo(char *cmd)
{
  StateDescr *descrP;
  UndoState state;
  char fileName[20];
  GFile gf;
  BOOL ok;
  int i;

  if (StateNo == 0) {
    CipSetError("no saved state");
    return EC_ERROR;
  }

  descrP = ListLast(StateList);
  state = descrP->state;

  getFileName(fileName, descrP->fileInd);
  gf = GFileOpenRead(fileName);
  if (gf == NULL)
    return EC_ERROR;

  ok = TRUE;

  if (state != US_NONE) {
    ok = (ParUndump(gf) == GF_RES_OK);
    if (ok) {
      GraphDrawInit();  /* initialize with new parameters */
      for (i = 0; i < SG_FEATURE_NO; i++)
	SgSetFeature(i, ParGetIntArrVal(PN_RENDERING, i));
    }
  }

  if (state == US_ALL) {
    if (ok) {
      DhDestroyAll();
      PrimDestroyAll();
    }
    ok = ok && DhUndump(gf, VERS_DUMP);
    ok = ok && PrimUndump(gf, VERS_DUMP);
  }

  GFileClose(gf);

  ListRemove(StateList, descrP);
  StateNo--;

  if (! ok)
    return EC_ERROR;

  if (state != US_NONE) {
    DhApplyMol(PropGetRef(PROP_ALL, FALSE), addMol, NULL);
    GraphRedrawNeeded();
  }

  return EC_OK;
}
