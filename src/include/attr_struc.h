/*
************************************************************************
*
*   attr_struc.h - attribute structure
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
*   Pathname of SCCS file     : /local/home/kor/molmol/include/SCCS/s.attr_struc.h
*   SCCS identification       : 1.9
*
************************************************************************
*/

#ifndef _ATTR_STRUC_H_
#define _ATTR_STRUC_H_

#include <attr_type.h>

#include <bool.h>
#include <dstr.h>
#include <mat_vec.h>
#include <sg_types.h>

#define LABEL_FORMAT_SIZE 16
#define SHADE_DEFAULT SG_SHADE_GOURAUD

typedef enum {
  AS_INVISIBLE,
  AS_SPHERE,
  AS_TETRAHEDRON
} AtomStyle;

typedef enum {
  BS_INVISIBLE,
  BS_LINE,
  BS_HALF_LINE,
  BS_CYLINDER,
  BS_HALF_CYLINDER,
  BS_NEON,
  BS_HALF_NEON,
  BS_CONE,
  BS_HALF_CONE
} BondStyle;

typedef enum {
  DS_INVISIBLE,
  DS_LINE,
  DS_VIOL,
  DS_CYLINDER,
  DS_NEON,
  DS_CONE
} DistStyle;

typedef enum {
  DL_INVISIBLE,
  DL_VAL,
  DL_LIMIT,
  DL_VIOL
} DistLabel;

struct AttrS {
  float colR, colG, colB;
  float secColR, secColG, secColB;
  float ambCoeff;
  float diffCoeff;
  float specCoeff;
  float shininess;
  float reflect;
  float alpha;
  float refract;
  char texture[SG_TEXTURE_NAME_SIZE];
  float textureScale;
  float bumpDepth;
  float bumpScale;
  AtomStyle atomStyle;
  BondStyle bondStyle;
  DistStyle distStyle;
  DistLabel distLabel;
  char labelFormat[LABEL_FORMAT_SIZE];
  float radius;
  float lineWidth;
  SgLineStyle lineStyle;
  short shadeModel;  /* -1 or one of SgShadeModel values */
  float fontSize;
  float nearClip;
};

struct MolAttrS {
  BOOL objDefined;
  SgObjId objId;
};

#endif  /* _ATTR_STRUC_H_ */
