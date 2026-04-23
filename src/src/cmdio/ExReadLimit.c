/*
************************************************************************
*
*   ExReadLimit.c - ReadUpl and ReadLol commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadLimit.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <break.h>
#include <dstr.h>
#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

#define LINE_LEN 100

typedef struct {
  int res1Ind;
  int res2Ind;
  DSTR atom1Name;
  DSTR atom2Name;
  DhDistKind kind;
  float limit;
} DistInfo;

static void
getLinePart(char *subBuf, char *lineBuf, int start, int len)
{
  if ((int) strlen(lineBuf) < start + len) {
    subBuf[0] = '\0';
    return;
  }

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
addDist(DhMolP molP, void *clientData)
{
  DistInfo *infoP = clientData;
  DhResP res1P, res2P;
  DhAtomP atom1P, atom2P;
  DhDistP distP;

  res1P = DhResFind(molP, infoP->res1Ind);
  res2P = DhResFind(molP, infoP->res2Ind);
  if (res1P == NULL || res2P == NULL)
    return;
  
  atom1P = DhAtomFindName(res1P, infoP->atom1Name, TRUE);
  atom2P = DhAtomFindName(res2P, infoP->atom2Name, TRUE);
  if (atom1P == NULL || atom2P == NULL)
    return;

  distP = DhDistNew(atom1P, atom2P);
  DhDistSetKind(distP, infoP->kind);
  DhDistSetLimit(distP, infoP->limit);
  DhDistInit(distP);
}

ErrCode
ExReadLimit(char *cmd)
{
  GFile gf;
  GFileRes res;
  DSTR fileName;
  ErrCode errCode;
  int breakCount;
  char lineBuf[LINE_LEN];
  char strBuf[80], numBuf[30];
  PropRefP refP;
  DistInfo info;

  fileName = DStrNew();
  if (strcmp(cmd, "ReadUpl") == 0) {
    errCode = ArgGetFilename(fileName, CurrDirGet(), "*.upl", TRUE);
    info.kind = DK_UPPER;
  } else {
    errCode = ArgGetFilename(fileName, CurrDirGet(), "*.lol", TRUE);
    info.kind = DK_LOWER;
  }
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  DhActivateGroups(TRUE);

  refP = PropGetRef(PROP_SELECTED, FALSE);
  info.atom1Name = DStrNew();
  info.atom2Name = DStrNew();

  breakCount = 1000 / SelMolGet(NULL, 0);
  BreakActivate(TRUE);

  for (;;) {
    if (GFileEOF(gf))
      break;

    if (BreakCheck(breakCount))
      break;

    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK) {
      DStrFree(info.atom1Name);
      DStrFree(info.atom2Name);
      GFileClose(gf);
      DhActivateGroups(FALSE);
      return EC_ERROR;
    }

    getLinePart(numBuf, lineBuf, 0, 3);
    if (numBuf[0] != '\0')
      info.res1Ind = atoi(numBuf);

    getLinePart(numBuf, lineBuf, 14, 4);
    if (numBuf[0] == '\0')
      continue;

    info.res2Ind = atoi(numBuf);

    getLinePart(strBuf, lineBuf, 9, 5);
    DStrAssignStr(info.atom1Name, strBuf);

    getLinePart(strBuf, lineBuf, 24, 5);
    DStrAssignStr(info.atom2Name, strBuf);

    getLinePart(numBuf, lineBuf, 29, 7);
    info.limit = (float) atof(numBuf);

    DhApplyMol(refP, addDist, &info);
  }

  BreakActivate(FALSE);
  DStrFree(info.atom1Name);
  DStrFree(info.atom2Name);
  GFileClose(gf);

  DhActivateGroups(FALSE);

  GraphRedrawNeeded();

  return EC_OK;
}
