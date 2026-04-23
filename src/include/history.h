/*
************************************************************************
*
*   history.h - history management
*
*   Copyright (c) 1997
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
*   Date of last modification : 97/02/24
*   Pathname of SCCS file     : /local/home/kor/molmol/include/SCCS/s.history.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#ifndef _HISTORY_H_
#define _HISTORY_H_

#include <bool.h>

typedef enum {
  HCR_EXECUTED,
  HCR_CANCELLED,
  HCR_ERROR
} HistoryCmdResult;

typedef void (* HistoryCB) (char *, void *);

extern void HistoryArgIdent(char *);

extern void HistoryArgStr(char *);

extern void HistoryArgExpr(char *);

extern void HistoryEndCmd(char *, HistoryCmdResult);

extern void HistoryComment(char *);

extern char *HistoryGetCmd(int);

extern void HistoryAddCB(HistoryCB, void *);

extern void HistoryRemoveCB(HistoryCB, void *);

extern void HistorySwitch(BOOL);

extern void HistoryLogWin(BOOL);

#endif   /* _HISTORY_H_ */
