/*
************************************************************************
*
*   ExReadDg.c - ReadDg and ReadListDg commands
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadDg.c
*   SCCS identification       : 1.19
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <break.h>
#include <dstr.h>
#include <file_name.h>
#include <linlist.h>
#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>
#include "unknown_err.h"

#define LINE_LEN 100

typedef struct {
  DhResP res1P;
  int atom1I, atom2I;
} BondListEntry;

static void
setInvalid(DhAtomP atomP, void *clientData)
{
  DhAtomSetCoord(atomP, NULL);
}

static void
getLinePart(char *subBuf, char *lineBuf, int start, int len)
{
  /* remove leading blanks */
  while (len > 0 && lineBuf[start] == ' ') {
    start++;
    len--;
  }

  (void) strncpy(subBuf, lineBuf + start, len);

  /* remove trailing blanks */
  while (len > 0 && subBuf[len - 1] == ' ')
    len--;

  subBuf[len] = '\0';
}

static ErrCode
readOneMol(DhMolP molP, GFile gf, char *lineBuf, char *molName,
    int *molIP, Vec3 cent)
{
  GFileRes res;
  BOOL isNewMol;
  ErrCode errCode;
  LINLIST bondL;
  DhResDefP resDefP;
  BOOL isNewRes;
  DhResP resP;
  DhAtomP atomP;
  char strBuf[80], numBuf[30];
  DSTR dstr;
  int lastResI, resI;
  int atom1I, atom2I;
  BondListEntry *entryP, entry;
  Vec3 coord;
  int i;

  if (molP == NULL) {
    molP = DhMolNew();
    isNewMol = TRUE;
  } else {
    isNewMol = FALSE;
    DhMolApplyAtom(PropGetRef(PROP_ALL, FALSE), molP, setInvalid, NULL);
  }

  errCode = EC_OK;
  resP = NULL;
  dstr = DStrNew();
  bondL = ListOpen(sizeof(BondListEntry));

  for (;;) {
    getLinePart(numBuf, lineBuf, 0, 5);
    atom1I = atoi(numBuf);

    getLinePart(numBuf, lineBuf, 11, 6);
    resI = atoi(numBuf);

    if (resP == NULL || resI > lastResI) {
      resP = NULL;
      if (! isNewMol)
	resP = DhResFind(molP, resI);

      if (resP == NULL) {
	getLinePart(strBuf, lineBuf, 18, 5);
	DStrAssignStr(dstr, strBuf);
	resDefP = DhResDefGet(dstr);
	isNewRes = (resDefP == NULL);
	if (isNewRes) {
	  resDefP = DhResDefNew(dstr);
	  UnknownErrAddRes(dstr);
	}

	resP = DhResNew(molP, resDefP, SP_LAST);
	DhResSetNumber(resP, resI);
      } else {
	isNewRes = FALSE;
      }

      lastResI = resI;
    } else if (resI < lastResI) {
      *molIP += 1;
      break;
    }

    getLinePart(strBuf, lineBuf, 6, 5);
    DStrAssignStr(dstr, strBuf);
    if (isNewRes) {
      atomP = DhAtomNew(resP, dstr);
    } else {
      atomP = DhAtomFindName(resP, dstr, TRUE);
      if (atomP == NULL) {
	UnknownErrAddAtom(DhResGetName(resP), dstr);
	atomP = DhAtomNew(resP, dstr);
      }
    }

    if (atomP != NULL) {
      getLinePart(numBuf, lineBuf, 23, 11);
      coord[0] = (float) atof(numBuf);
      getLinePart(numBuf, lineBuf, 34, 11);
      coord[1] = (float) atof(numBuf);
      getLinePart(numBuf, lineBuf, 45, 11);
      coord[2] = (float) atof(numBuf);
      DhAtomSetCoord(atomP, coord);

      if (isNewMol) {
	entryP = ListFirst(bondL);
	while (entryP != NULL) {
	  if (entryP->atom2I == atom1I) {
	    (void) DhBondNew(
		DhAtomFindNumber(entryP->res1P, entryP->atom1I, TRUE), atomP);
	    ListRemove(bondL, entryP);
	  }
	  entryP = ListNext(bondL, entryP);
	}

	for (i = 0; i < 4; i++) {
	  getLinePart(numBuf, lineBuf, 57 + 6 * i, 5);
	  if (numBuf[0] == '\0')  /* no connectivities */
	    break;
	  atom2I = atoi(numBuf);
	  if (atom2I == 0)
	    break;
	  if (atom1I > atom2I)
	    continue;
	  entry.res1P = resP;
	  entry.atom1I = DhAtomGetNumber(atomP);
	  entry.atom2I = atom2I;
	  ListInsertLast(bondL, &entry);
	}
      }
    }

    if (GFileEOF(gf)) {
      if (*molIP > 0)
	*molIP += 1;
      break;
    }

    res = GFileReadLine(gf, lineBuf, LINE_LEN);
    if (res != GF_RES_OK) {
      errCode = EC_ERROR;
      break;
    }
  }

  ListClose(bondL);

  if (errCode != EC_OK) {
    DhMolDestroy(molP);
  } else {
    DStrAssignStr(dstr, molName);
    if (*molIP > 0) {
      (void) sprintf(numBuf, "%03d", *molIP);
      DStrAppStr(dstr, numBuf);
    } else {
      *molIP += 1;
    }
    DhMolSetName(molP, dstr);
    DhMolCoordsChanged(molP);

    if (isNewMol) {
      DhMolInit(molP);
      if (*molIP == 1)
	DhCalcCenter(cent, molP);
      DhMolSetRotPoint(molP, cent);
      GraphMolAdd(molP);
    }
  }

  DStrFree(dstr);

  return errCode;
}

