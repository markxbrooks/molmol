/*
************************************************************************
*
*   RIBInit.c - init RIB device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBInit.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "rib_init.h"

#include <sg_plot.h>
#include "rib_util.h"
#include "rib_view.h"

SgRes
SgRIBInit(int *argcP, char *argv[])
{
  float gamma;

  gamma = SgPlotGetGamma();

  if (gamma != 1.0f) {
    SgPlotOutputStr("Exposure 1.0 ");
    SgPlotOutputFloat(1.0f / gamma);
    SgPlotOutputStr("\n");
  }

  RIBInit2D();
  RIBInitView();

  return SG_RES_OK;
}

SgRes
SgRIBCleanup(void)
{
  return SG_RES_OK;
}
