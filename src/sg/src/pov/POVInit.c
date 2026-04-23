/*
************************************************************************
*
*   POVInit.c - init POV device
*
*   Copyright (c) 1994-2000
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVInit.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include "pov_init.h"

#include <sg_plot.h>
#include "pov_util.h"
#include "pov_view.h"

SgRes
SgPOVInit(int *argcP, char *argv[])
{
  float gamma;

  SgPlotOutputStr("#include \"colors.inc\"\n");
  SgPlotOutputStr("#include \"textures.inc\"\n");
  SgPlotOutputStr("\n");

  gamma = SgPlotGetGamma();

  if (gamma != 1.0f) {
    SgPlotOutputStr("global_settings {assumed_gamma ");
    SgPlotOutputFloat(gamma);
    SgPlotOutputStr("}\n");
  }

  POVInit2D();
  POVInitView();

  return SG_RES_OK;
}

SgRes
SgMPOVInit(int *argcP, char *argv[])
{
  SgPlotOutputStr("#version unofficial MegaPov 0.4;\n");

  return SgPOVInit(argcP, argv);
}

SgRes
SgPOVCleanup(void)
{
  return SG_RES_OK;
}
