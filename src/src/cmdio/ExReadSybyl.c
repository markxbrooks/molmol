/*
************************************************************************
*
*   ExReadSybyl.c - ReadSybyl command
*
*   Copyright (c) 1996-98
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadSybyl.c
*   SCCS identification       : 1.13
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <dstr.h>
#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <data_hand.h>
#include <data_ent.h>
#include <graph_draw.h>

#define LINE_LEN 10000

typedef enum {
  FS_ATOM,
  FS_BOND,
  FS_RES,
  FS_SET,
  FS_OTHER
} FileSection;

typedef struct {
  BOOL read;
  DSTR name;
  Vec3 coord;
  float charge;
  int resI;
  BOOL unique;
  DhAtomP atomP;
} AtomDescr;

typedef struct {
  BOOL read;
  int atom1I, atom2I;
} BondDescr;

typedef struct {
  BOOL read;
  BOOL catalog;
  DSTR name;
  int num;
  DhResP resP;
} ResDescr;

typedef struct {
  BOOL read;
  PropRefP refP;
  DataEntityType ent;
  int *indA;
  int indNo;
} SetDescr;

static GFileRes
readLine(GFile gf, char *line)
{
  GFileRes res;
  int pos, len;

  pos = 0;
  for (;;) {
    res = GFileReadLine(gf, line + pos, LINE_LEN - pos);
    if (res != GF_RES_OK)
      return res;

    len = strlen(line + pos);
    if (len > 0 && line[pos + len - 1] == '\\')
      pos += len - 1;
    else
      break;
  }

  return GF_RES_OK;
}

static DhMolP
buildMol(AtomDescr *atomA, int atomNo, BOOL setCharges,
    BondDescr *bondA, int bondNo, ResDescr *resA, int resNo,
    SetDescr *setA, int setNo)
{
  DhMolP molP;
  int atomI, bondI, resI, setI;
  DhResDefP resDefP;
  DhResP resP;
  DhAtomP atomP;
  DhBondP bondP;
  char buf[20];
  int ind, i;

  molP = DhMolNew();

  if (resA[0].read) {
    /* there are atoms without a residue, create an artificial one */
    resA[0].name = DStrNew();
    DStrAssignStr(resA[0].name, "none");
    resA[0].num = 0;
  }

  for (resI = 0; resI <= resNo; resI++) {
    if (! resA[resI].read)
      continue;

    resDefP = NULL;
    if (resA[resI].catalog)
      resDefP = DhResDefGet(resA[resI].name);
    if (resDefP == NULL)
      resDefP = DhResDefNew(resA[resI].name);
    DStrFree(resA[resI].name);
    
    resA[resI].resP = DhResNew(molP, resDefP, SP_LAST);
    DhResSetNumber(resA[resI].resP, resA[resI].num);
  }

  /* make atom names unique */

  for (atomI = 0; atomI < atomNo; atomI++) {
    if (! atomA[atomI].read)
      continue;

    if (! resA[atomA[atomI].resI].read) {
      atomA[atomI].read = FALSE;
      continue;
    }

    atomA[atomI].unique = TRUE;
  }

  for (atomI = 0; atomI < atomNo; atomI++) {
    if (! atomA[atomI].read)
      continue;

    if (! atomA[atomI].unique)
      continue;

    for (i = atomI + 1; i < atomNo; i++) {
      if (! atomA[i].read)
	continue;

      if (! atomA[i].unique)
	continue;

      if (atomA[atomI].resI == atomA[i].resI &&
	  DStrCmp(atomA[atomI].name, atomA[i].name) == 0) {
	atomA[atomI].unique = FALSE;
	atomA[i].unique = FALSE;
      }
    }
  }

  for (atomI = 0; atomI < atomNo; atomI++) {
    if (! atomA[atomI].read)
      continue;

    if (! atomA[atomI].unique) {
      (void) sprintf(buf, "<%d>", atomI + 1);
      DStrAppStr(atomA[atomI].name, buf);
    }
  }

  for (atomI = 0; atomI < atomNo; atomI++) {
    if (! atomA[atomI].read)
      continue;

    resP = resA[atomA[atomI].resI].resP;
    atomP = DhAtomFindName(resP, atomA[atomI].name, TRUE);
    if (atomP == NULL)
      atomP = DhAtomNew(resP, atomA[atomI].name);

    DhAtomSetCoord(atomP, atomA[atomI].coord);
    if (setCharges)
      DhAtomSetCharge(atomP, atomA[atomI].charge);
  }

  /* atom pointers can change when adding new atoms to a residue,
     get updated ones */
  for (atomI = 0; atomI < atomNo; atomI++) {
    if (! atomA[atomI].read)
      continue;

    resP = resA[atomA[atomI].resI].resP;
    atomA[atomI].atomP = DhAtomFindName(resP, atomA[atomI].name, FALSE);
    DStrFree(atomA[atomI].name);
  }

  for (bondI = 0; bondI < bondNo; bondI++) {
    if (! bondA[bondI].read)
      continue;
    
    if (! (atomA[bondA[bondI].atom1I].read &&
	atomA[bondA[bondI].atom2I].read)) {
      bondA[bondI].read = FALSE;
      continue;
    }

    bondP = DhBondNew(
	atomA[bondA[bondI].atom1I].atomP, atomA[bondA[bondI].atom2I].atomP);

    /* we set the properties here, the bondP can change when we add
       new bonds to a residue, and getting the updated one later is
       a pain */
    for (setI = 0; setI < setNo; setI++) {
      if (! setA[setI].read)
	continue;
      
      if (setA[setI].ent != DE_BOND)
	continue;

      for (i = 0; i < setA[setI].indNo; i++) {
	if (setA[setI].indA[i] == bondI)
	  DhBondSetProp(setA[setI].refP, bondP, TRUE);
      }
    }
  }

  for (setI = 0; setI < setNo; setI++) {
    if (! setA[setI].read)
      continue;
    
    for (i = 0; i < setA[setI].indNo; i++) {
      ind = setA[setI].indA[i];

      switch (setA[setI].ent) {
	case DE_ATOM:
	  if (ind >= 0 && ind < atomNo && atomA[ind].read)
	    DhAtomSetProp(setA[setI].refP, atomA[ind].atomP, TRUE);
	  break;
	case DE_BOND:
	  /* already done, see above */
	  break;
	case DE_RES:
	  if (ind >= 0 && ind <= resNo && resA[ind].read)
	    DhResSetProp(setA[setI].refP, resA[ind].resP, TRUE);
	  break;
      }
    }

    free(setA[setI].indA);
  }

  return molP;
}

