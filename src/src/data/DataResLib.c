/*
************************************************************************
*
*   DataResLib.c - manage residue library
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/data/SCCS/s.DataResLib.c
*   SCCS identification       : 1.33
*
************************************************************************
*/

#include <data_hand.h>
#include <data_lib.h>
#include "data_res_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <strmatch.h>
#include <par_names.h>
#include <setup_file.h>
#include "data_struc.h"

#define PSEUDO_NAME "PSEUD"
#define LINE_LEN 200
#define NAME_LEN 30
#define NUM_LEN 30

typedef struct {
  DSTR resName;
  DSTR atomName;
  float charge;
} ChargeEntry;

static struct DhAtomTypeS UnknownAtomType = {NULL, -1, AK_OTHER, -10.0f, 0};

static LINLIST AtomTypeList = NULL;
static LINLIST ResDefList = NULL;
static LINLIST ChargeList = NULL;

static DhResLibReportFunc ReportF = NULL;

static void
reportStr(char *str)
{
  if (ReportF != NULL)
    ReportF(str);
}

static void
reportInt(int i)
{
  char buf[20];

  if (ReportF != NULL) {
    (void) sprintf(buf, "%d", i);
    ReportF(buf);
  }
}

static void
freeAtomType(void *p, void *clientData)
{
  DhAtomTypeP atomTypeP = p;

  DStrFree(atomTypeP->name);
}

static void
freeResDef(void *p, void *clientData)
{
  DhResDefP resDefP = p;
  int i;

  DStrFree(resDefP->name);

  for (i = 0; i < resDefP->atomNo; i++)
    DStrFree(resDefP->atomA[i].name);

  for (i = 0; i < resDefP->angleNo; i++)
    DStrFree(resDefP->angleA[i].name);

  free(resDefP->atomA);
  free(resDefP->bondA);
  free(resDefP->angleA);
}

static void
openList(void)
{
  AtomTypeList = ListOpen(sizeof(struct DhAtomTypeS));
  ListAddDestroyCB(AtomTypeList, freeAtomType, NULL, NULL);

  ResDefList = ListOpen(sizeof(struct DhResDefS));
  ListAddDestroyCB(ResDefList, freeResDef, NULL, NULL);
}

static void
getLinePart(char *subBuf, char *lineBuf, int start, int len)
{
  (void) strncpy(subBuf, lineBuf + start, len);

  /* remove trailing blanks */
  while (len > 0 && subBuf[len - 1] == ' ')
    len--;

  subBuf[len] = '\0';
}

static BOOL
readAtomTypes(GFile gf, int typeNo)
{
  GFileRes res;
  char lineBuf[LINE_LEN];
  char name[NAME_LEN], numBuf[NUM_LEN];
  struct DhAtomTypeS typeS;
  int i;

  for (i = 0; i < typeNo; i++) {
    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK) {
      reportStr("read error after ");
      reportInt(i);
      reportStr(" ATOMTYPES\n");
      return FALSE;
    }

    getLinePart(name, lineBuf, 5, 5);
    typeS.name = DStrNew();
    DStrAssignStr(typeS.name, name);

    getLinePart(numBuf, lineBuf, 10, 10);
    typeS.vdw = (float) atof(numBuf);

    getLinePart(numBuf, lineBuf, 20, 5);
    typeS.hBondCap = atoi(numBuf);

    getLinePart(numBuf, lineBuf, 25, 5);

    if (atoi(numBuf) > 1)
      typeS.kind = AK_HEAVY;
    else if (atoi(numBuf) == 1)
      typeS.kind = AK_HYDROGEN;
    else if (strcmp(name, PSEUDO_NAME) == 0)
      typeS.kind = AK_PSEUDO;
    else
      typeS.kind = AK_OTHER;

    (void) ListInsertLast(AtomTypeList, &typeS);
  }

  reportInt(typeNo);
  reportStr(" ATOMTYPES read\n");

  return TRUE;
}

static DhAtomTypeP
findAtomType(char *name)
{
  DhAtomTypeP typeP;

  typeP = ListFirst(AtomTypeList);
  while (typeP != NULL) {
    if (strcmp(DStrToStr(typeP->name), name) == 0)
      return typeP;

    typeP = ListNext(AtomTypeList, typeP);
  }

  return NULL;
}

static DhAtomTypeP
getUnknownAtomType(void)
{
  if (UnknownAtomType.name == NULL) {
    UnknownAtomType.name = DStrNew();
    DStrAssignStr(UnknownAtomType.name, "UNK");
  }

  return &UnknownAtomType;
}

static BOOL
readAngles(GFile gf, DhAngleDefP angleA, int angleNo)
{
  GFileRes res;
  char lineBuf[LINE_LEN];
  char name[NAME_LEN], numBuf[NUM_LEN];
  int i;

  for (i = 0; i < angleNo; i++) {
    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK) {
      reportStr("read error after ");
      reportInt(i);
      reportStr(" angles\n");
      return FALSE;
    }

    getLinePart(name, lineBuf, 5, 5);
    angleA[i].name = DStrNew();
    DStrAssignStr(angleA[i].name, name);

    getLinePart(numBuf, lineBuf, 30, 5);
    angleA[i].atom1I = atoi(numBuf) - 1;

    getLinePart(numBuf, lineBuf, 35, 5);
    angleA[i].atom2I = atoi(numBuf) - 1;

    getLinePart(numBuf, lineBuf, 40, 5);
    angleA[i].atom3I = atoi(numBuf) - 1;

    getLinePart(numBuf, lineBuf, 45, 5);
    angleA[i].atom4I = atoi(numBuf) - 1;

    getLinePart(numBuf, lineBuf, 50, 5);
    angleA[i].lastAtomI = atoi(numBuf) - 1;
  }

  reportStr("    ");
  reportInt(angleNo);
  reportStr(" angles read\n");

  return TRUE;
}

