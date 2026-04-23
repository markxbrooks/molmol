/*
************************************************************************
*
*   ExCalcSecond.c - CalcSecondary command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcSecond.c
*   SCCS identification       : 1.13
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <mat_vec.h>
#include <dstr.h>
#include <linlist.h>
#include <pu.h>
#include <prop_tab.h>
#include <prop_second.h>
#include <data_hand.h>

typedef enum {
  BT_PARALLEL,
  BT_ANTI_PARALLEL
} BridgeType;

typedef struct BridgeS *BridgeP;

typedef struct BridgeS {
  int ind1, ind2;
  BridgeType type;
  BridgeP prevP, nextP;
  BOOL visited;
} Bridge;

typedef struct {
  DhResP resP;
  BOOL setN, setH, setCA, setC, setO;
  Vec3 xN, xH, xCA, xC, xO;
  BOOL setNH, setCO;
  int *bonds;
  int bondNo;
  Bridge *bridges;
  int bridgeNo, bridgeSize;
  BOOL sheet, bulge;
  BOOL turn3, turn4, turn5;
  BOOL helix3, helix4, helix5;
  BOOL bend;
  int num, numStrand;
} Res;

static PuTextWindow TextW;
static Res *ResList;
static int ResNo, ResSize;

static void
addAtom(DhAtomP atomP, void *clientData)
{
  Res *dataP = clientData;
  char *name;

  name = DStrToStr(DhAtomGetName(atomP));

  if (strcmp(name, "N") == 0) {
    DhAtomGetCoord(atomP, dataP->xN);
    dataP->setN = TRUE;
  } else if (strcmp(name, "HN") == 0 || strcmp(name, "H") == 0) {
    DhAtomGetCoord(atomP, dataP->xH);
    dataP->setH = TRUE;
  } else if (strcmp(name, "CA") == 0) {
    DhAtomGetCoord(atomP, dataP->xCA);
    dataP->setCA = TRUE;
  } else if (strcmp(name, "C") == 0) {
    DhAtomGetCoord(atomP, dataP->xC);
    dataP->setC = TRUE;
  } else if (strcmp(name, "O") == 0) {
    DhAtomGetCoord(atomP, dataP->xO);
    dataP->setO = TRUE;
  }
}

static void
addRes(DhResP resP, void *clientData)
{
  if (ResNo == ResSize) {
    ResSize *= 2;
    ResList = realloc(ResList, ResSize * sizeof(*ResList));
  }

  ResList[ResNo].setN = FALSE;
  ResList[ResNo].setH = FALSE;
  ResList[ResNo].setCA = FALSE;
  ResList[ResNo].setC = FALSE;
  ResList[ResNo].setO = FALSE;
  DhResApplyAtom(PropGetRef(PROP_ALL, FALSE), resP, addAtom, ResList + ResNo);

  ResList[ResNo].resP = resP;

  ResList[ResNo].setNH = (ResList[ResNo].setN && ResList[ResNo].setH);
  ResList[ResNo].setCO = (ResList[ResNo].setC && ResList[ResNo].setO);

  ResNo++;
}

static void
setBend(void)
{
  int i;
  Vec3 v1, v2;
  float c, s;

  for (i = 0; i < ResNo; i++) {
    if (i < 2 || i >= ResNo - 2) {
      ResList[i].bend = FALSE;
      continue;
    }

    if (! (ResList[i - 2].setCA && ResList[i].setCA &&
	ResList[i + 2].setCA)) {
      ResList[i].bend = FALSE;
      continue;
    }

    Vec3Copy(v1, ResList[i].xCA);
    Vec3Sub(v1, ResList[i - 2].xCA);
    Vec3Norm(v1);

    Vec3Copy(v2, ResList[i + 2].xCA);
    Vec3Sub(v2, ResList[i].xCA);
    Vec3Norm(v2);

    c = Vec3Scalar(v1, v2);
    Vec3Cross(v1, v2);
    s = Vec3Abs(v1);

    ResList[i].bend = (atan2f(s, c) * 180.0f / (float) M_PI > 70.0f);
  }
}

static void
findBond(void)
{
  int bondSize, *bonds, bondNo;
  int i1, i2;

  for (i1 = 0; i1 < ResNo; i1++) {
    if (! ResList[i1].setCO) {
      ResList[i1].bonds = NULL;
      ResList[i1].bondNo = 0;
      continue;
    }

    bondSize = 2;
    bonds = malloc(bondSize * sizeof(*bonds));
    bondNo = 0;

    for (i2 = 0; i2 < ResNo; i2++) {
      if (i2 == i1)
	continue;

      if (! ResList[i2].setNH)
	continue;

      if (0.42f * 0.20f *
	  (1.0f / Vec3DiffAbs(ResList[i1].xO, ResList[i2].xN) +
	   1.0f / Vec3DiffAbs(ResList[i1].xC, ResList[i2].xH) -
	   1.0f / Vec3DiffAbs(ResList[i1].xO, ResList[i2].xH) -
	   1.0f / Vec3DiffAbs(ResList[i1].xC, ResList[i2].xN)) *
	  332.0f < - 0.5f) {
	if (bondNo == bondSize) {
	  bondSize *= 2;
	  bonds = realloc(bonds, bondSize * sizeof(*bonds));
	}

	bonds[bondNo++] = i2;
      }
    }

    if (bondNo == 0) {
      free(bonds);
      ResList[i1].bonds = NULL;
    } else {
      ResList[i1].bonds = realloc(bonds, bondNo * sizeof(*bonds));
    }
    ResList[i1].bondNo = bondNo;
  }
}

static BOOL
hbond(int i1, int i2)
{
  int i;

  if (i1 < 0 || i1 >=ResNo)
    return FALSE;

  if (i2 < 0 || i2 >=ResNo)
    return FALSE;

  for (i = 0; i < ResList[i1].bondNo; i++)
    if (ResList[i1].bonds[i] == i2)
      return TRUE;
  
  return FALSE;
}

static void
addBridge(int i1, int i2, BridgeType type)
{
  BridgeP bridgeP;

  if (i1 <= i2 + 2 && i1 >= i2 - 2)
    return;

  if (ResList[i1].helix4 || ResList[i2].helix4)
    return;

  if (ResList[i1].bridgeNo == ResList[i1].bridgeSize) {
    ResList[i1].bridgeSize *= 2;
    ResList[i1].bridges = realloc(ResList[i1].bridges,
	ResList[i1].bridgeSize * sizeof(Bridge));
  }

  bridgeP = ResList[i1].bridges + ResList[i1].bridgeNo++;

  bridgeP->ind1 = i1;
  bridgeP->ind2 = i2;
  bridgeP->type = type;
  bridgeP->nextP = NULL;
  bridgeP->prevP = NULL;
  bridgeP->visited = FALSE;
}

static void
setBridge(void)
{
  int bondI, i1, i2;

  for (i1 = 0; i1 < ResNo; i1++) {
    ResList[i1].bridgeSize = 2;
    ResList[i1].bridges = malloc(ResList[i1].bridgeSize * sizeof(Bridge));
    ResList[i1].bridgeNo = 0;
  }

  for (i1 = 0; i1 < ResNo; i1++) {
    for (bondI = 0; bondI < ResList[i1].bondNo; bondI++) {
      i2 = ResList[i1].bonds[bondI];

      if (hbond(i2, i1))
	addBridge(i1, i2, BT_ANTI_PARALLEL);

      if (hbond(i2 - 2, i1 + 2))
	addBridge(i1 + 1, i2 - 1, BT_ANTI_PARALLEL);

      if (hbond(i2, i1 + 2))
	addBridge(i1 + 1, i2, BT_PARALLEL);

      if (hbond(i2 - 2, i1))
	addBridge(i1, i2 - 1, BT_PARALLEL);
    }
  }

  for (i1 = 0; i1 < ResNo; i1++) {
    if (ResList[i1].bridgeNo == 0) {
      free(ResList[i1].bridges);
      ResList[i1].bridges = NULL;
    } else {
      ResList[i1].bridges = realloc(ResList[i1].bridges,
	  ResList[i1].bridgeNo * sizeof(Bridge));
    }
  }
}

static BridgeP
getBridge(int i1, int i2, BridgeType type)
{
  BridgeP bridgeP;
  int i;

  for (i = 0; i < ResList[i1].bridgeNo; i++) {
    bridgeP = ResList[i1].bridges + i;

    if (bridgeP->ind2 == i2 && bridgeP->type == type)
      return bridgeP;
  }

  return NULL;
}

static void
connectBridge(void)
{
  BridgeP bridgeP;
  int resI, bridgeI, i1, i2, inc1, inc2;

  for (resI = 0; resI < ResNo; resI++) {
    ResList[resI].sheet = FALSE;
    ResList[resI].bulge = FALSE;

    for (bridgeI = 0; bridgeI < ResList[resI].bridgeNo; bridgeI++) {
      bridgeP = ResList[resI].bridges + bridgeI;

      if (bridgeP->prevP != NULL) {  /* already connected */
	ResList[resI].sheet = TRUE;
	continue;
      }
      
      i1 = resI;
      i2 = bridgeP->ind2;

      if (bridgeP->type == BT_PARALLEL)
	inc2 = 1;
      else
	inc2 = -1;

      for (;;) {
	i1++;
	if (i1 == ResNo)
	  break;

	i2 += inc2;
	if (i2 == -1 || i2 == ResNo)
	  break;

	bridgeP->nextP = getBridge(i1, i2, bridgeP->type);
	if (bridgeP->nextP == NULL)
	  break;

	bridgeP->nextP->prevP = bridgeP;
	bridgeP = bridgeP->nextP;

	ResList[resI].sheet = TRUE;
      }
    }
  }
 
  /* connect ladders (bulges) */
  for (resI = 0; resI < ResNo; resI++) {
    for (bridgeI = 0; bridgeI < ResList[resI].bridgeNo; bridgeI++) {
      bridgeP = ResList[resI].bridges + bridgeI;

      for (inc1 = 1; inc1 <= 5; inc1++) {
	if (bridgeP->nextP != NULL)  /* already connected */
	  break;
      
	if (resI + inc1 == ResNo)
	  break;

	if (ResList[resI + inc1].helix4)
	  break;

	for (inc2 = 1; inc2 <= 5; inc2++) {
	  if (bridgeP->nextP != NULL)  /* already connected */
	    break;
      
	  if (inc1 == 1 && inc2 == 1)
	    continue;

	  if (inc1 > 2 && inc2 > 2)
	    continue;
	  
	  i1 = resI + inc1;
	  if (bridgeP->type == BT_PARALLEL)
	    i2 = bridgeP->ind2 + inc2;
	  else
	    i2 = bridgeP->ind2 - inc2;

	  if (i2 == -1 || i2 == ResNo)
	    break;

	  if (ResList[i2].helix4)
	    break;

	  bridgeP->nextP = getBridge(i1, i2, bridgeP->type);
	  if (bridgeP->nextP == NULL)
	    continue;
	  
	  if (bridgeP->nextP->prevP != NULL) {
	    bridgeP->nextP = NULL;
	    continue;
	  }

	  bridgeP->nextP->prevP = bridgeP;

	  for (i1 = resI; i1 <= resI + inc1; i1++)
	    if (! ResList[i1].sheet) {
	      ResList[i1].sheet = TRUE;
	      ResList[i1].bulge = TRUE;
	    }

	  if (bridgeP->type == BT_PARALLEL)
	    for (i2 = bridgeP->ind2; i2 <= bridgeP->ind2 + inc2; i2++) {
	      if (! ResList[i2].sheet) {
		ResList[i2].sheet = TRUE;
		ResList[i2].bulge = TRUE;
	      }
	    }
	  else
	    for (i2 = bridgeP->ind2 - inc2; i2 <= bridgeP->ind2; i2++) {
	      if (! ResList[i2].sheet) {
		ResList[i2].sheet = TRUE;
		ResList[i2].bulge = TRUE;
	      }
	    }
	}
      }
    }
  }
}

