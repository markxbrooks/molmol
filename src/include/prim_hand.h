/*
************************************************************************
*
*   prim_hand.h - data handler for primitives
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.prim_hand.h
*   SCCS identification       : 1.27
*
************************************************************************
*/

#ifndef _PRIM_HAND_H_
#define _PRIM_HAND_H_

#include <bool.h>
#include <g_file.h>
#include <prop_def.h>
#include <data_hand.h>
#include <attr_type.h>

typedef struct PrimObjS *PrimObjP;

typedef enum {
  PT_ALL,
  PT_CIRCLE,
  PT_PLATE,
  PT_CYLINDER,
  PT_SOLID,
  PT_RIBBON,
  PT_SHEET,
  PT_TEXT,
  PT_DOT_SURFACE,
  PT_SURFACE,
  PT_DRAWOBJ,
  PT_TRAJEC,
  PT_MAP
} PrimType;

typedef enum {
  PMS_INVISIBLE,
  PMS_LINE,
  PMS_CYLINDER,
  PMS_NEON
} PrimMapStyle;

typedef enum {
  PTS_INVISIBLE,
  PTS_LINE,
  PTS_NEON,
  PTS_ELLIPSOID
} PrimTrajecStyle;

typedef enum {
  PCS_INVISIBLE,
  PCS_NORMAL,
  PCS_ARROW
} PrimCylinderStyle;

typedef enum {
  PSK_SPHERE,
  PSK_ELLIPSOID,
  PSK_CYLINDER,
  PSK_CONE,
  PSK_BOX
} PrimSolidKind;

typedef enum {
  REC_START,
  REC_END
} RibbonEndChoice;

typedef enum {
  RS_INTERPOL,
  RS_SMOOTH
} RibbonShape;

typedef enum {
  RO_FREE,
  RO_CYLIND,
  RO_SPACING,
  RO_NEIGH
} RibbonOrient;

typedef enum {
  RS_INVISIBLE,
  RS_RECT,
  RS_ROUND,
  RS_ROUNDED,
  RS_ELLIP,
  RS_LENSE
} RibbonStyle;

typedef enum {
  RE_OPEN,
  RE_SHARP,
  RE_SOFT,
  RE_ARROW
} RibbonEnd;

typedef enum {
  RP_ONE,
  RP_TWO,
  RP_ATOM,
  RP_ATOM_SMOOTH
} RibbonPaint;

typedef enum {
  RR_CONST,
  RR_ATOM
} RibbonRadius;

typedef enum {
  PDT_LINE,
  PDT_RECT,
  PDT_CIRCLE
} PrimDrawobjType;

typedef enum {
  PDS_INVISIBLE,
  PDS_LINE,
  PDS_CYLINDER,
  PDS_NEON
} PrimDrawobjStyle;

typedef void (*PrimApplyFunc) (PrimObjP, void *);
typedef void (*PrimPropCB) (PrimObjP, PropRefP, BOOL, void *);
typedef void (* PrimSurfaceTrimFunc) (float *, Vec3, void *);
typedef void (* PrimSurfaceSetColorFunc) (Vec3, Vec3, Vec3, float, void *);
typedef void (* PrimSurfaceSetPotFunc) (float *, Vec3, void *);

extern void PrimDestroyAll(void);
extern PrimObjP PrimNew(PrimType, DhMolP);
extern PrimObjP PrimNewTrajec(DhAtomP[], int);
extern PrimObjP PrimNewPlate(DhAtomP[], int);
extern void PrimDestroy(PrimObjP);

extern void PrimMapAddPolyline(PrimObjP, Vec3 *, int);
extern void PrimDotSurfaceAddPatch(PrimObjP, DhAtomP, Vec3 *, int);
extern void PrimSurfaceSetPoints(PrimObjP, Vec3[], Vec3[], int);
extern void PrimSurfaceAddMesh(PrimObjP, int[], int);
extern void RibbonNew(DhMolP);
extern void RibbonSplit(PrimObjP);

extern void PrimApply(PrimType, PropRefP, PrimApplyFunc, void *);
extern void PrimMolApply(PrimType, PropRefP, DhMolP, PrimApplyFunc, void *);

extern PrimObjP PrimFindNumber(int);

