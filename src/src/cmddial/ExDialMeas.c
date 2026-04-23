/*
************************************************************************
*
*   ExDialMeas.c - DialMeasure command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmddial/SCCS/s.ExDialMeas.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_dial.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <values.h>

#include <mat_vec.h>
#include <pu.h>
#include <arg.h>
#include <cip.h>
#include <data_hand.h>
#include <data_sel.h>

#define BOND_NO 2
#define ATOM_NO 4
#define VALUE_NO 6

static BOOL DialOn = FALSE;
static PuGizmo DialGizmo = NULL, AtomGizmo, BondGizmo;
static PuGizmo ItemGizmoA[VALUE_NO], ValGizmoA[VALUE_NO];

static DhBondP BondPA[BOND_NO];
static DhAtomP AtomPA[ATOM_NO];
static int BondNo = 0, AtomNo = 0;

static char *LabelList[] = {
  "Bond Length",
  "Bond Angle",
  "Bond Dih.",
  "Atom Dist.",
  "Atom Angle",
  "Atom Dih."
};

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSwitchGizmo(DialGizmo, FALSE);
  DialOn = FALSE;
}

static void
showDist(int fieldI, DhAtomP atom1P, DhAtomP atom2P)
{
  DSTR str;
  char buf[20];
  Vec3 c1, c2;
  float d;

  str = DStrNew();

  (void) sprintf(buf, "%d ", DhResGetNumber(DhAtomGetRes(atom1P)));
  DStrAssignStr(str, buf);
  DStrAppDStr(str, DhAtomGetName(atom1P));
  DStrAppStr(str, "-");
  DStrAppDStr(str, DhAtomGetName(atom2P));
  PuSetStr(ItemGizmoA[fieldI], PU_SC_TEXT, DStrToStr(str));

  DhAtomGetCoordTransf(atom1P, c1);
  DhAtomGetCoordTransf(atom2P, c2);
  d = Vec3DiffAbs(c1, c2);
  (void) sprintf(buf, "%7.3f", d);
  PuSetStr(ValGizmoA[fieldI], PU_SC_TEXT, buf);

  DStrFree(str);
}

static void
showAngle(int fieldI, DhAtomP atom1P, DhAtomP atom2P, DhAtomP atom3P)
{
  DSTR str;
  char buf[20];
  Vec3 c2, v1, v2;
  float c, s, ang;

  str = DStrNew();

  (void) sprintf(buf, "%d ", DhResGetNumber(DhAtomGetRes(atom2P)));
  DStrAssignStr(str, buf);
  DStrAppDStr(str, DhAtomGetName(atom1P));
  DStrAppStr(str, "-");
  DStrAppDStr(str, DhAtomGetName(atom2P));
  DStrAppStr(str, "-");
  DStrAppDStr(str, DhAtomGetName(atom3P));
  PuSetStr(ItemGizmoA[fieldI], PU_SC_TEXT, DStrToStr(str));

  DhAtomGetCoordTransf(atom1P, v1);
  DhAtomGetCoordTransf(atom2P, c2);
  DhAtomGetCoordTransf(atom3P, v2);
  Vec3Sub(v1, c2);
  Vec3Sub(v2, c2);
  Vec3Norm(v1);
  Vec3Norm(v2);
  c = Vec3Scalar(v1, v2);
  Vec3Cross(v1, v2);
  s = Vec3Abs(v1);
  ang = atan2f(s, c) * 180.0f / (float) M_PI;
  (void) sprintf(buf, "%6.1f", ang);
  PuSetStr(ValGizmoA[fieldI], PU_SC_TEXT, buf);

  DStrFree(str);
}


static void
showDih(int fieldI,
    DhAtomP atom1P, DhAtomP atom2P, DhAtomP atom3P, DhAtomP atom4P)
{
  DSTR str;
  char buf[20];
  Vec3 c1, c2, c3, c4, vx, vy, vz, v34;
  Mat3 btM;
  float c, s, a, ang;
  int i;

  str = DStrNew();

  (void) sprintf(buf, "%d ", DhResGetNumber(DhAtomGetRes(atom2P)));
  DStrAssignStr(str, buf);
  DStrAppDStr(str, DhAtomGetName(atom1P));
  DStrAppStr(str, "-");
  DStrAppDStr(str, DhAtomGetName(atom2P));
  DStrAppStr(str, "-");
  DStrAppDStr(str, DhAtomGetName(atom3P));
  DStrAppStr(str, "-");
  DStrAppDStr(str, DhAtomGetName(atom4P));
  PuSetStr(ItemGizmoA[fieldI], PU_SC_TEXT, DStrToStr(str));

  DhAtomGetCoordTransf(atom1P, c1);
  DhAtomGetCoordTransf(atom2P, c2);
  DhAtomGetCoordTransf(atom3P, c3);
  DhAtomGetCoordTransf(atom4P, c4);

  /* calculate new basis vx, vy, vz */
  Vec3Copy(vz, c2);
  Vec3Sub(vz, c3);
  Vec3Norm(vz);

  Vec3Copy(vx, c1);
  Vec3Sub(vx, c2);
  Vec3ScaleSub(vx, Vec3Scalar(vz, vx), vz);
  Vec3Norm(vx);

  Vec3Copy(vy, vz);
  Vec3Cross(vy, vx);

  for (i = 0; i < 3; i++) {
    btM[i][0] = vx[i];
    btM[i][1] = vy[i];
    btM[i][2] = vz[i];
  }

  Vec3Copy(v34, c4);
  Vec3Sub(v34, c3);

  Mat3VecMult(v34, btM);

  c = v34[0];
  s = v34[1];
  a = sqrtf(c * c + s * s);
  c /= a;
  s /= a;
  ang = - atan2f(s, c) * 180.0f / (float) M_PI;
  (void) sprintf(buf, "%6.1f", ang);
  PuSetStr(ValGizmoA[fieldI], PU_SC_TEXT, buf);

  DStrFree(str);
}