static void
freeChargeEntry(void *p, void *clientData)
{
  ChargeEntry *entryP = p;

  DStrFree(entryP->resName);
  DStrFree(entryP->atomName);
}

static float
getSimpleCharge(DSTR resName, DSTR atomName)
{
  GFile gf;
  GFileRes res;
  ChargeEntry entry, *entryP;
  char resBuf[100], atomBuf[100];
  FLOAT32 f32;

  if (ChargeList == NULL) {
    ChargeList = ListOpen(sizeof(ChargeEntry));
    ListAddDestroyCB(ChargeList, freeChargeEntry, NULL, NULL);

    gf = SetupOpen(PN_ATOM_CHARGE, "AtomCharge", FALSE);
    if (gf == NULL)
      return 0.0f;

    while (! GFileEOF(gf)) {
      res = GFileReadStr(gf, resBuf, sizeof(resBuf));
      if (res != GF_RES_OK)
	break;

      res = GFileReadStr(gf, atomBuf, sizeof(atomBuf));
      if (res != GF_RES_OK)
	break;

      res = GFileReadFLOAT32(gf, &f32);
      if (res != GF_RES_OK)
	break;
      
      entry.resName = DStrNew();
      DStrAssignStr(entry.resName, resBuf);
      entry.atomName = DStrNew();
      DStrAssignStr(entry.atomName, atomBuf);
      entry.charge = f32;

      (void) ListInsertLast(ChargeList, &entry);
    }

    GFileClose(gf);
  }

  entryP = ListFirst(ChargeList);
  while (entryP != NULL) {
    if (StrMatch(DStrToStr(resName), DStrToStr(entryP->resName)) &&
	StrMatch(DStrToStr(atomName), DStrToStr(entryP->atomName)))
      return entryP->charge;
    entryP = ListNext(ChargeList, entryP);
  }

  return 0.0f;
}

static BOOL
oldIsPseudo(DhAtomDefP atomA, int pAtomI, int atomI)
/* Return whether atom with index pAtomI is pseudo atom for
   atom with index atomI. Old method used for backwards compatibility
   of old dump files, information is now in residue library. */
{
  char *pName, *name;
  int len;

  if (atomA[pAtomI].typeP->kind != AK_PSEUDO)
    return FALSE;

  if (pAtomI == atomI)
    return FALSE;

  pName = DStrToStr(atomA[pAtomI].name);
  name = DStrToStr(atomA[atomI].name);

  pName++;  /* leave away leading Q */
  len = strlen(pName);
  if (len == 0)
    return FALSE;

  if (pName[0] == 'R')
    /* ring pseudo atom */
    return (strcmp(name, "QD") == 0 || strcmp(name, "QE") == 0 ||
	strcmp(name, "HZ") == 0);
  
  if (name[0] == 'H')
    name++;
  
  if (strncmp(pName, name, len - 1) != 0)
    return FALSE;

  if (pName[len - 1] == '\'') {
    /* make single quote match double quote (DNA residues) */
    if (name[len - 1] != '\'' && name[len - 1] != '"')
      return FALSE;
  } else if (pName[len - 1] == '*') {
    /* make * match double quote (DNA residues with PDB naming) */
    if (name[len - 1] != '*' && name[len - 1] != '"')
      return FALSE;
  } else {
    if (pName[len - 1] != name[len - 1])
      return FALSE;
  }
  
  return (len <= (int) strlen(name));
}

static BOOL
readAtoms(GFile gf, DSTR resName, DhAtomDefP atomA, int atomNo,
    DhBondDefP bondA, int *bondNoP)
{
  GFileRes res;
  int bondNo;
  char lineBuf[LINE_LEN];
  char name[NAME_LEN], numBuf[NUM_LEN];
  int i, k, i2;

  bondNo = 0;

  for (i = 0; i < atomNo; i++) {
    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK) {
      *bondNoP = bondNo;
      reportStr("read error after ");
      reportInt(i);
      reportStr(" atoms\n");
      return FALSE;
    }

    getLinePart(name, lineBuf, 5, 5);
    atomA[i].name = DStrNew();
    DStrAssignStr(atomA[i].name, name);

    getLinePart(name, lineBuf, 10, 5);
    atomA[i].typeP = findAtomType(name);
    if (atomA[i].typeP == NULL) {
      reportStr("unknown atom type: ");
      reportStr(name);
      reportStr("\n");
      return FALSE;
    }

    getLinePart(numBuf, lineBuf, 20, 10);
    atomA[i].charge = (float) atof(numBuf);
    atomA[i].heavyCharge = 0.0f;
    atomA[i].avgCharge = 0.0f;
    atomA[i].simpleCharge = getSimpleCharge(resName, atomA[i].name);

    getLinePart(numBuf, lineBuf, 30, 10);
    atomA[i].coord[0] = (float) atof(numBuf);
    getLinePart(numBuf, lineBuf, 40, 10);
    atomA[i].coord[1] = (float) atof(numBuf);
    getLinePart(numBuf, lineBuf, 50, 10);
    atomA[i].coord[2] = (float) atof(numBuf);

    for (k = 0; k < 4; k++) {
      getLinePart(numBuf, lineBuf, 60 + 5 * k, 5);
      i2 = atoi(numBuf) - 1;
      if (i2 < 0)
	break;

      if (i2 >= atomNo) {
	reportStr("atom index (");
	reportInt(i2 + 1);
	reportStr(") out of range in bonds of atom ");
	reportInt(i + 1);
	reportStr("\n");
	return FALSE;
      }

      if (i2 <= i)  /* take bond only in one direction */
	continue;

      bondA[bondNo].atom1I = i;
      bondA[bondNo].atom2I = i2;
      bondA[bondNo].pseudoI = i2;
      bondNo++;
    }

    getLinePart(numBuf, lineBuf, 80, 5);
    atomA[i].pseudoI = atoi(numBuf) - 1;
  }

  *bondNoP = bondNo;

  reportStr("    ");
  reportInt(atomNo);
  reportStr(" atoms and ");
  reportInt(bondNo);
  reportStr(" bonds read\n");

  /* handle old library format */
  for (i = 0; i < atomNo; i++) {
    k = atomA[i].pseudoI;
    while (k >= 0) {
      if (k == i)
	break;
      k = atomA[k].pseudoI;
    }

    if (k >= 0) {
      reportStr("    ");
      /* cycle detected, old format */
      reportStr("warning: cycle in pseudo atom definiton (old format?)\n");
      for (k = 0; k < atomNo; k++)
	if (oldIsPseudo(atomA, k, i)) {
	  atomA[i].pseudoI = k;
	  break;
	}
    }
  }

  return TRUE;
}

