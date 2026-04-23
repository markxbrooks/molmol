/*
************************************************************************
*
*   DataWriteRes.c - write entry for residue library
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/12/05
*   Pathname of SCCS file     : /local/home/kor/molmol/src/data/SCCS/s.DataWriteRes.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include <data_hand.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_struc.h"
#include "data_res_lib.h"

#define LINE_LEN 200

typedef enum {
  TS_INVALID,
  TS_NOT_REACHED,
  TS_PROCESSING,
  TS_DONE
} TravState;

typedef struct {
  BOOL bb;
  int newI;
  int lastI;
  TravState state;
} TransTabEntry;

static BOOL
containsLast(DhResDefP defP, TransTabEntry *tab, int lastI, int ind)
{
  int i;
  BOOL res;

  if (tab[ind].state == TS_INVALID)
    return FALSE;

  if (ind == lastI) {
    tab[ind].bb = TRUE;
    tab[ind].state = TS_DONE;

    return TRUE;
  }

  if (tab[ind].state == TS_PROCESSING)
    return FALSE;

  if (tab[ind].state == TS_DONE)
    return tab[ind].bb;

  tab[ind].state = TS_PROCESSING;

  res = FALSE;
  for (i = 0; i < defP->bondNo; i++) {
    if (defP->bondA[i].atom1I == ind)
      res = res | containsLast(defP, tab, lastI, defP->bondA[i].atom2I);
    else if (defP->bondA[i].atom2I == ind)
      res = res | containsLast(defP, tab, lastI, defP->bondA[i].atom1I);
  }

  tab[ind].bb = res;
  tab[ind].state = TS_DONE;

  return res;
}

static int
addPseudo(DhResDefP defP, TransTabEntry *tab, int oldI, int newI)
{
  int i;

  for (i = 0; i < defP->atomNo; i++) {
    if (tab[i].state != TS_NOT_REACHED)
      continue;
    
    if (! DhResDefIsPseudo(defP, i, oldI))
      continue;
    
    newI = addPseudo(defP, tab, i, newI);

    tab[i].newI = newI;
    tab[i].state = TS_DONE;
    newI++;
  }

  return newI;
}

static int
setNew(DhResDefP defP, TransTabEntry *tab, int maxI, int oldI, int newI)
{
  int pass, i1, i2;

  if (oldI > maxI)
    return newI;

  if (tab[oldI].state != TS_NOT_REACHED)
    return newI;

  newI = addPseudo(defP, tab, oldI, newI);

  tab[oldI].newI = newI;
  tab[oldI].state = TS_DONE;
  newI++;

  /* pass 0: not bb, pass 1: bb */
  for (pass = 0; pass < 2; pass++) {
    for (i1 = 0; i1 < defP->bondNo; i1++) {
      if (defP->bondA[i1].atom1I == oldI)
	i2 = defP->bondA[i1].atom2I;
      else if (defP->bondA[i1].atom2I == oldI)
	i2 = defP->bondA[i1].atom1I;
      else
	continue;

      if (pass == 0 && tab[i2].bb)
	continue;

      if (pass == 1 && ! tab[i2].bb)
	continue;

      newI = setNew(defP, tab, maxI, i2, newI);
    }
  }

  tab[oldI].lastI = newI - 1;

  return newI;
}

static void
findResConnect(DhResP resP, DhResP nResP, int *indP, int *nIndP)
{
  DhBondAddP bondP;

  bondP = ListFirst(resP->molP->bondL);
  while (bondP != NULL) {
    if (bondP->res1P == resP && bondP->res2P == nResP) {
      *indP = bondP->atom1I;
      *nIndP = bondP->atom2I;
      return;
    }
    if (bondP->res2P == resP && bondP->res1P == nResP) {
      *indP = bondP->atom2I;
      *nIndP = bondP->atom1I;
      return;
    }
    bondP = ListNext(resP->molP->bondL, bondP);
  }
}

