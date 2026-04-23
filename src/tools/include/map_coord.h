/*
************************************************************************
*
*   map_coord.h - find least squares superposition
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
*   Date of last modification : 95/06/09
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.map_coord.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _MAP_COORD_H_
#define _MAP_COORD_H_

#include <bool.h>
#include <mat_vec.h>

extern void MapCoord(Vec3[], int, Vec3[], BOOL, Mat4);

#endif  /* _MAP_COORD_H_ */