static ErrCode
readFile(DhMolP *molPA, int molNo, DSTR name, int *readNoP)
{
  GFile gf;
  ErrCode errCode;
  char lineBuf[LINE_LEN];
  int molI;
  Vec3 cent;

  gf = GFileOpenRead(DStrToStr(name));
  if (gf == NULL)
    return EC_ERROR;

  /* skip three lines */
  (void) GFileReadLine(gf, lineBuf, sizeof(lineBuf));
  (void) GFileReadLine(gf, lineBuf, sizeof(lineBuf));
  (void) GFileReadLine(gf, lineBuf, sizeof(lineBuf));

  /* read first line */
  (void) GFileReadLine(gf, lineBuf, sizeof(lineBuf));

  FileNameBase(name);
  molI = 0;

  errCode = EC_OK;
  for (;;) {
    if (GFileEOF(gf))
      break;
    
    if (BreakCheck(1))
      break;

    if (molI < molNo)
      errCode = readOneMol(molPA[molI], gf, lineBuf, DStrToStr(name),
	  &molI, cent);
    else
      errCode = readOneMol(NULL, gf, lineBuf, DStrToStr(name),
	  &molI, cent);
    
    if (errCode != EC_OK)
      break;
  }
    
  *readNoP = molI;

  GFileClose(gf);

  GraphRedrawNeeded();

  return errCode;
}

ErrCode
ExReadDg(char *cmd)
{
  BOOL replace;
  DSTR fileName;
  int molNo, readNo;
  DhMolP *molPA;
  ErrCode errCode;
  DSTR errStr;

  replace = (strncmp(cmd, "Replace", 7) == 0);

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.cor", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  UnknownErrInit();

  if (replace) {
    molNo = SelMolGet(NULL, 0);
    if (molNo > 0) {
      molPA = malloc(molNo * sizeof(*molPA));
      (void) SelMolGet(molPA, molNo);
    }
  } else {
    molNo = 0;
    molPA = NULL;
  }

  BreakActivate(TRUE);
  errCode = readFile(molPA, molNo, fileName, &readNo);
  BreakActivate(FALSE);

  DStrFree(fileName);
  if (molNo > 0)
    free(molPA);

  if (errCode != EC_OK)
    return EC_ERROR;

  if (replace)
    GraphMolChanged(PROP_SELECTED);

  errStr = UnknownErrGet();
  if (errStr != NULL) {
    CipSetError(DStrToStr(errStr));
    DStrFree(errStr);
    return EC_WARNING;
  }

  return EC_OK;
}

ErrCode
ExReadListDg(char *cmd)
{
  BOOL replace;
  DSTR fileName, inName, dgName;
  ErrCode errCode;
  GFile gf;
  GFileRes res;
  int molNo, molI, readNo;
  DhMolP *molPA;
  char lineBuf[LINE_LEN];
  DSTR errStr;

  replace = (strncmp(cmd, "Replace", 7) == 0);

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.nam", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  if (gf == NULL) {
    DStrFree(fileName);
    return EC_ERROR;
  }

  if (replace) {
    molNo = SelMolGet(NULL, 0);
    if (molNo > 0) {
      molPA = malloc(molNo * sizeof(*molPA));
      (void) SelMolGet(molPA, molNo);
    }
  } else {
    molNo = 0;
    molPA = NULL;
  }

  FileNamePath(fileName);

  UnknownErrInit();
  inName = DStrNew();
  dgName = DStrNew();
  BreakActivate(TRUE);

  res = GF_RES_OK;
  molI = 0;
  for(;;) {
    if (GFileEOF(gf))
      break;

    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK)
      break;

    DStrAssignStr(inName, lineBuf);
    FileNameStrip(inName);
    if (DStrLen(inName) == 0)  /* allow empty line */
      continue;

    if (FileNameIsAbs(inName)) {
      DStrAssignDStr(dgName, inName);
    } else {
      DStrAssignDStr(dgName, fileName);
      DStrAppDStr(dgName, inName);
    }

    errCode = readFile(molPA + molI, molNo - molI, dgName, &readNo);
    if (errCode != EC_OK) {
      res = GF_RES_ERR;
      break;
    }
    if (BreakInterrupted())
      break;
    
    molI += readNo;
  }

  BreakActivate(FALSE);
  DStrFree(fileName);
  DStrFree(inName);
  DStrFree(dgName);
  if (molNo > 0)
    free(molPA);

  GFileClose(gf);

  if (res != GF_RES_OK)
    return EC_ERROR;

  if (replace)
    GraphMolChanged(PROP_SELECTED);

  errStr = UnknownErrGet();
  if (errStr != NULL) {
    CipSetError(DStrToStr(errStr));
    DStrFree(errStr);
    return EC_WARNING;
  }

  return EC_OK;
}
