/*
************************************************************************
*
*   ExAco.c - AddAco and RemoveAco commands
*
*   Copyright (c) 1994-95
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExAco.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_struc.h>

#include <arg.h>
#include <data_hand.h>

typedef struct {
  float minVal;
  float maxVal;
} AngleData;

static void
getVal(DhAngleP angleP, void *clientData)
{
  AngleData *dataP = clientData;

  dataP->minVal = DhAngleGetMinVal(angleP);
  dataP->maxVal = DhAngleGetMaxVal(angleP);
}

static void
setVal(DhAngleP angleP, void *clientData)
{
  AngleData *dataP = clientData;

  DhAngleSetMinVal(angleP, dataP->minVal);
  DhAngleSetMaxVal(angleP, dataP->maxVal);
}

#define ARG_NUM 2

ErrCode
ExAddAco(char *cmd)
{
  PropRefP refP;
  AngleData data;
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.minVal = DH_ANGLE_MIN;
  data.maxVal = DH_ANGLE_MAX;
  DhApplyAngle(refP, getVal, &data);

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Min. Angle";
  arg[0].v.doubleVal = data.minVal;

  arg[1].prompt = "Max. Angle";
  arg[1].v.doubleVal = data.maxVal;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.minVal = (float) arg[0].v.doubleVal;
  data.maxVal = (float) arg[1].v.doubleVal;

  if (data.minVal != DH_ANGLE_MIN) {
    if (data.minVal < -180.0f) {
      data.minVal += (int) ((- data.minVal + 180.0f) / 360.0f) * 360.0f;
    } else {
      data.minVal -= (int) ((data.minVal + 180.0f) / 360.0f) * 360.0f;
    }
  }

  if (data.maxVal != DH_ANGLE_MAX) {
    if (data.maxVal < data.minVal) {
      data.maxVal +=
	  (int) ((data.minVal - data.maxVal + 360.0f) / 360.0f) * 360.0f;
      if (data.maxVal > 360.0f) {
	data.maxVal -= 360.0f;
	data.minVal -= 360.0f;
      }
    } else {
      data.maxVal -= (int) ((data.maxVal - data.minVal) / 360.0f) * 360.0f;
    }
  }

  ArgCleanup(arg, ARG_NUM);

  DhApplyAngle(refP, setVal, &data);

  return EC_OK;
}

ErrCode
ExRemoveAco(char *cmd)
{
  AngleData data;

  data.minVal = DH_ANGLE_MIN;
  data.maxVal = DH_ANGLE_MAX;
  DhApplyAngle(PropGetRef(PROP_SELECTED, FALSE), setVal, &data);

  return EC_OK;
}
