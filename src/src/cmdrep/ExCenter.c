/*
************************************************************************
*
*   ExCenter.c - Center command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExCenter.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_rep.h>

#include <mat_vec.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

typedef struct {
  Vec3 sum;
  int n;
} AvgData;

static void
getCenter(DhAtomP atomP, void *clientData)
{
  AvgData *dataP = clientData;
  Vec3 cent;

  DhAtomGetCoordTransf(atomP, cent);
  Vec3Add(dataP->sum, cent);
  dataP->n++;
}

static void
molCenter(DhMolP molP, void *clientData)
{
  Vec3 cent, transV, oldRotV, newRotV;
  Vec4 v4;
  Mat4 rotM;

  Vec3Copy(cent, clientData);

  DhMolGetTransVect(molP, transV);
  DhMolSetTransVect(molP, cent);

  DhMolGetRotMat(molP, rotM);
  DhMolGetRotPoint(molP, oldRotV);

  Vec3Copy(newRotV, cent);
  Vec3Sub(newRotV, transV);
  Vec3To4(v4, newRotV);
  Mat4Transp(rotM);
  Mat4VecMult(v4, rotM);
  Vec4To3(newRotV, v4);
  Vec3Add(newRotV, oldRotV);

  DhMolSetRotPoint(molP, newRotV);
}

ErrCode
ExCenter(char *cmd)
{
  AvgData data;

  Vec3Zero(data.sum);
  data.n = 0;

  DhApplyAtom(PropGetRef(PROP_SELECTED, FALSE), getCenter, &data);
  if (data.n == 0) {
    CipSetError("at least 1 atom must be selected");
    return EC_ERROR;
  }

  Vec3Scale(data.sum, 1.0f / data.n);

  DhApplyMol(PropGetRef(PROP_MOVABLE, FALSE), molCenter, data.sum);

  GraphRedrawNeeded();

  return EC_OK;
}