static void
setTurn(void)
{
  int i;

  for (i = 0; i < ResNo; i++) {
    ResList[i].turn3 = hbond(i, i + 3);
    ResList[i].turn4 = hbond(i, i + 4);
    ResList[i].turn5 = hbond(i, i + 5);
  }
}

static void
setHelix(void)
{
  int startI, i, k;

  for (i = 0; i < ResNo; i++) {
    ResList[i].helix3 = FALSE;
    ResList[i].helix4 = FALSE;
    ResList[i].helix5 = FALSE;
  }

  for (i = 1; i < ResNo; i++)
    if (ResList[i - 1].turn5 && ResList[i].turn5)
      for (k = 0; k < 5 && i + k < ResNo; k++)
	ResList[i + k].helix5 = TRUE;

  for (i = 1; i < ResNo; i++)
    if (ResList[i - 1].turn3 && ResList[i].turn3)
      for (k = 0; k < 3 && i + k < ResNo; k++) {
	ResList[i + k].helix3 = TRUE;
	ResList[i + k].helix5 = FALSE;
      }

  for (i = 1; i < ResNo; i++)
    if (ResList[i - 1].turn4 && ResList[i].turn4)
      for (k = 0; k < 4 && i + k < ResNo; k++) {
	ResList[i + k].helix4 = TRUE;
	ResList[i + k].helix5 = FALSE;
	ResList[i + k].helix3 = FALSE;
      }
  
  /* detect pieces of 3-helix that got too short because of
     overlap with 4-helix */
  startI = -1;
  for (i = 0; i < ResNo; i++) {
    if (ResList[i].helix3) {
      if (startI < 0)
	startI = i;
    } else {
      if (startI >= 0 && i - startI < 3)
	for (k = startI; k < i; k++) {
	  ResList[k].helix3 = FALSE;
	  ResList[k].turn3 = TRUE;
	}
      startI = -1;
    }
  }
  
  /* detect pieces of 5-helix that got too short because of
     overlap with 4-helix and 3-helix */
  startI = -1;
  for (i = 0; i < ResNo; i++) {
    if (ResList[i].helix5) {
      if (startI < 0)
	startI = i;
    } else {
      if (startI >= 0 && i - startI < 5)
	for (k = startI; k < i; k++) {
	  ResList[k].helix5 = FALSE;
	  ResList[k].turn5 = TRUE;
	}
      startI = -1;
    }
  }
}

