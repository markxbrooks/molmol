/*
************************************************************************
*
*   ExPdb.c - ReadPdb, ReadOldPdb, ReadListPdb and WritePdb commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExPdb.c
*   SCCS identification       : 1.32
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <break.h>
#include <dstr.h>
#include <file_name.h>
#include <tree.h>
#include <g_file.h>
#include <curr_dir.h>
#include <prog_vers.h>
#include <arg.h>
#include <par_names.h>
#include <setup_file.h>
#include <prop_second.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>
#include "unknown_err.h"

#define NO_RES -999
#define LINE_LEN 100
#define ATOM_NAME_SIZE 7

typedef struct {
  char resName[5];
  char oldAtomName[ATOM_NAME_SIZE];
  char newAtomName[ATOM_NAME_SIZE];
} TransTabEntry;

typedef struct {
  char chain;
  PropRefP ref1P, ref2P, ref3P;
  int startI, endI;
} HelixDesc;

typedef struct {
  char chain;
  char name[4];
  char num;
  PropRefP ref1P, ref2P, ref3P;
  int startI, endI, parI;
  BOOL isPar;
} SheetDesc;

typedef struct {
  char chain;
  PropRefP ref1P;
  int startI, endI;
} TurnDesc;

typedef struct {
  GFile gf;
  int atomIdx;
  BOOL ok;
} WriteData;

static TREE TransTab;

static int HelixNo, SheetNo, TurnNo;
static HelixDesc *HelixTab;
static SheetDesc *SheetTab;
static TurnDesc *TurnTab;

static int
compFunc(void *p1, void *p2)
{
  TransTabEntry *entry1P = p1;
  TransTabEntry *entry2P = p2;
  int res;

  res = strcmp(entry1P->resName, entry2P->resName);
  if (res != 0)
    return res;
  
  return strcmp(entry1P->oldAtomName, entry2P->oldAtomName);
}

static void
readTransTab(BOOL pdbToIntern, BOOL isNewNomenclature)
{
  GFile gf;
  GFileRes res;
  TransTabEntry tabEntry;
  char *pdbName, *internName;

  TransTab = TreeOpen(sizeof(TransTabEntry), compFunc);
  gf = SetupOpen(PN_PDB_ATOMS, "PdbAtoms", FALSE);
  if (gf == NULL || isNewNomenclature)
    return;

  if (pdbToIntern) {
    pdbName = tabEntry.oldAtomName;
    internName = tabEntry.newAtomName;
  } else {
    pdbName = tabEntry.newAtomName;
    internName = tabEntry.oldAtomName;
  }

  while (! GFileEOF(gf)) {
    res = GFileReadStr(gf, tabEntry.resName, sizeof(tabEntry.resName));
    if (res != GF_RES_OK)
      return;
    res = GFileReadStr(gf, pdbName, ATOM_NAME_SIZE);
    if (res != GF_RES_OK)
      return;
    res = GFileReadStr(gf, internName, ATOM_NAME_SIZE);
    if (res != GF_RES_OK)
      return;
    
    (void) TreeInsert(TransTab, &tabEntry, TIB_RETURN_OLD);
  }

  GFileClose(gf);
}

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

static void
addHelix(char *lineBuf)
{
  HelixDesc *helixP;
  char numBuf[30], propName[20];
  int helixClass;

  if (HelixNo == 0)
    HelixTab = malloc(sizeof(*HelixTab));
  else
    HelixTab = realloc(HelixTab, (HelixNo + 1) * sizeof(*HelixTab));

  helixP = HelixTab + HelixNo;

  helixP->chain = lineBuf[19];

  helixP->ref1P = PropGetRef(PROP_HELIX, TRUE);

  getLinePart(numBuf, lineBuf, 38, 2);
  helixClass = atoi(numBuf);
  if (helixClass == 1 || helixClass == 6)
    helixP->ref2P = PropGetRef(PROP_HELIX_ALPHA, TRUE);
  else if (helixClass == 3)
    helixP->ref2P = PropGetRef(PROP_HELIX_PI, TRUE);
  else if (helixClass == 5)
    helixP->ref2P = PropGetRef(PROP_HELIX_3_10, TRUE);
  else
    helixP->ref2P = PropGetRef(PROP_HELIX, TRUE);

  getLinePart(numBuf, lineBuf, 7, 3);
  (void) sprintf(propName, "%s%d", PROP_HELIX, atoi(numBuf));
  helixP->ref3P = PropGetRef(propName, TRUE);

  getLinePart(numBuf, lineBuf, 21, 4);
  helixP->startI = atoi(numBuf);
  getLinePart(numBuf, lineBuf, 33, 4);
  helixP->endI = atoi(numBuf);

  HelixNo++;
}

static void
addSheet(char *lineBuf)
{
  SheetDesc *sheetP;
  char numBuf[30], propName[20];

  if (SheetNo == 0)
    SheetTab = malloc(sizeof(*SheetTab));
  else
    SheetTab = realloc(SheetTab, (SheetNo + 1) * sizeof(*SheetTab));

  sheetP = SheetTab + SheetNo;

  sheetP->chain = lineBuf[21];

  sheetP->ref1P = PropGetRef(PROP_SHEET, TRUE);

  getLinePart(sheetP->name, lineBuf, 11, 3);
  if (SheetNo == 0) {
    sheetP->num = 1;
  } else if (strcmp(sheetP->name, SheetTab[SheetNo - 1].name) == 0) {
    sheetP->num = SheetTab[SheetNo - 1].num;
  } else {
    sheetP->num = SheetTab[SheetNo - 1].num + 1;
  }

  (void) sprintf(propName, "%s%d", PROP_SHEET, sheetP->num);
  sheetP->ref2P = PropGetRef(propName, TRUE);

  getLinePart(numBuf, lineBuf, 7, 3);
  (void) sprintf(propName, "%s%d_%d", PROP_STRAND, sheetP->num, atoi(numBuf));
  sheetP->ref3P = PropGetRef(propName, TRUE);

  getLinePart(numBuf, lineBuf, 22, 4);
  sheetP->startI = atoi(numBuf);
  getLinePart(numBuf, lineBuf, 33, 4);
  sheetP->endI = atoi(numBuf);

  getLinePart(numBuf, lineBuf, 38, 2);
  sheetP->isPar = (atoi(numBuf) == 1);

  getLinePart(numBuf, lineBuf, 65, 4);
  if (numBuf[0] == '\0') {
    sheetP->parI = NO_RES;
  } else {
    sheetP->parI = atoi(numBuf);
    getLinePart(numBuf, lineBuf, 50, 4);
    if (sheetP->isPar)
      sheetP->parI -= (atoi(numBuf) - sheetP->startI);
    else
      sheetP->parI += (atoi(numBuf) - sheetP->startI);
  }

  SheetNo++;
}

static void
addTurn(char *lineBuf)
{
  TurnDesc *turnP;
  char numBuf[30];

  if (TurnNo == 0)
    TurnTab = malloc(sizeof(*TurnTab));
  else
    TurnTab = realloc(TurnTab, (TurnNo + 1) * sizeof(*TurnTab));

  turnP = TurnTab + TurnNo;

  turnP->chain = lineBuf[19];

  turnP->ref1P = PropGetRef(PROP_TURN, TRUE);

  getLinePart(numBuf, lineBuf, 20, 4);
  turnP->startI = atoi(numBuf);
  getLinePart(numBuf, lineBuf, 31, 4);
  turnP->endI = atoi(numBuf);

  TurnNo++;
}

static void
setSecond(DhResP resP, void *clientData)
{
  char chain = * (char *) clientData;
  DhMolP molP;
  DhResP parP;
  int resI, parI, i;

  molP = DhResGetMol(resP);
  resI = DhResGetNumber(resP);

  for (i = 0; i < HelixNo; i++)
    if ((HelixTab[i].chain == ' ' || HelixTab[i].chain == chain) &&
	resI >= HelixTab[i].startI && resI <= HelixTab[i].endI) {
      DhResSetProp(HelixTab[i].ref1P, resP, TRUE);
      DhResSetProp(HelixTab[i].ref2P, resP, TRUE);
      DhResSetProp(HelixTab[i].ref3P, resP, TRUE);
      return;
    }

  /* check sheets in backwards direction because first sheet of
     a barrel is listed twice, where the second entry contains the
     information about the parellel sheet */
  for (i = SheetNo - 1; i >= 0; i--)
    if ((SheetTab[i].chain == ' ' || SheetTab[i].chain == chain) &&
	resI >= SheetTab[i].startI && resI <= SheetTab[i].endI) {
      DhResSetProp(SheetTab[i].ref1P, resP, TRUE);
      DhResSetProp(SheetTab[i].ref2P, resP, TRUE);
      DhResSetProp(SheetTab[i].ref3P, resP, TRUE);

      if (SheetTab[i].parI != NO_RES) {
	if (SheetTab[i].isPar)
	  parI = SheetTab[i].parI + (resI - SheetTab[i].startI);
	else
	  parI = SheetTab[i].parI - (resI - SheetTab[i].startI);
	
	if (parI >= SheetTab[i - 1].startI && parI <= SheetTab[i - 1].endI) {
	  parP = DhResFind(molP, parI);
	  if (parP != NULL)
	    DhResSetNeigh(resP, NC_LEFT, parP);
	}
      }
      return;
    }

  for (i = 0; i < TurnNo; i++)
    if ((TurnTab[i].chain == ' ' || TurnTab[i].chain == chain) &&
	resI >= TurnTab[i].startI && resI <= TurnTab[i].endI) {
      DhResSetProp(TurnTab[i].ref1P, resP, TRUE);
      return;
    }

  DhResSetProp(PropGetRef(PROP_COIL, TRUE), resP, TRUE);
}

