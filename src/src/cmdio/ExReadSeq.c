/*
************************************************************************
*
*   ExReadSeq.c - ReadSeq command
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
*   Date of last modification : 98/07/21
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadSeq.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdlib.h>
#include <string.h>

#include <dstr.h>
#include <file_name.h>
#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <data_hand.h>
#include <graph_draw.h>

#define LINE_LEN 20

ErrCode
ExReadSeq(char *cmd)
{
  GFile gf;
  GFileRes res;
  DSTR fileName;
  ErrCode errCode;
  DhMolP molP;
  DhResP resP;
  DSTR resName;
  DhResDefP resDefP;
  char lineBuf[LINE_LEN];
  Vec3 cent;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.seq", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  if (gf == NULL) {
    DStrFree(fileName);
    return EC_ERROR;
  }

  molP = DhMolNew();
  FileNameBase(fileName);
  DhMolSetName(molP, fileName);
  DStrFree(fileName);

  resName = DStrNew();

  for (;;) {
    if (GFileEOF(gf))
      break;

    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK) {
      DStrFree(resName);
      DhMolDestroy(molP);
      GFileClose(gf);
      return EC_ERROR;
    }

    if (lineBuf[0] == '#')
      continue;

    DStrAssignStr(resName, lineBuf);
    resDefP = DhResDefGet(resName);
    if (resDefP == NULL) {
      DStrFree(resName);
      DhMolDestroy(molP);
      GFileClose(gf);
      CipSetError("unknown residue type");
      return EC_ERROR;
    }

    resP = DhResNew(molP, resDefP, SP_LAST);
    DhResStandGeom(resP);
    DhResDockPrev(resP);
  }

  DStrFree(resName);
  GFileClose(gf);

  DhMolInit(molP);
  DhCalcCenter(cent, molP);
  DhMolSetRotPoint(molP, cent);
  GraphMolAdd(molP);
  GraphRedrawNeeded();

  return EC_OK;
}
