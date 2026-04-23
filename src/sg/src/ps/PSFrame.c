/*
************************************************************************
*
*   PSFrame.c - PostScript end frame
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ps/SCCS/s.PSFrame.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "ps_frame.h"

#include <sg_plot.h>

void
SgPSEndFrame(void)
{
  SgPlotOutputStr("showpage\n");
}