static void
showEmpty(int fieldI)
{
  PuSetStr(ItemGizmoA[fieldI], PU_SC_TEXT, "");
  PuSetStr(ValGizmoA[fieldI], PU_SC_TEXT, "");
}

static void
atomsChanged(void)
{
  DSTR str;
  DhAtomP atom1P, atom2P, atom3P, atom4P;
  int i;

  str = DStrNew();
  
  for (i = 0; i < AtomNo; i++) {
    if (i > 0)
      DStrAppStr(str, ", ");
    DStrAppDStr(str, DhAtomGetName(AtomPA[i]));
  }
  PuSetStr(AtomGizmo, PU_SC_TEXT, DStrToStr(str));

  if (AtomNo >= 2) {
    atom1P = AtomPA[AtomNo - 2];
    atom2P = AtomPA[AtomNo - 1];
    showDist(3, atom1P, atom2P);
  } else {
    showEmpty(3);
  }

  if (AtomNo >= 3) {
    atom1P = AtomPA[AtomNo - 3];
    atom2P = AtomPA[AtomNo - 2];
    atom3P = AtomPA[AtomNo - 1];
    showAngle(4, atom1P, atom2P, atom3P);
  } else {
    showEmpty(4);
  }

  if (AtomNo >= 4) {
    atom1P = AtomPA[AtomNo - 4];
    atom2P = AtomPA[AtomNo - 3];
    atom3P = AtomPA[AtomNo - 2];
    atom4P = AtomPA[AtomNo - 1];
    showDih(5, atom1P, atom2P, atom3P, atom4P);
  } else {
    showEmpty(5);
  }

  DStrFree(str);
}

