/*
************************************************************************
*
*   gl_view.h - GL viewing parameters
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/gl/SCCS/s.gl_view.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <sg_types.h>

extern void SgGLSetStereo(BOOL);

extern void SgGLSetEye(SgEye);

extern void SgGLSetProjection(SgProjection);

extern void SgGLSetViewPoint(float[3]);

extern void SgGLSetViewAngle(float);

extern void SgGLSetNearPlane(float, BOOL);

extern void SgGLSetFarPlane(float, BOOL);

extern void SgGLUpdateView(void);

extern void SgGLSetViewport(float, float, float, float);

extern void SgGLConvCoord(int, int, float *, float *);

extern void SgGLSetBuffer(SgBuffer);

extern void SgGLFlushFrame(void);

extern void SgGLEndFrame(void);
