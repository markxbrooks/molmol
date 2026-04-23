/*
************************************************************************
*
*   ExAttr.c - Attr* commands
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExAttr.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_attr.h>

#include <stdio.h>
#include <string.h>

#include <dstr.h>
#include <pu.h>
#include <data_ent.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>

typedef struct {
  int entNo;
  AttrP attrP;
} AttrDescr;

static void
getAttr(AttrP attrP, AttrDescr *descrP)
{
  if (descrP->entNo == 0)
    descrP->attrP = attrP;
  else if (descrP->attrP != attrP)
    descrP->attrP = NULL;

  descrP->entNo++;
}

static void
getAttrAtom(DhAtomP atomP, void *clientData)
{
  getAttr(DhAtomGetAttr(atomP), clientData);
}

static void
getAttrBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getAttr(DhBondGetAttr(bondP), clientData);
}

static void
getAttrDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getAttr(DhDistGetAttr(distP), clientData);
}

static void
getAttrPrim(PrimObjP primP, void *clientData)
{
  getAttr(PrimGetAttr(primP), clientData);
}

ErrCode
ExAttr(char *cmd)
{
  AttrDescr descr;
  PropRefP refP;
  DataEntityType entType;
  AttrP attrP;
  PuTextWindow textW;
  char *name, buf[100], *str;
  char *atomStyle, *bondStyle, *distStyle, *distLabel;
  DSTR macro;

  descr.entNo = 0;
  descr.attrP = NULL;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  if (strcmp(cmd, "AttrAtom") == 0) {
    DhApplyAtom(refP, getAttrAtom, &descr);
    entType = DE_ATOM;
  } else if (strcmp(cmd, "AttrBond") == 0) {
    DhApplyBond(refP, getAttrBond, &descr);
    entType = DE_BOND;
  } else if (strcmp(cmd, "AttrDist") == 0) {
    DhApplyDist(refP, getAttrDist, &descr);
    entType = DE_DIST;
  } else if (strcmp(cmd, "AttrPrim") == 0) {
    PrimApply(PT_ALL, refP, getAttrPrim, &descr);
    entType = DE_PRIM;
  } else {
    descr.entNo = 1;
    descr.attrP = AttrGetInit();
    entType = DE_NONE;
  }

  if (descr.entNo == 0) {
    CipSetError("no item selected");
    return EC_ERROR;
  }

  attrP = descr.attrP;
  if (attrP == NULL) {
    CipSetError("not all selected items have same attributes");
    return EC_ERROR;
  }

  textW = PuCreateTextWindow(cmd);
  macro = DStrNew();
  name = cmd + 4;

  (void) sprintf(buf, "Values of attr %d:\n\n", AttrGetIndex(attrP));
  PuWriteStr(textW, buf);

  (void) sprintf(buf, "Color:           %5.3f %5.3f %5.3f\n",
      attrP->colR, attrP->colG, attrP->colB);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Color%s %5.3f %5.3f %5.3f\n",
      name, attrP->colR, attrP->colG, attrP->colB);
  DStrAppStr(macro, buf);
  if (entType == DE_DIST || entType == DE_PRIM) {
    (void) sprintf(buf, "Secondary Color: %5.3f %5.3f %5.3f\n",
	attrP->secColR, attrP->secColG, attrP->secColB);
    PuWriteStr(textW, buf);
    (void) sprintf(buf, "Tint%s %5.3f %5.3f %5.3f\n",
	name, attrP->secColR, attrP->secColG, attrP->secColB);
    DStrAppStr(macro, buf);
  }

  (void) sprintf(buf, "Ambient Coeff.:  %5.3f\n", attrP->ambCoeff);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Diffuse Coeff.:  %5.3f\n", attrP->diffCoeff);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Specular Coeff.: %5.3f\n", attrP->specCoeff);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Shininess:       %.1f\n", attrP->shininess);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Reflection:      %5.3f\n", attrP->reflect);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Opacity:         %5.3f\n", attrP->alpha);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Refraction:      %5.3f\n", attrP->refract);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Material%s %5.3f %5.3f %5.3f %.1f %5.3f %5.3f %5.3f\n",
      name, attrP->ambCoeff, attrP->diffCoeff, attrP->specCoeff,
      attrP->shininess, attrP->reflect, attrP->alpha, attrP->refract);
  DStrAppStr(macro, buf);

  (void) sprintf(buf, "Texture Name:    %s\n", attrP->texture);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Texture Scale:   %.2f\n", attrP->textureScale);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Bumpiness:       %5.3f\n", attrP->bumpDepth);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Bump Scale:      %.2f\n", attrP->bumpScale);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Texture%s '%s' %.2f %5.3f %.2f\n",
      name, attrP->texture, attrP->textureScale,
      attrP->bumpDepth, attrP->bumpScale);
  DStrAppStr(macro, buf);

  if (entType == DE_ATOM || entType == DE_NONE) {
    if (attrP->atomStyle == AS_INVISIBLE)
      atomStyle = "invisible";
    else if (attrP->atomStyle == AS_SPHERE)
      atomStyle = "sphere";
    else
      atomStyle = "tetrahedron";

    (void) sprintf(buf, "Atom Style:      %s\n", atomStyle);
    PuWriteStr(textW, buf);
    (void) sprintf(buf, "Atom Label:      %s\n", attrP->labelFormat);
    PuWriteStr(textW, buf);

    if (entType == DE_ATOM) {
      (void) sprintf(buf, "StyleAtom %s\n", atomStyle);
      DStrAppStr(macro, buf);
      (void) sprintf(buf, "LabelAtom '%s'\n", attrP->labelFormat);
      DStrAppStr(macro, buf);
    }
  } 
  
  if (entType == DE_BOND || entType == DE_NONE) {
    if (attrP->bondStyle == BS_INVISIBLE)
      bondStyle = "invisible";
    else if (attrP->bondStyle == BS_LINE)
      bondStyle = "line";
    else if (attrP->bondStyle == BS_HALF_LINE)
      bondStyle = "half_line";
    else if (attrP->bondStyle == BS_CYLINDER)
      bondStyle = "cylinder";
    else if (attrP->bondStyle == BS_HALF_CYLINDER)
      bondStyle = "half_cylinder";
    else if (attrP->bondStyle == BS_NEON)
      bondStyle = "neon";
    else if (attrP->bondStyle == BS_HALF_NEON)
      bondStyle = "half_neon";
    else if (attrP->bondStyle == BS_CONE)
      bondStyle = "cone";
    else
      bondStyle = "half_cone";

    (void) sprintf(buf, "Bond Style:      %s\n", bondStyle);
    PuWriteStr(textW, buf);

    if (entType == DE_BOND) {
      (void) sprintf(buf, "StyleBond %s\n", bondStyle);
      DStrAppStr(macro, buf);
    }
  } 
  
  if (entType == DE_DIST || entType == DE_NONE) {
    if (attrP->distStyle == DS_INVISIBLE)
      distStyle = "invisible";
    else if (attrP->distStyle == DS_LINE)
      distStyle = "line";
    else if (attrP->distStyle == DS_VIOL)
      distStyle = "viol";
    else if (attrP->distStyle == DS_CYLINDER)
      distStyle = "cylinder";
    else if (attrP->distStyle == DS_NEON)
      distStyle = "neon";
    else
      distStyle = "cone";

    (void) sprintf(buf, "Dist Style:      %s\n", distStyle);
    PuWriteStr(textW, buf);

    if (attrP->distLabel == DL_INVISIBLE)
      distLabel = "invisible";
    else if (attrP->distLabel == DL_VAL)
      distLabel = "val";
    else if (attrP->distLabel == DL_LIMIT)
      distLabel = "limit";
    else
      distLabel = "viol";

    (void) sprintf(buf, "Dist Label:      %s\n", distLabel);
    PuWriteStr(textW, buf);

    if (entType == DE_DIST) {
      (void) sprintf(buf, "StyleDist %s\n", distStyle);
      DStrAppStr(macro, buf);
      (void) sprintf(buf, "LabelDist %s\n", distLabel);
      DStrAppStr(macro, buf);
    }
  }

  (void) sprintf(buf, "Radius:          %.3f\n", attrP->radius);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Radius%s %.3f\n", name, attrP->radius);
  DStrAppStr(macro, buf);

  if (attrP->lineStyle == SG_LINE_SOLID)
    str = "solid";
  else
    str = "dashed";
  (void) sprintf(buf, "Line Style:      %s\n", str);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Line Width:      %.1f\n", attrP->lineWidth);
  PuWriteStr(textW, buf);
  (void) sprintf(buf, "Line%s %s %.1f\n", name, str, attrP->lineWidth);
  DStrAppStr(macro, buf);

  if (entType != DE_NONE) {
    if (attrP->shadeModel == -1)
      str = "none";
    else if (attrP->shadeModel == SG_SHADE_FLAT)
      str = "flat";
    else if (attrP->shadeModel == SG_SHADE_GOURAUD)
      str = "gouraud";
    else if (attrP->shadeModel == SG_SHADE_PHONG)
      str = "phong";
    else if (attrP->shadeModel == SG_SHADE_DOTS)
      str = "dots";
    else
      str = "lines";
    (void) sprintf(buf, "Shading:         %s\n", str);
    PuWriteStr(textW, buf);
    (void) sprintf(buf, "Shade%s %s\n", name, str);
    DStrAppStr(macro, buf);
  }

  if (entType == DE_PRIM) {
    (void) sprintf(buf, "Font Size:       %5.3f\n", attrP->fontSize);
    PuWriteStr(textW, buf);
    (void) sprintf(buf, "SizeText %5.3f\n", attrP->fontSize);
    DStrAppStr(macro, buf);
  }

  if (entType == DE_NONE) {
    (void) sprintf(buf, "StyleInit %s %s %s\n",
	atomStyle, bondStyle, distStyle);
    DStrAppStr(macro, buf);

    (void) sprintf(buf, "LabelInit '%s' %s\n", attrP->labelFormat, distLabel);
    DStrAppStr(macro, buf);

    AttrReturn(attrP);
  }

  PuWriteStr(textW, "\nMacro:\n\n");
  PuWriteStr(textW, DStrToStr(macro));

  DStrFree(macro);

  return EC_OK;
}