static void
bondsChanged(void)
{
  DSTR str;
  char buf[20];
  DhBondP bond1P, bond2P;
  DhAtomP atom11P, atom12P, atom21P, atom22P;
  DhAtomP atom1P, atom2P, atom3P;
  DhAngleP angleP;
  int i;

  str = DStrNew();
  
  for (i = 0; i < BondNo; i++) {
    if (i > 0)
      DStrAppStr(str, ", ");
    DStrAppDStr(str, DhAtomGetName(DhBondGetAtom1(BondPA[i])));
    DStrAppStr(str, "-");
    DStrAppDStr(str, DhAtomGetName(DhBondGetAtom2(BondPA[i])));
  }
  PuSetStr(BondGizmo, PU_SC_TEXT, DStrToStr(str));

  if (BondNo >= 1) {
    bond1P = BondPA[BondNo - 1];
    atom1P = DhBondGetAtom1(bond1P);
    atom2P = DhBondGetAtom2(bond1P);
    showDist(0, atom1P, atom2P);
  } else {
    showEmpty(0);
  }

  if (BondNo >= 2) {
    bond1P = BondPA[BondNo - 2];
    atom11P = DhBondGetAtom1(bond1P);
    atom12P = DhBondGetAtom2(bond1P);
    bond2P = BondPA[BondNo - 1];
    atom21P = DhBondGetAtom1(bond2P);
    atom22P = DhBondGetAtom2(bond2P);

    if (atom11P == atom21P) {
      atom1P = atom12P;
      atom2P = atom11P;
      atom3P = atom22P;
    } else if (atom11P == atom22P) {
      atom1P = atom12P;
      atom2P = atom11P;
      atom3P = atom21P;
    } else if (atom12P == atom21P) {
      atom1P = atom11P;
      atom2P = atom12P;
      atom3P = atom22P;
    } else if (atom12P == atom22P) {
      atom1P = atom11P;
      atom2P = atom12P;
      atom3P = atom21P;
    } else {
      atom1P = NULL;
    }

    if (atom1P == NULL) {
      showEmpty(1);
    } else {
      showAngle(1, atom1P, atom2P, atom3P);
    }
  } else {
    showEmpty(1);
  }

  if (BondNo >= 1) {
    bond1P = BondPA[BondNo - 1];
    angleP = DhBondGetAngle(bond1P);
    if (angleP == NULL) {
      showEmpty(2);
    } else {
      (void) sprintf(buf, "%d ", DhResGetNumber(DhAngleGetRes(angleP)));
      DStrAssignStr(str, buf);
      DStrAppDStr(str, DhAngleGetName(angleP));
      PuSetStr(ItemGizmoA[2], PU_SC_TEXT, DStrToStr(str));
      (void) sprintf(buf, "%6.1f", DhAngleGetVal(angleP));
      PuSetStr(ValGizmoA[2], PU_SC_TEXT, buf);
    }
  } else {
    showEmpty(2);
  }

  DStrFree(str);
}

static void
selCB(DataEntityType entType, void *entP, SelKind kind)
{
  int i;

  if (kind == SK_REMOVE || ! DialOn)
    return;

  switch(entType) {
    case DE_ATOM:
      if (AtomNo > 0 && entP == AtomPA[AtomNo - 1])
	break;

      if (AtomNo == ATOM_NO)
	for (i = 1; i < ATOM_NO; i++)
	  AtomPA[i - 1] = AtomPA[i];
      else
	AtomNo++;
      AtomPA[AtomNo - 1] = entP;

      atomsChanged();

      break;
    case DE_BOND:
      if (BondNo > 0 && entP == BondPA[BondNo - 1])
	break;

      if (BondNo == BOND_NO)
	for (i = 1; i < BOND_NO; i++)
	  BondPA[i - 1] = BondPA[i];
      else
	BondNo++;
      BondPA[BondNo - 1] = entP;

      bondsChanged();

      break;
  }
}

static void
helpCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  CipShowHelpFile(DialGizmo, "DialMeasure");
}

