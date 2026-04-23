/*
************************************************************************
*
*   ExReadPot.c - ReadPot command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExReadPot.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <stdlib.h>

#include <dstr.h>
#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <prim_hand.h>

typedef struct {
  float *valA;
  int sx, sy, sz;
  float x0, y0, z0, dx, dy, dz;
  int surfNo;
} PotData;

static void
setPot(float *potP, Vec3 p, void *clientData)
{
  PotData *dataP = clientData;
  float x, y, z;
  int xi, yi, zi;
  float xp, yp, zp;
  int sx, sxy;
  float *valP;

  x = p[0];
  y = p[1];
  z = p[2];

  /* trilinear interpolation */

  xi = (int) ((x - dataP->x0) / dataP->dx);
  yi = (int) ((y - dataP->y0) / dataP->dy);
  zi = (int) ((z - dataP->z0) / dataP->dz);

  xp = x - (dataP->x0 + xi * dataP->dx);
  yp = y - (dataP->y0 + yi * dataP->dy);
  zp = z - (dataP->z0 + zi * dataP->dz);

  if (xi < 0 || xp < -0.001f) {
    xi = 0;
    xp = 0.0f;
  } else if (xi >= dataP->sx - 1) {
    xi = dataP->sx - 2;
    xp = 1.0f;
  } else {
    xp /= dataP->dx;
  }

  if (yi < 0 || yp < -0.001f) {
    yi = 0;
    yp = 0.0f;
  } else if (yi >= dataP->sy - 1) {
    yi = dataP->sy - 2;
    yp = 1.0f;
  } else {
    yp /= dataP->dy;
  }

  if (zi < 0 || zp < -0.001f) {
    zi = 0;
    zp = 0.0f;
  } else if (zi >= dataP->sz - 1) {
    zi = dataP->sz - 2;
    zp = 1.0f;
  } else {
    zp /= dataP->dz;
  }

  sx = dataP->sx;
  sxy = sx * dataP->sy;
  valP = dataP->valA + zi * sxy + yi * sx + xi;

  *potP = (1.0f - xp) * (1.0f - yp) * (1.0f - zp) * valP[0];
  *potP += xp * (1.0f - yp) * (1.0f - zp) * valP[1];
  *potP += (1.0f - xp) * yp * (1.0f - zp) * valP[sx];
  *potP += xp * yp * (1.0f - zp) * valP[sx + 1];
  *potP += (1.0f - xp) * (1.0f - yp) * zp * valP[sxy];
  *potP += xp * (1.0f - yp) * zp * valP[sxy + 1];
  *potP += (1.0f - xp) * yp * zp * valP[sxy + sx];
  *potP += xp * yp * zp * valP[sxy + sx + 1];
}

static void
setSurfPot(PrimObjP primP, void *clientData)
{
  PotData *dataP = clientData;

  PrimSurfaceSetPot(primP, setPot, dataP);
  dataP->surfNo++;
}

#define CHECK_RES(s) if (s != GF_RES_OK) {GFileClose(gf); return EC_ERROR;}

ErrCode
ExReadPot(char *cmd)
{
  GFile gf;
  GFileRes res;
  DSTR fileName;
  ErrCode errCode;
  int sx, sy, sz, valI;
  PotData data;
  INT32 i32;
  FLOAT32 f32;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.pot", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  if (gf == NULL) {
    DStrFree(fileName);
    return EC_ERROR;
  }

  DStrFree(fileName);

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  data.x0 = f32;
  CHECK_RES(GFileReadINT32(gf, &i32));
  sx = i32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  data.dx = f32;

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  data.y0 = f32;
  CHECK_RES(GFileReadINT32(gf, &i32));
  sy = i32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  data.dy = f32;

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  data.z0 = f32;
  CHECK_RES(GFileReadINT32(gf, &i32));
  sz = i32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  data.dz = f32;

  if (sx < 2 || sy < 2 || sz < 2) {
    GFileClose(gf);
    CipSetError("sizes must be at least 2");
    return EC_ERROR;
  }

  data.valA = malloc(sx * sy * sz * sizeof(*data.valA));
  if (data.valA == NULL) {
    GFileClose(gf);
    CipSetError("out of memory");
    return EC_ERROR;
  }

  valI = 0;
  res = GF_RES_OK;
  for (;;) {
    if (GFileEOF(gf))
      break;

    res = GFileReadFLOAT32(gf, &f32);
    if (res != GF_RES_OK)
      break;

    data.valA[valI++] = f32;

    if (valI == sx * sy * sz)
      break;
  }

  GFileClose(gf);

  if (res != GF_RES_OK) {
    free(data.valA);
    return EC_ERROR;
  }

  if (valI < sx * sy * sz) {
    free(data.valA);
    CipSetError("not enough data values");
    return EC_ERROR;
  }

  data.sx = sx;
  data.sy = sy;
  data.sz = sz;

  data.surfNo = 0;
  PrimApply(PT_SURFACE, PropGetRef(PROP_SELECTED, FALSE), setSurfPot, &data);

  free(data.valA);

  if (data.surfNo == 0) {
    CipSetError("no surfaces selected");
    return EC_WARNING;
  }

  return EC_OK;
}
