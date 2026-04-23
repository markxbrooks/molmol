/*
************************************************************************
*
*   ExWriteDg.c - WriteDg command
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
*   Date of last modification : 00/03/11
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExWriteDg.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>

#include <break.h>
#include <g_file.h>
#include <curr_dir.h>
#include <prog_vers.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>

#define LINE_LEN 100

typedef struct {
  GFile gf;
  int atomIdx;
  BOOL ok;
} WriteData;

static void
countRes(DhResP resP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
countAtom(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
writeAtom(DhAtomP atomP, void *clientData)
{
  WriteData *dataP = clientData;
  Vec3 coord;
  DhResP resP;
  char lineBuf[LINE_LEN];

  if (! dataP->ok)
    return;

  resP = DhAtomGetRes(atomP);

  DhAtomGetCoordTransf(atomP, coord);

  (void) sprintf(lineBuf, "%5d %-5.5s%6d %-5.5s%#11.4f%#11.4f%#11.4f",
      dataP->atomIdx, DStrToStr(DhAtomGetName(atomP)),
      DhResGetNumber(resP), DStrToStr(DhResGetName(resP)),
      coord[0], coord[1], coord[2]);
  dataP->ok = (GFileWriteLine(dataP->gf, lineBuf) == GF_RES_OK);

  /* atom index is 5 digits only */
  if (dataP->atomIdx < 99999)
    dataP->atomIdx++;
}

#define CHECK_RES(s) if (s != GF_RES_OK) \
    {free(molPA); GFileClose(gf); return EC_ERROR;}

ErrCode
ExWriteDg(char *cmd)
{
  GFile gf;
  DSTR fileName;
  ErrCode errCode;
  DhMolP *molPA;
  PropRefP refP;
  int molNo, molI;
  int resNo, atomNo;
  WriteData writeData;
  char lineBuf[LINE_LEN];

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.cor", FALSE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0) {
    CipSetError("at least 1 molecule must be selected");
    return EC_ERROR;
  }

  gf = GFileOpenNew(DStrToStr(fileName), GF_FORMAT_ASCII);
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  refP = PropGetRef(PROP_ALL, FALSE);

  resNo = 0;
  atomNo = 0;
  for (molI = 0; molI < molNo; molI++) {
    DhMolApplyRes(refP, molPA[molI], countRes, &resNo);
    DhMolApplyAtom(refP, molPA[molI], countAtom, &atomNo);
  }

  CHECK_RES(GFileWriteStr(gf, "Structure of"));
  CHECK_RES(GFileWriteStr(gf, DStrToStr(DhMolGetName(molPA[0]))));
  CHECK_RES(GFileWriteNL(gf));
  (void) sprintf(lineBuf, "%s version %s.%d.%d",
      PROG_NAME, VERS_MAJOR, VERS_MINOR, VERS_REV);
  CHECK_RES(GFileWriteLine(gf, lineBuf));
  (void) sprintf(lineBuf,
      "Number of residues:%6d Number of atoms:%8d Number of subunits:%5d",
      resNo, atomNo, molNo);
  CHECK_RES(GFileWriteLine(gf, lineBuf));

  writeData.gf = gf;
  writeData.atomIdx = 1;
  writeData.ok = TRUE;
  BreakActivate(TRUE);
  for (molI = 0; molI < molNo; molI++) {
    if (BreakCheck(1))
      break;

    DhMolApplyAtom(refP, molPA[molI], writeAtom, &writeData);
    if (! writeData.ok)
      break;
  }
  BreakActivate(FALSE);

  free(molPA);

  GFileClose(gf);

  if (! writeData.ok)
    return EC_ERROR;

  return EC_OK;
}
