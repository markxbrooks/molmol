/*
************************************************************************
*
*   data_struc.h - data structures
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/02/19
*   Pathname of SCCS file     : /local/home/kor/molmol/src/data/SCCS/s.data_struc.h
*   SCCS identification       : 1.19
*
************************************************************************
*/

#include <mat_vec.h>
#include <linlist.h>
#include <data_hand.h>

#define EQUIV_NO 3

typedef struct DhBondAddS *DhBondAddP;
typedef struct DhAngleAddS *DhAngleAddP;
typedef struct DhAtomDefS *DhAtomDefP;
typedef struct DhAtomTypeS *DhAtomTypeP;
typedef struct DhBondDefS *DhBondDefP;
typedef struct DhAngleDefS *DhAngleDefP;

typedef struct {
  DhMolP prevP, nextP, headP;
  int num;
  Mat4 rotMat;
  Vec3 transVect;
  int resNumOffs;
  BOOL mapMod;
} GroupData;

struct DhMolS {
  DSTR name;
  int num;
  GroupData *groupP;
  LINLIST resL;  /* list of struct DhResS */
  LINLIST bondL;  /* list of struct DhBondAddS */
  LINLIST distL;  /* list of struct DhDistS */
  Mat4 rotMat;
  Vec3 rotPoint;
  Vec3 transVect;
  unsigned *propTab;
  MolAttrP attrP;
};

struct DhResS {
  DhMolP molP;
  DhResDefP defP;
  int num;
  DhResP neighLeftP, neighRightP;
  DhAtomP atomA;
  int equivI[EQUIV_NO];
  DhBondP bondA;
  DhAngleP angleA;
  unsigned *propTab;
};

typedef enum {
  AS_UNKNOWN_COORD,
  AS_VALID,
  AS_DELETED
} AtomState;

struct DhAtomS {
  DhResP resP;
  Vec3 coord;
  AtomState state;
#ifdef NMR
  int shiftI;
  float shift;
#endif
  float bFactor;
  unsigned *propTab;
  AttrP attrP;
};

struct DhBondS {
  DhResP resP;
  unsigned *propTab;
  AttrP attrP;
};

struct DhBondAddS {
  struct DhBondS bondS;
  DhResP res1P;
  DhResP res2P;
  int atom1I;
  int atom2I;
};

struct DhAngleS {
  DhResP resP;
  float val;
  BOOL changed;
  float minVal, maxVal;
  unsigned *propTab;
};

struct DhAngleAddS {
  DSTR name;
  DhResP res1P;
  DhResP res2P;
  DhResP res3P;
  DhResP res4P;
  int atom1I;
  int atom2I;
  int atom3I;
  int atom4I;
};

struct DhDistS {
  DhDistKind kind;
  DhResP res1P;
  DhResP res2P;
  int atom1I;
  int atom2I;
  float limit;
  float val;
  unsigned *propTab;
  AttrP attrP;
};

struct DhResDefS {
  DSTR name;
  int num;
  int atomNo;
  DhAtomDefP atomA;
  int firstAtomI;
  int lastAtomI;
  int bondNo;
  DhBondDefP bondA;
  int firstBondI;
  int lastBondI;
  int angleNo;
  DhAngleDefP angleA;
  int refCount;
  BOOL isFromLib;
  BOOL isCopy;
};

struct DhAtomDefS {
  DSTR name;
  Vec3 coord;
  float charge;
  float heavyCharge;
  float avgCharge;
  float simpleCharge;
  int pseudoI;
  DhAtomTypeP typeP;
};

typedef enum {
  AK_HEAVY,
  AK_HYDROGEN,
  AK_PSEUDO,
  AK_OTHER
} AtomKind;

struct DhAtomTypeS {
  DSTR name;
  int num;
  AtomKind kind;
  float vdw;
  int hBondCap;
};

struct DhBondDefS {
  int atom1I;
  int atom2I;
  int pseudoI;
};

struct DhAngleDefS {
  DSTR name;
  int atom1I;
  int atom2I;
  int atom3I;
  int atom4I;
  int lastAtomI;
};

typedef struct DhAltCoordS {
  DhAtomP atomP;
  Vec3 coord;
} *DhAltCoordP;

extern LINLIST DhMolListGet(void);

extern LINLIST DhBondListGet(void);

extern LINLIST DhAngleListGet(void);

extern LINLIST DhDistListGet(void);

extern LINLIST DhAltCoordListGet(void);
