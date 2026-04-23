/*
************************************************************************
*
*   POVFrame.c - POV end of frame
*
*   Copyright (c) 1994
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
*   Date of last modification : 94/11/15
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/pov/SCCS/s.POVFrame.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "pov_frame.h"

#include <sg_plot.h>
#include "pov_light.h"

void
SgPOVEndFrame(void)
{
  POVPlotLightSource();
}
