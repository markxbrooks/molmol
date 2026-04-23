/*
************************************************************************
*
*   GLUtil.c - GL utility functions
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/gl/SCCS/s.GLUtil.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "gl_util.h"

#include <math.h>
#include <values.h>

Angle
Rad2GLAng(float ang)
{
  /* argument is in rad, GL angles are in tenths of degrees */
  return ang * 180.0f / (float) M_PI * 10.0f + 0.5f;
}