static void
buildDial(void)
{
  PuGizmo g;
  PuConstraints con;
  int i;

  DialGizmo = PuCreateDialog("Measure Dialog", 6, 1 + VALUE_NO);
  PuSetBool(DialGizmo, PU_BC_PLACE_OUTSIDE, TRUE);

  con.y = 0;
  con.h = 1;

  g = PuCreateLabel(DialGizmo, "Bonds");
  con.x = 0;
  con.w = 1;
  PuSetConstraints(g, con);
  PuSwitchGizmo(g, TRUE);

  BondGizmo = PuCreateTextField(DialGizmo, "BondList", "");
  con.x = 1;
  con.w = 2;
  PuSetConstraints(BondGizmo, con);
  PuSwitchGizmo(BondGizmo, TRUE);

  g = PuCreateLabel(DialGizmo, "Atoms");
  con.x = 3;
  con.w = 1;
  PuSetConstraints(g, con);
  PuSwitchGizmo(g, TRUE);

  AtomGizmo = PuCreateTextField(DialGizmo, "AtomList", "");
  con.x = 4;
  con.w = 2;
  PuSetConstraints(AtomGizmo, con);
  PuSwitchGizmo(AtomGizmo, TRUE);

  for (i = 0; i < VALUE_NO; i++) {
    con.y = 1 + i;
    con.w = 2;

    con.x = 0;
    g = PuCreateLabel(DialGizmo, LabelList[i]);
    PuSetConstraints(g, con);
    PuSwitchGizmo(g, TRUE);

    con.x = 2;
    ItemGizmoA[i] = PuCreateTextField(DialGizmo, "Item", "");
    PuSetConstraints(ItemGizmoA[i], con);
    PuSwitchGizmo(ItemGizmoA[i], TRUE);

    con.x = 4;
    ValGizmoA[i] = PuCreateTextField(DialGizmo, "Item", "");
    PuSetConstraints(ValGizmoA[i], con);
    PuSwitchGizmo(ValGizmoA[i], TRUE);
  }

  PuAddGizmoCB(DialGizmo, PU_CT_CLOSE, popdownCB, NULL, NULL);
  PuAddGizmoCB(DialGizmo, PU_CT_HELP, helpCB, NULL, NULL);
}

static void
atomInvalidCB(DhAtomP firstAtomP, DhAtomP lastAtomP, void *clientData)
{
  BOOL changed;
  int i, k;

  changed = FALSE;
  for (i = 0; i < AtomNo; i++)
    if (AtomPA[i] >= firstAtomP && AtomPA[i] <= lastAtomP) {
      for (k = i + 1; k < AtomNo; k++)
	AtomPA[k - 1] = AtomPA[k];
      AtomNo--;
      i--;
      changed = TRUE;
    }

  if (changed)
    atomsChanged();
}

static void
bondInvalidCB(DhBondP bondP, void *clientData)
{
  BOOL changed;
  int i, k;

  changed = FALSE;
  for (i = 0; i < BondNo; i++)
    if (BondPA[i] == bondP) {
      for (k = i + 1; k < BondNo; k++)
	BondPA[k - 1] = BondPA[k];
      BondNo--;
      i--;
      changed = TRUE;
    }

  if (changed)
    bondsChanged();
}

ErrCode
ExDialMeasure(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  arg.prompt = "Measure Dialog";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.n = 2;

  enumEntry[0].str = "off";
  enumEntry[1].str = "on";

  enumEntry[0].onOff = DialOn;
  enumEntry[1].onOff = ! DialOn;
  if (DialOn)
    arg.v.intVal = 0;
  else
    arg.v.intVal = 1;

  errCode = ArgGet(&arg, 1);
  if (errCode != EC_OK) {
    ArgCleanup(&arg, 1);
    return errCode;
  }

  DialOn = (arg.v.intVal == 1);

  ArgCleanup(&arg, 1);

  if (DialOn) {
    if (DialGizmo == NULL) {
      buildDial();
      SelAddCB(selCB);
      DhAddAtomInvalidCB(atomInvalidCB, NULL);
      DhAddBondInvalidCB(bondInvalidCB, NULL);
    }
    PuSwitchGizmo(DialGizmo, TRUE);
  } else {
    if (DialGizmo != NULL)
      PuSwitchGizmo(DialGizmo, FALSE);
  }

  return EC_OK;
}
