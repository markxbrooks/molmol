/*
************************************************************************
*
*   xgl_view.h - XGL viewing parameters
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
*   Date of last modification : 97/07/17
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/xgl/SCCS/s.xgl_view.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_types.h>

extern void SgXGLSetProjection(SgProjection);

extern void SgXGLSetViewPoint(float[3]);

extern void SgXGLSetViewAngle(float);

extern void SgXGLSetNearPlane(float, BOOL);

extern void SgXGLSetFarPlane(float, BOOL);

extern void SgXGLUpdateView(void);

extern void SgXGLSetViewport(float, float, float, float);

extern void SgXGLSetBuffer(SgBuffer);

extern void SgXGLEndFrame(void);
