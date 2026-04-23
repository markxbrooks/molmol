/*
************************************************************************
*
*   ExReadOmap.c - ReadOmap command
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
*   Date of last modification : 99/10/23
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadOmap.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dstr.h>
#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>

#define CHECK_RES(s) if (s != GF_RES_OK) {GFileClose(gf); return EC_ERROR;}

ErrCode
ExReadOmap(char *cmd)
{
  GFile gf;
  GFileRes res;
  DSTR fileName;
  ErrCode errCode;
  DhMolP molP;
  PrimObjP primP;
  char keyword[100], dummyBuf[500];
  float x, y, z;
  Vec3 *vecA;
  int vecNo, vecSize;
  FLOAT32 f32;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.plt", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  if (SelMolGet(&molP, 1) != 1) {
    CipSetError("exactly one molecule must be selected");
    return EC_ERROR;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  primP = PrimNew(PT_MAP, molP);
  PrimSetProp(PropGetRef("map", TRUE), primP, TRUE);

  vecNo = 0;

  res = GF_RES_OK;
  for (;;) {
    if (GFileEOF(gf))
      break;

    res = GFileReadStr(gf, keyword, sizeof(keyword));
    if (res != GF_RES_OK)
      break;

    if (strcmp(keyword, "move") == 0) {
      if (vecNo > 0) {
	PrimMapAddPolyline(primP, vecA, vecNo);
	vecNo = 0;
      }

      res = GFileReadFLOAT32(gf, &f32);
      if (res != GF_RES_OK)
        break;
      x = f32;

      res = GFileReadFLOAT32(gf, &f32);
      if (res != GF_RES_OK)
        break;
      y = f32;

      res = GFileReadFLOAT32(gf, &f32);
      if (res != GF_RES_OK)
        break;
      z = f32;

      vecSize = 2;
      vecA = malloc(vecSize * sizeof(*vecA));

      vecA[0][0] = x;
      vecA[0][1] = y;
      vecA[0][2] = z;
      vecNo = 1;
    } else if (strcmp(keyword, "line") == 0 ||
        strcmp(keyword, "rline") == 0) {
      res = GFileReadFLOAT32(gf, &f32);
      if (res != GF_RES_OK)
        break;
      x = f32;

      res = GFileReadFLOAT32(gf, &f32);
      if (res != GF_RES_OK)
        break;
      y = f32;

      res = GFileReadFLOAT32(gf, &f32);
      if (res != GF_RES_OK)
        break;
      z = f32;

      if (vecNo >= vecSize) {
	vecSize *= 2;
	vecA = realloc(vecA, vecSize * sizeof(*vecA));
      }

      if (keyword[0] == 'r' && vecNo > 0) {
        vecA[vecNo][0] = vecA[vecNo - 1][0] + x;
        vecA[vecNo][1] = vecA[vecNo - 1][1] + y;
        vecA[vecNo][2] = vecA[vecNo - 1][2] + z;
      } else {
        vecA[vecNo][0] = x;
        vecA[vecNo][1] = y;
        vecA[vecNo][2] = z;
      }

      vecNo++;
    } else {
      /* unknown keyword, skip until end of line */
      res = GFileReadLine(gf, dummyBuf, sizeof(dummyBuf));
      if (res != GF_RES_OK)
        break;
    }
  }

  if (vecNo > 0)
    PrimMapAddPolyline(primP, vecA, vecNo);

  GFileClose(gf);

  if (res != GF_RES_OK)
    return EC_WARNING;

  return EC_OK;
}
