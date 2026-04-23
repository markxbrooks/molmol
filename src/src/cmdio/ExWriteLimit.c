/*
************************************************************************
*
*   ExWriteLimit.c - WriteUpl and WriteLol commands
*
*   Copyright (c) 1998
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
*   Date of last modification : 98/05/22
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/cmdio/SCCS/s.ExWriteLimit.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <data_dist.h>

#define LINE_LEN 100

typedef struct {
  DhDistKind kind;
  GFile gf;
  BOOL ok;
} WriteData;

static void
writeDist(DhDistP distP, void *clientData)
{
  WriteData *dataP = clientData;
  DhAtomP atom1P, atom2P;
  DhResP res1P, res2P;
  float limit;
  char lineBuf[LINE_LEN];

  if (! dataP->ok)
    return;

  atom1P = DhDistGetAtom1(distP);
  atom2P = DhDistGetAtom2(distP);

  res1P = DhAtomGetRes(atom1P);
  res2P = DhAtomGetRes(atom2P);

  if (dataP->kind == DK_ACTUAL)
    limit = DhDistGetVal(distP);
  else
    limit = DhDistGetLimit(distP);

  (void) sprintf(lineBuf, "%3d %-5s%-5s %3d %-5s%-5s%#7.2f",
       DhResGetNumber(res1P), DStrToStr(DhResGetName(res1P)),
       DStrToStr(DhAtomGetName(atom1P)),
       DhResGetNumber(res2P), DStrToStr(DhResGetName(res2P)),
       DStrToStr(DhAtomGetName(atom2P)),
       limit);

  dataP->ok = (GFileWriteLine(dataP->gf, lineBuf) == GF_RES_OK);
}

ErrCode
ExWriteLimit(char *cmd)
{
  GFile gf;
  DSTR fileName;
  ErrCode errCode;
  DhDistKind kind;
  PropRefP refP;
  int molNo;
  DhMolP *molPA;
  DistList tab;
  WriteData writeData;

  fileName = DStrNew();
  if (strcmp(cmd, "WriteUpl") == 0) {
    errCode = ArgGetFilename(fileName, CurrDirGet(), "*.upl", TRUE);
    kind = DK_UPPER;
  } else {
    errCode = ArgGetFilename(fileName, CurrDirGet(), "*.lol", TRUE);
    kind = DK_LOWER;
  }
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenNew(DStrToStr(fileName), GF_FORMAT_ASCII);
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  DhActivateGroups(TRUE);

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0) {
    GFileClose(gf);
    DhActivateGroups(FALSE);
    CipSetError("no molecules selected");
    return EC_WARNING;
  }

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  writeData.kind = kind;
  writeData.gf = gf;
  writeData.ok = TRUE;

  tab = DistListBuildIntra(refP, molPA, molNo, kind);
  DistListApply(tab, writeDist, &writeData);
  DistListFree(tab);

  if (kind == DK_UPPER) {
    tab = DistListBuildIntra(refP, molPA, molNo, DK_ACTUAL);
    writeData.kind = DK_ACTUAL;
    DistListApply(tab, writeDist, &writeData);
    DistListFree(tab);
  }

  GFileClose(gf);
  DhActivateGroups(FALSE);

  if (! writeData.ok)
    return EC_ERROR;

  return EC_OK;
}
