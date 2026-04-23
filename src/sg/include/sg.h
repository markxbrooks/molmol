/*
************************************************************************
*
*   sg.h - Sg graphics library
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/include/SCCS/s.sg.h
*   SCCS identification       : 1.23
*
************************************************************************
*/

#ifndef _SG_H_
#define _SG_H_

#include <sg_types.h>


/* administration */

extern SgRes SgInit(int *, char *[]);

extern SgRes SgCleanup(void);

extern SgRes SgSetDevice(char *);

extern char *SgGetDeviceName(void);

extern void SgSetErrorHandler(SgErrorHandler);


/* plotting */

extern SgRes SgStartPlot(char *, char *,
    SgPlotOrient, float, float, float, float, float);

extern void SgEndPlot(void);


/* object management */

extern SgObjId SgCreateObj(SgDrawFunc, void *);

extern void SgReplaceObj(SgObjId, SgDrawFunc, void *);

extern void SgDestroyObj(SgObjId);


/* clear window */

extern void SgClear(void);


/* general drawing */

extern void SgDrawObj(SgObjId);


/* 2D drawing */

extern void SgDrawMarkers2D(float[][2], int);

extern void SgDrawLine2D(float[2], float[2]);

extern void SgDrawArrow2D(float[2], float[2]);

extern void SgDrawPolyline2D(float[][2], int);

extern void SgDrawPolygon2D(float[][2], int);

extern void SgDrawCircle2D(float[2], float);

extern void SgDrawDisc2D(float[2], float);

extern void SgDrawAnnot2D(float[2], char *);

extern void SgDrawText2D(float[2], char *);


/* simple 3D drawing */

extern void SgDrawMarkers(float[][3], int);

extern void SgDrawLine(float[3], float[3]);

extern void SgDrawArrow(float[3], float[3]);

extern void SgDrawPolyline(float[][3], int);

extern void SgDrawPolygon(float[][3], int, float[3]);

extern void SgDrawCircle(float[3], float[3], float);

extern void SgDrawDisc(float[3], float[3], float);

extern void SgDrawAnnot(float[3], char *);

extern void SgDrawText(float[3], float[2], char *);

extern void SgDrawSphere(float[3], float);

extern void SgDrawCone(float[3], float[3], float, float,
    SgConeEnd, SgConeEnd);


/* surface drawing */

extern void SgStartSurface(SgSurfaceType);

extern void SgDrawTriMesh(float[][3], float[][3], int);

extern void SgDrawColorTriMesh(float[][3], float[][3], float[][3], int);

extern void SgDrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);

extern void SgDrawColorStrip(float[][3], float[][3],
    float[][3], float[][3], float[][3], float[][3], int);

extern void SgEndSurface(void);


/* picking */

extern void SgStartPick(float, float);

extern void SgSetPickObj(void *, void *);

extern void SgEndPick(void **, void **, int *, float *);


/* range determination */

extern void SgStartRange(void);

extern void SgEndRange(float *, float *, float *, float *);


/* double buffering */

extern void SgSetDoubleBuffer(BOOL);

extern BOOL SgGetDoubleBuffer(void);

extern void SgSetBuffer(SgBuffer);

extern void SgFlushFrame(void);

extern void SgEndFrame(void);

extern BOOL SgRefresh(void);


/* enable/disable features */

extern void SgSetFeature(SgFeature, BOOL);


/* set graphics attributes */

extern void SgSetBackgroundColor(float, float, float);

extern void SgSetColor(float, float, float);

extern void SgSetMatProp(SgMatProp, float);

extern void SgSetTexture(char *);

extern void SgSetPrecision(int);

extern void SgSetLineWidth(float);

extern void SgSetLineStyle(SgLineStyle);

extern void SgSetShadeModel(SgShadeModel);

extern void SgSetFogMode(SgFogMode);

extern void SgSetFogPar(SgFogPar, float);

extern void SgSetFont(char *);

extern void SgSetFontSize(float);


/* get text sizes */

extern float SgGetTextWidth(char *, float);

extern void SgGetTextBox(float[3], float[3], float[2], float[2],
    float[3], float[2], char *, float);


/* modeling transformations */

extern void SgRotateX(float);

extern void SgRotateY(float);

extern void SgRotateZ(float);

extern void SgTranslate(float[3]);

extern void SgScale(float, float, float);

extern void SgSetMatrix(float[4][4]);

extern void SgMultMatrix(float[4][4]);

extern void SgPushMatrix(void);

extern void SgPopMatrix(void);


/* stereo display */

extern void SgSetStereo(BOOL);

extern BOOL SgGetStereo(void);

extern void SgSetEye(SgEye);


/* projection */

extern void SgSetProjection(SgProjection);

extern void SgSetViewPoint(float[3]);

extern void SgSetViewAngle(float);

extern void SgSetNearPlane(float, BOOL);

extern void SgSetFarPlane(float, BOOL);

extern void SgUpdateView(void);

extern void SgSetViewport(float, float, float, float);

extern void SgGetViewport(float *, float *, float *, float *);

extern float SgGetDepthFact(float);


/* convert screen coordinates */

extern void SgConvCoord(int, int, float *, float *);


/* light source */

extern void SgSetLight(SgLightState);

extern void SgSetLightPosition(float[3]);

#endif  /* _SG_H_ */
