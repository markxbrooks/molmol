/*
************************************************************************
*
*   sg_def.h - default functions for Sg
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/include/SCCS/s.sg_def.h
*   SCCS identification       : 1.17
*
************************************************************************
*/

#include <sg_types.h>


extern void SgDefClear(void);


extern void SgDefDrawMarkers2D(float[][2], int);

extern void SgDefDrawArrow2D(float[2], float[2]);

extern void SgDefDrawCircle2D(float[2], float);

extern void SgDefDrawDisc2D(float[2], float);

extern void SgDefDrawAnnot2D(float[2], char *);

extern void SgDefDrawText2D(float[2], char *);


extern void SgDefDrawMarkers(float[][3], int);

extern void SgDefDrawLine(float[3], float[3]);

extern void SgDefDrawArrow(float[3], float[3]);

extern void SgDefDrawPolyline(float[][3], int);

extern void SgDefDrawPolygon(float[][3], int, float[3]);

extern void SgDefDrawCircle(float[3], float[3], float);

extern void SgDefDrawDisc(float[3], float[3], float);

extern void SgDefDrawAnnot(float[3], char *);

extern void SgDefDrawText(float[3], char *);

extern void SgDefDrawSphere(float[3], float);

extern void SgDefDrawCone(float[3], float[3], float, float,
    SgConeEnd, SgConeEnd);


extern void SgDefStartSurface(SgSurfaceType);

extern void SgDefDrawTriMesh(float[][3], float[][3], int);

extern void SgDefDrawColorTriMesh(float[][3], float[][3], float[][3], int);

extern void SgDefDrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);

extern void SgDefDrawColorStrip(float[][3], float[][3],
    float[][3], float[][3], float[][3], float[][3], int);

extern void SgDefEndSurface(void);

extern void SgDefCalcHidden(void);

extern void SgDefSetBuffer(SgBuffer);

extern void SgDefFlushFrame(void);

extern void SgDefEndFrame(void);

extern BOOL SgDefRefresh(void);


extern void SgDefSetFeature(SgFeature, BOOL);


extern void SgDefSetColor(float, float, float, float, BOOL);

extern void SgDefSetMatProp(SgMatProp, float);

extern void SgDefSetTexture(char *);

extern void SgDefSetPrecision(int);

extern void SgDefSetLineWidth(float);

extern void SgDefSetLineStyle(SgLineStyle);

extern void SgDefSetShadeModel(SgShadeModel);

extern void SgDefSetFogMode(SgFogMode);

extern void SgDefSetFogPar(SgFogPar, float);

extern void SgDefSetFont(char *);

extern void SgDefSetFontSize2D(float);

extern void SgDefSetFontSize(float);


extern float SgDefGetTextWidth(char *, float);


extern void SgDefRotateX(float);

extern void SgDefRotateY(float);

extern void SgDefRotateZ(float);

extern void SgDefTranslate(float[3]);

extern void SgDefScale(float, float, float);

extern void SgDefSetMatrix(float[4][4]);

extern void SgDefMultMatrix(float[4][4]);

extern void SgDefPushMatrix(void);

extern void SgDefPopMatrix(void);


extern void SgDefSetProjection(SgProjection);

extern void SgDefSetViewPoint(float[3]);

extern void SgDefSetViewAngle(float);

extern void SgDefSetNearPlane(float, BOOL);

extern void SgDefSetFarPlane(float, BOOL);

extern void SgDefUpdateView(void);

extern void SgDefSetViewport(float, float, float, float);


extern void SgDefConvCoord(int, int, float *, float *);


extern void SgDefSetLight(SgLightState);

extern void SgDefSetLightPosition(float[3]);
