/*
************************************************************************
*
*   data_hand.h - data handler
*
*   Copyright (c) 1994-98
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.data_hand.h
*   SCCS identification       : 1.39
*
************************************************************************
*/

#ifndef _DATA_HAND_H_
#define _DATA_HAND_H_

#include <bool.h>
#include <dstr.h>
#include <mat_vec.h>
#include <g_file.h>
#include <prop_def.h>
#include <attr_type.h>

#define DH_SHIFT_UNKNOWN 999.0f
#define DH_ANGLE_MIN -999.0f
#define DH_ANGLE_MAX 999.0f

typedef struct DhMolS *DhMolP;
typedef struct DhResS *DhResP;
typedef struct DhAtomS *DhAtomP;
typedef struct DhBondS *DhBondP;
typedef struct DhAngleS *DhAngleP;
typedef struct DhDistS *DhDistP;
typedef struct DhResDefS *DhResDefP;

typedef enum {
  SP_FIRST,
  SP_LAST
} DhSeqPos;

typedef enum {
  MK_KEEP_ANGLES,
  MK_KEEP_ATOMS
} DhMutationKind;

typedef enum {
  DK_UPPER,
  DK_LOWER,
  DK_ACTUAL,
  DK_HBOND
} DhDistKind;

typedef enum {
  CK_CHARGE,
  CK_HEAVY,
  CK_AVG,
  CK_SIMPLE
} DhChargeKind;

typedef enum {
  NC_LEFT,
  NC_RIGHT
} DhNeighChoice;

typedef enum {
  BFC_AVG_B,
  BFC_MAX_DISP,
  BFC_AVG_DISP,
  BFC_RMS_DISP,
  BFC_XRAY
} BFactorCalc;

typedef void (*DhMolApplyFunc) (DhMolP, void *);
typedef void (*DhResApplyFunc) (DhResP, void *);
typedef void (*DhAtomApplyFunc) (DhAtomP, void *);
typedef void (*DhBondApplyFunc) (DhBondP, DhAtomP, DhAtomP, void *);
typedef void (*DhDistApplyFunc) (DhDistP, DhAtomP, DhAtomP, void *);
typedef void (*DhAngleApplyFunc) (DhAngleP, void *);

typedef void (*DhMolListCB) (void *);
typedef void (*DhMolChangedCB) (DhMolP, void *);
typedef void (*DhMolInvalidCB) (DhMolP, void *);
typedef void (*DhAtomInvalidCB) (DhAtomP, DhAtomP, void *);
typedef void (*DhBondInvalidCB) (DhBondP, void *);

typedef void (*DhRotFunc) (Mat4);

/* residue definitions */

extern DhResDefP DhResDefNew(DSTR);
extern DhResDefP DhResDefGet(DSTR);

/* create and destroy structures */

extern void DhDestroyAll(void);

extern DhMolP DhMolNew(void);
extern DhMolP DhMolCopy(DhMolP);
extern void DhMolMoveFirst(DhMolP);
extern void DhMolDestroy(DhMolP);

extern DhResP DhResNew(DhMolP, DhResDefP, DhSeqPos);
extern void DhResMutate(DhResP, DhResDefP, DhMutationKind);
extern void DhResSetNeigh(DhResP, DhNeighChoice, DhResP);
extern BOOL DhResDestroy(DhResP);

extern DhAtomP DhAtomNew(DhResP, DSTR);
extern DhAtomP DhAtomNewPseudo(DhAtomP[], int, DSTR);
extern void DhAtomDestroy(DhAtomP);

extern DhBondP DhBondNew(DhAtomP, DhAtomP);
extern DhBondP DhBondNewIndex(DhResP, int, int);
extern void DhBondDestroy(DhBondP);

extern BOOL DhAngleNew(DhAtomP, DhAtomP, DhAtomP, DhAtomP, DSTR);
extern void DhAngleDestroy(DhAngleP);

extern DhDistP DhDistNew(DhAtomP, DhAtomP);
extern void DhDistDestroy(DhDistP);

/* grouping */

extern void DhMakeGroup(DhMolP[], int);
extern void DhActivateGroups(BOOL);

/* callbacks for change notification */

extern void DhAddMolListCB(DhMolListCB, void *);
extern void DhAddMolChangedCB(DhMolP, DhMolChangedCB, void *);
extern void DhAddMolInvalidCB(DhMolInvalidCB, void *);
extern void DhRemoveMolListCB(DhMolListCB, void *);
extern void DhRemoveMolChangedCB(DhMolP, DhMolChangedCB, void *);
extern void DhRemoveMolInvalidCB(DhMolInvalidCB, void *);
extern void DhCallMolListCB(void *);

extern void DhAddAtomInvalidCB(DhAtomInvalidCB, void *);
extern void DhRemoveAtomInvalidCB(DhAtomInvalidCB, void *);

