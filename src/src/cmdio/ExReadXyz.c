/*
************************************************************************
*
*   ExReadXyz.c - ReadXyz command
*
*   Copyright (c) 1994-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadXyz.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>

#include <dstr.h>
#include <file_name.h>
#include <curr_dir.h>
#include <par_names.h>
#include <setup_file.h>
#include <arg.h>
#include <data_hand.h>
#include <graph_draw.h>

#define FIRST_LETTER 'A'
#define LAST_LETTER 'Z'
#define LETTER_NO (LAST_LETTER - FIRST_LETTER + 1)

static BOOL RadTabRead = FALSE;
static float RadTab[LETTER_NO];

static void
readRadTab(void)
{
  /* this code partly duplicates the one in ExCalcBond.c, would be
     nice to share it */
  GFile gf;
  GFileRes res;
  char buf[20], ch;
  FLOAT32 f32;
  INT16 i16;
  int i;

  if (RadTabRead)
    return;

  gf = SetupOpen(PN_ATOM_RADIUS, "AtomRadius", FALSE);
  if (gf == NULL)
    return;

  for (i = 0; i < LETTER_NO; i++)
    RadTab[i] = 0.0f;

  while (! GFileEOF(gf)) {
    res = GFileReadStr(gf, buf, sizeof(buf));
    if (res != GF_RES_OK)
      break;
    ch = buf[0];

    res = GFileReadFLOAT32(gf, &f32);
    if (res != GF_RES_OK)
      break;

    res = GFileReadINT16(gf, &i16);
    if (res != GF_RES_OK)
      break;

    res = GFileReadINT16(gf, &i16);
    if (res != GF_RES_OK)
      break;

    if (ch < FIRST_LETTER || ch > LAST_LETTER)
      continue;

    RadTab[ch - FIRST_LETTER] = f32;
  }

  GFileClose(gf);

  RadTabRead = TRUE;
}

static void
checkPair(DhAtomP atom2P, void *clientData)
{
  DhAtomP atom1P = clientData;
  Vec3 x1, dx;
  char ch1, ch2;
  float radSum;

  if (atom1P >= atom2P)
    return;  /* check each pair only once */

  DhAtomGetCoord(atom1P, x1);
  DhAtomGetCoord(atom2P, dx);
  Vec3Sub(dx, x1);

  ch1 = DStrToStr(DhAtomGetName(atom1P))[0];
  ch2 = DStrToStr(DhAtomGetName(atom2P))[0];

  radSum = RadTab[ch1 - FIRST_LETTER] + RadTab[ch2 - FIRST_LETTER];
  if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] < radSum * radSum)
    (void) DhBondNew(atom1P, atom2P);
}

static void
addAtomBond(DhAtomP atomP, void *clientData)
{
  DhMolP molP;

  molP = DhResGetMol(DhAtomGetRes(atomP));
  DhMolApplyAtom(PropGetRef(PROP_SELECTED, FALSE), molP, checkPair, atomP);
}

ErrCode
ExReadXyz(char *cmd)
{
  GFile gf;
  GFileRes res;
  DSTR fileName;
  ErrCode errCode;
  int molI;
  DSTR resName, atomName;
  DhMolP molP;
  DhResP resP;
  DhResDefP resDefP;
  DhAtomP atomP;
  BOOL isNew;
  int TypeIndA[LETTER_NO];
  char buf[200];
  int atomNo, atomI;
  Vec3 coord;
  FLOAT32 f32;
  Vec3 cent;
  char ch;
  int i;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.xyz", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  if (gf == NULL) {
    DStrFree(fileName);
    return EC_ERROR;
  }

  readRadTab();
  molI = 0;

  for (;;) {   /* loop over molecules */
    for (;;) {   /* skip empty lines */
      if (GFileEOF(gf))
	break;

      res = GFileReadLine(gf, buf, sizeof(buf));
      if (res != GF_RES_OK) {
	DStrFree(fileName);
	GFileClose(gf);
	return EC_ERROR;
      }

      if (sscanf(buf, "%d", &atomNo) == 1)
	break;
    }

    if (GFileEOF(gf))
      break;

    res = GFileReadLine(gf, buf, sizeof(buf));
    if (res != GF_RES_OK) {
      DStrFree(fileName);
      GFileClose(gf);
      return EC_ERROR;
    }

    molP = DhMolNew();
    FileNameBase(fileName);
    DhMolSetName(molP, fileName);

    resName = DStrNew();
    if (buf[0] == '\0' || buf[0] == ' ' || buf[0] == '\n') {
      /* create unique name */
      for (i = 1; ; i++) {
	(void) sprintf(buf, "xyz[%d]", i);
	DStrAssignStr(resName, buf);
	resDefP = DhResDefGet(resName);
	if (resDefP == NULL)
	  break;
      }
    } else {
      DStrAssignStr(resName, buf);
      resDefP = DhResDefGet(resName);
    }

    isNew = (resDefP == NULL);
    if (isNew)
      resDefP = DhResDefNew(resName);

    DStrFree(resName);

    resP = DhResNew(molP, resDefP, SP_LAST);
    atomName = DStrNew();

    for (i = 0; i < LETTER_NO; i++)
      TypeIndA[i] = 1;

    for (atomI = 0; atomI < atomNo; atomI++) {
      if (GFileEOF(gf)) {
	DStrFree(fileName);
	DStrFree(atomName);
	DhMolDestroy(molP);
	GFileClose(gf);
	CipSetError("premature end of file");
	return EC_ERROR;
      }

      res = GFileReadStr(gf, buf, sizeof(buf));
      if (res != GF_RES_OK) {
	DStrFree(fileName);
	DStrFree(atomName);
	DhMolDestroy(molP);
	GFileClose(gf);
	return EC_ERROR;
      }

      ch = buf[0];
      if (ch < FIRST_LETTER || ch > LAST_LETTER) {
	atomP = NULL;
      } else {
	DStrAssignStr(atomName, buf);
	(void) sprintf(buf, "%d", TypeIndA[ch - FIRST_LETTER]++);
	DStrAppStr(atomName, buf);
	if (isNew) {
	  atomP = DhAtomNew(resP, atomName);
	} else {
	  atomP = DhAtomFindName(resP, atomName, TRUE);
	  if (atomP == NULL)
	    atomP = DhAtomNew(resP, atomName);
	}
      }

      for (i = 0; i < 3; i++) {
	res = GFileReadFLOAT32(gf, &f32);
	if (res != GF_RES_OK) {
	  DStrFree(fileName);
	  DStrFree(atomName);
	  DhMolDestroy(molP);
	  GFileClose(gf);
	  return EC_ERROR;
	}
	coord[i] = f32;
      }

      if (atomP != NULL)
	DhAtomSetCoord(atomP, coord);
    }

    DStrFree(atomName);

    DhMolApplyAtom(PropGetRef(PROP_SELECTED, FALSE), molP, addAtomBond, NULL);

    DhMolInit(molP);
    if (molI == 0)
      DhCalcCenter(cent, molP);
    DhMolSetRotPoint(molP, cent);
    GraphMolAdd(molP);

    molI++;
  }

  DStrFree(fileName);
  GFileClose(gf);

  GraphRedrawNeeded();

  return EC_OK;
}