static void
findPrevConnect(DhResP resP, DhResP pResP,
    int *indP, int *pInd1P, int *pInd2P)
{
  DhResDefP defP, pDefP;
  DSTR name;
  int ind, i;

  *indP = -1;
  *pInd1P = -1;
  *pInd2P = -1;

  if (pResP == NULL)
    return;

  defP = resP->defP;
  pDefP = pResP->defP;

  for (i = 0; i < pDefP->bondNo; i++) {
    if (pDefP->bondA[i].atom2I > pDefP->lastAtomI) {
      ind = pDefP->bondA[i].atom2I;
      *pInd2P = pDefP->bondA[i].atom1I;
      break;
    }
  }

  if (i < pDefP->bondNo) {
    name = pDefP->atomA[ind].name;
    for (i = 0; i < defP->atomNo; i++)
      if (DStrCmp(name, defP->atomA[i].name) == 0) {
	*indP = i;
	break;
      }
  }

  if (*indP < 0) {
    findResConnect(resP, pResP, indP, pInd2P);
    ind = -1;
  }
  
  if (*indP >= 0) {
    for (i = pDefP->bondNo - 1; i >= 0; i--) {
      if (pDefP->bondA[i].atom1I == *pInd2P &&
	  pDefP->bondA[i].atom2I != ind) {
	*pInd1P = pDefP->bondA[i].atom2I;
	break;
      }
      if (pDefP->bondA[i].atom2I == *pInd2P &&
	  pDefP->bondA[i].atom1I != ind) {
	*pInd1P = pDefP->bondA[i].atom1I;
	break;
      }
    }

    if (*pInd1P < 0)  /* just take something */
      *pInd1P = *pInd2P - 1;
  }
}

static void
findNextConnect(DhResP resP, DhResP nResP, int *indP, int *nIndP)
{
  *indP = -1;
  *nIndP = -1;

  if (nResP == NULL)
    return;

  findResConnect(resP, nResP, indP, nIndP);
}

static void
sortInd(int *indA, int n)
{
  int i, k, t;

  for (i = 0; i < n - 1; i++)
    for (k = i + 1; k < n; k++)
      if (indA[i] > indA[k]) {
	t = indA[i];
	indA[i] = indA[k];
	indA[k] = t;
      }
}

static void
writeAngle(char *buf, int ind, DSTR name, int *atomIA)
{
  (void) sprintf(buf, "%4d %-5s%5d%5d%10.4f%5d%5d%5d%5d%5d",
      ind + 1, DStrToStr(name), 0, 0, 0.0,
      atomIA[0] + 1, atomIA[1] + 1, atomIA[2] + 1, atomIA[3] + 1,
      atomIA[4] + 1);
}

static void
writeAtom(char *buf, int ind, DhAtomDefP defP, DhAtomP atomP,
    int bonds[4], int pseudoI)
{
  (void) sprintf(buf,
      "%4d %-5s%-5s%5d%10.4f%10.4f%10.4f%10.4f%5d%5d%5d%5d%5d",
      ind + 1, DStrToStr(defP->name), DStrToStr(defP->typeP->name), 0,
      defP->charge, atomP->coord[0], atomP->coord[1], atomP->coord[2],
      bonds[0] + 1, bonds[1] + 1, bonds[2] + 1, bonds[3] + 1,
      pseudoI + 1);
}

