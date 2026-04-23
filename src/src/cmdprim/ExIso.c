/*
************************************************************************
*
*   ExIso.c - AddIso command
*
*   Copyright (c) 1996-97
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExIso.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iso_surface.h>
#include <g_file.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>

static float CurrLevel = 0.01f;

#define CHECK_RES(s) if (s != GF_RES_OK) {GFileClose(gf); return EC_ERROR;}

#define ARG_NUM 2

ErrCode
ExAddIsosurface(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  DhMolP molP;
  GFile gf;
  GFileRes res;
  float x0, y0, z0;
  int sx, sy, sz, valI;
  float dx, dy, dz;
  float *valA;
  PrimObjP primP;
  IsoSurface *surfA;
  int surfNo, surfI, i;
  INT32 i32;
  FLOAT32 f32;

  arg[0].type = AT_STR;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "File Name";
  DStrAssignStr(arg[0].v.strVal, "tt.pot");

  arg[1].prompt = "Level";
  arg[1].v.doubleVal = CurrLevel;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrLevel = (float) arg[1].v.doubleVal;

  gf = GFileOpenRead(DStrToStr(arg[0].v.strVal));
  ArgCleanup(arg, ARG_NUM);
  if (gf == NULL)
    return EC_ERROR;

  if (SelMolGet(&molP, 1) != 1) {
    CipSetError("exactly one molecule must be selected");
    GFileClose(gf);
    return EC_ERROR;
  }

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  x0 = f32;
  CHECK_RES(GFileReadINT32(gf, &i32));
  sx = i32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  dx = f32;

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  y0 = f32;
  CHECK_RES(GFileReadINT32(gf, &i32));
  sy = i32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  dy = f32;

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  z0 = f32;
  CHECK_RES(GFileReadINT32(gf, &i32));
  sz = i32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  dz = f32;

  if (sx < 2 || sy < 2 || sz < 2) {
    GFileClose(gf);
    CipSetError("sizes must be at least 2");
    return EC_ERROR;
  }

  valA = malloc(sx * sy * sz * sizeof(*valA));
  if (valA == NULL) {
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

    if (CurrLevel < 0.0f)
      /* for negative levels we invert all signs so that the
	 orientation of surface normals is correct (outside) */
      valA[valI++] = - f32;
    else
      valA[valI++] = f32;
  }

  GFileClose(gf);

  if (res == GF_RES_OK && valI < sx * sy * sz) {
    free(valA);
    CipSetError("not enough data values");
    return EC_ERROR;
  }

  IsoSurfaceCalc(valA, sx, sy, sz, x0, y0, z0,
      (sx - 1) * dx, (sy - 1) * dy, (sz - 1) * dz, fabsf(CurrLevel), TRUE,
      &surfA, &surfNo);

  free(valA);

  for (surfI = 0; surfI < surfNo; surfI++) {
    primP = PrimNew(PT_SURFACE, molP);
    PrimSurfaceSetPoints(primP,
	surfA[surfI].xA, surfA[surfI].nvA, surfA[surfI].pointNo);
    for (i = 0; i < surfA[surfI].meshNo; i++)
      PrimSurfaceAddMesh(primP,
	surfA[surfI].meshA[i].indA, surfA[surfI].meshA[i].pointNo);
    free(surfA[surfI].meshA);
    PrimSetProp(PropGetRef("surface", TRUE), primP, TRUE);
    PrimSetProp(PropGetRef("isosurface", TRUE), primP, TRUE);
  }
  free(surfA);

  return EC_OK;
}
