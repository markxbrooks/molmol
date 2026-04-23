/*
************************************************************************
*
*   RIBWorld.c - RIB status management
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBWorld.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "rib_world.h"

#include <sg_plot.h>
#include "rib_light.h"
#include "rib_color.h"

static BOOL InWorld = FALSE;

void
RIBWorldStart(void)
{
  SgPlotOutputStr("WorldBegin\n");
  /* light source used when lighting is turned off */
  SgPlotOutputStr("LightSource \"ambientlight\" 0\n");

  RIBStartLight();
  RIBStartColor();

  InWorld = TRUE;
}

void
RIBWorldEnd(void)
{
  SgPlotOutputStr("WorldEnd\n");
  InWorld = FALSE;
}

BOOL
RIBInWorld(void)
{
  return InWorld;
}
