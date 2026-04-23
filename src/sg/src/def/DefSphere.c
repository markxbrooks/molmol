/*
************************************************************************
*
*   DefSphere.c - default sphere drawing for Sg
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/def/SCCS/s.DefSphere.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include <sg_def.h>
#include "def_sphere.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <sg.h>
#include <sg_get.h>
#include <sg_map.h>

#define MAX_PREC_RAD 1.0f  /* min radius drawn with max precision */

static int MaxPrec = -1;

typedef struct {
  int stripNo;
  float (* points0)[3];
  float (* points1)[3];
  float (* points2)[3];
  float (* points3)[3];
  float (* points4)[3];
  float (* points5)[3];
  float (* points6)[3];
  float (* points7)[3];
  int *meshStartInd, *meshSize;
} SphereData;

static SphereData *Spheres;

int
DefGetSpherePrecision(float r)
{
  int prec;

  prec = SgGetPrecision();
  if (prec < 0)
    prec = 0;

  while (r < MAX_PREC_RAD && prec > 0) {
    r *= 2.0f;
    prec--;
  }
  
  return prec;
}

static void
calcTriangles(SphereData *sphereP,
    double x0, double y0, double z0,
    double x1, double y1, double z1,
    double x2, double y2, double z2,
    int topStrip, int bottomStrip, int *currInd)
{
  if (topStrip + 1 == bottomStrip || bottomStrip + 1 == topStrip) {
    int strip, ind;
    if (topStrip + 1 == bottomStrip)
      strip = topStrip;
    else
      strip = bottomStrip;
    ind = currInd[strip];
    sphereP->points0[ind][0] = (float) x1;
    sphereP->points0[ind][1] = (float) y1;
    sphereP->points0[ind][2] = (float) z1;
    sphereP->points0[ind + 1][0] = (float) x2;
    sphereP->points0[ind + 1][1] = (float) y2;
    sphereP->points0[ind + 1][2] = (float) z2;
    sphereP->points0[ind + 2][0] = (float) x0;
    sphereP->points0[ind + 2][1] = (float) y0;
    sphereP->points0[ind + 2][2] = (float) z0;
    currInd[strip]++;
  } else {
    double x01, y01, z01;
    double x12, y12, z12;
    double x20, y20, z20;
    double a;
    int middleStrip;

    x01 = x0 + x1;
    y01 = y0 + y1;
    z01 = z0 + z1;
    a = sqrt(x01 * x01 + y01 * y01 + z01 * z01);
    x01 /= a;
    y01 /= a;
    z01 /= a;
    x12 = x1 + x2;
    y12 = y1 + y2;
    z12 = z1 + z2;
    a = sqrt(x12 * x12 + y12 * y12 + z12 * z12);
    x12 /= a;
    y12 /= a;
    z12 /= a;
    x20 = x2 + x0;
    y20 = y2 + y0;
    z20 = z2 + z0;
    a = sqrt(x20 * x20 + y20 * y20 + z20 * z20);
    x20 /= a;
    y20 /= a;
    z20 /= a;
    middleStrip = (topStrip + bottomStrip) / 2;
    calcTriangles(sphereP,
	x01, y01, z01,
	x1, y1, z1,
	x12, y12, z12,
	middleStrip, bottomStrip, currInd);
    calcTriangles(sphereP,
	x20, y20, z20,
	x12, y12, z12,
	x01, y01, z01,
	bottomStrip, middleStrip, currInd);
    calcTriangles(sphereP,
	x0, y0, z0,
	x01, y01, z01,
	x20, y20, z20,
	middleStrip, bottomStrip, currInd);
    calcTriangles(sphereP,
	x20, y20, z20,
	x12, y12, z12,
	x2, y2, z2,
	topStrip, middleStrip, currInd);
  }
}

