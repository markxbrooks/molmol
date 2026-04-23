/*
************************************************************************
*
*   GraphTransf.c - transformations of graphical display
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/graph/SCCS/s.GraphTransf.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <graph_transf.h>

#include <values.h>
#include <math.h>

#include <sg.h>
#include <data_hand.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

typedef struct {
  Mat4 rotM;
  Vec3 transV;
  BOOL transSet;
} RotData;

float
GraphDegToRad(float ang)
{
  return ang / 180.0f * (float) M_PI;
}

static void
molRotate(DhMolP molP, void *clientData)
{
  RotData *dataP = clientData;
  Mat4 m, rotM;
  Vec3 transV;
  Vec4 v4;

  Mat4Copy(m, dataP->rotM);
  DhMolGetRotMat(molP, rotM);
  Mat4Mult(m, rotM);
  Mat4Ortho(m);
  DhMolSetRotMat(molP, m);

  /* adapt translation vectors so that all molecules rotate simultanously */
  if (dataP->transSet) {
    DhMolGetTransVect(molP, transV);
    Vec3Sub(transV, dataP->transV);
    Vec3To4(v4, transV);
    Mat4VecMult(v4, dataP->rotM);
    Vec4To3(transV, v4);
    Vec3Add(transV, dataP->transV);
    DhMolSetTransVect(molP, transV);
  } else {
    DhMolGetTransVect(molP, dataP->transV);
    dataP->transSet = TRUE;
  }
}

void
GraphRotate(Mat4 m)
{
  RotData data;

  Mat4Copy(data.rotM, m);
  data.transSet = FALSE;
  DhApplyMol(PropGetRef(PROP_MOVABLE, FALSE), molRotate, &data);
}

void
GraphRotateX(float ang)
{
  Mat4 m;
  float c = cosf(ang);
  float s = sinf(ang);

  Mat4Ident(m);
  m[1][1] = c;
  m[2][2] = c;
  m[1][2] = s;
  m[2][1] = -s;

  GraphRotate(m);
}

void
GraphRotateY(float ang)
{
  Mat4 m;
  float c = cosf(ang);
  float s = sinf(ang);

  Mat4Ident(m);
  m[0][0] = c;
  m[2][2] = c;
  m[0][2] = -s;
  m[2][0] = s;

  GraphRotate(m);
}

void
GraphRotateZ(float ang)
{
  Mat4 m;
  float c = cosf(ang);
  float s = sinf(ang);

  Mat4Ident(m);
  m[0][0] = c;
  m[1][1] = c;
  m[0][1] = s;
  m[1][0] = -s;

  GraphRotate(m);
}

static void
molTranslate(DhMolP molP, void *clientData)
{
  Vec3 transV;

  DhMolGetTransVect(molP, transV);
  Vec3Add(transV, clientData);
  DhMolSetTransVect(molP, transV);
}

void
GraphMove(Vec3 d)
{
  DhApplyMol(PropGetRef(PROP_MOVABLE, FALSE),  molTranslate, d);
}

static void
autoScale(BOOL setOrtho, int currProj)
{
  float xMin, xMax, yMin, yMax;
  int w, h;
  float aspect;
  float xScale, yScale, scale, oldZoomFact, zoomFact;
  float dx, dy, dScale;
  Vec3 vMove;

  SgStartRange();

  if (setOrtho) {
    SgSetProjection(SG_PROJ_ORTHO);
    SgUpdateView();
  }

  GraphDraw();

  if (setOrtho) {
    SgSetProjection(currProj);
    SgUpdateView();
  }

  SgEndRange(&xMin, &xMax, &yMin, &yMax);
  if (xMin >= xMax || yMin >= yMax)
    return;

  w = ParGetIntVal(PN_CURR_WIDTH);
  h = ParGetIntVal(PN_CURR_HEIGHT);
  aspect = (float) w / h;

  if (setOrtho && xMin > - aspect && xMax < aspect &&
      yMin > -1.0f && yMax < 1.0f)
    return;

  xScale = 2.0f * aspect / (xMax - xMin);
  yScale = 2.0f / (yMax - yMin);
  if (xScale < yScale)
    scale = xScale;
  else
    scale = yScale;

  scale *= 0.95f;
  oldZoomFact = (float) ParGetDoubleVal(PN_ZOOM_FACT);
  zoomFact = oldZoomFact * scale;
  ParSetDoubleVal(PN_ZOOM_FACT, zoomFact);
  GraphZoomSet(zoomFact);

  dx = 0.5f * (xMin + xMax);
  dy = 0.5f * (yMin + yMax);
  dScale = SgGetDepthFact(0.0f) / oldZoomFact;
  dx *= dScale;
  dy *= dScale;

  vMove[0] = - dx;
  vMove[1] = - dy;
  vMove[2] = 0.0f;
  DhApplyMol(PropGetRef(PROP_ALL, FALSE),  molTranslate, vMove);
}

void
GraphAutoScale(void)
{
  int proj;

  proj = ParGetIntVal(PN_PROJECTION);
  if (proj != SG_PROJ_ORTHO) {
    autoScale(TRUE, proj);
    autoScale(FALSE, proj);
    autoScale(FALSE, proj);
  } else {
    autoScale(FALSE, proj);
  }
}
