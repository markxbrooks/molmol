/*
************************************************************************
*
*   cip.h - command interpreter
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/07/24
*   Pathname of SCCS file     : /local/home/kor/molmol/include/SCCS/s.cip.h
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <bool.h>
#include <pu_types.h>
#include <cmd.h>

typedef enum {
  CM_NONE,
  CM_UNIQUE,
  CM_MULTIPLE
} CipMatch;

typedef void (*CipUpdateCB) (void *);

typedef void (*CipMenuAddCB) (char *, BOOL);

extern void CipInit(char *);

extern void CipAddUpdateCB(CipUpdateCB, void *);

extern void CipRemoveUpdateCB(CipUpdateCB, void *);

extern void CipShowHelpFile(PuGizmo, char *);

extern void CipSetMenu(char *, char *);

extern void CipSetMenuAddCB(CipMenuAddCB);

extern void CipHelpApropos(char *);

extern void CipShowHelpLine(char *);

extern ErrCode CipExecCmd(char *);

extern ErrCode CipExecFile(char *);

extern void CipCmdSeqBegin(void);

extern void CipCmdSeqEnd(void);
