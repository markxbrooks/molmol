/*
************************************************************************
*
*   SgDev.c - Sg device management
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgDev.c
*   SCCS identification       : 1.24
*
************************************************************************
*/

#include <string.h>

#include <sg_dev.h>

#include <sg_def.h>
#include "sg_name.h"

extern void SgPickSetDev(void);
extern void SgRangeSetDev(void);
#ifdef SG_DEV_NO
extern void SgNOSetDev(void);
#endif
#ifdef SG_DEV_X11
extern void SgX11SetDev(void);
#endif
#ifdef SG_DEV_GL
extern void SgGLSetDev(void);
extern void SgGLDSetDev(void);
#endif
#ifdef SG_DEV_OGL
extern void SgOGLSetDev(void);
extern void SgOGLDSetDev(void);
#endif
#ifdef SG_DEV_XGL
extern void SgXGLSetDev(void);
#endif
#ifdef SG_DEV_POV
extern void SgPOVSetDev(void);
extern void SgMPOVSetDev(void);
#endif
#ifdef SG_DEV_RIB
extern void SgRIBSetDev(void);
#endif
#ifdef SG_DEV_VRML1
extern void SgVRML1SetDev(void);
#endif
#ifdef SG_DEV_VRML2
extern void SgVRML2SetDev(void);
#endif
#ifdef SG_DEV_PS
extern void SgPSSetDev(void);
#endif
#ifdef SG_DEV_CPS
extern void SgCPSSetDev(void);
#endif
#ifdef SG_DEV_FM3
extern void SgFM3SetDev(void);
#endif
#ifdef SG_DEV_FM4
extern void SgFM4SetDev(void);
#endif

SgDev *CurrSgDevP;

typedef void (*DevSetF) (void);

typedef struct {
  char *name;
  DevSetF set;
} DevListEntry;

static DevListEntry DevList[] = {
  {"Pick", SgPickSetDev},
  {"Range", SgRangeSetDev},
#ifdef SG_DEV_NO
  {"NO", SgNOSetDev},
#endif
#ifdef SG_DEV_X11
  {"X11", SgX11SetDev},
#endif
#ifdef SG_DEV_GL
  {"GL", SgGLSetDev},
  {"GLD", SgGLDSetDev},
#endif
#ifdef SG_DEV_OGL
  {"OpenGL", SgOGLSetDev},
  {"OpenGLD", SgOGLDSetDev},
#endif
#ifdef SG_DEV_XGL
  {"XGL", SgXGLSetDev},
#endif
#ifdef SG_DEV_POV
  {"POV", SgPOVSetDev},
  {"MPOV", SgMPOVSetDev},
#endif
#ifdef SG_DEV_RIB
  {"RIB", SgRIBSetDev},
#endif
#ifdef SG_DEV_VRML1
  {"VRML1", SgVRML1SetDev},
#endif
#ifdef SG_DEV_VRML2
  {"VRML2", SgVRML2SetDev},
#endif
#ifdef SG_DEV_PS
  {"PS", SgPSSetDev},
#endif
#ifdef SG_DEV_CPS
  {"CPS", SgCPSSetDev},
#endif
#ifdef SG_DEV_FM3
  {"FM3", SgFM3SetDev},
#endif
#ifdef SG_DEV_FM4
  {"FM4", SgFM4SetDev},
#endif
};