ErrCode
ExReadSybyl(char *cmd)
{
  GFile gf;
  GFileRes res;
  DSTR fileName;
  ErrCode errCode;
  FileSection sect;
  int molNo, atomNo, bondNo, resNo, setNo, setI;
  AtomDescr *atomA;
  BondDescr *bondA;
  ResDescr *resA;
  SetDescr *setA;
  char line[LINE_LEN], buf1[100], buf2[100], buf3[100], dummyStr[100], *str;
  DSTR molName;
  BOOL setCharges;
  DhMolP molP;
  Vec3 cent;
  int dummyInt, readNo, ind, indI, i;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.mol2", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  molNo = 0;
  atomNo = 0;
  bondNo = 0;
  resNo = 0;
  setNo = 0;
  molName = DStrNew();
  sect = FS_OTHER;
  res = GF_RES_OK;

  while (! GFileEOF(gf)) {
    res = readLine(gf, line);
    if (res != GF_RES_OK)
      break;

    if (line[0] == '\0' || line[0] == '#')
      continue;

    if (strcmp(line, "@<TRIPOS>MOLECULE") == 0) {
      if (atomNo > 0) {
	molP = buildMol(atomA, atomNo, setCharges, bondA, bondNo,
	    resA, resNo, setA, setNo);
	DhMolSetName(molP, molName);
	DhMolInit(molP);
	if (molNo == 0)
	  DhCalcCenter(cent, molP);
	DhMolSetRotPoint(molP, cent);
	GraphMolAdd(molP);
	molNo++;
	free(atomA);
	if (bondNo > 0)
	  free(bondA);
	free(resA);
	if (setNo > 0)
	  free(setA);
      }

      res = readLine(gf, line);
      if (res != GF_RES_OK)
	break;
      DStrAssignStr(molName, line);

      res = readLine(gf, line);
      if (res != GF_RES_OK)
	break;
      readNo = sscanf(line, "%d %d %d %d %d",
	  &atomNo, &bondNo, &resNo, &dummyInt, &setNo);
      if (readNo < 3 || atomNo <= 0 || resNo < 0) {
	CipSetError("incomplete MOLECULE definition");
	res = GF_RES_ERR;
	break;
      }

      atomA = malloc(atomNo * sizeof(*atomA));
      for (i = 0; i < atomNo; i++)
	atomA[i].read = FALSE;

      if (bondNo > 0) {
	bondA = malloc(bondNo * sizeof(*bondA));
	for (i = 0; i < bondNo; i++)
	  bondA[i].read = FALSE;
      }

      resA = malloc((resNo + 1) * sizeof(*resA));
      for (i = 0; i <= resNo; i++)
	resA[i].read = FALSE;

      if (setNo > 0) {
	setA = malloc(setNo * sizeof(*setA));
	for (i = 0; i < setNo; i++)
	  setA[i].read = FALSE;
      }
      setI = 0;

      (void) readLine(gf, line);  /* skip molecule type */
      res = readLine(gf, line);
      if (res != GF_RES_OK)
	break;

      setCharges = (strcmp(line, "NO_CHARGES") != 0);

      sect = FS_OTHER;
    } else if (strcmp(line, "@<TRIPOS>ATOM") == 0) {
      sect = FS_ATOM;
    } else if (strcmp(line, "@<TRIPOS>BOND") == 0) {
      sect = FS_BOND;
    } else if (strcmp(line, "@<TRIPOS>SUBSTRUCTURE") == 0) {
      sect = FS_RES;
    } else if (strcmp(line, "@<TRIPOS>SET") == 0) {
      sect = FS_SET;
    } else if (line[0] == '@') {
      sect = FS_OTHER;
    } else {
      ind = atoi(line) - 1;

      if (sect == FS_ATOM && ind >= 0 && ind < atomNo) {
	readNo = sscanf(line, "%d %s %f %f %f %s %d %s %f",
	    &dummyInt, buf1,
	    atomA[ind].coord + 0, atomA[ind].coord + 1, atomA[ind].coord + 2,
	    buf2, &atomA[ind].resI, dummyStr, &atomA[ind].charge);
	if (readNo < 5) {
	  CipSetError("incomplete ATOM definition");
	  res = GF_RES_ERR;
	  break;
	}

	if (readNo < 7)
          atomA[ind].resI = 0;

	if (readNo < 9)
          atomA[ind].charge = 0.0f;

	if (strcmp(buf2, "Du") != 0 &&
	    atomA[ind].resI >= 0 && atomA[ind].resI <= resNo) {
	  if (buf1[0] == '*')
	    (void) sprintf(buf1, "%s<%d>", buf2, ind + 1);
	  atomA[ind].read = TRUE;
	  atomA[ind].name = DStrNew();
	  if (buf1[0] != buf2[0]) {
	    /* The way things are currently implemented, MOLMOL
	       determines the atom type from the first letter. So
	       if this does not match with the atom type specified
	       in the file, we prepend the first letter of it to
	       the name. */
	    DStrAppChar(atomA[ind].name, buf2[0]);
	    DStrAppStr(atomA[ind].name, buf1);
	  } else {
	    DStrAssignStr(atomA[ind].name, buf1);
	  }
	  if (atomA[ind].resI == 0)
	    resA[0].read = TRUE;
	}
      } else if (sect == FS_BOND && ind >= 0 && ind < bondNo) {
	readNo = sscanf(line, "%d %d %d %s",
	    &dummyInt, &bondA[ind].atom1I, &bondA[ind].atom2I, buf1);
	if (readNo < 3) {
	  CipSetError("incomplete BOND definition");
	  res = GF_RES_ERR;
	  break;
	}

	if (strcmp(buf1, "du") != 0 &&
	    bondA[ind].atom1I > 0 && bondA[ind].atom1I <= atomNo &&
	    bondA[ind].atom2I > 0 && bondA[ind].atom2I <= atomNo) {
	  bondA[ind].read = TRUE;
	  bondA[ind].atom1I--;
	  bondA[ind].atom2I--;
	}
      } else if (sect == FS_RES && ind >= 0 && ind < resNo) {
	readNo = sscanf(line, "%d %s %d %s %d %s %s",
	    &dummyInt, buf1, &dummyInt, dummyStr, &dummyInt, dummyStr, buf2);
	if (readNo < 7) {
	  CipSetError("incomplete SUBSTRUCTURE definition");
	  res = GF_RES_ERR;
	  break;
	}

	ind++;  /* start at 1, 0 is "none" */
	resA[ind].read = TRUE;
	resA[ind].name = DStrNew();
	if (buf1[0] == '*') {
	  resA[ind].catalog = FALSE;
	  (void) sprintf(buf1, "<%d>", ind + 1);
	  DStrAssignStr(resA[ind].name, buf1);
	  resA[ind].num = ind + 1;
	} else if (buf2[0] == '*') {
	  resA[ind].catalog = FALSE;
	  DStrAssignStr(resA[ind].name, buf1);
	  resA[ind].num = ind + 1;
	} else {
	  resA[ind].catalog = TRUE;
	  DStrAssignStr(resA[ind].name, buf2);
	  if (strncmp(buf1, buf2, strlen(buf2)) == 0)
	    resA[ind].num = atoi(buf1 + strlen(buf2));
	  else
	    resA[ind].num = ind + 1;
	}
      } else if (sect == FS_SET && setI < setNo) {
	readNo = sscanf(line, "%s %s %s",
	    buf1, buf2, buf3);
	if (readNo < 3) {
	  CipSetError("incomplete SET definition");
	  res = GF_RES_ERR;
	  break;
	}

	if (strcmp(buf2, "STATIC") == 0) {
	  /* SYBYL uses special characters like $ in set names, we
	     don't like that, so we replace them by _ */
	  str = buf1;
	  while (*str != '\0') {
	    if (! isalnum(*str))
	      *str = '_';
	    str++;
	  }

	  setA[setI].read = TRUE;
	  setA[setI].refP = PropGetRef(buf1, TRUE);
	  if (strcmp(buf3, "ATOMS") == 0)
	    setA[setI].ent = DE_ATOM;
	  else if (strcmp(buf3, "BONDS") == 0)
	    setA[setI].ent = DE_BOND;
	  else
	    setA[setI].ent = DE_RES;

	  setA[setI].indNo = 0;
	  for (;;) {
	    res = GFileReadLine(gf, line, LINE_LEN);
	    if (res != GF_RES_OK)
	      break;
	    str = line;

	    if (setA[setI].indNo == 0) {
	      setA[setI].indNo = atoi(line);
	      setA[setI].indA = malloc(setA[setI].indNo *
		  sizeof(*setA[setI].indA));
	      while (isdigit(*str))
		str++;
	      indI = 0;
	    }

	    for (;;) {
	      while (isspace(*str))
		str++;

	      if (! isdigit(*str))
		break;

	      if (indI >= setA[setI].indNo) {
		CipSetError("inconsistent SET definition");
		res = GF_RES_ERR;
		break;
	      }

	      setA[setI].indA[indI] = atoi(str) - 1;
	      indI++;
	      while (isdigit(*str))
		str++;
	    }

	    if (res != GF_RES_OK)
	      break;

	    if (*str != '\\')
	      break;
	  }

	  if (res != GF_RES_OK)
	    break;

	  if (indI < setA[setI].indNo) {
	    CipSetError("incomplete SET definition");
	    res = GF_RES_ERR;
	    break;
	  }
	} else {
	  (void) readLine(gf, line);
	}

	setI++;
      }
    }
  }

  if (res == GF_RES_OK && atomNo > 0) {
    molP = buildMol(atomA, atomNo, setCharges, bondA, bondNo,
	resA, resNo, setA, setNo);
    DhMolSetName(molP, molName);
    DhMolInit(molP);
    if (molNo == 0)
      DhCalcCenter(cent, molP);
    DhMolSetRotPoint(molP, cent);
    GraphMolAdd(molP);
    free(atomA);
    if (bondNo > 0)
      free(bondA);
    free(resA);
    if (setNo > 0)
      free(setA);
  }

  DStrFree(molName);
  GFileClose(gf);

  GraphRedrawNeeded();

  if (res != GF_RES_OK)
    return EC_ERROR;

  return EC_OK;
}
