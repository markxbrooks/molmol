/*
************************************************************************
*
*   ExSurface.c - AddSurface command
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExSurface.c
*   SCCS identification       : 1.25
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <mat_vec.h>
#include <surface.h>
#include <pu.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <graph_draw.h>

static float SolventRad = 1.4f;
static int SurfKind = 2;
static BOOL SurfDots = FALSE;
static BOOL DoReport = FALSE;

static SphereDescr *SphereA;
static int SphereNo;

static PuTextWindow TextW;

static void
countAtom(DhAtomP atomP, void *clientData)
{
  (* (int *) clientData)++;
}

static void
fillData(DhAtomP atomP, void *clientData)
{
  ExprP exprP = clientData;
  ExprRes exprRes;

  SphereA[SphereNo].userData = atomP;
  DhAtomGetCoord(atomP, SphereA[SphereNo].cent);

  ExprEval(atomP, exprP, &exprRes);
  if (exprRes.resType == ER_INT)
    SphereA[SphereNo].rad = (float) exprRes.u.intVal;
  else
    SphereA[SphereNo].rad = exprRes.u.floatVal;

  SphereNo++;
}

static void
sumSurfVol(float *surfP, float *volP, Vec3 p1, Vec3 p2, Vec3 p3)
{
  Vec3 v2, v3, vc;

  Vec3Copy(v2, p2);
  Vec3Sub(v2, p1);
  Vec3Copy(v3, p3);
  Vec3Sub(v3, p1);

  Vec3Copy(vc, v2);
  Vec3Cross(vc, v3);

  *surfP += Vec3Abs(vc);
  *volP += Vec3Scalar(vc, p1);
}

static void
addSurface(DhMolP molP, void *clientData)
{
  PropRefP refP;
  int atomNo;
  PatchDescr *patchA;
  int patchNo;
  IsoSurface *surfA;
  int surfNo, surfI;
  PrimObjP primP;
  char buf[20];
  float surf, vol;
  int i, k;

  if (BreakInterrupted())
    return;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  atomNo = 0;
  DhMolApplyAtom(refP, molP, countAtom, &atomNo);
  if (atomNo == 0)
    return;

  SphereA = malloc(atomNo * sizeof(*SphereA));
  SphereNo = 0;
  DhMolApplyAtom(refP, molP, fillData, clientData);

  if (SurfKind == 1)
    for (i = 0; i < SphereNo; i++)
      SphereA[i].rad += SolventRad;

  if (SurfDots) {
    SurfaceDots(SphereA, SphereNo, SurfKind == 2 ? SolventRad : 0.0f,
	ParGetIntVal(PN_DRAW_PREC), FALSE,
	&patchA, &patchNo);
    primP = PrimNew(PT_DOT_SURFACE, molP);
    for (i = 0; i < patchNo; i++)
      if (patchA[i].pointNo > 0)
	PrimDotSurfaceAddPatch(primP,
	    patchA[i].userData, patchA[i].pointA, patchA[i].pointNo);
    free(patchA);
    PrimSetProp(PropGetRef("surface", TRUE), primP, TRUE);
    PrimSetProp(PropGetRef("dotsurface", TRUE), primP, TRUE);
  } else {
    SurfaceSolid(SphereA, SphereNo, SurfKind == 2 ? SolventRad : 0.0f,
        ParGetIntVal(PN_DRAW_PREC),
	&surfA, &surfNo);
    for (surfI = 0; surfI < surfNo; surfI++) {
      primP = PrimNew(PT_SURFACE, molP);

      if (DoReport) {
        (void) sprintf(buf, "%3d", PrimGetNumber(primP));
        PuWriteStr(TextW, buf);
        PuWriteStr(TextW, ": ");
       	surf = 0.0f;
	vol = 0.0f;
      }

      PrimSurfaceSetPoints(primP,
	      surfA[surfI].xA, surfA[surfI].nvA, surfA[surfI].pointNo);

      for (i = 0; i < surfA[surfI].meshNo; i++) {
        PrimSurfaceAddMesh(primP,
            surfA[surfI].meshA[i].indA, surfA[surfI].meshA[i].pointNo);

	for (k = 0; k < surfA[surfI].meshA[i].pointNo - 2; k += 2) {
	  sumSurfVol(&surf, &vol,
	      surfA[surfI].xA[surfA[surfI].meshA[i].indA[k]],
	      surfA[surfI].xA[surfA[surfI].meshA[i].indA[k + 1]],
	      surfA[surfI].xA[surfA[surfI].meshA[i].indA[k + 2]]);

	  if (k + 3 == surfA[surfI].meshA[i].pointNo)
 	    break;

	  sumSurfVol(&surf, &vol,
	      surfA[surfI].xA[surfA[surfI].meshA[i].indA[k + 2]],
	      surfA[surfI].xA[surfA[surfI].meshA[i].indA[k + 1]],
	      surfA[surfI].xA[surfA[surfI].meshA[i].indA[k + 3]]);
	}
      }

      surf *= 0.5f;
      vol /= 6.0f;

      if (DoReport) {
        (void) sprintf(buf, "%8.1f", surf);
        PuWriteStr(TextW, buf);
        PuWriteStr(TextW, ", ");
        (void) sprintf(buf, "%8.1f", vol);
        PuWriteStr(TextW, buf);
        PuWriteStr(TextW, "\n");
      }

      free(surfA[surfI].meshA);

      PrimSetProp(PropGetRef("surface", TRUE), primP, TRUE);
    }
    free(surfA);
  }

  free(SphereA);
}

#define ARG_NUM 5
#define OPT_NUM 1

ErrCode
ExAddSurface(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enum1Entry[3];
  EnumEntryDescr enum2Entry[3];
  EnumEntryDescr optEntry[OPT_NUM];
  ErrCode errCode;

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_ENUM;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_ENUM;
  arg[4].type = AT_MULT_ENUM;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Atom Radius";
  arg[0].entType = DE_ATOM;
  DStrAssignStr(arg[0].v.strVal, "vdw");

  enum1Entry[0].str = "vdw";
  enum1Entry[0].onOff = FALSE;
  enum1Entry[1].str = "solvent";
  enum1Entry[1].onOff = FALSE;
  enum1Entry[2].str = "contact";
  enum1Entry[2].onOff = FALSE;

  enum1Entry[SurfKind].onOff = TRUE;

  arg[1].prompt = "Surface Kind";
  arg[1].u.enumD.entryP = enum1Entry;
  arg[1].u.enumD.n = 3;
  arg[1].v.intVal = SurfKind;

  arg[2].prompt = "Solvent Radius";
  arg[2].v.doubleVal = SolventRad;

  enum2Entry[0].str = "dots";
  enum2Entry[0].onOff = SurfDots;
  enum2Entry[1].str = "shaded";
  enum2Entry[1].onOff = ! SurfDots;

  arg[3].prompt = "Surface Style";
  arg[3].u.enumD.entryP = enum2Entry;
  arg[3].u.enumD.n = 2;
  if (SurfDots)
    arg[3].v.intVal = 0;
  else
    arg[3].v.intVal = 1;

  optEntry[0].str = "report";
  optEntry[0].onOff = DoReport;

  arg[4].prompt = "Options";
  arg[4].u.enumD.entryP = optEntry;
  arg[4].u.enumD.n = OPT_NUM;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  SurfKind = arg[1].v.intVal;
  SolventRad = (float) arg[2].v.doubleVal;
  SurfDots = (arg[3].v.intVal == 0);
  DoReport = optEntry[0].onOff;

  if (! SurfDots)
    /* do not allow grouping for dot surfaces because atom
       pointers are stored in the primitive there */
    DhActivateGroups(TRUE);

  BreakActivate(TRUE);

  if (DoReport && ! SurfDots) {
    TextW = PuCreateTextWindow(cmd);
    PuWriteStr(TextW, "Prim #: Surface, Volume\n");
  }

  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), addSurface, arg[0].v.exprP);

  BreakActivate(FALSE);

  if (! SurfDots)
    DhActivateGroups(FALSE);

  ArgCleanup(arg, ARG_NUM);
  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