static int
getValidAtomI(DhResDefP defP, int atomI)
{
  int i;

  if (atomI < defP->firstAtomI) {
    for (i = defP->atomNo - 1; i >= defP->firstAtomI; i--)
      if (DStrCmp(defP->atomA[i].name, defP->atomA[atomI].name) == 0)
	return i;
  } else if (atomI > defP->lastAtomI) {
    for (i = 0; i <= defP->lastAtomI; i++)
      if (DStrCmp(defP->atomA[i].name, defP->atomA[atomI].name) == 0)
	return i;
  }

  return atomI;
}

static void
calcCharges(DhResDefP defP)
{
  int neighNo;
  float potPart;
  int i, k, i2;

  for (i = 0; i < defP->atomNo; i++)
    defP->atomA[i].avgCharge = 0.0f;

  for (i = defP->firstAtomI; i <= defP->lastAtomI; i++) {
    if (defP->atomA[i].typeP->kind != AK_HEAVY) {
      defP->atomA[i].heavyCharge = 0.0f;
      continue;
    }

    neighNo = 0;
    defP->atomA[i].heavyCharge = defP->atomA[i].charge;

    for (k = 0; k < defP->bondNo; k++) {
      if (defP->bondA[k].atom1I == i)
	i2 = defP->bondA[k].atom2I;
      else if (defP->bondA[k].atom2I == i)
	i2 = defP->bondA[k].atom1I;
      else
	continue;
      
      if (defP->atomA[i2].typeP->kind == AK_HEAVY)
	neighNo++;
      else
	defP->atomA[i].heavyCharge += defP->atomA[i2].charge;
    }

    potPart = defP->atomA[i].heavyCharge / (neighNo + 1);
    defP->atomA[i].avgCharge += potPart;

    for (k = 0; k < defP->bondNo; k++) {
      if (defP->bondA[k].atom1I == i)
	i2 = getValidAtomI(defP, defP->bondA[k].atom2I);
      else if (defP->bondA[k].atom2I == i)
	i2 = getValidAtomI(defP, defP->bondA[k].atom1I);
      else
	continue;
      
      if (defP->atomA[i2].typeP->kind == AK_HEAVY)
        defP->atomA[i2].avgCharge += potPart;
    }
  }

  for (i = 0; i < defP->firstAtomI; i++) {
    i2 = getValidAtomI(defP, i);
    defP->atomA[i].heavyCharge = defP->atomA[i2].heavyCharge;
    defP->atomA[i].avgCharge = defP->atomA[i2].avgCharge;
  }

  for (i = defP->lastAtomI + 1; i < defP->atomNo; i++) {
    i2 = getValidAtomI(defP, i);
    defP->atomA[i].heavyCharge = defP->atomA[i2].heavyCharge;
    defP->atomA[i].avgCharge = defP->atomA[i2].avgCharge;
  }
}

static void
fillPseudoI(DhResDefP defP)
{
  int bondI, atomI;
  DhBondDefP bondDefP;
  int i;

  for (bondI = 0; bondI < defP->bondNo; bondI++) {
    bondDefP = defP->bondA + bondI;
    atomI = defP->atomA[bondDefP->atom2I].pseudoI;
    if (atomI < 0)
      continue;

    if (strcmp(DStrToStr(defP->atomA[atomI].name), "QR") == 0)
      continue;  /* don't do anything for ring pseudo atom */

    bondDefP->pseudoI = atomI;
    i = bondI - 1;
    while (i >= 0 && defP->bondA[i].atom1I == bondDefP->atom1I) {
      if (defP->bondA[i].pseudoI == atomI) {
	bondDefP->pseudoI = -1;
	break;
      }
      i--;
    }
  }
}