static void
calcSphere(SphereData *sphereP, int prec)
{
  int triNo, pointNo;
  int *currInd;
  float x, y, z;
  int i;

  sphereP->stripNo = 1 << prec;
  triNo = sphereP->stripNo * sphereP->stripNo;
  pointNo = triNo + 2 * sphereP->stripNo;

  sphereP->points0 = malloc(pointNo * sizeof(*sphereP->points0));
  sphereP->points1 = malloc(pointNo * sizeof(*sphereP->points1));
  sphereP->points2 = malloc(pointNo * sizeof(*sphereP->points2));
  sphereP->points3 = malloc(pointNo * sizeof(*sphereP->points3));
  sphereP->points4 = malloc(pointNo * sizeof(*sphereP->points4));
  sphereP->points5 = malloc(pointNo * sizeof(*sphereP->points5));
  sphereP->points6 = malloc(pointNo * sizeof(*sphereP->points6));
  sphereP->points7 = malloc(pointNo * sizeof(*sphereP->points7));

  sphereP->meshStartInd = malloc(sphereP->stripNo *
      sizeof(*sphereP->meshStartInd));
  sphereP->meshSize = malloc(sphereP->stripNo *
      sizeof(*sphereP->meshSize));
  currInd = malloc(sphereP->stripNo * sizeof(*currInd));

  sphereP->meshStartInd[0] = 0;
  currInd[0] = 0;
  sphereP->meshSize[0] = 3;
  for (i = 1; i < sphereP->stripNo; i++) {
    sphereP->meshStartInd[i] = sphereP->meshStartInd[i - 1] +
	sphereP->meshSize[i - 1];
    currInd[i] = sphereP->meshStartInd[i];
    sphereP->meshSize[i] = sphereP->meshSize[i - 1] + 2;
  };

  calcTriangles(sphereP,
      1.0, 0.0, 0.0,    
      0.0, 1.0, 0.0,
      0.0, 0.0, 1.0,
      0, sphereP->stripNo, currInd);

  free(currInd);

  for (i = 0; i < pointNo; i++) {
    x = sphereP->points0[i][0];
    y = sphereP->points0[i][1];
    z = sphereP->points0[i][2];
    sphereP->points1[i][0] = -y;
    sphereP->points1[i][1] = x;
    sphereP->points1[i][2] = z;
    sphereP->points2[i][0] = -x;
    sphereP->points2[i][1] = -y;
    sphereP->points2[i][2] = z;
    sphereP->points3[i][0] = y;
    sphereP->points3[i][1] = -x;
    sphereP->points3[i][2] = z;
    sphereP->points4[i][0] = y;
    sphereP->points4[i][1] = x;
    sphereP->points4[i][2] = -z;
    sphereP->points5[i][0] = x;
    sphereP->points5[i][1] = -y;
    sphereP->points5[i][2] = -z;
    sphereP->points6[i][0] = -y;
    sphereP->points6[i][1] = -x;
    sphereP->points6[i][2] = -z;
    sphereP->points7[i][0] = -x;
    sphereP->points7[i][1] = y;
    sphereP->points7[i][2] = -z;
  }
}

static SphereData *
getSphere(int prec)
{
  int i;

  if (prec > MaxPrec) {
    if (MaxPrec < 0)
      Spheres = malloc((prec + 1) * sizeof(*Spheres));
    else
      Spheres = realloc(Spheres, (prec + 1) * sizeof(*Spheres));

    for (i = MaxPrec + 1; i <= prec; i++)
      calcSphere(Spheres + i, i);

    MaxPrec = prec;
  }

  return Spheres + prec;
}

void
SgDefDrawSphere(float x[3], float r)
{
  float xm[3];
  SphereData *sphereP;
  int i;

  if (SgGetFeature(SG_FEATURE_FAST_SPHERE)) {
    if (SgMapSphere(xm, &r, NULL, x, r) > 0)
      SgDrawDisc2D(xm, r);
    return;
  }

  sphereP = getSphere(DefGetSpherePrecision(r));

  SgPushMatrix();
  SgTranslate(x);
  SgScale(r, r, r);
  SgStartSurface(SG_SURF_UNIFORM);
  for (i = 0; i < sphereP->stripNo; i++) {
    SgDrawTriMesh(sphereP->points0 + sphereP->meshStartInd[i],
        sphereP->points0 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
    SgDrawTriMesh(sphereP->points1 + sphereP->meshStartInd[i],
        sphereP->points1 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
    SgDrawTriMesh(sphereP->points2 + sphereP->meshStartInd[i],
        sphereP->points2 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
    SgDrawTriMesh(sphereP->points3 + sphereP->meshStartInd[i],
        sphereP->points3 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
    SgDrawTriMesh(sphereP->points4 + sphereP->meshStartInd[i],
        sphereP->points4 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
    SgDrawTriMesh(sphereP->points5 + sphereP->meshStartInd[i],
        sphereP->points5 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
    SgDrawTriMesh(sphereP->points6 + sphereP->meshStartInd[i],
        sphereP->points6 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
    SgDrawTriMesh(sphereP->points7 + sphereP->meshStartInd[i],
        sphereP->points7 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
  }
  SgEndSurface();
  SgPopMatrix();
}

void
DefDrawHalfSphere(BOOL top, int prec)
{
  SphereData *sphereP;
  int i;

  sphereP = getSphere(prec);

  if (top) {
    for (i = 0; i < sphereP->stripNo; i++) {
      SgDrawTriMesh(sphereP->points0 + sphereP->meshStartInd[i],
	  sphereP->points0 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
      SgDrawTriMesh(sphereP->points1 + sphereP->meshStartInd[i],
	  sphereP->points1 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
      SgDrawTriMesh(sphereP->points2 + sphereP->meshStartInd[i],
	  sphereP->points2 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
      SgDrawTriMesh(sphereP->points3 + sphereP->meshStartInd[i],
	  sphereP->points3 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
    }
  } else {
    for (i = 0; i < sphereP->stripNo; i++) {
      SgDrawTriMesh(sphereP->points4 + sphereP->meshStartInd[i],
	  sphereP->points4 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
      SgDrawTriMesh(sphereP->points5 + sphereP->meshStartInd[i],
	  sphereP->points5 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
      SgDrawTriMesh(sphereP->points6 + sphereP->meshStartInd[i],
	  sphereP->points6 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
      SgDrawTriMesh(sphereP->points7 + sphereP->meshStartInd[i],
	  sphereP->points7 + sphereP->meshStartInd[i], sphereP->meshSize[i]);
    }
  }
}
