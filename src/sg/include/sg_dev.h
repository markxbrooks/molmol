/*
************************************************************************
*
*   sg_dev.h - structure for Sg device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/include/SCCS/s.sg_dev.h
*   SCCS identification       : 1.18
*
************************************************************************
*/

#include <sg.h>

typedef enum {
  VPH_NORMAL,
  VPH_UPSIDE_DOWN
} ViewPortHandling;

typedef SgRes (*SgInitF) (int *, char *[]);
typedef SgRes (*SgCleanupF) (void);

typedef SgObjId (*SgBeginObjF) (void);
typedef void (*SgEndObjF) (void);
typedef void (*SgReplaceObjF) (SgObjId);
typedef void (*SgDestroyObjF) (SgObjId);

typedef void (* SgClearF) (void);

typedef void (* SgDrawObjF) (SgObjId);

typedef void (* SgDrawMarkers2DF) (float[][2], int);
typedef void (* SgDrawLine2DF) (float[2], float[2]);
typedef void (* SgDrawArrow2DF) (float[2], float[2]);
typedef void (* SgDrawPolyline2DF) (float[][2], int);
typedef void (* SgDrawPolygon2DF) (float[][2], int);
typedef void (* SgDrawShadedPolygon2DF) (float[][2], float[][3], int);
typedef void (* SgDrawCircle2DF) (float[2], float);
typedef void (* SgDrawDisc2DF) (float[2], float);
typedef void (* SgDrawAnnot2DF) (float[2], char *);
typedef void (* SgDrawText2DF) (float[2], char *);

typedef void (* SgDrawMarkersF) (float[][3], int);
typedef void (* SgDrawLineF) (float[3], float[3]);
typedef void (* SgDrawArrowF) (float[3], float[3]);
typedef void (* SgDrawPolylineF) (float[][3], int);
typedef void (* SgDrawPolygonF) (float[][3], int, float[3]);
typedef void (* SgDrawCircleF) (float[3], float[3], float);
typedef void (* SgDrawDiscF) (float[3], float[3], float);
typedef void (* SgDrawAnnotF) (float[3], char *);
typedef void (* SgDrawTextF) (float[3], char *);
typedef void (* SgDrawSphereF) (float[3], float);
typedef void (* SgDrawConeF) (float[3], float[3], float, float,
    SgConeEnd, SgConeEnd);

typedef void (* SgStartSurfaceF) (SgSurfaceType);
typedef void (* SgDrawTriMeshF) (float[][3], float[][3], int);
typedef void (* SgDrawColorTriMeshF) (float[][3], float[][3], float[][3], int);
typedef void (* SgDrawStripF) (float[][3], float[][3],
    float[][3], float[][3], int);
typedef void (* SgDrawColorStripF) (float[][3], float[][3],
    float[][3], float[][3], float[][3], float[][3], int);
typedef void (* SgEndSurfaceF) (void);

typedef void (* SgCalcHiddenF) (void);
typedef void (* SgSetBufferF) (SgBuffer);
typedef void (* SgFlushFrameF) (void);
typedef void (* SgEndFrameF) (void);
typedef BOOL (* SgRefreshF) (void);

typedef void (* SgSetFeatureF) (SgFeature, BOOL);

typedef void (* SgSetColorF) (float, float, float, float, BOOL);
typedef void (* SgSetMatPropF) (SgMatProp, float);
typedef void (* SgSetTextureF) (char *);
typedef void (* SgSetPrecisionF) (int);
typedef void (* SgSetLineWidthF) (float);
typedef void (* SgSetLineStyleF) (SgLineStyle);
typedef void (* SgSetShadeModelF) (SgShadeModel);
typedef void (* SgSetFogModeF) (SgFogMode);
typedef void (* SgSetFogParF) (SgFogPar, float);
typedef void (* SgSetFontF) (char *);
typedef void (* SgSetFontSize2DF) (float);
typedef void (* SgSetFontSizeF) (float);

typedef float (* SgGetTextWidthF) (char *, float);

typedef void (* SgRotateXF) (float);
typedef void (* SgRotateYF) (float);
typedef void (* SgRotateZF) (float);
typedef void (* SgTranslateF) (float[3]);
typedef void (* SgScaleF) (float, float, float);
typedef void (* SgSetMatrixF) (float[4][4]);
typedef void (* SgMultMatrixF) (float[4][4]);
typedef void (* SgPushMatrixF) (void);
typedef void (* SgPopMatrixF) (void);

