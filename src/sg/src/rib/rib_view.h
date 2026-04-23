/*
************************************************************************
*
*   rib_view.h - RIB viewing parameters
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.rib_view.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgRIBSetProjection(SgProjection);

extern void SgRIBSetViewPoint(float[3]);

extern void SgRIBSetViewAngle(float);

extern void SgRIBSetNearPlane(float, BOOL);

extern void SgRIBSetFarPlane(float, BOOL);

extern void SgRIBUpdateView(void);

extern void RIBInitView(void);

extern void RIBGetClipPlanes(float *, float *);
