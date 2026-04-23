/*
************************************************************************
*
*   sg_types.h - types for Sg
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/12/03
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/include/SCCS/s.sg_types.h
*   SCCS identification       : 1.13
*
************************************************************************
*/

#ifndef _SG_TYPES_H_
#define _SG_TYPES_H_

#include <bool.h>

#define SG_FALSE 0
#define SG_TRUE  1

#define SG_POLY_SIZE 256

typedef int SgObjId;

typedef enum {
  SG_RES_OK,
  SG_RES_ERR
} SgRes;

typedef enum {
  SG_ERR_ILLDEV,
  SG_ERR_ILLMODE,
  SG_ERR_ILLOBJ,
  SG_ERR_ILLVAL,
  SG_ERR_NOMEM,
  SG_ERR_STACKEMPTY,
  SG_ERR_STACKFULL
} SgError;

typedef enum {
  SG_BUF_BACK,
  SG_BUF_FRONT
} SgBuffer;

typedef enum {
  SG_FEATURE_ANTI_ALIASING,
  SG_FEATURE_TRANSPARENCY,
  SG_FEATURE_FAST_SPHERE,
  SG_FEATURE_BACKFACE,
  SG_FEATURE_SHADE,
  SG_FEATURE_TWO_SIDE,
  SG_FEATURE_DEPTH_SORT,
  SG_FEATURE_HIDDEN
} SgFeature;

#define SG_FEATURE_NO (SG_FEATURE_HIDDEN + 1)

typedef enum {
  SG_MAT_AMBIENT_FACTOR,
  SG_MAT_DIFFUSE_FACTOR,
  SG_MAT_SPECULAR_FACTOR,
  SG_MAT_SHININESS,
  SG_MAT_REFLECTION,
  SG_MAT_ALPHA,
  SG_MAT_REFRACTION_INDEX,
  SG_MAT_TEXTURE_SCALE,
  SG_MAT_BUMP_DEPTH,
  SG_MAT_BUMP_SCALE
} SgMatProp;

#define SG_MAT_PROP_NO (SG_MAT_BUMP_SCALE + 1)

typedef enum {
  SG_FOG_MODE_OFF,
  SG_FOG_MODE_EXP,
  SG_FOG_MODE_EXP2,
  SG_FOG_MODE_LINEAR
} SgFogMode;

typedef enum {
  SG_FOG_DENSITY,
  SG_FOG_START,
  SG_FOG_END
} SgFogPar;

#define SG_FOG_PAR_NO (SG_FOG_END + 1)

#define SG_TEXTURE_NAME_SIZE 20

typedef enum {
  SG_PLOT_NORMAL,
  SG_PLOT_ROT90
} SgPlotOrient;

typedef enum {
  SG_LINE_SOLID,
  SG_LINE_DASHED
} SgLineStyle;

typedef enum {
  SG_CONE_OPEN,
  SG_CONE_CAP_FLAT,
  SG_CONE_CAP_ROUND
} SgConeEnd;

typedef enum {
  SG_SURF_UNIFORM,
  SG_SURF_COLOR
} SgSurfaceType;

typedef enum {
  SG_SHADE_FLAT,
  SG_SHADE_GOURAUD,
  SG_SHADE_PHONG,
  SG_SHADE_DOTS,
  SG_SHADE_LINES
} SgShadeModel;

typedef enum {
  SG_EYE_LEFT,
  SG_EYE_RIGHT
} SgEye;

typedef enum {
  SG_PROJ_ORTHO,
  SG_PROJ_PERSP
} SgProjection;

typedef enum {
  SG_LIGHT_OFF,
  SG_LIGHT_POINT,
  SG_LIGHT_INFINITE
} SgLightState;

typedef void (*SgDrawFunc) (void *);

typedef void (*SgErrorHandler) (SgError, char *);

#endif  /* _SG_TYPES_H_ */