static void
completeDev(SgDev *devP)
{
  if (devP->clear == NULL)
    devP->clear = SgDefClear;

  if (devP->drawMarkers2D == NULL)
    devP->drawMarkers2D = SgDefDrawMarkers2D;
  if (devP->drawArrow2D == NULL)
    devP->drawArrow2D = SgDefDrawArrow2D;
  if (devP->drawCircle2D == NULL)
    devP->drawCircle2D = SgDefDrawCircle2D;
  if (devP->drawDisc2D == NULL)
    devP->drawDisc2D = SgDefDrawDisc2D;
  if (devP->drawAnnot2D == NULL)
    devP->drawAnnot2D = SgDefDrawAnnot2D;
  if (devP->drawText2D == NULL)
    devP->drawText2D = SgDefDrawText2D;

  if (devP->drawMarkers == NULL)
    devP->drawMarkers = SgDefDrawMarkers;
  if (devP->drawLine == NULL)
    devP->drawLine = SgDefDrawLine;
  if (devP->drawArrow == NULL)
    devP->drawArrow = SgDefDrawArrow;
  if (devP->drawPolyline == NULL)
    devP->drawPolyline = SgDefDrawPolyline;
  if (devP->drawPolygon == NULL)
    devP->drawPolygon = SgDefDrawPolygon;
  if (devP->drawCircle == NULL)
    devP->drawCircle = SgDefDrawCircle;
  if (devP->drawDisc == NULL)
    devP->drawDisc = SgDefDrawDisc;
  if (devP->drawAnnot == NULL)
    devP->drawAnnot = SgDefDrawAnnot;
  if (devP->drawText == NULL)
    devP->drawText = SgDefDrawText;
  if (devP->drawSphere == NULL)
    devP->drawSphere = SgDefDrawSphere;
  if (devP->drawCone == NULL)
    devP->drawCone = SgDefDrawCone;

  if (devP->startSurface == NULL)
    devP->startSurface = SgDefStartSurface;
  if (devP->drawTriMesh == NULL)
    devP->drawTriMesh = SgDefDrawTriMesh;
  if (devP->drawColorTriMesh == NULL)
    devP->drawColorTriMesh = SgDefDrawColorTriMesh;
  if (devP->drawStrip == NULL)
    devP->drawStrip = SgDefDrawStrip;
  if (devP->drawColorStrip == NULL)
    devP->drawColorStrip = SgDefDrawColorStrip;
  if (devP->endSurface == NULL)
    devP->endSurface = SgDefEndSurface;

  if (devP->setFeature == NULL)
    devP->setFeature = SgDefSetFeature;

  if (devP->calcHidden == NULL)
    devP->calcHidden = SgDefCalcHidden;
  if (devP->setBuffer == NULL)
    devP->setBuffer = SgDefSetBuffer;
  if (devP->flushFrame == NULL)
    devP->flushFrame = SgDefFlushFrame;
  if (devP->endFrame == NULL)
    devP->endFrame = SgDefEndFrame;
  if (devP->refresh == NULL)
    devP->refresh = SgDefRefresh;

  if (devP->setColor == NULL)
    devP->setColor = SgDefSetColor;
  if (devP->setMatProp == NULL)
    devP->setMatProp = SgDefSetMatProp;
  if (devP->setTexture == NULL)
    devP->setTexture = SgDefSetTexture;
  if (devP->setPrecision == NULL)
    devP->setPrecision = SgDefSetPrecision;
  if (devP->setLineWidth == NULL)
    devP->setLineWidth = SgDefSetLineWidth;
  if (devP->setLineStyle == NULL)
    devP->setLineStyle = SgDefSetLineStyle;
  if (devP->setShadeModel == NULL)
    devP->setShadeModel = SgDefSetShadeModel;
  if (devP->setFogMode == NULL)
    devP->setFogMode = SgDefSetFogMode;
  if (devP->setFogPar == NULL)
    devP->setFogPar = SgDefSetFogPar;
  if (devP->setFont == NULL)
    devP->setFont = SgDefSetFont;
  if (devP->setFontSize2D == NULL)
    devP->setFontSize2D = SgDefSetFontSize2D;
  if (devP->setFontSize == NULL)
    devP->setFontSize = SgDefSetFontSize;

  if (devP->getTextWidth == NULL)
    devP->getTextWidth = SgDefGetTextWidth;

  if (devP->rotateX == NULL) {
    devP->rotateX = SgDefRotateX;
    devP->rotateY = SgDefRotateY;
    devP->rotateZ = SgDefRotateZ;
    devP->translate = SgDefTranslate;
    devP->scale = SgDefScale;
    devP->setMatrix = SgDefSetMatrix;
    devP->multMatrix = SgDefMultMatrix;
    devP->pushMatrix = SgDefPushMatrix;
    devP->popMatrix = SgDefPopMatrix;
  }

  if (devP->setProjection == NULL) {
    devP->setProjection = SgDefSetProjection;
    devP->setViewPoint = SgDefSetViewPoint;
    devP->setViewAngle = SgDefSetViewAngle;
    devP->setNearPlane = SgDefSetNearPlane;
    devP->setFarPlane = SgDefSetFarPlane;
    devP->updateView = SgDefUpdateView;
  }

  if (devP->setViewport == NULL)
    devP->setViewport = SgDefSetViewport;

  if (devP->convCoord == NULL)
    devP->convCoord = SgDefConvCoord;

  if (devP->setLight == NULL) {
    devP->setLight = SgDefSetLight;
    devP->setLightPosition = SgDefSetLightPosition;
  }

  devP->completed = TRUE;
}

SgRes
SgSetDevice(char *devName)
{
  int listSize, i;

  listSize = sizeof(DevList) / sizeof(DevList[0]);

  for (i = 0; i < listSize; i++)
    if (strcmp(DevList[i].name, devName) == 0) {
      SgSetDeviceName(DevList[i].name);
      DevList[i].set();
      if (! CurrSgDevP->completed)
	completeDev(CurrSgDevP);
      return SG_RES_OK;
    }
  
  return SG_RES_ERR;
}
