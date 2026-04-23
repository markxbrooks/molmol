/*
************************************************************************
*
*   ExFlipAtom.c - FlipAtom command
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExFlipAtom.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <values.h>

#include <break.h>
#include <dstr.h>
#include <linlist.h>
#include <pu.h>
#include <arg.h>
#include <par_names.h>
#include <setup_file.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

#define LINE_LEN 200

typedef struct {
  int atomNo;
  DSTR *nameA;
} TupleData;

typedef struct {
  DSTR resName;
  int tupleNo;
  TupleData *tupleA;
} FlipData;

static LINLIST FlipList;
static PuTextWindow TextW;
static DhMolP RefMolP;

static void
freeFlipData(void *p, void *clientData)
{
  FlipData *flipP = p;
  int ti, ai;

  DStrFree(flipP->resName);

  for (ti = 0; ti < flipP->tupleNo; ti++) {
    for (ai = 0; ai < flipP->tupleA[ti].atomNo; ai++)
      DStrFree(flipP->tupleA[ti].nameA[ai]);

    if (flipP->tupleA[ti].atomNo > 0)
      free(flipP->tupleA[ti].nameA);
  }

  if (flipP->tupleNo > 0)
    free(flipP->tupleA);
}

static GFileRes
readList(void)
{
  GFile gf;
  GFileRes res;
  FlipData flipData;
  TupleData *tupleP;
  char lineBuf[LINE_LEN];
  int li;
  
  gf = SetupOpen(PN_FLIP_ATOMS, "FlipAtoms", FALSE);
  if (gf == NULL)
    return GF_RES_ERR;
  
  FlipList = ListOpen(sizeof(FlipData));
  ListAddDestroyCB(FlipList, freeFlipData, NULL, NULL);

  res = GF_RES_OK;
  while (! GFileEOF(gf)) {
    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK)
      break;
    
    if (lineBuf[0] == '#')
      continue;
    
    flipData.resName = DStrNew();
    flipData.tupleNo = 0;

    li = 0;
    while (lineBuf[li] != '\0' && ! isspace(lineBuf[li])) {
      DStrAppChar(flipData.resName, lineBuf[li]);
      li++;
    }

    for (;;) {
      while (isspace(lineBuf[li]))
	li++;
      
      if (lineBuf[li] != '(') {
	if (lineBuf[li] != '\0')
	  res = GF_RES_ERR;
	break;
      }

      flipData.tupleNo++;
      if (flipData.tupleNo == 1)
	flipData.tupleA = malloc(sizeof(*flipData.tupleA));
      else
	flipData.tupleA = realloc(flipData.tupleA,
	    flipData.tupleNo * sizeof(*flipData.tupleA));

      tupleP = flipData.tupleA + flipData.tupleNo - 1;
      tupleP->atomNo = 0;

      li++;
      for (;;) {
	while (isspace(lineBuf[li]))
	  li++;
	
	if (lineBuf[li] == ')') {
	  li++;
	  break;
	}
	
	if (lineBuf[li] == '\0') {
	  res = GF_RES_ERR;
	  break;
	}
	
	tupleP->atomNo++;
	if (tupleP->atomNo == 1)
	  tupleP->nameA = malloc(sizeof(*tupleP->nameA));
	else
	  tupleP->nameA = realloc(tupleP->nameA,
	      tupleP->atomNo * sizeof(*tupleP->nameA));
	
	tupleP->nameA[tupleP->atomNo - 1] = DStrNew();

	while (lineBuf[li] != '\0' && lineBuf[li] != ')' &&
	    ! isspace(lineBuf[li])) {
	  DStrAppChar(tupleP->nameA[tupleP->atomNo - 1], lineBuf[li]);
	  li++;
	}
      }

      if (tupleP->atomNo != flipData.tupleA[0].atomNo) {
	res = GF_RES_ERR;
	break;
      }
    }

    if (res == GF_RES_ERR)
      break;
    
    (void) ListInsertLast(FlipList, &flipData);
  }

  GFileClose(gf);

  return res;
}

static BOOL
nameInList(DSTR name, DSTR nameList)
{
  DSTR part;
  char *s;
  BOOL res;

  part = DStrNew();
  s = DStrToStr(nameList);
  res = FALSE;

  for (;;) {
    if (*s == '\0') {
      if (DStrCmp(name, part) == 0)
	res = TRUE;
      break;
    } else if (*s == ' ') {
      if (DStrCmp(name, part) == 0) {
	res = TRUE;
	break;
      } else {
	DStrAssignStr(part, "");
      }
    } else {
      DStrAppChar(part, *s);
    }

    s++;
  }

  DStrFree(part);

  return res;
}

static void
getResNames(DhResP resP, void *clientData)
{
  DSTR nameList = clientData;
  DSTR name;
  FlipData *flipP;

  name = DhResGetName(resP);
  if (nameInList(name, nameList))
    return;
  
  flipP = ListFirst(FlipList);
  while (flipP != NULL) {
    if (DStrCmp(name, flipP->resName) == 0)
      break;
    flipP = ListNext(FlipList, flipP);
  }
  if (flipP == NULL)
    return;

  DStrAppDStr(nameList, name);
  DStrAppStr(nameList, " ");
}

static void
checkResFlip(FlipData *flipP, DhResP resP, DhResP refResP)
{
  int atomNo, firstAtomI, minI;
  float minRmsd, rmsd;
  DSTR *nameA;
  DhAtomP atomP, refAtomP;
  Vec3 v, vRef, *vA;
  char buf[20];
  int ti, air, ai;

  atomNo = flipP->tupleA[0].atomNo;
  minRmsd = MAXFLOAT;

  for (firstAtomI = 0; firstAtomI < atomNo; firstAtomI++) {
    rmsd = 0.0f;

    for (ti = 0; ti < flipP->tupleNo; ti++) {
      nameA = flipP->tupleA[ti].nameA;

      for (air = 0; air < atomNo; air++) {
	ai = (firstAtomI + air) % atomNo;

	atomP = DhAtomFindName(resP, nameA[ai], FALSE);
	if (atomP == NULL)
	  return;

	refAtomP = DhAtomFindName(refResP, nameA[air], FALSE);
	if (refAtomP == NULL)
	  return;

	DhAtomGetCoordTransf(atomP, v);
	DhAtomGetCoordTransf(refAtomP, vRef);
	Vec3Sub(v, vRef);

	rmsd += v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
      }
    }

    if (rmsd < minRmsd) {
      minRmsd = rmsd;
      minI = firstAtomI;
    }
  }

  if (minI == 0)
    return;

  (void) sprintf(buf, "%-5s", DStrToStr(DhResGetName(resP)));
  PuWriteStr(TextW, buf);
  (void) sprintf(buf, "%5d", DhResGetNumber(resP));
  PuWriteStr(TextW, buf);
  PuWriteStr(TextW, ":");
  for (ti = 0; ti < flipP->tupleNo; ti++) {
    nameA = flipP->tupleA[ti].nameA;
    for (ai = 0; ai < atomNo; ai++) {
      if (ai == 0)
	PuWriteStr(TextW, " (");
      else
	PuWriteStr(TextW, " ");
      PuWriteStr(TextW, DStrToStr(nameA[ai]));
    }
    PuWriteStr(TextW, ")");
  }
  PuWriteStr(TextW, "\n");

  vA = malloc(atomNo * sizeof(*vA));

  for (ti = 0; ti < flipP->tupleNo; ti++) {
    nameA = flipP->tupleA[ti].nameA;

    for (ai = 0; ai < atomNo; ai++) {
      atomP = DhAtomFindName(resP, nameA[ai], FALSE);
      DhAtomGetCoord(atomP, vA[ai]);
    }

    for (ai = 0; ai < atomNo; ai++) {
      atomP = DhAtomFindName(resP, nameA[ai], FALSE);
      DhAtomSetCoord(atomP, vA[(minI + ai) % atomNo]);
    }
  }

  free(vA);
}

static void
checkRes(DhResP resP, void *clientData)
{
  DSTR nameList = clientData;
  DSTR name;
  int num;
  DhResP refResP;
  FlipData *flipP;

  name = DhResGetName(resP);
  if (! nameInList(name, nameList))
    return;
  
  if (BreakCheck(1))
    return;

  num = DhResGetNumber(resP);
  refResP = DhResFind(RefMolP, num);
  if (refResP == NULL)
    return;
  if (DStrCmp(name, DhResGetName(refResP)) != 0)
    return;
  
  flipP = ListFirst(FlipList);
  while (flipP != NULL) {
    if (DStrCmp(name, flipP->resName) == 0)
      checkResFlip(flipP, resP, refResP);
    flipP = ListNext(FlipList, flipP);
  }
}

static void
checkMol(DhMolP molP, void *clientData)
{
  if (BreakCheck(1))
    return;

  if (RefMolP == NULL) {
    RefMolP = molP;
  } else {
    PuWriteStr(TextW, DStrToStr(DhMolGetName(molP)));
    PuWriteStr(TextW, ":\n\n");

    DhMolApplyRes(PropGetRef(PROP_SELECTED, FALSE), molP,
	checkRes, clientData);
    DhMolCoordsChanged(molP);

    PuWriteStr(TextW, "\n");
  }
}

#define ARG_NUM 1

ErrCode
ExFlipAtom(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  DhMolP molP;
  int molNo;

  if (readList() == GF_RES_ERR) {
    ListClose(FlipList);
    CipSetError("error while reading setup file");
    return EC_ERROR;
  }

  molNo = SelMolGet(&molP, 1);

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Residue Types";
  if (molNo > 0)
    DhMolApplyRes(PropGetRef(PROP_SELECTED, FALSE), molP,
	getResNames, arg[0].v.strVal);

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ListClose(FlipList);
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  BreakActivate(TRUE);

  TextW = PuCreateTextWindow(cmd);

  RefMolP = NULL;
  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), checkMol, arg[0].v.strVal);

  BreakActivate(FALSE);

  ArgCleanup(arg, ARG_NUM);

  ListClose(FlipList);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