static void
readResLib(void)
{
  char *fileName;
  GFile gf;
  GFileRes res;
  struct DhResDefS resDefS;
  char lineBuf[LINE_LEN];
  char name[NAME_LEN], numBuf[NUM_LEN];
  int i;

  if (ResDefList == NULL)
    openList();
  
  if (ListSize(ResDefList) > 0) {
    reportStr("library already read (do InitAll first)\n");
    return;
  }

  fileName = SetupGetName(PN_RES_LIB, "amber94.lib", FALSE);
  reportStr("library name: ");
  reportStr(fileName);
  reportStr("\n");

  gf = SetupOpen(PN_RES_LIB, "amber94.lib", FALSE);
  if (gf == NULL) {
    reportStr("open failed\n");
    return;
  }

  while (! GFileEOF(gf)) {
    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK) {
      reportStr("read error\n");
      break;
    }
      
    if (strncmp(lineBuf, "ATOMTYPES ", 10) == 0) {
      getLinePart(numBuf, lineBuf, 10, 5);
      if (! readAtomTypes(gf, atoi(numBuf))) {
	reportStr("error while reading ATOMTYPES, terminating\n");
	return;
      }
    } else if (strncmp(lineBuf, "RESIDUE   ", 10) == 0 ||
	strncmp(lineBuf, "HEADRES   ", 10) == 0 ||
	strncmp(lineBuf, "TAILRES   ", 10) == 0) {
      getLinePart(name, lineBuf, 10, 5);
      resDefS.name = DStrNew();
      DStrAssignStr(resDefS.name, name);

      getLinePart(numBuf, lineBuf, 15, 5);
      resDefS.angleNo = atoi(numBuf);

      getLinePart(numBuf, lineBuf, 20, 5);
      resDefS.atomNo = atoi(numBuf);

      reportStr("reading ");
      reportStr(name);
      reportStr(" (");
      reportInt(resDefS.angleNo);
      reportStr(" angles, ");
      reportInt(resDefS.atomNo);
      reportStr(" atoms)\n");

      getLinePart(numBuf, lineBuf, 25, 5);
      resDefS.firstAtomI = atoi(numBuf) - 1;

      getLinePart(numBuf, lineBuf, 30, 5);
      resDefS.lastAtomI = atoi(numBuf) - 1;

      resDefS.angleA = malloc(resDefS.angleNo * sizeof(struct DhAngleDefS));
      if (! readAngles(gf, resDefS.angleA, resDefS.angleNo)) {
	reportStr("error while reading angles, terminating\n");
	return;
      }

      resDefS.atomA = malloc(resDefS.atomNo * sizeof(struct DhAtomDefS));
      /* allocate safe upper bound, realloc later */
      resDefS.bondA = malloc(4 * resDefS.atomNo * sizeof(struct DhBondDefS));
      if (! readAtoms(gf, resDefS.name, resDefS.atomA, resDefS.atomNo,
	  resDefS.bondA, &resDefS.bondNo)) {
	reportStr("error while reading atoms, terminating\n");
	return;
      }

      resDefS.bondA = realloc(resDefS.bondA,
	  resDefS.bondNo * sizeof(struct DhBondDefS));

      calcCharges(&resDefS);

      resDefS.firstBondI = 0;
      for (i = 0; i < resDefS.bondNo; i++)
	if (resDefS.bondA[i].atom1I >= resDefS.firstAtomI) {
	  resDefS.firstBondI = i;
	  break;
	}

      resDefS.lastBondI = resDefS.bondNo - 1;
      for (i = resDefS.bondNo - 1; i >= 0; i--)
	if (resDefS.bondA[i].atom1I <= resDefS.lastAtomI) {
	  resDefS.lastBondI = i;
	  break;
	}

      fillPseudoI(&resDefS);

      resDefS.refCount = 0;
      resDefS.isFromLib = TRUE;
      resDefS.isCopy = FALSE;

      (void) ListInsertLast(ResDefList, &resDefS);
    }
  }

  if (GFileEOF(gf))
    reportStr("library successfully read\n");

  GFileClose(gf);
}

DhResDefP
DhResDefNew(DSTR name)
{
  struct DhResDefS resDefS;

  readResLib();  /* atom types are needed! */

  resDefS.name = DStrNew();
  DStrAssignDStr(resDefS.name, name);
  resDefS.atomNo = 0;
  resDefS.atomA = NULL;
  resDefS.firstAtomI = 0;
  resDefS.lastAtomI = -1;
  resDefS.bondNo = 0;
  resDefS.bondA = NULL;
  resDefS.firstBondI = 0;
  resDefS.lastBondI = -1;
  resDefS.angleNo = 0;
  resDefS.angleA = NULL;
  resDefS.refCount = 1;
  resDefS.isFromLib = FALSE;
  resDefS.isCopy = FALSE;

  return ListInsertLast(ResDefList, &resDefS);
}

static DhResDefP
findName(DSTR name)
{
  DhResDefP resDefP;

  resDefP = ListFirst(ResDefList);
  while (resDefP != NULL) {
    if (DStrCmp(resDefP->name, name) == 0 &&
	! resDefP->isCopy) {
      resDefP->refCount++;
      return resDefP;
    }
    resDefP = ListNext(ResDefList, resDefP);
  }

  return NULL;
}

DhResDefP
DhResDefGet(DSTR name)
{
  DhResDefP resDefP;
  DSTR altName;

  readResLib();

  resDefP = findName(name);
  if (resDefP == NULL) {
    altName = DStrNew();
    DStrAssignDStr(altName, name);
    DStrAppChar(altName, '+');
    resDefP = findName(altName);

    if (resDefP == NULL) {
      DStrAssignDStr(altName, name);
      DStrAppChar(altName, '-');
      resDefP = findName(altName);
    }

    DStrFree(altName);
  }

  return resDefP;
}

DhResDefP
DhResDefGetRef(DhResDefP resDefP)
{
  resDefP->refCount++;
  return resDefP;
}

void
DhResDefReturn(DhResDefP resDefP)
{
  resDefP->refCount--;

  if (resDefP->refCount == 0 && ! resDefP->isFromLib)
    ListRemove(ResDefList, resDefP);
}

