/*
************************************************************************
*
*   pov_view.h - POV viewing parameters
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/09/26
*   Pathname of SCCS file     : /files/kor/molmol/sg/src/pov/SCCS/s.pov_view.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_types.h>

extern void SgPOVSetProjection(SgProjection);

extern void SgPOVSetViewPoint(float[3]);

extern void SgPOVSetViewAngle(float);

extern void SgPOVSetNearPlane(float, BOOL);

extern void SgPOVSetFarPlane(float, BOOL);

extern void SgPOVUpdateView(void);

extern void POVInitView(void);
