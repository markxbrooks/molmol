/*
************************************************************************
*
*   ExWriteSec.c - WriteSecondary command
*
*   Copyright (c) 1994-95
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
*   Date of last modification : 96/04/21
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdio/SCCS/s.ExWriteSec.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>

#include <dstr.h>
#include <linlist.h>
#include <pu.h>
#include <prop_second.h>
#include <data_hand.h>
#include <data_sel.h>

#define NO_RES -999
#define MAX_SEC_NO 1000

typedef enum {
  SK_HELIX,
  SK_SHEET,
  SK_COIL
} SecKind;

typedef struct {
  DhResP *resPA;
  int resI;
} ResData;

typedef struct {
  int num;
  DSTR str;
} SheetData;

static PuTextWindow TextW;

static void
countRes(DhResP resP, void *clientData)
{
  (* (int *) clientData)++;
}

static void
getRes(DhResP resP, void *clientData)
{
  ResData *dataP = clientData;

  dataP->resPA[dataP->resI++] = resP;
}

static void
writeNeigh(int lStart, int lEnd, int rStart, int rEnd)
{
  char buf[20];

  if (lStart == NO_RES)
    return;

  PuWriteStr(TextW, "SetNeigh");
  (void) sprintf(buf, " %d", lStart);
  PuWriteStr(TextW, buf);
  (void) sprintf(buf, " %d", lEnd);
  PuWriteStr(TextW, buf);
  (void) sprintf(buf, " %d", rStart);
  PuWriteStr(TextW, buf);
  (void) sprintf(buf, " %d", rEnd);
  PuWriteStr(TextW, buf);
  PuWriteStr(TextW, "\n");
}

static void
writeSecondary(DhResP *resPA, int resNo)
{
  PropRefP helixRefP, sheetRefP, secRefP, sheetIRefP;
  SecKind kind, lastKind;
  DSTR helixStr, sheetStr;
  LINLIST sheetList;
  BOOL newElem;
  char prop[20], buf[20];
  int helixNum, sheetNum, strandNum, num;
  SheetData sheetS, *sheetP;
  int neighLStart, neighLEnd, neighRStart, neighREnd, neighNum;
  DhResP neighP;
  int startI, resI;

  helixRefP = PropGetRef(PROP_HELIX, FALSE);
  sheetRefP = PropGetRef(PROP_SHEET, FALSE);

  helixStr = DStrNew();
  sheetStr = DStrNew();
  sheetList = ListOpen(sizeof(SheetData));

  helixNum = 0;
  sheetNum = 0;
  strandNum = 0;

  for (resI = 0; resI <= resNo; resI++) {
    if (resI == resNo)
      kind = SK_COIL;
    else if (DhResGetProp(helixRefP, resPA[resI]))
      kind = SK_HELIX;
    else if (DhResGetProp(sheetRefP, resPA[resI]))
      kind = SK_SHEET;
    else
      kind = SK_COIL;
    
    newElem = FALSE;
    if (resI == 0) {
      newElem = TRUE;
    } else if (kind == SK_COIL) {
      if (lastKind != kind)
        newElem = TRUE;
    } else if (kind != lastKind) {
      newElem = TRUE;
    } else {
      if (! DhResGetProp(secRefP, resPA[resI]))
        newElem = TRUE;
    }

    if (newElem && resI > 0 && lastKind != SK_COIL) {
      PuWriteStr(TextW, "DefPropRes '");
      PuWriteStr(TextW, prop);
      PuWriteStr(TextW, "' 'num = ");
      (void) sprintf(buf, "%d", DhResGetNumber(resPA[startI]));
      PuWriteStr(TextW, buf);
      PuWriteStr(TextW, "..");
      (void) sprintf(buf, "%d", DhResGetNumber(resPA[resI - 1]));
      PuWriteStr(TextW, buf);
      PuWriteStr(TextW, "'\n");

      if (lastKind == SK_HELIX) {
	if (DStrLen(helixStr) > 0)
	  DStrAppStr(helixStr, " | ");
	DStrAppStr(helixStr, prop);
      } else if (lastKind == SK_SHEET) {
	writeNeigh(neighLStart, neighLEnd, neighRStart, neighREnd);

	sheetP = ListFirst(sheetList);
	while (sheetP != NULL) {
	  if (sheetP->num == sheetNum) {
	    DStrAppStr(sheetP->str, " | ");
	    DStrAppStr(sheetP->str, prop);
	    break;
	  }
	  sheetP = ListNext(sheetList, sheetP);
	}

	if (sheetP == NULL) {
	  sheetS.num = sheetNum;
	  sheetS.str = DStrNew();
	  DStrAssignStr(sheetS.str, prop);
	  (void) ListInsertLast(sheetList, &sheetS);

	  if (DStrLen(sheetStr) > 0)
	    DStrAppStr(sheetStr, " | ");
	  DStrAppStr(sheetStr, PROP_SHEET);
	  (void) sprintf(buf, "%d", sheetNum);
	  DStrAppStr(sheetStr, buf);
	}
      }
    }

    if (resI == resNo)
      break;

    if (newElem) {
      if (kind == SK_HELIX) {
	for (num = 1; num < MAX_SEC_NO; num++) {
	  (void) sprintf(prop, "%s%d", PROP_HELIX, num);
	  secRefP = PropGetRef(prop, FALSE);
	  if (DhResGetProp(secRefP, resPA[resI]))
	    break;
	}

	if (num == MAX_SEC_NO) {
	  helixNum++;
	  (void) sprintf(prop, "%s%d", PROP_HELIX, helixNum);
	  secRefP = helixRefP;
	} else {
	  helixNum = num;
	}
      } else if (kind == SK_SHEET) {
	for (num = 1; num < MAX_SEC_NO; num++) {
	  (void) sprintf(prop, "%s%d", PROP_SHEET, num);
	  sheetIRefP = PropGetRef(prop, FALSE);
	  if (DhResGetProp(sheetIRefP, resPA[resI]))
	    break;
	}

	if (num == MAX_SEC_NO) {
	  sheetNum++;
	  strandNum = 1;
	  (void) sprintf(prop, "%s%d_%d", PROP_STRAND, sheetNum, strandNum);
	  secRefP = sheetRefP;
	} else {
	  sheetNum = num;
	  for (num = 1; num < MAX_SEC_NO; num++) {
	    (void) sprintf(prop, "%s%d_%d", PROP_STRAND, sheetNum, num);
	    secRefP = PropGetRef(prop, FALSE);
	    if (DhResGetProp(secRefP, resPA[resI]))
	      break;
	  }
	  if (num == MAX_SEC_NO) {
	    strandNum++;
	    (void) sprintf(prop, "%s%d_%d", PROP_STRAND, sheetNum, strandNum);
	  } else {
	    strandNum = num;
	  }
	}

	neighLStart = NO_RES;
	neighLEnd = NO_RES;
	neighRStart = NO_RES;
	neighREnd = NO_RES;
      }

      startI = resI;
    }

    if (kind == SK_SHEET) {
      neighP = DhResGetNeigh(resPA[resI], NC_RIGHT);
      if (neighP == NULL) {
	writeNeigh(neighLStart, neighLEnd, neighRStart, neighREnd);
	neighLStart = NO_RES;
      } else {
	num = DhResGetNumber(resPA[resI]);
	neighNum = DhResGetNumber(neighP);

	if (neighLStart != NO_RES) {
	  if (num != neighLEnd + 1 ||
	      (neighNum != neighREnd + 1 && neighNum != neighREnd - 1) ||
	      (neighREnd > neighRStart && neighNum != neighREnd + 1) ||
	      (neighREnd < neighRStart && neighNum != neighREnd - 1)) {
	    writeNeigh(neighLStart, neighLEnd, neighRStart, neighREnd);
	    neighLStart = NO_RES;
	  } else {
	    neighLEnd = num;
	    neighREnd = neighNum;
	  }
	}

	if (neighLStart == NO_RES) {
	  neighLStart = num;
	  neighLEnd = num;
	  neighRStart = neighNum;
	  neighREnd = neighNum;
	}
      }
    }

    lastKind = kind;
  }

  PuWriteStr(TextW, "\nDefPropRes 'helix' '");
  if (DStrLen(helixStr) > 0)
    PuWriteStr(TextW, DStrToStr(helixStr));
  else
    PuWriteStr(TextW, "0");
  PuWriteStr(TextW, "'\n");

  sheetP = ListFirst(sheetList);
  while (sheetP != NULL) {
    PuWriteStr(TextW, "DefPropRes 'sheet");
    (void) sprintf(buf, "%d", sheetP->num);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, "' '");
    PuWriteStr(TextW, DStrToStr(sheetP->str));
    PuWriteStr(TextW, "'\n");
    DStrFree(sheetP->str);
    sheetP = ListNext(sheetList, sheetP);
  }

  PuWriteStr(TextW, "DefPropRes 'sheet' '");
  if (DStrLen(sheetStr) > 0)
    PuWriteStr(TextW, DStrToStr(sheetStr));
  else
    PuWriteStr(TextW, "0");
  PuWriteStr(TextW, "'\n");

  PuWriteStr(TextW, "DefPropRes 'coil' '! (helix | sheet)'\n");

  DStrFree(helixStr);
  DStrFree(sheetStr);
  ListClose(sheetList);
}

ErrCode
ExWriteSecondary(char *cmd)
{
  DhMolP molP;
  PropRefP refP;
  int resNo;
  ResData data;
  DSTR title;

  if (SelMolGet(&molP, 1) != 1) {
    CipSetError("exactly one molecule must be selected");
    return EC_ERROR;
  }

  refP = PropGetRef(PROP_SELECTED, FALSE);

  resNo = 0;
  DhMolApplyRes(refP, molP, countRes, &resNo);
  if (resNo < 1) {
    CipSetError("at least one residue must be selected");
    return EC_ERROR;
  }

  data.resPA = malloc(resNo * sizeof(*data.resPA));
  data.resI = 0;
  DhMolApplyRes(refP, molP, getRes, &data);

  title = DStrNew();
  DStrAssignStr(title, cmd);
  DStrAppStr(title, ".mac");
  TextW = PuCreateTextWindow(DStrToStr(title));
  DStrFree(title);

  writeSecondary(data.resPA, resNo);

  free(data.resPA);

  return EC_OK;
}