typedef void (* SgSetStereoF) (BOOL);
typedef void (* SgSetEyeF) (SgEye);

typedef void (* SgSetProjectionF) (SgProjection);
typedef void (* SgSetViewPointF) (float[3]);
typedef void (* SgSetViewAngleF) (float);
typedef void (* SgSetNearPlaneF) (float, BOOL);
typedef void (* SgSetFarPlaneF) (float, BOOL);
typedef void (* SgUpdateViewF) (void);
typedef void (* SgSetViewportF) (float, float, float, float);

typedef void (* SgConvCoordF) (int, int, float *, float *);

typedef void (* SgSetLightF) (SgLightState);
typedef void (* SgSetLightPositionF) (float[3]);

typedef struct {
  BOOL completed;

  ViewPortHandling vpHandling;
  int dim;

  SgInitF init;
  SgCleanupF cleanup;

  SgBeginObjF beginObj;
  SgEndObjF endObj;
  SgReplaceObjF replaceObj;
  SgDestroyObjF destroyObj;

  SgClearF clear;

  SgDrawObjF drawObj;

  SgDrawMarkers2DF drawMarkers2D;
  SgDrawLine2DF drawLine2D;
  SgDrawArrow2DF drawArrow2D;
  SgDrawPolyline2DF drawPolyline2D;
  SgDrawPolygon2DF drawPolygon2D;
  SgDrawShadedPolygon2DF drawShadedPolygon2D;
  SgDrawCircle2DF drawCircle2D;
  SgDrawDisc2DF drawDisc2D;
  SgDrawAnnot2DF drawAnnot2D;
  SgDrawText2DF drawText2D;

  SgDrawMarkersF drawMarkers;
  SgDrawLineF drawLine;
  SgDrawArrowF drawArrow;
  SgDrawPolylineF drawPolyline;
  SgDrawPolygonF drawPolygon;
  SgDrawCircleF drawCircle;
  SgDrawDiscF drawDisc;
  SgDrawAnnotF drawAnnot;
  SgDrawTextF drawText;
  SgDrawSphereF drawSphere;
  SgDrawConeF drawCone;

  SgStartSurfaceF startSurface;
  SgDrawTriMeshF drawTriMesh;
  SgDrawColorTriMeshF drawColorTriMesh;
  SgDrawStripF drawStrip;
  SgDrawColorStripF drawColorStrip;
  SgEndSurfaceF endSurface;

  SgCalcHiddenF calcHidden;
  SgSetBufferF setBuffer;
  SgFlushFrameF flushFrame;
  SgEndFrameF endFrame;
  SgRefreshF refresh;

  SgSetFeatureF setFeature;

  SgSetColorF setColor;
  SgSetMatPropF setMatProp;
  SgSetTextureF setTexture;
  SgSetPrecisionF setPrecision;
  SgSetLineWidthF setLineWidth;
  SgSetLineStyleF setLineStyle;
  SgSetShadeModelF setShadeModel;
  SgSetFogModeF setFogMode;
  SgSetFogParF setFogPar;
  SgSetFontF setFont;
  SgSetFontSize2DF setFontSize2D;
  SgSetFontSizeF setFontSize;

  SgGetTextWidthF getTextWidth;

  SgRotateXF rotateX;
  SgRotateYF rotateY;
  SgRotateZF rotateZ;
  SgTranslateF translate;
  SgScaleF scale;
  SgSetMatrixF setMatrix;
  SgMultMatrixF multMatrix;
  SgPushMatrixF pushMatrix;
  SgPopMatrixF popMatrix;

  SgSetStereoF setStereo;
  SgSetEyeF setEye;

  SgSetProjectionF setProjection;
  SgSetViewPointF setViewPoint;
  SgSetViewAngleF setViewAngle;
  SgSetNearPlaneF setNearPlane;
  SgSetFarPlaneF setFarPlane;
  SgUpdateViewF updateView;
  SgSetViewportF setViewport;

  SgConvCoordF convCoord;

  SgSetLightF setLight;
  SgSetLightPositionF setLightPosition;
} SgDev;

extern SgDev *CurrSgDevP;