static void
numHelix(void)
{
  int num, i;

  num = 0;
  i = 0;
  for (;;) {
    if (ResList[i].bridgeNo > 0) {
      ResList[i].helix3 = FALSE;
      ResList[i].helix5 = FALSE;
    }

    if (ResList[i].helix4) {
      num++;
      for (;;) {
	ResList[i++].num = num;
	if (i == ResNo || ! ResList[i].helix4)
	  break;
      }
    } else if (ResList[i].helix3) {
      num++;
      for (;;) {
	ResList[i++].num = num;
	if (i == ResNo || ! ResList[i].helix3)
	  break;
      }
    } else if (ResList[i].helix5) {
      num++;
      for (;;) {
	ResList[i++].num = num;
	if (i == ResNo || ! ResList[i].helix5)
	  break;
      }
    } else {
      i++;
    }

    if (i == ResNo)
      break;
  }
}

static void
setSheetNum(BridgeP bridgeP, int num, BOOL fromLeft)
{
  int i1, i2;
  BridgeP bP;
  int neighNo, pass, i;

  if (bridgeP == NULL || bridgeP->visited)
    return;

  bridgeP->visited = TRUE;

  i1 = bridgeP->ind1;
  i2 = bridgeP->ind2;

  ResList[i1].num = num;
  ResList[i2].num = num;

  if (fromLeft)
    DhResSetNeigh(ResList[i1].resP, NC_LEFT, ResList[i2].resP);
  else
    DhResSetNeigh(ResList[i1].resP, NC_RIGHT, ResList[i2].resP);

  setSheetNum(bridgeP->prevP, num, fromLeft);
  setSheetNum(bridgeP->nextP, num, fromLeft);

  neighNo = 0;

  /* make 2 passes through list of bridges, in first pass
     only take bridges that are part of a sheet (preferred) */
  for (pass = 0; pass < 2; pass++) {
    for (i = 0; i < ResList[i1].bridgeNo; i++) {
      if (neighNo == 1)  /* visit at most 1 additional neighbour */
	break;

      bP = ResList[i1].bridges + i;
      if (pass == 0 && bP->prevP == NULL && bP->nextP == NULL)
	continue;
      if (pass == 1 && (bP->prevP != NULL || bP->nextP != NULL))
	continue;
      
      if (bP->ind2 != i2) {
	setSheetNum(bP, num, ! fromLeft);
	neighNo++;
      }
    }
  }

  neighNo = 0;

  for (pass = 0; pass < 2; pass++) {
    for (i = 0; i < ResList[i2].bridgeNo; i++) {
      if (neighNo == 1)  /* visit at most 1 additional neighbour */
	break;

      bP = ResList[i2].bridges + i;
      if (pass == 0 && bP->prevP == NULL && bP->nextP == NULL)
	continue;
      if (pass == 1 && (bP->prevP != NULL || bP->nextP != NULL))
	continue;
      
      if (bP->ind2 != i1) {
	setSheetNum(bP, num, fromLeft);
	neighNo++;
      }
    }
  }
}