static GFileRes
findAtomLine(GFile gf, char *lineBuf, int size)
{
  GFileRes res;

  for (;;) {
    if (GFileEOF(gf))
      return GF_RES_ERR;

    res = GFileReadLine(gf, lineBuf, size);
    if (res != GF_RES_OK)
      return res;

    if (strncmp(lineBuf, "ATOM", 4) == 0 ||
	strncmp(lineBuf, "HETATM", 6) == 0)
      return GF_RES_OK;

    if (strncmp(lineBuf, "HELIX", 5) == 0)
      addHelix(lineBuf);
    else if (strncmp(lineBuf, "SHEET", 5) == 0)
      addSheet(lineBuf);
    else if (strncmp(lineBuf, "TURN", 4) == 0)
      addTurn(lineBuf);
  }
}

static void
transAtomName(DhResP resP, DSTR atomName)
{
  int resLen, atomLen;
  char *resStr, *atomStr, ch;
  TransTabEntry tabEntry, *tabEntryP;
  DSTR resName;
  int i;

  resName = DhResGetName(resP);
  resLen = DStrLen(resName);
  resStr = DStrToStr(resName);
  if (resLen > 3)
    resLen = 3;
  if (resStr[resLen - 1] == '+' || resStr[resLen - 1] == '-')
    resLen--;

  atomLen = DStrLen(atomName);
  atomStr = DStrToStr(atomName);
  ch = atomStr[0];
  if (isdigit(ch)) {
    /* move leading digit to the end */
    DStrPart(atomName, 1, atomLen - 1);
    DStrAppChar(atomName, ch);
    atomStr = DStrToStr(atomName);
  }

  /* replace blanks by underscores */
  for (i = 0; i < atomLen; i++)
    if (atomStr[i] == ' ')
      atomStr[i] = '_';

  (void) strncpy(tabEntry.resName, resStr,
      sizeof(tabEntry.resName));
  if (resLen < sizeof(tabEntry.resName))
    tabEntry.resName[resLen] = '\0';
  else
    tabEntry.resName[sizeof(tabEntry.resName) - 1] = '\0';

  (void) strncpy(tabEntry.oldAtomName, atomStr,
      sizeof(tabEntry.oldAtomName));
  tabEntry.oldAtomName[sizeof(tabEntry.oldAtomName) - 1] = '\0';
  tabEntryP = TreeSearch(TransTab, &tabEntry);
  if (tabEntryP == NULL) {
    (void) strcpy(tabEntry.resName, "*");
    tabEntryP = TreeSearch(TransTab, &tabEntry);
  }
  if (tabEntryP != NULL)
    DStrAssignStr(atomName, tabEntryP->newAtomName);
}

