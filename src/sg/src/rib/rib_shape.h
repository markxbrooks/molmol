/*
************************************************************************
*
*   rib_shape.h - RIB discs, spheres and cylinders
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.rib_shape.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgRIBDrawDisc2D(float[2], float);

extern void SgRIBDrawDisc(float[3], float[3], float);

extern void SgRIBDrawSphere(float[3], float);

extern void SgRIBDrawCone(float[3], float[3], float, float,
    SgConeEnd, SgConeEnd);