static DhResDefP
copyEntry(DhResDefP resDefP)
{
  struct DhResDefS copyS;
  int i;

  copyS.name = DStrNew();
  DStrAssignDStr(copyS.name, resDefP->name);
  copyS.atomNo = resDefP->atomNo;
  if (copyS.atomNo > 0) {
    copyS.atomA = malloc(copyS.atomNo * sizeof(*copyS.atomA));
    for (i = 0; i < copyS.atomNo; i++) {
      copyS.atomA[i] = resDefP->atomA[i];
      copyS.atomA[i].name = DStrNew();
      DStrAssignDStr(copyS.atomA[i].name, resDefP->atomA[i].name);
    }
  } else {
    copyS.atomA = NULL;
  }
  copyS.firstAtomI = resDefP->firstAtomI;
  copyS.lastAtomI = resDefP->lastAtomI;

  copyS.bondNo = resDefP->bondNo;
  if (copyS.bondNo > 0) {
    copyS.bondA = malloc(copyS.bondNo * sizeof(*copyS.bondA));
    for (i = 0; i < copyS.bondNo; i++)
      copyS.bondA[i] = resDefP->bondA[i];
  } else {
    copyS.bondA = NULL;
  }
  copyS.firstBondI = resDefP->firstBondI;
  copyS.lastBondI = resDefP->lastBondI;

  copyS.angleNo = resDefP->angleNo;
  if (copyS.angleNo > 0) {
    copyS.angleA = malloc(copyS.angleNo * sizeof(*copyS.angleA));
    for (i = 0; i < copyS.angleNo; i++) {
      copyS.angleA[i] = resDefP->angleA[i];
      copyS.angleA[i].name = DStrNew();
      DStrAssignDStr(copyS.angleA[i].name, resDefP->angleA[i].name);
    }
  } else {
    copyS.angleA = NULL;
  }

  copyS.refCount = 1;
  copyS.isFromLib = FALSE;
  copyS.isCopy = TRUE;

  return ListInsertLast(ResDefList, &copyS);
}

static BOOL
compEntry(DhResDefP def1P, DhResDefP def2P)
{
  int i, k;

  if (DStrCmp(def1P->name, def2P->name) != 0)
    return FALSE;

  if (def1P->atomNo != def2P->atomNo)
    return FALSE;

  for (i = 0; i < def1P->atomNo; i++) {
    if (DStrCmp(def1P->atomA[i].name, def2P->atomA[i].name) != 0)
      return FALSE;
    for (k = 0; k < 3; k++)
      if (def1P->atomA[i].coord[k] != def2P->atomA[i].coord[k])
        return FALSE;
    if (def1P->atomA[i].charge != def2P->atomA[i].charge)
      return FALSE;
    if (def1P->atomA[i].heavyCharge != def2P->atomA[i].heavyCharge)
      return FALSE;
    if (def1P->atomA[i].avgCharge != def2P->atomA[i].avgCharge)
      return FALSE;
    if (def1P->atomA[i].simpleCharge != def2P->atomA[i].simpleCharge)
      return FALSE;
    if (def1P->atomA[i].pseudoI != def2P->atomA[i].pseudoI)
      return FALSE;
    if (def1P->atomA[i].typeP != def2P->atomA[i].typeP)
      return FALSE;
  }

  if (def1P->firstAtomI != def2P->firstAtomI)
    return FALSE;
  if (def1P->lastAtomI != def2P->lastAtomI)
    return FALSE;

  if (def1P->bondNo != def2P->bondNo)
    return FALSE;

  for (i = 0; i < def1P->bondNo; i++) {
    if (def1P->bondA[i].atom1I != def2P->bondA[i].atom1I)
      return FALSE;
    if (def1P->bondA[i].atom2I != def2P->bondA[i].atom2I)
      return FALSE;
    if (def1P->bondA[i].pseudoI != def2P->bondA[i].pseudoI)
      return FALSE;
  }

  if (def1P->firstBondI != def2P->firstBondI)
    return FALSE;
  if (def1P->lastBondI != def2P->lastBondI)
    return FALSE;

  if (def1P->angleNo != def2P->angleNo)
    return FALSE;

  for (i = 0; i < def1P->angleNo; i++) {
    if (DStrCmp(def1P->angleA[i].name, def2P->angleA[i].name) != 0)
      return FALSE;
    if (def1P->angleA[i].atom1I != def2P->angleA[i].atom1I)
      return FALSE;
    if (def1P->angleA[i].atom2I != def2P->angleA[i].atom2I)
      return FALSE;
    if (def1P->angleA[i].atom3I != def2P->angleA[i].atom3I)
      return FALSE;
    if (def1P->angleA[i].atom4I != def2P->angleA[i].atom4I)
      return FALSE;
    if (def1P->angleA[i].lastAtomI != def2P->angleA[i].lastAtomI)
      return FALSE;
  }

  return TRUE;
}

static DhResDefP
findEntry(DhResDefP resDefP)
{
  DhResDefP compP;

  compP = ListFirst(ResDefList);
  while (compP != resDefP) {
    if (compEntry(resDefP, compP)) {
      DhResDefReturn(resDefP);
      compP->refCount++;
      return compP;
    }

    compP = ListNext(ResDefList, compP);
  }

  return resDefP;
}

BOOL
DhResDefIsPseudo(DhResDefP defP, int pAtomI, int atomI)
/* return whether atom with index pAtomI is pseudo atom for
   atom with index atomI */
{
  return defP->atomA[atomI].pseudoI == pAtomI;
}

