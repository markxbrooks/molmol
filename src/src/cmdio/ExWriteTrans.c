/*
************************************************************************
*
*   ExWriteTrans.c - WriteTransform command
*
*   Copyright (c) 1994-2001
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
*   Date of last modification : 01/07/04
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExWriteTrans.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <values.h>
#include <math.h>

#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <data_hand.h>
#include <data_sel.h>

ErrCode
ExWriteTransform(char *cmd)
{
  GFile gf;
  DSTR fileName;
  ErrCode errCode;
  DhMolP molP;
  Mat4 rotMat;
  float xAng, yAng, zAng;
  Vec3 rotV, transV;
  Vec4 v4;
  float zoomFact;
  BOOL ok;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.mac", FALSE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  if (SelMolGet(&molP, 1) == 0) {
    CipSetError("at least 1 molecule must be selected");
    return EC_ERROR;
  }

  gf = GFileOpenNew(DStrToStr(fileName), GF_FORMAT_ASCII);
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  DhMolGetRotMat(molP, rotMat);
  Mat4GetAng(rotMat, &xAng, &yAng, &zAng);

  DhMolGetRotPoint(molP, rotV);
  DhMolGetTransVect(molP, transV);
  Vec3To4(v4, rotV);
  Mat4VecMult(v4, rotMat);
  Vec4To3(rotV, v4);
  Vec3Sub(transV, rotV);

  zoomFact = (float) ParGetDoubleVal(PN_ZOOM_FACT);

  ok = GFileWriteLine(gf, "RotateInit") == GF_RES_OK;
  ok = ok && GFileWriteStr(gf, "RotateX") == GF_RES_OK;
  ok = ok && GFileWriteFLOAT32(gf, xAng * 180.0f / (float) M_PI) == GF_RES_OK;
  ok = ok && GFileWriteNL(gf) == GF_RES_OK;
  ok = ok && GFileWriteStr(gf, "RotateY") == GF_RES_OK;
  ok = ok && GFileWriteFLOAT32(gf, yAng * 180.0f / (float) M_PI) == GF_RES_OK;
  ok = ok && GFileWriteNL(gf) == GF_RES_OK;
  ok = ok && GFileWriteStr(gf, "RotateZ") == GF_RES_OK;
  ok = ok && GFileWriteFLOAT32(gf, zAng * 180.0f / (float) M_PI) == GF_RES_OK;
  ok = ok && GFileWriteNL(gf) == GF_RES_OK;
  ok = ok && GFileWriteNL(gf) == GF_RES_OK;

  ok = GFileWriteLine(gf, "MoveInit") == GF_RES_OK;
  ok = ok && GFileWriteStr(gf, "MoveX") == GF_RES_OK;
  ok = ok && GFileWriteFLOAT32(gf, transV[0]) == GF_RES_OK;
  ok = ok && GFileWriteNL(gf) == GF_RES_OK;
  ok = ok && GFileWriteStr(gf, "MoveY") == GF_RES_OK;
  ok = ok && GFileWriteFLOAT32(gf, transV[1]) == GF_RES_OK;
  ok = ok && GFileWriteNL(gf) == GF_RES_OK;
  ok = ok && GFileWriteStr(gf, "MoveZ") == GF_RES_OK;
  ok = ok && GFileWriteFLOAT32(gf, transV[2]) == GF_RES_OK;
  ok = ok && GFileWriteNL(gf) == GF_RES_OK;
  ok = ok && GFileWriteNL(gf) == GF_RES_OK;

  ok = ok && GFileWriteStr(gf, "ZoomAbs") == GF_RES_OK;
  ok = ok && GFileWriteFLOAT32(gf, zoomFact) == GF_RES_OK;
  ok = ok && GFileWriteNL(gf) == GF_RES_OK;

  GFileClose(gf);

  if (! ok)
    return EC_ERROR;

  return EC_OK;
}
