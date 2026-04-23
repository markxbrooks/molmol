/*
************************************************************************
*
*   cmd_p.h - private functions for Cip
*
*   Copyright (c) 1994
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
*   Date of last modification : 96/02/08
*   Pathname of SCCS file     : /sgiext/molmol/src/cip/SCCS/s.cmd_p.h
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <dstr.h>
#include <linlist.h>

#include <cmd.h>
#include <undo.h>

typedef ErrCode (* CmdExecF) (char *);

typedef struct {
  DSTR name;
  DSTR prompt;
  DSTR defaultVal;
  DSTR val;
} ArgDef;

typedef struct {
  DSTR fileName;
  LINLIST argList;
  LINLIST cmdSeq;
} CmdDef;

typedef struct {
  char *cmd;
  CmdExecF func;
  UndoState undo;
  char *menu;
  char *helpLine;
  CmdDef *defP;
} CmdTabEntry, *CmdTabEntryP;