int
DhResDefAddAtom(DhResDefP *resDefPP, int ind, DSTR name)
{
  DhResDefP resDefP;
  char firstCh;
  DhAtomTypeP typeP;
  int i;

  resDefP = *resDefPP;
  for (i = 0; i < resDefP->atomNo; i++)
    if (DStrCmp(name, resDefP->atomA[i].name) == 0)
      return -1;

  if ((*resDefPP)->refCount > 1 || (*resDefPP)->isFromLib)
    *resDefPP = copyEntry(*resDefPP);
  
  resDefP = *resDefPP;

  resDefP->atomNo++;
  if (resDefP->atomA == NULL) {
    resDefP->atomA = malloc(sizeof(*resDefP->atomA));
  } else {
    resDefP->atomA = realloc(resDefP->atomA,
	resDefP->atomNo * sizeof(*resDefP->atomA));
  }

  resDefP->lastAtomI++;

  for (i = resDefP->atomNo - 1; i > ind; i--)
    resDefP->atomA[i] = resDefP->atomA[i - 1];
  
  for (i = 0; i < resDefP->bondNo; i++) {
    if (resDefP->bondA[i].atom1I >= ind)
      resDefP->bondA[i].atom1I++;
    if (resDefP->bondA[i].atom2I >= ind)
      resDefP->bondA[i].atom2I++;
    if (resDefP->bondA[i].pseudoI >= ind)
      resDefP->bondA[i].pseudoI++;
  }

  for (i = 0; i < resDefP->angleNo; i++) {
    if (resDefP->angleA[i].atom1I >= ind)
      resDefP->angleA[i].atom1I++;
    if (resDefP->angleA[i].atom2I >= ind)
      resDefP->angleA[i].atom2I++;
    if (resDefP->angleA[i].atom3I >= ind)
      resDefP->angleA[i].atom3I++;
    if (resDefP->angleA[i].atom4I >= ind)
      resDefP->angleA[i].atom4I++;
  }

  resDefP->atomA[ind].name = DStrNew();
  DStrAssignDStr(resDefP->atomA[ind].name, name);
  Vec3Zero(resDefP->atomA[ind].coord);
  resDefP->atomA[ind].charge = 0.0f;
  resDefP->atomA[ind].heavyCharge = 0.0f;
  resDefP->atomA[ind].avgCharge = 0.0f;
  resDefP->atomA[ind].simpleCharge =
      getSimpleCharge(resDefP->name, resDefP->atomA[ind].name);
  resDefP->atomA[ind].pseudoI = -1;

  firstCh = DStrToStr(name)[0];
  typeP = ListFirst(AtomTypeList);
  while (typeP != NULL) {
    if (typeP->kind != AK_PSEUDO && DStrToStr(typeP->name)[0] == firstCh)
      break;
    typeP = ListNext(AtomTypeList, typeP);
  }

  if (typeP == NULL)
    typeP = getUnknownAtomType();

  resDefP->atomA[ind].typeP = typeP;

  calcCharges(resDefP);

  if (resDefP->refCount == 1 && ! resDefP->isFromLib)
    *resDefPP = findEntry(resDefP);

  return ind;
}

int
DhResDefAddBond(DhResDefP *resDefPP, int atom1I, int atom2I)
{
  DhResDefP resDefP;
  int ind, i;

  resDefP = *resDefPP;

  for (ind = 0; ind < resDefP->bondNo; ind++) {
    if (resDefP->bondA[ind].atom1I < atom1I)
      continue;
    if (resDefP->bondA[ind].atom1I > atom1I)
      break;
    if (resDefP->bondA[ind].atom2I < atom2I)
      continue;
    if (resDefP->bondA[ind].atom2I > atom2I)
      break;
    return -1;  /* already exists */
  }

  if (resDefP->refCount > 1 || resDefP->isFromLib) {
    resDefP = copyEntry(resDefP);
    *resDefPP = resDefP;
  }
  
  resDefP->bondNo++;
  if (resDefP->bondA == NULL) {
    resDefP->bondA = malloc(sizeof(*resDefP->bondA));
  } else {
    resDefP->bondA = realloc(resDefP->bondA,
	resDefP->bondNo * sizeof(*resDefP->bondA));
  }

  resDefP->lastBondI++;

  for (i = resDefP->bondNo - 1; i > ind; i--)
    resDefP->bondA[i] = resDefP->bondA[i - 1];
  
  resDefP->bondA[ind].atom1I = atom1I;
  resDefP->bondA[ind].atom2I = atom2I;
  resDefP->bondA[ind].pseudoI = atom2I;

  if (resDefP->refCount == 1 && ! resDefP->isFromLib)
    *resDefPP = findEntry(resDefP);

  return ind;
}

void
DhResDefRemoveBond(DhResDefP *resDefPP, int bondI)
{
  DhResDefP resDefP;
  int i;

  if ((*resDefPP)->refCount > 1 || (*resDefPP)->isFromLib)
    *resDefPP = copyEntry(*resDefPP);
  
  resDefP = *resDefPP;
  
  for (i = bondI + 1; i < resDefP->bondNo; i++)
    resDefP->bondA[i - 1] = resDefP->bondA[i];

  resDefP->bondNo--;
  resDefP->lastBondI--;

  if (resDefP->refCount == 1 && ! resDefP->isFromLib)
    *resDefPP = findEntry(resDefP);
}

void
DhResDefRemoveAngle(DhResDefP *resDefPP, int angleI)
{
  DhResDefP resDefP;
  int i;

  if ((*resDefPP)->refCount > 1 || (*resDefPP)->isFromLib)
    *resDefPP = copyEntry(*resDefPP);
  
  resDefP = *resDefPP;
  
  for (i = angleI + 1; i < resDefP->angleNo; i++)
    resDefP->angleA[i - 1] = resDefP->angleA[i];

  resDefP->angleNo--;

  if (resDefP->refCount == 1 && ! resDefP->isFromLib)
    *resDefPP = findEntry(resDefP);
}

void
DhResDefSetPseudo(DhResDefP *resDefPP, int pseudoI, int indA[], int atomNo)
{
  DhResDefP resDefP;
  int i;

  if ((*resDefPP)->refCount > 1 || (*resDefPP)->isFromLib)
    *resDefPP = copyEntry(*resDefPP);

  resDefP = *resDefPP;
  
  resDefP->atomA[pseudoI].typeP = findAtomType(PSEUDO_NAME);

  for (i = 0; i < atomNo; i++)
    resDefP->atomA[indA[i]].pseudoI = pseudoI;

  if (resDefP->refCount == 1 && ! resDefP->isFromLib)
    *resDefPP = findEntry(resDefP);
}

