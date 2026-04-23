/*
************************************************************************
*
*   graph_transf.h - transformations of graphical display
*
*   Copyright (c) 1994-99
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
*   Date of last modification : 99/10/09
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.graph_transf.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#ifndef _GRAPH_TRANSF_H_
#define _GRAPH_TRANSF_H_

#include <mat_vec.h>

extern float GraphDegToRad(float);

extern void GraphRotate(Mat4);

extern void GraphRotateX(float);

extern void GraphRotateY(float);

extern void GraphRotateZ(float);

extern void GraphMove(Vec3);

extern void GraphAutoScale(void);

#endif  /* _GRAPH_TRANSF_H_ */
