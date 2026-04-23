/*
************************************************************************
*
*   RIBFrame.c - RIB end of frame
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBFrame.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "rib_frame.h"

#include <sg_plot.h>
#include "rib_world.h"

void
SgRIBEndFrame(void)
{
  RIBWorldEnd();
}
