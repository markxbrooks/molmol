/*
************************************************************************
*
*   surf_check.h - check for points inside surface
*
*   Copyright (c) 1996
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
*   Date of last modification : 96/04/16
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.surf_check.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _SURF_CHECK_H_
#define _SURF_CHECK_H_

#include <bool.h>
#include <potential.h>

typedef struct SurfaceCheckDescrS *SurfaceCheckDescrP;

extern SurfaceCheckDescrP SurfaceCheckPrepareContact(PotAtomDescr[], int,
    float, int);

extern SurfaceCheckDescrP SurfaceCheckPrepareSolvent(PotAtomDescr[], int,
    float);

extern BOOL SurfaceCheckPoint(SurfaceCheckDescrP, Vec3);

extern void SurfaceCheckFree(SurfaceCheckDescrP);

#endif   /* _SURF_CHECK_H_ */
