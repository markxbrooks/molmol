/*
************************************************************************
*
*   ExReadShift.c - ReadShift command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadShift.c
*   SCCS identification       : 1.7
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

#define LINE_LEN 100

typedef struct {
  int resI;
  DSTR atomName;
  int shiftI;
  float shift;
} ShiftData;

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
setShift(DhMolP molP, void *clientData)
{
  ShiftData *dataP = clientData;
  DhResP resP;
  DhAtomP atomP;

  resP = DhResFind(molP, dataP->resI);
  if (resP == NULL)
    return;

  atomP = DhAtomFindName(resP, dataP->atomName, TRUE);
  if (atomP == NULL)
    return;
  
  DhAtomSetShiftIndex(atomP, dataP->shiftI);
  DhAtomSetShift(atomP, dataP->shift);
}

ErrCode
ExReadShift(char *cmd)
{
  DSTR fileName;
  ErrCode errCode;
  GFile gf;
  GFileRes res;
  char lineBuf[LINE_LEN], buf[30];
  PropRefP refP;
  ShiftData data;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.prot", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  data.atomName = DStrNew();
  refP = PropGetRef(PROP_SELECTED, FALSE);

  res = GF_RES_OK;
  while (! GFileEOF(gf)) {
    res = GFileReadLine(gf, lineBuf, LINE_LEN);
    if (res != GF_RES_OK)
      break;

    if (lineBuf[0] == '#')
      continue;

    getLinePart(buf, lineBuf, 0, 4);
    data.shiftI = atoi(buf);

    getLinePart(buf, lineBuf, 5, 7);
    data.shift = (float) atof(buf);

    /* this is not very nice, but it's the same way XEASY
       distinguished between old and new proton lists */
    if (strlen(lineBuf) > 40) {
      getLinePart(buf, lineBuf, 32, 5);
      DStrAssignStr(data.atomName, buf);

      getLinePart(buf, lineBuf, 37, 5);
      data.resI = atoi(buf);
    } else {
      getLinePart(buf, lineBuf, 19, 5);
      DStrAssignStr(data.atomName, buf);

      getLinePart(buf, lineBuf, 24, 4);
      data.resI = atoi(buf);
    }

    DhApplyMol(refP, setShift, &data);
  }

  DStrFree(data.atomName);

  GFileClose(gf);

  if (res == GF_RES_OK)
    return EC_OK;
  else
    return EC_ERROR;
}