static void
numSheet(void)
{
  int sheetNo, i, k;
  BridgeP bridgeP;
  int *strandNumA, sheetNum;

  sheetNo = 0;
  for (i = 0; i < ResNo; i++) {
    if (! ResList[i].sheet)
      continue;

    if (ResList[i].num > 0)
      continue;

    if (i > 0 && ResList[i - 1].sheet) {
      ResList[i].num = ResList[i - 1].num;
      continue;
    }

    sheetNo++;

    for (k = 0; k < ResList[i].bridgeNo; k++) {
      bridgeP = ResList[i].bridges + k;
      if (bridgeP->prevP != NULL || bridgeP->nextP != NULL) {
	setSheetNum(bridgeP, sheetNo, TRUE);
	break;
      }
    }
  }

  if (sheetNo == 0)
    return;

  strandNumA = malloc(sheetNo * sizeof(*strandNumA));
  for (i = 0; i < sheetNo; i++)
    strandNumA[i] = 0;

  i = 0;
  for (;;) {
    if (ResList[i].sheet) {
      sheetNum = ResList[i].num - 1;
      strandNumA[sheetNum]++;
      for (;;) {
	ResList[i++].numStrand = strandNumA[sheetNum];
	if (i == ResNo || ! ResList[i].sheet ||
	    ResList[i].num - 1 != sheetNum)
	  break;
      }
    } else {
      i++;
    }

    if (i == ResNo)
      break;
  }

  free(strandNumA);
}

