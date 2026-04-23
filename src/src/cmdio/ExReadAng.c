/*
************************************************************************
*
*   ExReadAng.c - ReadAng and ReadListAng command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadAng.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <break.h>
#include <dstr.h>
#include <file_name.h>
#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <data_hand.h>
#include <graph_draw.h>

#define LINE_LEN 100

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

static ErrCode
readOneMol(GFile gf, char *lineBuf, char *molName, int *molIndP, Vec3 cent)
{
  GFileRes res;
  DhMolP molP;
  ErrCode errCode;
  DhResDefP resDefP;
  DhResP resP;
  DhAngleP angleP;
  char strBuf[80], numBuf[30];
  DSTR dstr;
  int lastResInd, resInd, i;

  if (lineBuf[0] != '#') {
    CipSetError("first line is not comment line");
    return EC_ERROR;
  }

  molP = DhMolNew();

  errCode = EC_OK;
  i = 0;
  lastResInd = -999;
  dstr = DStrNew();

  for (;;) {
    if (GFileEOF(gf)) {
      if (*molIndP > 0)
	*molIndP += 1;
      break;
    }

    if (i == 0) {
      res = GFileReadLine(gf, lineBuf, LINE_LEN);
      if (res != GF_RES_OK) {
	errCode = EC_ERROR;
	break;
      }

      if (lineBuf[0] == '#') {
	*molIndP += 1;
	break;
      }

      getLinePart(numBuf, lineBuf, 0, 3);
      if (numBuf[0] == '\0')
	resInd = lastResInd;
      else
	resInd = atoi(numBuf);

      if (resInd != lastResInd) {
	getLinePart(strBuf, lineBuf, 4, 5);
	DStrAssignStr(dstr, strBuf);
	resDefP = DhResDefGet(dstr);
	if (resDefP == NULL) {
	  CipSetError("unknown residue type");
	  errCode = EC_ERROR;
	  break;
	}

	resP = DhResNew(molP, resDefP, SP_LAST);
	DhResSetNumber(resP, resInd);
	DhResStandGeom(resP);
	DhResDockPrev(resP);

	/* set OMEGA angle to 180 by default, files often don't
	   contain OMEGA angles and assume that it's planar */
	DStrAssignStr(dstr, "OMEGA");
	angleP = DhAngleFind(resP, dstr);
	if (angleP != NULL)
	  DhAngleSetVal(angleP, 180.0f);

	lastResInd = resInd;
      }
    }

    getLinePart(strBuf, lineBuf, 10 + 15 * i, 5);
    if (strBuf[0] == '\0') {
      i = 0;
      continue;
    }
    DStrAssignStr(dstr, strBuf);
    angleP = DhAngleFind(resP, dstr);
    if (angleP == NULL) {
      CipSetError("unknown angle name");
      errCode = EC_ERROR;
      break;
    }

    getLinePart(numBuf, lineBuf, 15 + 15 * i, 9);
    DhAngleSetVal(angleP, (float) atof(numBuf));

    i = (i + 1) % 4;
  }

  if (errCode != EC_OK) {
    DhMolDestroy(molP);
  } else {
    DStrAssignStr(dstr, molName);
    if (*molIndP > 0) {
      (void) sprintf(numBuf, "%03d", *molIndP);
      DStrAppStr(dstr, numBuf);
    }
    DhMolSetName(molP, dstr);
    DhMolAnglesChanged(molP);
    DhMolInit(molP);
    if (*molIndP <= 1)
      DhCalcCenter(cent, molP);
    DhMolSetRotPoint(molP, cent);
    GraphMolAdd(molP);
  }

  DStrFree(dstr);

  return errCode;
}

static ErrCode
readFile(DSTR name)
{
  GFile gf;
  GFileRes res;
  ErrCode errCode;
  char lineBuf[LINE_LEN];
  int molInd;
  Vec3 cent;

  gf = GFileOpenRead(DStrToStr(name));
  if (gf == NULL) {
    DStrFree(name);
    return EC_ERROR;
  }

  FileNameBase(name);

  molInd = 0;

  res = GFileReadLine(gf, lineBuf, LINE_LEN);
  if (res != GF_RES_OK) {
    DStrFree(name);
    GFileClose(gf);
    return EC_ERROR;
  }

  errCode = EC_OK;
  for (;;) {
    if (GFileEOF(gf))
      break;

    if (BreakCheck(1))
      break;

    errCode = readOneMol(gf, lineBuf, DStrToStr(name), &molInd, cent);
    if (errCode != EC_OK)
      break;
  }

  GFileClose(gf);

  GraphRedrawNeeded();

  return errCode;
}

ErrCode
ExReadAng(char *cmd)
{
  DSTR name;
  ErrCode errCode;

  name = DStrNew();
  errCode = ArgGetFilename(name, CurrDirGet(), "*.ang", TRUE);
  if (errCode != EC_OK) {
    DStrFree(name);
    return errCode;
  }

  BreakActivate(TRUE);
  errCode = readFile(name);
  BreakActivate(FALSE);
  DStrFree(name);

  return errCode;
}

ErrCode
ExReadListAng(char *cmd)
{
  DSTR fileName, inName, angName;
  ErrCode errCode;
  GFile gf;
  GFileRes res;
  char lineBuf[200];

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.nam", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  if (gf == NULL) {
    DStrFree(fileName);
    return EC_ERROR;
  }

  FileNamePath(fileName);

  inName = DStrNew();
  angName = DStrNew();
  BreakActivate(TRUE);

  res = GF_RES_OK;
  for(;;) {
    if (GFileEOF(gf))
      break;

    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK) {
      errCode = EC_ERROR;
      break;
    }

    DStrAssignStr(inName, lineBuf);
    FileNameStrip(inName);
    if (DStrLen(inName) == 0)  /* allow empty line */
      continue;

    if (FileNameIsAbs(inName)) {
      DStrAssignDStr(angName, inName);
    } else {
      DStrAssignDStr(angName, fileName);
      DStrAppDStr(angName, inName);
    }

    errCode = readFile(angName);
    if (errCode != EC_OK)
      break;
    if (BreakInterrupted())
      break;
  }

  BreakActivate(FALSE);
  DStrFree(fileName);
  DStrFree(inName);
  DStrFree(angName);
  GFileClose(gf);

  return errCode;
}
