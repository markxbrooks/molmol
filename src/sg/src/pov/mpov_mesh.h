/*
************************************************************************
*
*   mpov_mesh.h - MPOV mesh drawing
*
*   Copyright (c) 1994-2000
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
*   Date of last modification : 00/02/26
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.mpov_mesh.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgMPOVDrawTriMesh(float[][3], float[][3], int);

extern void SgMPOVDrawColorTriMesh(float[][3], float[][3], float [][3], int);

extern void SgMPOVDrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);

extern void SgMPOVDrawColorStrip(float[][3], float[][3],
    float[][3], float [][3], float [][3], float[][3], int);
