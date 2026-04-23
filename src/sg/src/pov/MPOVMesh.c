/*
************************************************************************
*
*   MPOVMesh.c - MPOV mesh drawing
*
*   Copyright (c) 1994-2000
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.MPOVMesh.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "mpov_mesh.h"

#include <stdio.h>

#include <sg.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "pov_util.h"
#include "pov_color.h"
#include "pov_clip.h"

static void
plotPoint(float xm[3], float x[3])
{
  SgMapModelPoint(xm, x);
  POVPlotVect(xm);
}

static void
plotVect(float v[3])
{
  float vm[3];

  SgMapModelVect(vm, v);
  POVPlotVect(vm);
}

static void
plotIndices(int n, BOOL hasCol)
{
  int i;

  for (i = 0; i < n - 2; i += 2) {
    SgPlotOutputStr("<");
    SgPlotOutputInt(i);
    SgPlotOutputStr(", ");
    SgPlotOutputInt(i + 1);
    SgPlotOutputStr(", ");
    SgPlotOutputInt(i + 2);
    SgPlotOutputStr(">");
    if (hasCol) {
      SgPlotOutputStr(", ");
      SgPlotOutputInt(i);
      SgPlotOutputStr(", ");
      SgPlotOutputInt(i + 1);
      SgPlotOutputStr(", ");
      SgPlotOutputInt(i + 2);
      SgPlotOutputStr(",");
    }
    SgPlotOutputStr("\n");

    if (i + 3 == n)
      break;

    SgPlotOutputStr("<");
    SgPlotOutputInt(i + 2);
    SgPlotOutputStr(", ");
    SgPlotOutputInt(i + 1);
    SgPlotOutputStr(", ");
    SgPlotOutputInt(i + 3);
    SgPlotOutputStr(">");
    if (hasCol) {
      SgPlotOutputStr(", ");
      SgPlotOutputInt(i + 2);
      SgPlotOutputStr(", ");
      SgPlotOutputInt(i + 1);
      SgPlotOutputStr(", ");
      SgPlotOutputInt(i + 3);
    }
    SgPlotOutputStr("\n");
  }
}

static void
drawMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  float xm[3];
  BOOL clip;
  int i;

  POVUpdateColor();

  SgPlotOutputStr("mesh2 {\n");

  clip = FALSE;

  SgPlotOutputStr("vertex_vectors {\n");
  SgPlotOutputInt(n);
  SgPlotOutputStr(",\n");
  for (i = 0; i < n; i++) {
    plotPoint(xm, x[i]);
    SgPlotOutputStr(",\n");
    if (! clip)
      clip = ! POVClipTest(xm, 0.0f);
  }
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("normal_vectors {\n");
  SgPlotOutputInt(n);
  SgPlotOutputStr(",\n");
  for (i = 0; i < n; i++) {
    plotVect(nv[i]);
    SgPlotOutputStr(",\n");
  }
  SgPlotOutputStr("}\n");

  if (col != NULL) {
    SgPlotOutputStr("texture_list {\n");
    SgPlotOutputInt(n);
    SgPlotOutputStr(",\n");
    for (i = 0; i < n; i++) {
      SgSetColor(col[i][0], col[i][1], col[i][2]);
      POVPlotTexture();
      SgPlotOutputStr(",\n");
    }
    SgPlotOutputStr("}\n");
  }

  SgPlotOutputStr("face_indices {\n");
  SgPlotOutputInt(n - 2);
  SgPlotOutputStr(",\n");
  plotIndices(n, col != NULL);
  SgPlotOutputStr("}\n");

  POVPlotColor();

  if (clip)
    POVClipOutput();

  SgPlotOutputStr("}\n");
}

void
SgMPOVDrawTriMesh(float x[][3], float nv[][3], int n)
{
  drawMesh(x, nv, NULL, n);
}

void
SgMPOVDrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  drawMesh(x, nv, col, n);
}

static void
drawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  float xm[3];
  BOOL clip;
  int i;

  if (nv2 == NULL)
    nv2 = nv1;

  if (col2 == NULL)
    col2 = col1;

  POVUpdateColor();

  SgPlotOutputStr("mesh2 {\n");

  clip = FALSE;

  SgPlotOutputStr("vertex_vectors {\n");
  SgPlotOutputInt(2 * n);
  SgPlotOutputStr(",\n");
  for (i = 0; i < n; i++) {
    plotPoint(xm, x1[i]);
    SgPlotOutputStr(",\n");
    if (! clip)
      clip = ! POVClipTest(xm, 0.0f);
    plotPoint(xm, x2[i]);
    SgPlotOutputStr(",\n");
    if (! clip)
      clip = ! POVClipTest(xm, 0.0f);
  }
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("normal_vectors {\n");
  SgPlotOutputInt(2 * n);
  SgPlotOutputStr(",\n");
  for (i = 0; i < n; i++) {
    plotVect(nv1[i]);
    SgPlotOutputStr(",\n");
    plotVect(nv2[i]);
    SgPlotOutputStr(",\n");
  }
  SgPlotOutputStr("}\n");

  if (col1 != NULL) {
    SgPlotOutputStr("texture_list {\n");
    SgPlotOutputInt(2 * n);
    SgPlotOutputStr(",\n");
    for (i = 0; i < n; i++) {
      SgSetColor(col1[i][0], col1[i][1], col1[i][2]);
      POVPlotTexture();
      SgPlotOutputStr(",\n");
      SgSetColor(col2[i][0], col2[i][1], col2[i][2]);
      POVPlotTexture();
      SgPlotOutputStr(",\n");
    }
    SgPlotOutputStr("}\n");
  }

  SgPlotOutputStr("face_indices {\n");
  SgPlotOutputInt(2 * n - 2);
  SgPlotOutputStr(",\n");
  plotIndices(2 * n, col1 != NULL);
  SgPlotOutputStr("}\n");

  POVPlotColor();

  if (clip)
    POVClipOutput();

  SgPlotOutputStr("}\n");
}

void
SgMPOVDrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  drawStrip(x1, x2, nv1, nv2, NULL, NULL, n);
}

void
SgMPOVDrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  drawStrip(x1, x2, nv1, nv2, col1, col2, n);
}