extern void DhAddBondInvalidCB(DhBondInvalidCB, void *);
extern void DhRemoveBondInvalidCB(DhBondInvalidCB, void *);

/* compare structures */

extern BOOL DhMolEqualStruc(DhMolP, DhMolP);

/* alternate atom positions */

extern void DhSetAltCoord(BOOL);

/* pseudo or normal bonds */

extern void DhSetPseudoMode(BOOL);

/* query data structure */

extern int DhGetMolNo(void);

extern void DhApplyMol(PropRefP, DhMolApplyFunc, void *);
extern DhMolP DhMolFindName(DSTR);
extern DhMolP DhMolFindNumber(int);
extern DhMolP DhResGetMol(DhResP);
extern DhMolP DhDistGetMol(DhDistP);

extern void DhApplyRes(PropRefP, DhResApplyFunc, void *);
extern void DhMolApplyRes(PropRefP, DhMolP, DhResApplyFunc, void *);
extern DhResP DhResFind(DhMolP, int);
extern DhResP DhResGetNeigh(DhResP, DhNeighChoice);
extern DhResP DhAtomGetRes(DhAtomP);
extern DhResP DhBondGetRes(DhBondP);
extern DhResP DhAngleGetRes(DhAngleP);

extern void DhApplyAtom(PropRefP, DhAtomApplyFunc, void *);
extern void DhMolApplyAtom(PropRefP, DhMolP, DhAtomApplyFunc, void *);
extern void DhResApplyAtom(PropRefP, DhResP, DhAtomApplyFunc, void *);
extern DhAtomP DhAtomFindName(DhResP, DSTR, BOOL);
extern DhAtomP DhAtomFindNumber(DhResP, int, BOOL);
extern DhAtomP DhBondGetAtom1(DhBondP);
extern DhAtomP DhBondGetAtom2(DhBondP);
extern DhAtomP DhAngleGetAtom1(DhAngleP);
extern DhAtomP DhAngleGetAtom2(DhAngleP);
extern DhAtomP DhAngleGetAtom3(DhAngleP);
extern DhAtomP DhAngleGetAtom4(DhAngleP);
extern DhAtomP DhDistGetAtom1(DhDistP);
extern DhAtomP DhDistGetAtom2(DhDistP);

extern void DhApplyBond(PropRefP, DhBondApplyFunc, void *);
extern void DhMolApplyBond(PropRefP, DhMolP, DhBondApplyFunc, void *);
extern void DhResApplyBond(PropRefP, DhResP, DhBondApplyFunc, void *);
extern void DhAtomApplyBond(PropRefP, DhAtomP, DhBondApplyFunc, void *);
extern DhBondP DhBondFind(DhResP, DSTR, DSTR);
extern DhBondP DhAngleGetBond(DhAngleP);

extern void DhApplyAngle(PropRefP, DhAngleApplyFunc, void *);
extern void DhMolApplyAngle(PropRefP, DhMolP, DhAngleApplyFunc, void *);
extern void DhResApplyAngle(PropRefP, DhResP, DhAngleApplyFunc, void *);
extern DhAngleP DhAngleFind(DhResP, DSTR);
extern DhAngleP DhBondGetAngle(DhBondP);

extern void DhApplyDist(PropRefP, DhDistApplyFunc, void *);
extern void DhApplyInterDist(PropRefP, DhDistApplyFunc, void *);
extern void DhMolApplyDist(PropRefP, DhMolP, DhDistApplyFunc, void *);

/* initialization of new data items (set standard properties, etc.) */

extern void DhAddMolInitCB(DhMolApplyFunc, void *);
extern void DhAddResInitCB(DhResApplyFunc, void *);
extern void DhAddAtomInitCB(DhAtomApplyFunc, void *);
extern void DhAddBondInitCB(DhBondApplyFunc, void *);
extern void DhAddDistInitCB(DhDistApplyFunc, void *);

extern void DhMolInit(DhMolP);
extern void DhResInit(DhResP);
extern void DhAtomInit(DhAtomP);
extern void DhBondInit(DhBondP);
extern void DhDistInit(DhDistP);

/* properties */

extern void DhMolSetProp(PropRefP, DhMolP, BOOL);
extern void DhResSetProp(PropRefP, DhResP, BOOL);
extern void DhAtomSetProp(PropRefP, DhAtomP, BOOL);
extern void DhBondSetProp(PropRefP, DhBondP, BOOL);
extern void DhAngleSetProp(PropRefP, DhAngleP, BOOL);
extern void DhDistSetProp(PropRefP, DhDistP, BOOL);

