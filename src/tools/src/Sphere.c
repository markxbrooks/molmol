/*
************************************************************************
*
*   Sphere.c - sphere calculation
*
*   Copyright (c) 1995
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.Sphere.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sphere.h>

#include <stdlib.h>
#include <math.h>

#include <mat_vec.h>

static void
calcTriangles(
    double x0, double y0, double z0,
    double x1, double y1, double z1,
    double x2, double y2, double z2,
    int rowStartA[], int rowNo, int quad,
    int row0, int ind0, int ind1,
    int row2, int ind2,
    Vec3 *pointA)
{
  if (row0 + 1 == row2 || row2 + 1 == row0) {
    int row0Size, row2Size;

    if (row0 == - rowNo || row0 == rowNo)
      row0Size = 1;
    else if (row0 < 0)
      row0Size = rowNo + row0;
    else
      row0Size = rowNo - row0;

    if (row2 == - rowNo || row2 == rowNo)
      row2Size = 1;
    else if (row2 < 0)
      row2Size = rowNo + row2;
    else
      row2Size = rowNo - row2;

    if (ind0 < (quad + 1) * row0Size) {
      ind0 += rowStartA[rowNo + row0];
      pointA[ind0][0] = (float) x0;
      pointA[ind0][1] = (float) y0;
      pointA[ind0][2] = (float) z0;
    }

    if (ind1 < (quad + 1) * row0Size) {
      ind1 += rowStartA[rowNo + row0];
      pointA[ind1][0] = (float) x1;
      pointA[ind1][1] = (float) y1;
      pointA[ind1][2] = (float) z1;
    }

    if (ind2 < (quad + 1) * row2Size) {
      ind2 += rowStartA[rowNo + row2];
      pointA[ind2][0] = (float) x2;
      pointA[ind2][1] = (float) y2;
      pointA[ind2][2] = (float) z2;
    }
  } else {
    double x01, y01, z01;
    double x12, y12, z12;
    double x20, y20, z20;
    double a;
    int rowMid, indMid01, indMid02, indMid12;

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

    rowMid = (row0 + row2) / 2;
    indMid01 = (ind0 + ind1) / 2;
    indMid02 = (ind0 + ind2) / 2;
    indMid12 = (ind1 + ind2) / 2;

    calcTriangles(
        x0, y0, z0,
        x01, y01, z01,
        x20, y20, z20,
	rowStartA, rowNo, quad,
	row0, ind0, indMid01,
	rowMid, indMid02,
	pointA);
    calcTriangles(
        x01, y01, z01,
        x1, y1, z1,
        x12, y12, z12,
	rowStartA, rowNo, quad,
	row0, indMid01, ind1,
	rowMid, indMid12,
	pointA);
    calcTriangles(
        x20, y20, z20,
        x12, y12, z12,
        x01, y01, z01,
	rowStartA, rowNo, quad,
	rowMid, indMid02, indMid12,
	row0, indMid01,
	pointA);
    calcTriangles(
        x20, y20, z20,
        x12, y12, z12,
        x2, y2, z2,
	rowStartA, rowNo, quad,
	rowMid, indMid02, indMid12,
	row2, ind2,
	pointA);
  }
}

void
SphereCalcPoints(int divNo, Vec3 **pointAP, int *pointNoP)
{
  Vec3 *pointA;
  int *rowStartA;
  int pointNo, rowNo, rowSize, i;

  rowNo = 1 << divNo;

  rowStartA = malloc((2 * rowNo + 1) * sizeof(*rowStartA));

  rowStartA[0] = 0;
  rowStartA[1] = 1;
  rowSize = 4;
  for (i = 2; i <= rowNo; i++) {
    rowStartA[i] = rowStartA[i - 1] + rowSize;
    rowSize += 4;
  }
  for (i = rowNo + 1; i < 2 * rowNo + 1; i++) {
    rowStartA[i] = rowStartA[i - 1] + rowSize;
    rowSize -= 4;
  }

  pointNo = 4 * rowNo * rowNo + 2;
  pointA = malloc(pointNo * sizeof(*pointA));

  calcTriangles(
      1.0, 0.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 0.0, 1.0,
      rowStartA, rowNo, 0,
      0, 0, rowNo,
      rowNo, 0,
      pointA);
  calcTriangles(
      0.0, 1.0, 0.0,
      -1.0, 0.0, 0.0,
      0.0, 0.0, 1.0,
      rowStartA, rowNo, 1,
      0, rowNo, 2 * rowNo,
      rowNo, 0,
      pointA);
  calcTriangles(
      -1.0, 0.0, 0.0,
      0.0, -1.0, 0.0,
      0.0, 0.0, 1.0,
      rowStartA, rowNo, 2,
      0, 2 * rowNo, 3 * rowNo,
      rowNo, 0,
      pointA);
  calcTriangles(
      0.0, -1.0, 0.0,
      1.0, 0.0, 0.0,
      0.0, 0.0, 1.0,
      rowStartA, rowNo, 3,
      0, 3 * rowNo, 4 * rowNo,
      rowNo, 0,
      pointA);
  calcTriangles(
      1.0, 0.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 0.0, -1.0,
      rowStartA, rowNo, 0,
      0, 0, rowNo,
      - rowNo, 0,
      pointA);
  calcTriangles(
      0.0, 1.0, 0.0,
      -1.0, 0.0, 0.0,
      0.0, 0.0, -1.0,
      rowStartA, rowNo, 1,
      0, rowNo, 2 * rowNo,
      - rowNo, 0,
      pointA);
  calcTriangles(
      -1.0, 0.0, 0.0,
      0.0, -1.0, 0.0,
      0.0, 0.0, -1.0,
      rowStartA, rowNo, 2,
      0, 2 * rowNo, 3 * rowNo,
      - rowNo, 0,
      pointA);
  calcTriangles(
      0.0, -1.0, 0.0,
      1.0, 0.0, 0.0,
      0.0, 0.0, -1.0,
      rowStartA, rowNo, 3,
      0, 3 * rowNo, 4 * rowNo,
      - rowNo, 0,
      pointA);

  free(rowStartA);

  *pointAP = pointA;
  *pointNoP = pointNo;
}
