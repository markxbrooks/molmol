/*
************************************************************************
*
*   SgTransf.c - Sg wrappers for transformation functions
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/sgdev/SCCS/s.SgTransf.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_dev.h>

void
SgRotateX(float ang)
{
  CurrSgDevP->rotateX(ang);
}

void
SgRotateY(float ang)
{
  CurrSgDevP->rotateY(ang);
}

void
SgRotateZ(float ang)
{
  CurrSgDevP->rotateZ(ang);
}

void
SgTranslate(float dx[3])
{
  CurrSgDevP->translate(dx);
}

void
SgScale(float sx, float sy, float sz)
{
  CurrSgDevP->scale(sx, sy, sz);
}

void
SgSetMatrix(float m[4][4])
{
  CurrSgDevP->setMatrix(m);
}

void
SgMultMatrix(float m[4][4])
{
  CurrSgDevP->multMatrix(m);
}

void
SgPushMatrix(void)
{
  CurrSgDevP->pushMatrix();
}

void
SgPopMatrix(void)
{
  CurrSgDevP->popMatrix();
}
