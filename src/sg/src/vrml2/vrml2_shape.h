/*
************************************************************************
*
*   vrml2_shape.h - VRML2 spheres and cylinders
*
*   Copyright (c) 1996-99
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
*   Date of last modification : 99/10/31
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.vrml2_shape.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgVRML2DrawSphere(float[3], float);

extern void SgVRML2DrawCone(float[3], float[3], float, float,
    SgConeEnd, SgConeEnd);
