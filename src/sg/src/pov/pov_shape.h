/*
************************************************************************
*
*   pov_shape.h - POV discs, spheres and cylinders
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
*   Date of last modification : 96/06/06
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/pov/SCCS/s.pov_shape.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_types.h>

extern void SgPOVDrawDisc2D(float[2], float);

extern void SgPOVDrawDisc(float[3], float[3], float);

extern void SgPOVDrawSphere(float[3], float);

extern void SgPOVDrawCone(float[3], float[3], float, float,
    SgConeEnd, SgConeEnd);