extern void PrimAddPropCB(PrimType, PropRefP, PrimPropCB, void *);

extern void PrimSetProp(PropRefP, PrimObjP, BOOL);
extern BOOL PrimGetProp(PropRefP, PrimObjP);
extern unsigned *PrimGetPropTab(PrimObjP);

extern void PrimSetAttr(PrimObjP, AttrP);

extern void PrimSetPos(PrimObjP, Vec3);
extern void PrimSetVec(PrimObjP, Vec3);
extern void PrimSetCylinderStyle(PrimObjP, PrimCylinderStyle);
extern void PrimSetMapStyle(PrimObjP, PrimMapStyle);
extern void PrimSetTrajecStyle(PrimObjP, PrimTrajecStyle);
extern void PrimSetThick(PrimObjP, float);
extern void PrimSetSolidKind(PrimObjP, PrimSolidKind);
extern void PrimSetCover(PrimObjP, float);
extern void PrimSetText(PrimObjP, char *);
extern void PrimSetDrawobjType(PrimObjP, PrimDrawobjType);
extern void PrimSetDrawobjStyle(PrimObjP, PrimDrawobjStyle);
extern void PrimSetPoint(PrimObjP, int, Vec3, Vec3);

extern void RibbonSetEndPar(PrimObjP, RibbonEndChoice, float);
extern void RibbonSetShape(PrimObjP, RibbonShape);
extern void RibbonSetOrient(PrimObjP, RibbonOrient);
extern void RibbonSetWidth(PrimObjP, float);
extern void RibbonSetStyle(PrimObjP, RibbonStyle);
extern void RibbonSetEndStyle(PrimObjP, RibbonEndChoice, RibbonEnd);
extern void RibbonSetEndLen(PrimObjP, RibbonEndChoice, float);
extern void RibbonSetArrowWidth(PrimObjP,  float);
extern void RibbonSetPaint(PrimObjP, RibbonPaint);
extern void RibbonSetRadius(PrimObjP, RibbonRadius);

extern void PrimSurfaceTrim(PrimObjP, PrimSurfaceTrimFunc, void *);
extern void PrimSurfaceSetColor(PrimObjP, PrimSurfaceSetColorFunc, void *);
extern void PrimSurfaceSetPot(PrimObjP, PrimSurfaceSetPotFunc, void *);

extern int PrimGetNumber(PrimObjP);
extern PrimType PrimGetType(PrimObjP);
extern DhMolP PrimGetMol(PrimObjP);
extern AttrP PrimGetAttr(PrimObjP);

extern void PrimGetPos(PrimObjP, Vec3);
extern void PrimGetVec(PrimObjP, Vec3);
extern PrimCylinderStyle PrimGetCylinderStyle(PrimObjP);
extern PrimMapStyle PrimGetMapStyle(PrimObjP);
extern PrimTrajecStyle PrimGetTrajecStyle(PrimObjP);
extern float PrimGetThick(PrimObjP);
extern PrimSolidKind PrimGetSolidKind(PrimObjP);
extern float PrimGetCover(PrimObjP);
extern char *PrimGetText(PrimObjP);
extern PrimDrawobjType PrimGetDrawobjType(PrimObjP);
extern PrimDrawobjStyle PrimGetDrawobjStyle(PrimObjP);
extern void PrimGetPoint(PrimObjP, int, Vec3, Vec3);
extern int PrimGetPointNo(PrimObjP);

extern float RibbonGetEndPar(PrimObjP, RibbonEndChoice);
extern RibbonShape RibbonGetShape(PrimObjP);
extern RibbonOrient RibbonGetOrient(PrimObjP);
extern float RibbonGetWidth(PrimObjP);
extern RibbonStyle RibbonGetStyle(PrimObjP);
extern RibbonEnd RibbonGetEndStyle(PrimObjP, RibbonEndChoice);
extern float RibbonGetEndLen(PrimObjP, RibbonEndChoice);
extern float RibbonGetArrowWidth(PrimObjP);
extern RibbonPaint RibbonGetPaint(PrimObjP);
extern RibbonRadius RibbonGetRadius(PrimObjP);

extern BOOL PrimDump(GFile);
extern BOOL PrimUndump(GFile, int);

#endif  /* _PRIM_HAND_H_ */
