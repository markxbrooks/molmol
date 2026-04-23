/*
************************************************************************
*
*   ExReadAco.c - ReadAco command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadAco.c
*   SCCS identification       : 1.5
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
  DSTR angName;
  float minVal;
  float maxVal;
} AcoData;

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
setAco(DhMolP molP, void *clientData)
{
  AcoData *dataP = clientData;
  DhResP resP;
  DhAngleP angleP;

  resP = DhResFind(molP, dataP->resI);
  if (resP == NULL)
    return;

  angleP = DhAngleFind(resP, dataP->angName);
  if (angleP == NULL)
    return;
  
  DhAngleSetMinVal(angleP, dataP->minVal);
  DhAngleSetMaxVal(angleP, dataP->maxVal);
}

ErrCode
ExReadAco(char *cmd)
{
  DSTR fileName;
  ErrCode errCode;
  GFile gf;
  GFileRes res;
  char lineBuf[LINE_LEN], buf[30];
  PropRefP refP;
  AcoData data;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.aco", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  data.angName = DStrNew();
  refP = PropGetRef(PROP_SELECTED, FALSE);

  res = GF_RES_OK;
  while (! GFileEOF(gf)) {
    res = GFileReadLine(gf, lineBuf, LINE_LEN);
    if (res != GF_RES_OK)
      break;

    getLinePart(buf, lineBuf, 0, 4);
    data.resI = atoi(buf);

    getLinePart(buf, lineBuf, 11, 4);
    DStrAssignStr(data.angName, buf);

    getLinePart(buf, lineBuf, 16, 8);
    data.minVal = (float) atof(buf);

    getLinePart(buf, lineBuf, 24, 8);
    data.maxVal = (float) atof(buf);

    DhApplyMol(refP, setAco, &data);
  }

  DStrFree(data.angName);

  GFileClose(gf);

  if (res == GF_RES_OK)
    return EC_OK;
  else
    return EC_ERROR;
}
