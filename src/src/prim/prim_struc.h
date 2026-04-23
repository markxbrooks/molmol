/*
************************************************************************
*
*   prim_struc.h - data structures for primitives
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.prim_struc.h
*   SCCS identification       : 1.15
*
************************************************************************
*/

#ifndef _PRIM_STRUC_H_
#define _PRIM_STRUC_H_

#include <mat_vec.h>
#include <cubic_spline.h>
#include <linlist.h>
#include <prim_hand.h>

typedef struct {
  Vec3 x, v;
} PrimCircle;

typedef struct {
  Vec3 *xA;
  int pointNo;
} PrimMapPolyline;

typedef struct {
  PrimMapStyle style;
  PrimMapPolyline *polylineA;
  int polylineNo;
} PrimMap;

typedef struct {
  DhAtomP *atomA;
  int atomNo;
  PrimTrajecStyle style;
  Vec3 *xA;
} PrimTrajec;

typedef struct {
  Vec3 xA[3];
  Vec3 nvA[3];
} PrimPlateTri;

typedef struct {
  Vec3 xA[4];
  Vec3 nv;
} PrimPlateQuad;

typedef struct {
  DhAtomP *atomA;
  int atomNo;
  float thick;
  BOOL pointValid;
  PrimPlateTri *triA;
  int triNo;
  PrimPlateQuad *quadA;
  int quadNo;
} PrimPlate;

typedef struct {
  PrimCylinderStyle style;
  Vec3 x1, x2;
} PrimCylinder;

typedef struct {
  Vec3 *xA;
  Vec3 *nvA;
  int pointNo;
} PrimSolidMesh;

typedef struct {
  Vec3 *xA;
  int pointNo;
  Vec3 nv;
} PrimSolidPoly;

typedef struct {
  PrimSolidKind kind;
  DhAtomP *atomA;
  int atomNo;
  float cover;
  BOOL geomValid;
  Vec3 cent;
  Vec3 axis1, axis2, axis3;
  float size1, size2, size3;
  BOOL pointValid;
  int prec;
  PrimSolidMesh *meshA;
  int meshNo;
  PrimSolidPoly *polyA;
  int polyNo;
} PrimSolid;

typedef struct {
  Vec3 x;
  Vec3 dx;
  char *str;
} PrimText;

typedef struct {
  Vec3 *p;
  int size;
  DhAtomP atomP;
} PrimDotSurfacePatch;

typedef struct {
  PrimDotSurfacePatch *p;
  int size;
} PrimDotSurface;

typedef struct {
  int *indA;
  int size;
} PrimSurfaceMesh;

typedef struct {
  Vec3 *xA;
  Vec3 *nvA;
  Vec3 *colA;
  float *potA;
  int pointNo;
  PrimSurfaceMesh *meshA;
  int meshNo, maxSize;
} PrimSurface;

typedef struct {
  int num;
  int primNo;
  DhAtomP *atomA;
  float *radA;
  int atomNo;
  BOOL splineValid;
  CubicSplineCoeff *coeffA;
} Ribbon;

typedef struct {
  Vec3 *xA;
  Vec3 *nvA;
} RibbonTrace;

typedef struct {
  int traceLeftI, traceRightI;
  int pointNo;
  BOOL flat;
  BOOL secColor;
  float *parA;
} RibbonStrip;

typedef struct {
  Vec3 *xA;
  int pointNo;
  Vec3 nv;
  float par;
} RibbonPoly;

typedef struct {
  Ribbon *ribbonP;
  float startPar, endPar;
  RibbonShape shape;
  RibbonOrient orient;
  float width, thick;
  RibbonStyle style;
  RibbonEnd startStyle, endStyle;
  float startLen, endLen;
  float arrowWidth;
  RibbonPaint paint;
  RibbonRadius radius;
  BOOL pointValid;
  int prec;
  float startRad, midRad, endRad;
  Vec3 endRefV;
  RibbonTrace *traceA;
  int traceNo;
  RibbonStrip *stripA;
  int stripNo;
  RibbonPoly *polyA;
  int polyNo;
} PrimRibbonPart;

typedef struct {
  DhAtomP *atomA;
  int atomNo;
  BOOL pointValid;
  int prec;
  Vec3 *xA1, *xA2;
  Vec3 *nvA1, *nvA2;
  int traceNo, traceLen;
  BOOL closed;
} PrimSheet;

typedef struct {
  PrimDrawobjType type;
  PrimDrawobjStyle style;
  Vec3 *xA, *dxA;
  int pointNo;
} PrimDrawobj;

struct PrimObjS {
  int num;
  PrimType type;
  DhMolP molP;
  union {
    PrimCircle circle;
    PrimMap map;
    PrimTrajec trajec;
    PrimPlate plate;
    PrimCylinder cylinder;
    PrimSolid solid;
    PrimText text;
    PrimDotSurface dotSurface;
    PrimSurface surface;
    PrimRibbonPart ribbon;
    PrimSheet sheet;
    PrimDrawobj drawobj;
  } u;
  unsigned *propTab;
  AttrP attrP;
};

extern LINLIST RibbonListGet(void);
extern Ribbon *RibbonAdd(void);

extern LINLIST PrimListGet(void);

extern PrimObjP PrimNewEmpty(PrimType, DhMolP);

extern void RibbonAddInvalidCB(Ribbon *);
extern void PrimAddInvalidCB(PrimObjP);

#endif  /* _PRIM_STRUC_H_ */