extern BOOL DhMolGetProp(PropRefP, DhMolP);
extern BOOL DhResGetProp(PropRefP, DhResP);
extern BOOL DhAtomGetProp(PropRefP, DhAtomP);
extern BOOL DhBondGetProp(PropRefP, DhBondP);
extern BOOL DhAngleGetProp(PropRefP, DhAngleP);
extern BOOL DhDistGetProp(PropRefP, DhDistP);

extern unsigned *DhMolGetPropTab(DhMolP);
extern unsigned *DhResGetPropTab(DhResP);
extern unsigned *DhAtomGetPropTab(DhAtomP);
extern unsigned *DhBondGetPropTab(DhBondP);
extern unsigned *DhAngleGetPropTab(DhAngleP);
extern unsigned *DhDistGetPropTab(DhDistP);

/* set attributes */

extern void DhMolSetName(DhMolP, DSTR);
extern void DhMolSetRotMat(DhMolP, Mat4);
extern void DhMolSetRotPoint(DhMolP, Vec3);
extern void DhMolSetTransVect(DhMolP, Vec3);
extern void DhMolSetAttr(DhMolP, MolAttrP);

extern void DhResSetNumber(DhResP, int);

extern void DhAtomSetCoord(DhAtomP, Vec3);
extern void DhAtomSetAltCoord(DhAtomP, Vec3);
extern void DhAtomSetShiftIndex(DhAtomP, int);
extern void DhAtomSetShift(DhAtomP, float);
extern void DhAtomSetBFactor(DhAtomP, float);
extern void DhAtomSetCharge(DhAtomP, float);
extern void DhAtomSetAttr(DhAtomP, AttrP);

extern void DhBondSetAttr(DhBondP, AttrP);

extern void DhAngleSetVal(DhAngleP, float);
extern void DhAngleSetMinVal(DhAngleP, float);
extern void DhAngleSetMaxVal(DhAngleP, float);

extern void DhDistSetAttr(DhDistP, AttrP);
extern void DhDistSetKind(DhDistP, DhDistKind);
extern void DhDistSetLimit(DhDistP, float);

/* get attributes */

extern DSTR DhMolGetName(DhMolP);
extern int DhMolGetNumber(DhMolP);
extern void DhMolGetRotMat(DhMolP, Mat4);
extern void DhMolGetRotPoint(DhMolP, Vec3);
extern void DhMolGetTransVect(DhMolP, Vec3);
extern MolAttrP DhMolGetAttr(DhMolP);

extern DSTR DhResGetName(DhResP);
extern int DhResGetNumber(DhResP);

extern DSTR DhAtomGetName(DhAtomP);
extern int DhAtomGetNumber(DhAtomP);
extern void DhAtomGetCoord(DhAtomP, Vec3);
extern void DhAtomGetCoordTransf(DhAtomP, Vec3);
extern int DhAtomGetShiftIndex(DhAtomP);
extern float DhAtomGetShift(DhAtomP);
extern float DhAtomGetBFactor(DhAtomP);
extern float DhAtomGetVdw(DhAtomP);
extern float DhAtomGetCharge(DhAtomP, DhChargeKind);
extern int DhAtomGetHBondCap(DhAtomP);
extern AttrP DhAtomGetAttr(DhAtomP);

extern AttrP DhBondGetAttr(DhBondP);

extern BOOL DhAngleIsBackbone(DhAngleP);
extern DSTR DhAngleGetName(DhAngleP);
extern float DhAngleGetVal(DhAngleP);
extern float DhAngleGetMinVal(DhAngleP);
extern float DhAngleGetMaxVal(DhAngleP);

extern AttrP DhDistGetAttr(DhDistP);
extern DhDistKind DhDistGetKind(DhDistP);
extern float DhDistGetLimit(DhDistP);
extern float DhDistGetVal(DhDistP);
extern float DhDistGetViol(DhDistP);

/* rotation of dihedral angles */

extern void DhRotSetFunc(DhRotFunc);
extern void DhRotAddAngle(DhAngleP, BOOL);
extern void DhRotSetAngle(DhAngleP, float);
extern void DhRotEnd(BOOL);
extern BOOL DhRotMode(void);

/* calculations */

extern void DhTransfVec(Vec3, DhMolP);
extern void DhCalcCenter(Vec3, DhMolP);
extern void DhMolCalcMeanStruc(DhMolP, DhMolP[], int, BFactorCalc, BOOL);
extern void DhMolAnglesChanged(DhMolP);
extern void DhMolCoordsChanged(DhMolP);
extern void DhResStandGeom(DhResP);
extern void DhResAnglesChanged(DhResP);
extern void DhResDockPrev(DhResP);
extern void DhResDockNext(DhResP);
extern void DhResCalcAtom(DhResP, DSTR);

/* dump/undump */

extern BOOL DhDump(GFile);
extern BOOL DhUndump(GFile, int);

extern BOOL DhResDefWrite(GFile, DhResP);

#endif  /* _DATA_HAND_H_ */
