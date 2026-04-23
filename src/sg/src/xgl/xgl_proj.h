/*
************************************************************************
*
*   xgl_proj.h - get projection
*
*   Copyright (c) 1994-95
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
*   Date of last modification : 96/03/05
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.xgl_proj.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <sg_types.h>

extern SgProjection XGLGetProjection(void);

extern float XGLGetNearPlane(void);

extern float XGLGetFarPlane(void);
