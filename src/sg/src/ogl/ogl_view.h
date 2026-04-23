/*
************************************************************************
*
*   ogl_view.h - OpenGL viewing parameters
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.ogl_view.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <sg_types.h>

extern void SgOGLSetProjection(SgProjection);

extern void SgOGLSetViewPoint(float[3]);

extern void SgOGLSetViewAngle(float);

extern void SgOGLSetNearPlane(float, BOOL);

extern void SgOGLSetFarPlane(float, BOOL);

extern void SgOGLUpdateView(void);

extern void SgOGLSetViewport(float, float, float, float);

extern void SgOGLConvCoord(int, int, float *, float *);

extern void SgOGLSetBuffer(SgBuffer);

extern void SgOGLFlushFrame(void);

extern void SgOGLEndFrame(void);
