/*
************************************************************************
*
*   ExCalcHelix.c - CalcHelix command
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
*   Date of last modification : 03/01/20
*   Pathname of SCCS file     : /home/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcHelix.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <pu.h>
#include <data_hand.h>
#include <prim_hand.h>

typedef struct {
  int *numA;
  Vec3 *axisA;
  int cylI;
} CylData;

static PuTextWindow TextW;

static void
countCyl(PrimObjP primP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
fillCyl(PrimObjP primP, void *clientData)
{
  CylData *dataP = clientData;
  DhMolP molP;
  Vec3 x0, v;

  molP = PrimGetMol(primP);

  Vec3Zero(x0);
  DhTransfVec(x0, molP);

  dataP->numA[dataP->cylI] = PrimGetNumber(primP);
  PrimGetVec(primP, v);
  DhTransfVec(v, molP);
  Vec3Sub(v, x0);

  Vec3Copy(dataP->axisA[dataP->cylI], v);

  dataP->cylI++;
}

ErrCode
ExCalcHelix(char *cmd)
{
  int cylNo, cyl1I, cyl2I;
  PropRefP refP;
  CylData data;
  char buf[20];
  float s;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  cylNo = 0;
  PrimApply(PT_CYLINDER, refP, countCyl, &cylNo);

  if (cylNo < 2) {
    CipSetError("at least two cylinders must be selected");
    return EC_ERROR;
  }

  data.numA = malloc(cylNo * sizeof(*data.numA));
  data.axisA = malloc(cylNo * sizeof(*data.axisA));

  data.cylI = 0;
  PrimApply(PT_CYLINDER, refP, fillCyl, &data);

  TextW = PuCreateTextWindow(cmd);

  PuWriteStr(TextW, "Angles between cylinder axes:\n");
  PuWriteStr(TextW, "\n");

  PuWriteStr(TextW, "      ");
  for (cyl1I = 0; cyl1I < cylNo; cyl1I++) {
    (void) sprintf(buf, "%6d", data.numA[cyl1I]);
    PuWriteStr(TextW, buf);
  }
  PuWriteStr(TextW, "\n");

  for (cyl1I = 0; cyl1I < cylNo; cyl1I++)
    Vec3Norm(data.axisA[cyl1I]);

  for (cyl1I = 0; cyl1I < cylNo; cyl1I++) {
    (void) sprintf(buf, "%6d", data.numA[cyl1I]);
    PuWriteStr(TextW, buf);

    for (cyl2I = 0; cyl2I < cylNo; cyl2I++) {
      s = Vec3Scalar(data.axisA[cyl1I], data.axisA[cyl2I]);
      if (s > 1.0f)
	s = 1.0f;
      else if (s < -1.0f)
	s = -1.0f;
      (void) sprintf(buf, "%6.1f", acosf(s) * 180.0f / (float) M_PI);
      PuWriteStr(TextW, buf);
    }

    PuWriteStr(TextW, "\n");
  }

  free(data.numA);
  free(data.axisA);

  return EC_OK;
}
