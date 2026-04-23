/*
************************************************************************
*
*   ExListSelected.c - ListSelected* commands
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
*   Date of last modification : 99/11/06
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdprop/SCCS/s.ExListSelected.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_prop.h>

#include <stdio.h>
#include <string.h>

#include <break.h>
#include <pu.h>
#include <data_hand.h>

static PuTextWindow TextW;
static DhMolP LastMolP;

static void
writeInt(int num)
{
  char buf[10];

  (void) sprintf(buf, "%5d ", num);
  PuWriteStr(TextW, buf);
}

static void
writeFloat(float val)
{
  char buf[20];

  (void) sprintf(buf, "%#8.3f ", val);
  PuWriteStr(TextW, buf);
}

static void
writeName(DSTR name)
{
  char buf[100];

  (void) sprintf(buf, "%-5s ", DStrToStr(name));
  PuWriteStr(TextW, buf);
}

static void
listMol(DhMolP molP, void *clientData)
{
  writeInt(DhMolGetNumber(molP) + 1);
  PuWriteStr(TextW, DStrToStr(DhMolGetName(molP)));
  PuWriteStr(TextW, "\n");
}

static void
writeMolSep(DhMolP molP)
{
  if (molP == LastMolP)
    return;

  if (BreakCheck(1))
    return;

  if (LastMolP != NULL)
    PuWriteStr(TextW, "\n");
  listMol(molP, NULL);
  PuWriteStr(TextW, "--------------------------------------------------\n");
  LastMolP = molP;
}

static void
listRes(DhResP resP, void *clientData)
{
  writeMolSep(DhResGetMol(resP));
  if (BreakInterrupted())
    return;

  writeInt(DhResGetNumber(resP));
  writeName(DhResGetName(resP));
  PuWriteStr(TextW, "\n");
}

static void
listAtom(DhAtomP atomP, void *clientData)
{
  DhResP resP;
  Vec3 coord;

  resP = DhAtomGetRes(atomP);
  writeMolSep(DhResGetMol(resP));
  if (BreakInterrupted())
    return;

  writeInt(DhResGetNumber(resP));
  writeName(DhResGetName(resP));
  writeName(DhAtomGetName(atomP));

  DhAtomGetCoordTransf(atomP, coord);
  writeFloat(coord[0]);
  writeFloat(coord[1]);
  writeFloat(coord[2]);

  PuWriteStr(TextW, "\n");
}

static void
listBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DhResP resP;
  Vec3 coord1, coord2;

  resP = DhBondGetRes(bondP);
  writeMolSep(DhResGetMol(resP));
  if (BreakInterrupted())
    return;

  writeInt(DhResGetNumber(resP));
  writeName(DhResGetName(resP));
  writeName(DhAtomGetName(atom1P));
  PuWriteStr(TextW, "- ");
  writeName(DhAtomGetName(atom2P));

  DhAtomGetCoordTransf(atom1P, coord1);
  DhAtomGetCoordTransf(atom2P, coord2);
  writeFloat(Vec3DiffAbs(coord1, coord2));

  PuWriteStr(TextW, "\n");
}

static void
listAngle(DhAngleP angleP, void *clientData)
{
  DhResP resP;

  resP = DhAngleGetRes(angleP);
  writeMolSep(DhResGetMol(resP));
  if (BreakInterrupted())
    return;

  writeInt(DhResGetNumber(resP));
  writeName(DhResGetName(resP));
  writeName(DhAngleGetName(angleP));
  PuWriteStr(TextW, " ");
  writeFloat(DhAngleGetVal(angleP));

  PuWriteStr(TextW, "\n");
}

static void
listDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DhResP res1P, res2P;

  res1P = DhAtomGetRes(atom1P);
  res2P = DhAtomGetRes(atom2P);
  writeMolSep(DhResGetMol(res1P));
  if (BreakInterrupted())
    return;

  writeInt(DhResGetNumber(res1P));
  writeName(DhResGetName(res1P));
  writeName(DhAtomGetName(atom1P));
  PuWriteStr(TextW, "- ");
  writeInt(DhResGetNumber(res2P));
  writeName(DhResGetName(res2P));
  writeName(DhAtomGetName(atom2P));

  writeFloat(DhDistGetVal(distP));

  PuWriteStr(TextW, "\n");
}

ErrCode
ExListSelected(char *cmd)
{
  PropRefP refP;

  TextW = PuCreateTextWindow(cmd);
  LastMolP = NULL;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  BreakActivate(TRUE);

  if (strcmp(cmd, "ListSelectedMol") == 0)
    DhApplyMol(refP, listMol, NULL);
  else if (strcmp(cmd, "ListSelectedRes") == 0)
    DhApplyRes(refP, listRes, NULL);
  else if (strcmp(cmd, "ListSelectedAtom") == 0)
    DhApplyAtom(refP, listAtom, NULL);
  else if (strcmp(cmd, "ListSelectedBond") == 0)
    DhApplyBond(refP, listBond, NULL);
  else if (strcmp(cmd, "ListSelectedAngle") == 0)
    DhApplyAngle(refP, listAngle, NULL);
  else
    DhApplyDist(refP, listDist, NULL);

  BreakActivate(FALSE);

  return EC_OK;
}
