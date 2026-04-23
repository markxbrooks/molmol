/*
************************************************************************
*
*   GLColor.c - GL color setting
*
*   Copyright (c) 1994
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/gl/SCCS/s.GLColor.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "gl_color.h"

#include <gl/gl.h>

#include <sg.h>
#include "gl_light.h"

static float FGCol[4], BGCol[3];
static SgFogMode FogMode;
static float FogDens, FogStart, FogEnd;

void
SgGLSetColor(float r, float g, float b, float a, BOOL fg)
{
  if (fg) {
    FGCol[0] = r;
    FGCol[1] = g;
    FGCol[2] = b;
    FGCol[3] = a;
    GLSetFG();
  } else {
    BGCol[0] = r;
    BGCol[1] = g;
    BGCol[2] = b;
    GLSetBG();
  }
}

void
SgGLSetFogMode(SgFogMode mode)
{
  FogMode = mode;
}

void
SgGLSetFogPar(SgFogPar par, float val)
{
  switch (par) {
    case SG_FOG_DENSITY:
      FogDens = val;
      break;
    case SG_FOG_START:
      FogStart = val;
      break;
    case SG_FOG_END:
      FogEnd = val;
      break;
  }
}

void
GLSetFG(void)
{
  GLSetMaterialColor(FGCol);
  c4f(FGCol);
}

void
GLSetBG(void)
{
  float param[5];

  lmcolor(LMC_COLOR);
  c3f(BGCol);
  lmcolor(GLGetColorMode());

  switch (FogMode) {
    case SG_FOG_MODE_OFF:
      fogvertex(FG_OFF, NULL);
      break;
    case SG_FOG_MODE_EXP:
      /* compensate factor in GL fog equation, 1 / 5.5 */
      param[0] = 0.1818f * FogDens;
      param[1] = BGCol[0];
      param[2] = BGCol[1];
      param[3] = BGCol[2];
      fogvertex(FG_VTX_EXP, param);
      fogvertex(FG_ON, NULL);
      break;
    case SG_FOG_MODE_EXP2:
      /* compensate factor in GL fog equation, 1 / sqrt(5.5) */
      param[0] = 0.4264f * FogDens;
      param[1] = BGCol[0];
      param[2] = BGCol[1];
      param[3] = BGCol[2];
      fogvertex(FG_VTX_EXP2, param);
      fogvertex(FG_ON, NULL);
      break;
    case SG_FOG_MODE_LINEAR:
      param[0] = FogStart;
      param[1] = FogEnd;
      param[2] = BGCol[0];
      param[3] = BGCol[1];
      param[4] = BGCol[2];
      fogvertex(FG_VTX_LIN, param);
      fogvertex(FG_ON, NULL);
      break;
  }
}
