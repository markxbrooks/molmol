/*
************************************************************************
*
*   PSStereo.c - PostScript stereo plots
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ps/SCCS/s.PSStereo.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "ps_stereo.h"

#include <sg.h>
#include <sg_plot.h>

static const float EyeDist = 2.6f;
static const float ImgDist = 0.0f;

void
SgPSSetStereo(BOOL onOff)
{
}

void
SgPSSetEye(SgEye eye)
{
  float x, y, w, h;
  float imgSize, t, s;

  SgPlotOutputStr("trmtx setmatrix\n");

  SgGetViewport(&x, &y, &w, &h);

  imgSize = EyeDist - ImgDist;

  if (eye == SG_EYE_LEFT) {
    t = - 0.5f * (EyeDist + imgSize);
  } else {
    t = 0.5f * ImgDist;
  }

  t += x + 0.5f * w;
  s = imgSize / w;

  SgPlotOutputFloat(t);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat((y + 0.5f * h) * (1.0f - s));
  SgPlotOutputStr(" translate\n");


  SgPlotOutputFloat(s);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(s);
  SgPlotOutputStr(" scale\n");
}
