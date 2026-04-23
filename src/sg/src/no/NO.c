/*
************************************************************************
*
*   NO.c - NO functions (all empty)
*
*   Copyright (c) 1995-99
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
*   Date of last modification : 99/10/30
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/no/SCCS/s.NO.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "no.h"

#include <stdlib.h>
#include <stdio.h>

#include <hashtab.h>

#define TABLE_SIZE 97

static HASHTABLE ObjTab = NULL;

SgRes
SgNOInit(int *argcP, char *argv[])
{
  return SG_RES_OK;
}

SgRes
SgNOCleanup(void)
{
  return SG_RES_OK;
}

static unsigned
hashFunc(void *p, unsigned size)
{
  SgObjId *objP = p;

  return *objP % size;
}

static int
compFunc(void *p1, void *p2)
{
  SgObjId *obj1P = p1;
  SgObjId *obj2P = p2;

  return *obj1P != *obj2P;
}

SgObjId
SgNOBeginObj(void)
/* must return a unique id! */
{
  SgObjId obj;

  if (ObjTab == NULL)
    ObjTab = HashtabOpen(TABLE_SIZE, sizeof(SgObjId), hashFunc, compFunc);

  for (;;) {
    obj = rand();
    if (HashtabSearch(ObjTab, &obj) == NULL)
      break;
  }

  (void) HashtabInsert(ObjTab, &obj, FALSE);

  return obj;
}

void
SgNOEndObj(void)
{
}

void
SgNOReplaceObj(SgObjId obj)
{
}

void
SgNODestroyObj(SgObjId obj)
{
  SgObjId *objP;

  objP = HashtabSearch(ObjTab, &obj);
  HashtabRemove(ObjTab, objP);
}

void
SgNODrawObj(SgObjId obj)
{
}

void
SgNODrawMarkers2D(float x[][2], int n)
{
}

void
SgNODrawLine2D(float x0[2], float x1[2])
{
}

void
SgNODrawArrow2D(float x0[2], float x1[2])
{
}

void
SgNODrawPolyline2D(float x[][2], int n)
{
}

void
SgNODrawPolygon2D(float x[][2], int n)
{
}

void
SgNODrawCircle2D(float x[2], float r)
{
}

void
SgNODrawDisc2D(float x[2], float r)
{
}

void
SgNODrawAnnot2D(float x[2], char *str)
{
}

void
SgNODrawText2D(float x[2], char *str)
{
}

void
SgNODrawMarkers(float x[][3], int n)
{
}

void
SgNODrawLine(float x0[3], float x1[3])
{
}

void
SgNODrawArrow(float x0[3], float x1[3])
{
}

void
SgNODrawPolyline(float x[][3], int n)
{
}

void
SgNODrawPolygon(float x[][3], int n, float nv[3])
{
}

void
SgNODrawCircle(float x[3], float v[3], float r)
{
}

void
SgNODrawDisc(float x[3], float v[3], float r)
{
}

void
SgNODrawAnnot(float x[3], char *str)
{
}

void
SgNODrawText(float x[3], char *str)
{
}

void
SgNODrawSphere(float x[3], float r)
{
}

void
SgNODrawCone(float x0[3], float x1[3], float r0, float r1,
    SgConeEnd end0, SgConeEnd end1)
{
}

void
SgNODrawTriMesh(float x[][3], float nv[][3], int n)
{
}

void
SgNODrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
}

void
SgNODrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
}

void
SgNODrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
}