void
DhResDefSetCharge(DhResDefP *resDefPP, int ind, float charge)
{
  DhResDefP resDefP;

  if ((*resDefPP)->atomA[ind].charge == charge)
    return;

  if ((*resDefPP)->refCount > 1 || (*resDefPP)->isFromLib)
    *resDefPP = copyEntry(*resDefPP);
  
  resDefP = *resDefPP;
  
  resDefP->atomA[ind].charge = charge;
  calcCharges(resDefP);

  if (resDefP->refCount == 1 && ! resDefP->isFromLib)
    *resDefPP = findEntry(resDefP);
}

void
DhResLibRead(DhResLibReportFunc reportF)
{
  ReportF = reportF;
  readResLib();
  ReportF = NULL;
}

void
DhResLibDestroyAll(void)
{
  ListClose(AtomTypeList);
  AtomTypeList = NULL;
  ListClose(ResDefList);
  ResDefList = NULL;
  ListClose(ChargeList);
  ChargeList = NULL;
}

#define CHECK_RES(s) if (s != GF_RES_OK) return FALSE

BOOL
DhResLibDump(GFile gf)
{
  int listSize;
  DhAtomTypeP typeP;
  DhResDefP defP;
  int num, i;

  listSize = ListSize(AtomTypeList);
  CHECK_RES(GFileWriteINT32(gf, listSize));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  typeP = ListFirst(AtomTypeList);
  num = 0;
  while (typeP != NULL) {
    typeP->num = num;
    num++;

    CHECK_RES(GFileWriteQuotedStr(gf, DStrToStr(typeP->name)));
    CHECK_RES(GFileWriteNL(gf));

    CHECK_RES(GFileWriteINT16(gf, (INT16) typeP->kind));
    CHECK_RES(GFileWriteFLOAT32(gf, typeP->vdw));
    CHECK_RES(GFileWriteINT32(gf, typeP->hBondCap));
    CHECK_RES(GFileWriteNL(gf));

    typeP = ListNext(AtomTypeList, typeP);
  }

  CHECK_RES(GFileWriteINT32(gf, ListSize(ResDefList)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  defP = ListFirst(ResDefList);
  num = 0;
  while (defP != NULL) {
    defP->num = num;
    num++;

    CHECK_RES(GFileWriteQuotedStr(gf, DStrToStr(defP->name)));
    CHECK_RES(GFileWriteNL(gf));

    CHECK_RES(GFileWriteINT32(gf, defP->atomNo));
    CHECK_RES(GFileWriteINT32(gf, defP->firstAtomI));
    CHECK_RES(GFileWriteINT32(gf, defP->lastAtomI));
    CHECK_RES(GFileWriteNL(gf));
    for (i = 0; i < defP->atomNo; i++) {
      CHECK_RES(GFileWriteQuotedStr(gf, DStrToStr(defP->atomA[i].name)));
      CHECK_RES(GFileWriteFLOAT32(gf, defP->atomA[i].coord[0]));
      CHECK_RES(GFileWriteFLOAT32(gf, defP->atomA[i].coord[1]));
      CHECK_RES(GFileWriteFLOAT32(gf, defP->atomA[i].coord[2]));
      CHECK_RES(GFileWriteFLOAT32(gf, defP->atomA[i].charge));
      CHECK_RES(GFileWriteFLOAT32(gf, defP->atomA[i].simpleCharge));
      CHECK_RES(GFileWriteINT32(gf, defP->atomA[i].pseudoI));
      CHECK_RES(GFileWriteINT32(gf, defP->atomA[i].typeP->num));
      CHECK_RES(GFileWriteNL(gf));
    }
    CHECK_RES(GFileWriteNL(gf));

    CHECK_RES(GFileWriteINT32(gf, defP->bondNo));
    CHECK_RES(GFileWriteINT32(gf, defP->firstBondI));
    CHECK_RES(GFileWriteINT32(gf, defP->lastBondI));
    CHECK_RES(GFileWriteNL(gf));
    for (i = 0; i < defP->bondNo; i++) {
      CHECK_RES(GFileWriteINT32(gf, defP->bondA[i].atom1I));
      CHECK_RES(GFileWriteINT32(gf, defP->bondA[i].atom2I));
      CHECK_RES(GFileWriteINT32(gf, defP->bondA[i].pseudoI));
      CHECK_RES(GFileWriteNL(gf));
    }
    CHECK_RES(GFileWriteNL(gf));

    CHECK_RES(GFileWriteINT32(gf, defP->angleNo));
    CHECK_RES(GFileWriteNL(gf));
    for (i = 0; i < defP->angleNo; i++) {
      CHECK_RES(GFileWriteQuotedStr(gf, DStrToStr(defP->angleA[i].name)));
      CHECK_RES(GFileWriteINT32(gf, defP->angleA[i].atom1I));
      CHECK_RES(GFileWriteINT32(gf, defP->angleA[i].atom2I));
      CHECK_RES(GFileWriteINT32(gf, defP->angleA[i].atom3I));
      CHECK_RES(GFileWriteINT32(gf, defP->angleA[i].atom4I));
      CHECK_RES(GFileWriteINT32(gf, defP->angleA[i].lastAtomI));
      CHECK_RES(GFileWriteNL(gf));
    }
    CHECK_RES(GFileWriteNL(gf));

    CHECK_RES(GFileWriteINT32(gf, defP->refCount));
    CHECK_RES(GFileWriteINT16(gf, (INT16) defP->isFromLib));
    CHECK_RES(GFileWriteINT16(gf, (INT16) defP->isCopy));
    CHECK_RES(GFileWriteNL(gf));

    defP = ListNext(ResDefList, defP);
  }

  return TRUE;
}

BOOL
DhResLibUndump(GFile gf, int vers)
{
  int listSize, idx;
  struct DhAtomTypeS typeS;
  struct DhResDefS defS;
  INT16 i16;
  INT32 i32;
  FLOAT32 f32;
  char name[NAME_LEN];
  int i, k;

  DhResLibDestroyAll();

  CHECK_RES(GFileReadINT32(gf, &i32));
  listSize = i32;

  openList();

  for (idx = 0; idx < listSize; idx++) {
    CHECK_RES(GFileReadQuotedStr(gf, name, sizeof(name)));
    typeS.name = DStrNew();
    DStrAssignStr(typeS.name, name);

    CHECK_RES(GFileReadINT16(gf, &i16));
    typeS.kind = i16;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    typeS.vdw = f32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    typeS.hBondCap = i32;

    (void) ListInsertLast(AtomTypeList, &typeS);
  }

  CHECK_RES(GFileReadINT32(gf, &i32));
  listSize = i32;

  for (idx = 0; idx < listSize; idx++) {
    CHECK_RES(GFileReadQuotedStr(gf, name, sizeof(name)));
    defS.name = DStrNew();
    DStrAssignStr(defS.name, name);

    CHECK_RES(GFileReadINT32(gf, &i32));
    defS.atomNo = i32;
    defS.atomA = malloc(defS.atomNo * sizeof(struct DhAtomDefS));
    CHECK_RES(GFileReadINT32(gf, &i32));
    defS.firstAtomI = i32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    defS.lastAtomI = i32;
    for (i = 0; i < defS.atomNo; i++) {
      CHECK_RES(GFileReadQuotedStr(gf, name, sizeof(name)));
      defS.atomA[i].name = DStrNew();
      DStrAssignStr(defS.atomA[i].name, name);
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      defS.atomA[i].coord[0] = f32;
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      defS.atomA[i].coord[1] = f32;
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      defS.atomA[i].coord[2] = f32;
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      defS.atomA[i].charge = f32;
      if (vers < 10) {
	/* pot used to be in dump file, the fields that replace
	   it (heavyCharge and avgCharge) are calculated now */
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
      }
      if (vers >= 11) {
	/* simpleCharge introduced in version 11 */
	CHECK_RES(GFileReadFLOAT32(gf, &f32));
	defS.atomA[i].simpleCharge = f32;
      } else {
	defS.atomA[i].simpleCharge =
	    getSimpleCharge(defS.name, defS.atomA[i].name);
      }
      if (vers >= 12) {
	/* pseudoI introduced in version 12 */
	CHECK_RES(GFileReadINT32(gf, &i32));
	defS.atomA[i].pseudoI = i32;
      } else {
	defS.atomA[i].pseudoI = -1;
      }
      CHECK_RES(GFileReadINT32(gf, &i32));
      if (i32 == -1)
	defS.atomA[i].typeP = getUnknownAtomType();
      else
	defS.atomA[i].typeP = ListPos(AtomTypeList, i32);
    }

    if (vers < 12)
      /* pseudoI missing, fill with old method */
      for (i = 0; i < defS.atomNo; i++)
	for (k = 0; k < defS.atomNo; k++)
	  if (oldIsPseudo(defS.atomA, k, i)) {
	    defS.atomA[i].pseudoI = k;
	    break;
	  }

    CHECK_RES(GFileReadINT32(gf, &i32));
    defS.bondNo = i32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    defS.firstBondI = i32;
    CHECK_RES(GFileReadINT32(gf, &i32));
    defS.lastBondI = i32;
    defS.bondA = malloc(defS.bondNo * sizeof(struct DhBondDefS));
    for (i = 0; i < defS.bondNo; i++) {
      CHECK_RES(GFileReadINT32(gf, &i32));
      defS.bondA[i].atom1I = i32;
      CHECK_RES(GFileReadINT32(gf, &i32));
      defS.bondA[i].atom2I = i32;
      if (vers >= 5)
	CHECK_RES(GFileReadINT32(gf, &i32));
      defS.bondA[i].pseudoI = i32;
    }

    if (vers >= 10)
      calcCharges(&defS);

    CHECK_RES(GFileReadINT32(gf, &i32));
    defS.angleNo = i32;
    defS.angleA = malloc(defS.angleNo * sizeof(struct DhAngleDefS));
    for (i = 0; i < defS.angleNo; i++) {
      CHECK_RES(GFileReadQuotedStr(gf, name, sizeof(name)));
      defS.angleA[i].name = DStrNew();
      DStrAssignStr(defS.angleA[i].name, name);
      CHECK_RES(GFileReadINT32(gf, &i32));
      defS.angleA[i].atom1I = i32;
      CHECK_RES(GFileReadINT32(gf, &i32));
      defS.angleA[i].atom2I = i32;
      CHECK_RES(GFileReadINT32(gf, &i32));
      defS.angleA[i].atom3I = i32;
      CHECK_RES(GFileReadINT32(gf, &i32));
      defS.angleA[i].atom4I = i32;
      CHECK_RES(GFileReadINT32(gf, &i32));
      defS.angleA[i].lastAtomI = i32;
    }

    CHECK_RES(GFileReadINT32(gf, &i32));
    defS.refCount = i32;
    CHECK_RES(GFileReadINT16(gf, &i16));
    defS.isFromLib = i16;
    CHECK_RES(GFileReadINT16(gf, &i16));
    defS.isCopy = i16;
    CHECK_RES(GFileWriteNL(gf));

    (void) ListInsertLast(ResDefList, &defS);
  }

  return TRUE;
}

DhResDefP
DhResDefFindIndex(int idx)
{
  return ListPos(ResDefList, idx);
}