static void
getProp(char *name, void *clientData)
{
  LINLIST propList = clientData;
  DSTR str;

  if (strncmp(name, PROP_HELIX, strlen(PROP_HELIX)) == 0 ||
      strncmp(name, PROP_SHEET, strlen(PROP_SHEET)) == 0 ||
      strncmp(name, PROP_BULGE, strlen(PROP_BULGE)) == 0 ||
      strncmp(name, PROP_STRAND, strlen(PROP_STRAND)) == 0 ||
      strncmp(name, PROP_BRIDGE, strlen(PROP_BRIDGE)) == 0 ||
      strncmp(name, PROP_TURN, strlen(PROP_TURN)) == 0 ||
      strncmp(name, PROP_BEND, strlen(PROP_BEND)) == 0 ||
      strncmp(name, PROP_COIL, strlen(PROP_COIL)) == 0) {
    /* directly unsetting the property here would be dangerous,
       since we would modify the property table while the
       PropList function still uses it */
    str = DStrNew();
    DStrAssignStr(str, name);
    (void) ListInsertLast(propList, &str);
  }
}

static void
setProp(DhMolP molP)
{
  int maxResI;
  char *form;
  DhResP resP;
  LINLIST propList;
  DSTR *strP;
  BOOL notSet;
  char buf[20], propName[30];
  int i;

  PuWriteStr(TextW, DStrToStr(DhMolGetName(molP)));
  PuWriteStr(TextW, "\n");

  maxResI = DhResGetNumber(ResList[ResNo - 1].resP);
  if (maxResI >= 100000)
    form = "%6d";
  else if (maxResI >= 10000)
    form = "%5d";
  else if (maxResI >= 1000)
    form = "%4d";
  else if (maxResI >= 100)
    form = "%3d";
  else if (maxResI >= 10)
    form = "%2d";
  else
    form = "%1d";

  for (i = 0; i < ResNo; i++) {
    resP = ResList[i].resP;

    /* clear old secondary structure properties */
    propList = ListOpen(sizeof(DSTR));
    PropList(DhResGetPropTab(resP), getProp, propList);
    strP = ListFirst(propList);
    while (strP != NULL) {
      DhResSetProp(PropGetRef(DStrToStr(*strP), FALSE), resP, FALSE);
      DStrFree(*strP);
      strP = ListNext(propList, strP);
    }
    ListClose(propList);

    notSet = TRUE;

    (void) sprintf(buf, form, DhResGetNumber(resP));
    PuWriteStr(TextW, buf);
    (void) sprintf(buf, " %-5s", DStrToStr(DhResGetName(resP)));
    PuWriteStr(TextW, buf);

    if (notSet && ResList[i].helix4) {
      DhResSetProp(PropGetRef(PROP_HELIX, TRUE), resP, TRUE);
      DhResSetProp(PropGetRef(PROP_HELIX_ALPHA, TRUE), resP, TRUE);
      PuWriteStr(TextW, PROP_HELIX);
      PuWriteStr(TextW, " (");
      PuWriteStr(TextW, PROP_HELIX_ALPHA);

      (void) sprintf(propName, "%s%d", PROP_HELIX, ResList[i].num);
      DhResSetProp(PropGetRef(propName, TRUE), resP, TRUE);
      PuWriteStr(TextW, ", ");
      PuWriteStr(TextW, propName);
      PuWriteStr(TextW, ")");

      notSet = FALSE;
    }

    if (notSet && ResList[i].sheet) {
      DhResSetProp(PropGetRef(PROP_SHEET, TRUE), resP, TRUE);
      PuWriteStr(TextW, PROP_SHEET);
      PuWriteStr(TextW, " (");

      (void) sprintf(propName, "%s%d", PROP_SHEET, ResList[i].num);
      DhResSetProp(PropGetRef(propName, TRUE), resP, TRUE);
      PuWriteStr(TextW, propName);

      (void) sprintf(propName, "%s%d_%d", PROP_STRAND,
	  ResList[i].num, ResList[i].numStrand);
      DhResSetProp(PropGetRef(propName, TRUE), resP, TRUE);
      PuWriteStr(TextW, ", ");
      PuWriteStr(TextW, propName);

      if (ResList[i].bulge) {
	DhResSetProp(PropGetRef(PROP_BULGE, TRUE), resP, TRUE);
	PuWriteStr(TextW, ", ");
	PuWriteStr(TextW, PROP_BULGE);
      }

      PuWriteStr(TextW, ")");

      notSet = FALSE;
    }

    if (notSet && ResList[i].bridgeNo > 0) {
      DhResSetProp(PropGetRef(PROP_BRIDGE, TRUE), resP, TRUE);
      PuWriteStr(TextW, PROP_BRIDGE);
      notSet = FALSE;
    }

    if (notSet && ResList[i].helix3) {
      DhResSetProp(PropGetRef(PROP_HELIX, TRUE), resP, TRUE);
      DhResSetProp(PropGetRef(PROP_HELIX_3_10, TRUE), resP, TRUE);
      PuWriteStr(TextW, PROP_HELIX);
      PuWriteStr(TextW, " (");
      PuWriteStr(TextW, PROP_HELIX_3_10);

      (void) sprintf(propName, "%s%d", PROP_HELIX, ResList[i].num);
      DhResSetProp(PropGetRef(propName, TRUE), resP, TRUE);
      PuWriteStr(TextW, ", ");
      PuWriteStr(TextW, propName);
      PuWriteStr(TextW, ")");

      notSet = FALSE;
    }

    if (notSet && ResList[i].helix5) {
      DhResSetProp(PropGetRef(PROP_HELIX, TRUE), resP, TRUE);
      DhResSetProp(PropGetRef(PROP_HELIX_PI, TRUE), resP, TRUE);
      PuWriteStr(TextW, PROP_HELIX);
      PuWriteStr(TextW, " (");
      PuWriteStr(TextW, PROP_HELIX_PI);

      (void) sprintf(propName, "%s%d", PROP_HELIX, ResList[i].num);
      DhResSetProp(PropGetRef(propName, TRUE), resP, TRUE);
      PuWriteStr(TextW, ", ");
      PuWriteStr(TextW, propName);
      PuWriteStr(TextW, ")");

      notSet = FALSE;
    }

    if (notSet && (ResList[i].turn3 || ResList[i].turn4 || ResList[i].turn5)) {
      DhResSetProp(PropGetRef(PROP_TURN, TRUE), resP, TRUE);
      PuWriteStr(TextW, PROP_TURN);
      notSet = FALSE;
    }

    if (notSet && ResList[i].bend) {
      DhResSetProp(PropGetRef(PROP_BEND, TRUE), resP, TRUE);
      PuWriteStr(TextW, PROP_BEND);
      notSet = FALSE;
    }

    if (notSet) {
      DhResSetProp(PropGetRef(PROP_COIL, TRUE), resP, TRUE);
      PuWriteStr(TextW, PROP_COIL);
      notSet = FALSE;
    }

    PuWriteStr(TextW, "\n");
  }

  PuWriteStr(TextW, "\n");
}