BOOL
DhResDefWrite(GFile gf, DhResP resP)
{
  DhResDefP defP = resP->defP;
  BOOL hasOverlap;
  DhResP prevP, nextP;
  int firstI, lastI, maxI, newFirstI;
  int prev1I, prev2I, nextI, newNextI;
  TransTabEntry *tab;
  int outAngleNo, outAtomNo, lineNo, lineInd;
  char (*lines)[LINE_LEN];
  LINLIST angleL;
  DhAngleAddP angleP;
  BOOL valid;
  int oldI, newI, angleI, atomIA[5], bonds[4], bondNo, pseudoI;
  int i;

  if (defP->isFromLib) {
    for (i = defP->firstAtomI; i <= defP->lastAtomI; i++)
      if (resP->atomA[i].state != AS_VALID)
	break;
    if (i == defP->lastAtomI + 1)
      return TRUE;
  }

  hasOverlap = (defP->lastAtomI - defP->firstAtomI + 1 < defP->atomNo);

  if (hasOverlap) {
    firstI = defP->firstAtomI;

    for (i = 0; i < defP->bondNo; i++)
      if (defP->bondA[i].atom2I > defP->lastAtomI) {
	lastI = defP->bondA[i].atom1I;
	break;
      }

    maxI = defP->lastAtomI;

    prev1I = -1;
    prev2I = -1;
    nextI = -1;
  } else {
    prevP = ListPrev(resP->molP->resL, resP);
    nextP = ListNext(resP->molP->resL, resP);

    findPrevConnect(resP, prevP, &firstI, &prev1I, &prev2I);
    if (firstI == -1)
      firstI = 0;

    findNextConnect(resP, nextP, &lastI, &nextI);
    if (lastI == -1)
      lastI = defP->atomNo - 1;

    maxI = defP->atomNo - 1;
  }

  tab = malloc(defP->atomNo * sizeof(*tab));

  for (i = 0; i < defP->atomNo; i++)
    tab[i].state = TS_INVALID;

  if (hasOverlap) {
    newFirstI = 0;
    for (i = 0; i < defP->firstAtomI; i++)
      if (resP->atomA[i].state == AS_VALID) {
	tab[i].newI = newFirstI;
	tab[i].lastI = -1;
	tab[i].state = TS_DONE;
	newFirstI++;
      }
  } else if (prev1I >= 0) {
    newFirstI = 2;
  } else {
    newFirstI = 0;
  }

  for (i = defP->firstAtomI; i <= defP->lastAtomI; i++) {
    tab[i].bb = FALSE;
    if (resP->atomA[i].state == AS_VALID)
      tab[i].state = TS_NOT_REACHED;
  }
  (void) containsLast(defP, tab, lastI, firstI);

  for (i = defP->firstAtomI; i <= defP->lastAtomI; i++)
    if (tab[i].state != TS_INVALID)
      tab[i].state = TS_NOT_REACHED;

  newNextI = setNew(defP, tab, maxI, firstI, newFirstI);

  newI = newNextI;
  for (i = defP->lastAtomI + 1; i < defP->atomNo; i++)
    if (resP->atomA[i].state == AS_VALID) {
      tab[i].newI = newI;
      tab[i].lastI = -1;
      tab[i].state = TS_DONE;
      newI++;
    }

  /* buffer output because exact number of angles and
     atoms is not known in advance */
  
  lineNo = 10;
  lines = malloc(lineNo * sizeof(*lines));
  lineInd = 1;

  outAngleNo = 0;
  angleL = DhAngleListGet();
  /* angles must be sorted by atom 3 */
  for (newI = 0; newI < newFirstI + defP->atomNo; newI++) {
    for (oldI = 0; oldI < defP->atomNo; oldI++)
      if (tab[oldI].state == TS_DONE && tab[oldI].newI == newI)
	break;

    if (oldI == defP->atomNo)
      continue;

    for (angleI = 0; angleI < defP->angleNo; angleI++) {
      atomIA[0] = defP->angleA[angleI].atom1I;
      atomIA[1] = defP->angleA[angleI].atom2I;
      atomIA[2] = defP->angleA[angleI].atom3I;
      atomIA[3] = defP->angleA[angleI].atom4I;

      for (i = 0; i < 4; i++) {
	if (tab[atomIA[i]].state != TS_DONE)
	  break;
	atomIA[i] = tab[atomIA[i]].newI;
      }
      if (i < 4)
	continue;

      sortInd(atomIA, 4);

      if (atomIA[2] != newI)
	continue;

      atomIA[4] = tab[oldI].lastI;
      if (atomIA[4] >= tab[lastI].newI)
	atomIA[4] = -1;

      if (lineInd == lineNo) {
	lineNo *= 2;
	lines = realloc(lines, lineNo * sizeof(*lines));
      }
      writeAngle(lines[lineInd], outAngleNo, defP->angleA[angleI].name,
	  atomIA);
      outAngleNo++;
      lineInd++;
    }

    angleP = ListFirst(angleL);
    while (angleP != NULL) {
      if (angleP->res3P == resP) {
	atomIA[1] = angleP->atom2I;
	valid = (tab[atomIA[1]].state == TS_DONE);
	atomIA[1] = tab[atomIA[1]].newI;

	atomIA[2] = angleP->atom3I;
	valid = valid && (tab[atomIA[2]].state == TS_DONE);
	atomIA[2] = tab[atomIA[2]].newI;

	if (angleP->res1P == resP) {
	  atomIA[0] = angleP->atom1I;
	  valid = valid && (tab[atomIA[0]].state == TS_DONE);
	  atomIA[0] = tab[atomIA[0]].newI;
	} else if (angleP->atom1I == prev1I) {
	  atomIA[0] = 1;
	} else if (angleP->atom1I == prev2I) {
	  atomIA[0] = 0;
	}

	if (angleP->res4P == resP) {
	  atomIA[3] = angleP->atom4I;
	  valid = valid && (tab[atomIA[3]].state == TS_DONE);
	  atomIA[3] = tab[atomIA[3]].newI;
	} else if (angleP->atom4I == nextI) {
	  atomIA[3] = newNextI;
	}

	sortInd(atomIA, 4);

	atomIA[4] = tab[oldI].lastI;
	if (atomIA[4] >= tab[lastI].newI)
	  atomIA[4] = -1;

	if (valid && atomIA[2] == newI) {
	  if (lineInd == lineNo) {
	    lineNo *= 2;
	    lines = realloc(lines, lineNo * sizeof(*lines));
	  }
	  writeAngle(lines[lineInd], outAngleNo, angleP->name, atomIA);
	  outAngleNo++;
	  lineInd++;
	}
      }

      angleP = ListNext(angleL, angleP);
    }
  }

  outAtomNo = 0;

  if (prev1I >= 0) {
    /* take overlap atoms from previous residue */
    bonds[0] = 1;
    bonds[1] = 2;
    bonds[2] = -1;
    bonds[3] = -1;
    if (lineInd == lineNo) {
      lineNo *= 2;
      lines = realloc(lines, lineNo * sizeof(*lines));
    }
    writeAtom(lines[lineInd], outAtomNo,
	prevP->defP->atomA + prev2I, prevP->atomA + prev2I, bonds, -1);
    lineInd++;
    outAtomNo++;

    bonds[0] = 0;
    bonds[1] = -1;
    bonds[2] = -1;
    bonds[3] = -1;
    if (lineInd == lineNo) {
      lineNo *= 2;
      lines = realloc(lines, lineNo * sizeof(*lines));
    }
    writeAtom(lines[lineInd], outAtomNo,
	prevP->defP->atomA + prev1I, prevP->atomA + prev1I, bonds, -1);
    lineInd++;
    outAtomNo++;
  }

  for (newI = 0; newI < newFirstI + defP->atomNo; newI++) {
    for (oldI = 0; oldI < defP->atomNo; oldI++)
      if (tab[oldI].state == TS_DONE && tab[oldI].newI == newI)
	break;

    if (oldI == defP->atomNo)
      continue;

    bondNo = 0;

    if (oldI == firstI && prev2I >= 0) {
      bonds[0] = 0;
      bondNo++;
    } else if (oldI == lastI && nextI >= 0) {
      bonds[0] = newNextI;
      bondNo++;
    }

    for (i = 0; i < defP->bondNo; i++) {
      if (defP->bondA[i].atom1I == oldI &&
	  tab[defP->bondA[i].atom2I].state == TS_DONE) {
	bonds[bondNo] = tab[defP->bondA[i].atom2I].newI;
	bondNo++;
      } else if (defP->bondA[i].atom2I == oldI &&
	  tab[defP->bondA[i].atom1I].state == TS_DONE) {
	bonds[bondNo] = tab[defP->bondA[i].atom1I].newI;
	bondNo++;
      }

      if (bondNo == 4)
	break;
    }

    sortInd(bonds, bondNo);

    for (i = bondNo; i < 4; i++)
      bonds[i] = -1;

    if (lineInd == lineNo) {
      lineNo *= 2;
      lines = realloc(lines, lineNo * sizeof(*lines));
    }

    pseudoI = defP->atomA[oldI].pseudoI;
    if (pseudoI >= 0)
      pseudoI = tab[pseudoI].newI;

    writeAtom(lines[lineInd], outAtomNo,
	defP->atomA + oldI, resP->atomA + oldI, bonds, pseudoI);
    lineInd++;
    outAtomNo++;
  }
  
  if (nextI >= 0) {
    /* take overlap atoms from next residue */
    bonds[0] = lastI;
    bonds[1] = -1;
    bonds[2] = -1;
    bonds[3] = -1;
    if (lineInd == lineNo) {
      lineNo *= 2;
      lines = realloc(lines, lineNo * sizeof(*lines));
    }
    writeAtom(lines[lineInd], outAtomNo,
	nextP->defP->atomA + nextI, nextP->atomA + nextI, bonds, -1);
    lineInd++;
    outAtomNo++;
  }

  lineNo = lineInd;

  (void) sprintf(lines[0], "RESIDUE   %-5s%5d%5d%5d%5d%5d",
      DStrToStr(defP->name), outAngleNo, outAtomNo,
      newFirstI + 1, newNextI, 0);

  for (lineInd = 0; lineInd < lineNo; lineInd++) {
    if (GFileWriteLine(gf, lines[lineInd]) != GF_RES_OK)
      break;
  }

  free(lines);
  free(tab);

  return (lineInd == lineNo);
}