static ErrCode
readOneMol(DhMolP molP, GFile gf, char *lineBuf, char *molName,
    int *molIP, Vec3 cent)
{
  GFileRes res;
  BOOL isNewMol;
  ErrCode errCode;
  DhResDefP resDefP;
  BOOL isNewRes;
  DhResP resP;
  char chain;
  DhAtomP atomP;
  char strBuf[80], numBuf[30];
  DSTR dstr;
  int lastResI, resI;
  Vec3 coord;

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

  for (;;) {
    if (strncmp(lineBuf, "ATOM", 4) == 0 ||
	strncmp(lineBuf, "HETATM", 6) == 0) {
      getLinePart(numBuf, lineBuf, 22, 4);
      resI = atoi(numBuf);

      if (resP != NULL && lineBuf[21] != chain) {
	*molIP += 1;
	break;
      }

      if (resP == NULL || resI != lastResI) {
	resP = NULL;
	if (! isNewMol)
	  resP = DhResFind(molP, resI);

	if (resP == NULL) {
	  getLinePart(strBuf, lineBuf, 17, 4);
	  DStrAssignStr(dstr, strBuf);
	  DStrAppChar(dstr, '+');  /* protonated form */
	  resDefP = DhResDefGet(dstr);
	  if (resDefP == NULL) {
	    DStrAssignStr(dstr, strBuf);  /* non-protonated form */
	    resDefP = DhResDefGet(dstr);
	  }
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
	chain = lineBuf[21];
      }

      /* ignore alternate positions */
      if (lineBuf[16] != 'B' && lineBuf[16] != 'C') {
	if (lineBuf[16] == 'A')
	  getLinePart(strBuf, lineBuf, 12, 4);
	else
	  /* atom names go up to position 16 sometimes (non-standard!) */
	  getLinePart(strBuf, lineBuf, 12, 5);

	DStrAssignStr(dstr, strBuf);
	transAtomName(resP, dstr);
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
	  getLinePart(numBuf, lineBuf, 30, 8);
	  coord[0] = (float) atof(numBuf);
	  getLinePart(numBuf, lineBuf, 38, 8);
	  coord[1] = (float) atof(numBuf);
	  getLinePart(numBuf, lineBuf, 46, 8);
	  coord[2] = (float) atof(numBuf);
	  DhAtomSetCoord(atomP, coord);

	  getLinePart(numBuf, lineBuf, 60, 6);
	  DhAtomSetBFactor(atomP, (float) atof(numBuf));
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

    if (strncmp(lineBuf, "TER", 3) == 0 ||
        strncmp(lineBuf, "ENDMDL", 6) == 0) {
      res = findAtomLine(gf, lineBuf, LINE_LEN);
      if (*molIP > 0 || res == GF_RES_OK)
	*molIP += 1;
      break;
    }

    if (strncmp(lineBuf, "END", 3) == 0) {
      if (*molIP > 0)
	*molIP += 1;

      /* ignore lines after END */
      while (! GFileEOF(gf)) {
	res = GFileReadLine(gf, lineBuf, LINE_LEN);
	if (res != GF_RES_OK) {
	  errCode = EC_ERROR;
	  break;
	}
      }
      break;
    }
  }

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
      DhMolApplyRes(PropGetRef(PROP_ALL, FALSE), molP, setSecond, &chain);

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
  GFileRes res;
  ErrCode errCode;
  char lineBuf[LINE_LEN];
  int molI;
  Vec3 cent;

  gf = GFileOpenRead(DStrToStr(name));
  if (gf == NULL)
    return EC_ERROR;

  FileNameBase(name);

  HelixNo = 0;
  SheetNo = 0;
  TurnNo = 0;

  molI = 0;

  res = findAtomLine(gf, lineBuf, LINE_LEN);
  if (res != GF_RES_OK) {
    GFileClose(gf);
    CipSetError("ATOM entry not found");
    return EC_ERROR;
  }

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

  if (HelixNo > 0)
    free(HelixTab);

  if (SheetNo > 0)
    free(SheetTab);

  if (TurnNo > 0)
    free(TurnTab);

  GraphRedrawNeeded();

  return errCode;
}

ErrCode
ExReadPdb(char *cmd)
{
  BOOL replace;
  DSTR name;
  int molNo, readNo;
  DhMolP *molPA;
  ErrCode errCode;
  DSTR errStr;

  replace = (strncmp(cmd, "Replace", 7) == 0);

  name = DStrNew();
  errCode = ArgGetFilename(name, CurrDirGet(), "*.pdb", TRUE);
  if (errCode != EC_OK) {
    DStrFree(name);
    return errCode;
  }

  readTransTab(TRUE, TRUE);
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
  errCode = readFile(molPA, molNo, name, &readNo);
  BreakActivate(FALSE);

  DStrFree(name);
  TreeClose(TransTab);
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
ExReadListPdb(char *cmd)
{
  BOOL replace;
  DSTR fileName, inName, pdbName;
  ErrCode errCode;
  GFile gf;
  GFileRes res;
  int molNo, molI, readNo;
  DhMolP *molPA;
  char lineBuf[200];
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

  readTransTab(TRUE,TRUE);
  UnknownErrInit();
  inName = DStrNew();
  pdbName = DStrNew();
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
      DStrAssignDStr(pdbName, inName);
    } else {
      DStrAssignDStr(pdbName, fileName);
      DStrAppDStr(pdbName, inName);
    }

    errCode = readFile(molPA + molI, molNo - molI, pdbName, &readNo);
    if (errCode != EC_OK) {
      res = GF_RES_ERR;
      break;
    }
    if (BreakInterrupted())
      break;

    molI += readNo;
  }

  BreakActivate(FALSE);
  GFileClose(gf);

  DStrFree(fileName);
  DStrFree(inName);
  DStrFree(pdbName);
  TreeClose(TransTab);
  if (molPA != NULL)
    free(molPA);

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

static void
writeAtom(DhAtomP atomP, void *clientData)
{
  WriteData *dataP = clientData;
  DhResP resP;
  DSTR name;
  char *nameStr, outName[5];
  int nameLen;
  Vec3 coord;
  char lineBuf[LINE_LEN];
  int i;

  if (! dataP->ok)
    return;

  resP = DhAtomGetRes(atomP);

  name = DStrNew();
  DStrAssignDStr(name, DhAtomGetName(atomP));
  transAtomName(resP, name);

  nameStr = DStrToStr(name);
  nameLen = DStrLen(name);

  /* replace underscores by blanks */
  for (i = 0; i < nameLen; i++)
    if (nameStr[i] == '_')
      nameStr[i] = ' ';

  if (nameStr[0] == 'H' && isdigit(nameStr[nameLen - 1])) {
    /* take digit to the front */
    outName[0] = nameStr[nameLen - 1];
    (void) strncpy(outName + 1, nameStr, 4);
    if (nameLen < 5)
      outName[nameLen] = '\0';
  } else {
    /* leading blank */
    outName[0] = ' ';
    (void) strncpy(outName + 1, nameStr, 4);
  }
  outName[4] = '\0';
  DStrFree(name);

  DhAtomGetCoordTransf(atomP, coord);

  (void) sprintf(lineBuf,
      "ATOM  %5d %-4.4s %-4.4s %4d    %#8.3f%#8.3f%#8.3f  1.00%#6.2f",
      dataP->atomIdx, outName,
      DStrToStr(DhResGetName(resP)), DhResGetNumber(resP),
      coord[0], coord[1], coord[2], DhAtomGetBFactor(atomP));
  dataP->ok = (GFileWriteLine(dataP->gf, lineBuf) == GF_RES_OK);

  dataP->atomIdx++;
}

#define CHECK_RES(s) if (s != GF_RES_OK) {dataP->ok = FALSE; return;}

static void
writeMol(DhMolP molP, WriteData *dataP, int model)
{
  GFile gf = dataP->gf;
  char lineBuf[LINE_LEN];

  if (! dataP->ok)
    return;

  if (model > 0) {
    (void) sprintf(lineBuf, "MODEL     %4d", model);
    CHECK_RES(GFileWriteLine(gf, lineBuf));
  }

  dataP->atomIdx = 1;
  DhMolApplyAtom(PropGetRef(PROP_ALL, FALSE), molP, writeAtom, dataP);

  CHECK_RES(GFileWriteLine(gf, "TER"));
  if (model > 0) {
    CHECK_RES(GFileWriteLine(gf, "ENDMDL"));
  }
}

#undef CHECK_RES
#define CHECK_RES(s) if (s != GF_RES_OK) {GFileClose(gf); return EC_ERROR;}

ErrCode
ExWritePdb(char *cmd)
{
  GFile gf;
  DSTR fileName;
  ErrCode errCode;
  int molNo, molI, model;
  DhMolP molP, *molPA;
  BOOL *sameA;
  WriteData writeData;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.pdb", FALSE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  molNo = SelMolGet(&molP, 1);
  if (molNo == 0) {
    CipSetError("at least 1 molecule must be selected");
    return EC_ERROR;
  }

  gf = GFileOpenNew(DStrToStr(fileName), GF_FORMAT_ASCII);
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  readTransTab(FALSE,TRUE);

  CHECK_RES(GFileWriteStr(gf, "HEADER    Structure from"));
  CHECK_RES(GFileWriteStr(gf, PROG_NAME));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteStr(gf, "COMPND   "));
  CHECK_RES(GFileWriteStr(gf, DStrToStr(DhMolGetName(molP))));
  CHECK_RES(GFileWriteNL(gf));

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  sameA = malloc(molNo * sizeof(*sameA));
  for (molI = 0; molI < molNo; molI++)
    sameA[molI] = FALSE;
  for (molI = 0; molI < molNo - 1; molI++)
    if (DhMolEqualStruc(molPA[molI], molPA[molI + 1]))
      sameA[molI] = TRUE;

  writeData.gf = gf;
  writeData.ok = TRUE;
  BreakActivate(TRUE);

  for (molI = 0; molI < molNo; molI++) {
    if (BreakCheck(1))
      break;

    if (molI > 0 && sameA[molI - 1])
      model++;
    else if (sameA[molI])
      model = 1;
    else
      model = 0;

    writeMol(molPA[molI], &writeData, model);
  }

  BreakActivate(FALSE);
  TreeClose(TransTab);

  free(molPA);
  free(sameA);

  if (! writeData.ok) {
    GFileClose(gf);
    return EC_ERROR;
  }

  CHECK_RES(GFileWriteStr(gf, "END"));
  CHECK_RES(GFileWriteNL(gf));

  GFileClose(gf);

  return EC_OK;
}

ErrCode
ExReadOldPdb(char *cmd)
{
  BOOL replace;
  DSTR name;
  int molNo, readNo;
  DhMolP *molPA;
  ErrCode errCode;
  DSTR errStr;

  replace = (strncmp(cmd, "Replace", 7) == 0);

  name = DStrNew();
  errCode = ArgGetFilename(name, CurrDirGet(), "*.pdb", TRUE);
  if (errCode != EC_OK) {
    DStrFree(name);
    return errCode;
  }

  readTransTab(TRUE, FALSE);
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
  errCode = readFile(molPA, molNo, name, &readNo);
  BreakActivate(FALSE);

  DStrFree(name);
  TreeClose(TransTab);
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