static void
calcSecond(DhMolP molP, void *clientData)
{
  int i;

  if (BreakCheck(1))
    return;

  ResNo = 0;
  DhMolApplyRes(PropGetRef(PROP_SELECTED, FALSE), molP, addRes, NULL);
  if (ResNo == 0)
    return;

  setBend();

  findBond();

  setTurn();
  setHelix();
  setBridge();

  connectBridge();

  for (i = 0; i < ResNo; i++) {
    ResList[i].num = 0;
    DhResSetNeigh(ResList[i].resP, NC_LEFT, NULL);
    DhResSetNeigh(ResList[i].resP, NC_RIGHT, NULL);
  }

  numHelix();
  numSheet();

  setProp(molP);

  for (i = 0; i < ResNo; i++) {
    if (ResList[i].bonds != NULL)
      free(ResList[i].bonds);

    if (ResList[i].bridges != NULL)
      free(ResList[i].bridges);
  }
}

ErrCode
ExCalcSecondary(char *cmd)
{
  BreakActivate(TRUE);

  TextW = PuCreateTextWindow(cmd);

  ResSize = 10;
  ResList = malloc(ResSize * sizeof(*ResList));

  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), calcSecond, NULL);
  BreakActivate(FALSE);

  free(ResList);

  return EC_OK;
}
