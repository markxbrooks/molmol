/*
************************************************************************
*
*   ExPlot.c - plot commands
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdmisc/SCCS/s.ExPlot.c
*   SCCS identification       : 1.28
*
************************************************************************
*/

#include <cmd_misc.h>

#include <stdio.h>
#include <string.h>

#include <sg.h>
#include <io.h>
#include <par_names.h>
#include <par_hand.h>
#include <arg.h>
#include <curr_dir.h>
#include <graph_draw.h>

#define DEFAULT_UNIT "cm"

/* inch sizes */
#define DEFAULT_PAPER_WIDTH_I 8.5f
#define DEFAULT_PAPER_HEIGHT_I 11.0f
#define DEFAULT_PLOT_WIDTH_I 7.0f
#define DEFAULT_PLOT_HEIGHT_I 0.0f

/* metric sizes (in inch) */
#define DEFAULT_PAPER_WIDTH_M (21.0f / 2.54f)
#define DEFAULT_PAPER_HEIGHT_M (29.7f / 2.54f)
#define DEFAULT_PLOT_WIDTH_M (18.0f / 2.54f)
#define DEFAULT_PLOT_HEIGHT_M 0.0f

#define DEFAULT_RESOL 1000
#define DEFAULT_QUALITY 75

typedef struct {
  char *name;
  float fact;
} UnitDescr;

static UnitDescr UnitTab[] = {
  {"cm", 2.54f},
  {"mm", 25.4f},
  {"inch", 1.0f}
};

#define UNIT_NO (sizeof(UnitTab) / sizeof(UnitTab[0]))

static char *
getUnit(void)
{
  if (ParDefined(PN_PLOT_UNIT))
    return ParGetStrVal(PN_PLOT_UNIT);
  else
    return DEFAULT_UNIT;
}

static int
getUnitInd(char *name)
{
  int i;

  for (i = 0; i < UNIT_NO; i++)
    if (strcmp(name, UnitTab[i].name) == 0)
      return i;
  
  return 0;
}

#define ARG_NUM 1

ErrCode
ExPlotUnit(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[UNIT_NO];
  ErrCode errCode;
  int oldVal, i;

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  oldVal = getUnitInd(getUnit());

  arg[0].prompt = "Unit";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = UNIT_NO;
  arg[0].v.intVal = oldVal;
  
  for (i = 0; i < UNIT_NO; i++) {
    enumEntry[i].str = UnitTab[i].name;
    enumEntry[i].onOff = FALSE;
  }
  
  enumEntry[oldVal].onOff = TRUE;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  if (arg[0].v.intVal != oldVal) {
    ParSetStrVal(PN_PLOT_UNIT, UnitTab[arg[0].v.intVal].name);
    ParUndefine(PN_PAPER_WIDTH);
    ParUndefine(PN_PAPER_HEIGHT);
  }

  ArgCleanup(arg, ARG_NUM);

  return EC_OK;
}

static float
getPaperW(void)
{
  if (ParDefined(PN_PAPER_WIDTH))
    return (float) ParGetDoubleVal(PN_PAPER_WIDTH);
  else if (strcmp(getUnit(), "inch") == 0)
    return DEFAULT_PAPER_WIDTH_I;
  else
    return DEFAULT_PAPER_WIDTH_M;
}

static float
getPaperH(void)
{
  if (ParDefined(PN_PAPER_HEIGHT))
    return (float) ParGetDoubleVal(PN_PAPER_HEIGHT);
  else if (strcmp(getUnit(), "inch") == 0)
    return DEFAULT_PAPER_HEIGHT_I;
  else
    return DEFAULT_PAPER_HEIGHT_M;
}

static float
getPlotW(void)
{
  if (ParDefined(PN_PLOT_WIDTH))
    return (float) ParGetDoubleVal(PN_PLOT_WIDTH);
  else if (strcmp(getUnit(), "inch") == 0)
    return DEFAULT_PLOT_WIDTH_I;
  else
    return DEFAULT_PLOT_WIDTH_M;
}

static float
getPlotH(void)
{
  if (ParDefined(PN_PLOT_HEIGHT))
    return (float) ParGetDoubleVal(PN_PLOT_HEIGHT);
  else if (strcmp(getUnit(), "inch") == 0)
    return DEFAULT_PLOT_HEIGHT_I;
  else
    return DEFAULT_PLOT_HEIGHT_M;
}

#undef ARG_NUM
#define ARG_NUM 9
#define OPT_NUM 4

ErrCode
ExPlotPar(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[OPT_NUM];
  ErrCode errCode;
  char prompt0[30], prompt1[30], prompt2[30], prompt3[30];
  char *unit;
  float fact, plotW, plotH;
  int i;

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;
  arg[4].type = AT_INT;
  arg[5].type = AT_MULT_ENUM;
  arg[6].type = AT_INT;
  arg[7].type = AT_DOUBLE;
  arg[8].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  unit = getUnit();

  (void) sprintf(prompt0, "Paper Width (%s)", unit);
  arg[0].prompt = prompt0;
  arg[0].v.doubleVal = getPaperW();

  (void) sprintf(prompt1, "Paper Height (%s)", unit);
  arg[1].prompt = prompt1;
  arg[1].v.doubleVal = getPaperH();

  (void) sprintf(prompt2, "Plot Width (%s)", unit);
  arg[2].prompt = prompt2;
  arg[2].v.doubleVal = getPlotW();

  (void) sprintf(prompt3, "Plot Height (%s)", unit);
  arg[3].prompt = prompt3;
  arg[3].v.doubleVal = getPlotH();

  fact = UnitTab[getUnitInd(unit)].fact;

  for (i = 0; i < 4; i++)
    arg[i].v.doubleVal *= fact;

  arg[4].prompt = "Resolution";
  if (ParDefined(PN_PLOT_RESOL))
    arg[4].v.intVal = ParGetIntVal(PN_PLOT_RESOL);
  else
    arg[4].v.intVal = DEFAULT_RESOL;

  arg[5].prompt = "Options";
  arg[5].u.enumD.entryP = enumEntry;
  arg[5].u.enumD.n = OPT_NUM;

  enumEntry[0].str = "Stereo";
  if (ParDefined(PN_PLOT_STEREO))
    enumEntry[0].onOff = ParGetIntVal(PN_PLOT_STEREO);
  else
    enumEntry[0].onOff = FALSE;

  enumEntry[1].str = "Shading";
  if (ParDefined(PN_PLOT_SHADE))
    enumEntry[1].onOff = ParGetIntVal(PN_PLOT_SHADE);
  else
    enumEntry[1].onOff = TRUE;

  enumEntry[2].str = "Hidden Surface Elimination";
  if (ParDefined(PN_PLOT_HIDDEN))
    enumEntry[2].onOff = ParGetIntVal(PN_PLOT_HIDDEN);
  else
    enumEntry[2].onOff = TRUE;

  enumEntry[3].str = "Rotate 90";
  if (ParDefined(PN_PLOT_ORIENT))
    enumEntry[3].onOff = (ParGetIntVal(PN_PLOT_ORIENT) == SG_PLOT_ROT90);
  else
    enumEntry[3].onOff = FALSE;

  arg[6].prompt = "Precision";
  if (ParDefined(PN_PLOT_PREC))
    arg[6].v.intVal = ParGetIntVal(PN_PLOT_PREC);
  else
    arg[6].v.intVal = ParGetIntVal(PN_DRAW_PREC);

  arg[7].prompt = "Gamma";
  if (ParDefined(PN_PLOT_GAMMA))
    arg[7].v.doubleVal = ParGetDoubleVal(PN_PLOT_GAMMA);
  else
    arg[7].v.doubleVal = 1.0;

  arg[8].prompt = "JPEG Quality";
  if (ParDefined(PN_PLOT_QUALITY))
    arg[8].v.intVal = ParGetIntVal(PN_PLOT_QUALITY);
  else
    arg[8].v.intVal = DEFAULT_QUALITY;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  for (i = 0; i < 4; i++)
    arg[i].v.doubleVal /= fact;

  ParSetDoubleVal(PN_PAPER_WIDTH, arg[0].v.doubleVal);
  ParSetDoubleVal(PN_PAPER_HEIGHT, arg[1].v.doubleVal);
  ParSetIntVal(PN_PLOT_RESOL, arg[4].v.intVal);
  ParSetIntVal(PN_PLOT_PREC, arg[6].v.intVal);
  ParSetDoubleVal(PN_PLOT_GAMMA, arg[7].v.doubleVal);
  ParSetIntVal(PN_PLOT_QUALITY, arg[8].v.intVal);

  plotW = (float) arg[2].v.doubleVal;
  if (plotW < 0.0f)
    plotW = 0.0f;

  plotH = (float) arg[3].v.doubleVal;
  if (plotH < 0.0f)
    plotH = 0.0f;

  ParSetDoubleVal(PN_PLOT_WIDTH, plotW);
  ParSetDoubleVal(PN_PLOT_HEIGHT, plotH);

  ParSetIntVal(PN_PLOT_STEREO, enumEntry[0].onOff);
  ParSetIntVal(PN_PLOT_SHADE, enumEntry[1].onOff);
  ParSetIntVal(PN_PLOT_HIDDEN, enumEntry[2].onOff);
  if (enumEntry[3].onOff)
    ParSetIntVal(PN_PLOT_ORIENT, SG_PLOT_ROT90);
  else
    ParSetIntVal(PN_PLOT_ORIENT, SG_PLOT_NORMAL);

  ArgCleanup(arg, ARG_NUM);

  return EC_OK;
}

ErrCode
ExPlot(char *cmd)
{
  DSTR fileName;
  ErrCode errCode;
  char *pattern, *dev;
  float paperW, paperH;
  float plotW, plotH;
  float vpX, vpY, vpW, vpH;
  SgPlotOrient orient;
  float gamma;
  int oldPrec, prec;
  StereoMode oldMode;
  BOOL oldStereo;
  SgRes res;

  if (strcmp(cmd, "PlotPov") == 0)
    pattern = "*.pov";
  else if (strcmp(cmd, "PlotMpov") == 0)
    pattern = "*.pov";
  else if (strcmp(cmd, "PlotRib") == 0)
    pattern = "*.rib";
  else if (strcmp(cmd, "PlotVrml1") == 0)
    pattern = "*.wrl";
  else if (strcmp(cmd, "PlotVrml2") == 0)
    pattern = "*.wrl";
  else if (strcmp(cmd, "PlotPs") == 0)
    pattern = "*.ps";
  else
    pattern = "*.fm";

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), pattern, FALSE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  if (strcmp(cmd, "PlotPov") == 0)
    dev = "POV";
  else if (strcmp(cmd, "PlotMpov") == 0)
    dev = "MPOV";
  else if (strcmp(cmd, "PlotRib") == 0)
    dev = "RIB";
  else if (strcmp(cmd, "PlotVrml1") == 0)
    dev = "VRML1";
  else if (strcmp(cmd, "PlotVrml2") == 0)
    dev = "VRML2";
  else if (strcmp(cmd, "PlotPs") == 0)
    dev = "CPS";
  else if (strcmp(cmd, "PlotFm3") == 0)
    dev = "FM3";
  else
    dev = "FM4";

  paperW = getPaperW();
  paperH = getPaperH();
  plotW = getPlotW();
  plotH = getPlotH();

  if (plotW == 0.0f && plotH == 0.0f)
    plotW = DEFAULT_PLOT_WIDTH_M;

  SgGetViewport(&vpX, &vpY, &vpW, &vpH);

  if (plotW == 0.0f) {
    plotW = plotH * vpW / vpH;
  } else if (plotH == 0.0f) {
    plotH = plotW * vpH / vpW;
  } else if (plotW * vpH < plotH * vpW) {
    plotH = plotW * vpH / vpW;
  }

  if (ParDefined(PN_PLOT_ORIENT))
    orient = ParGetIntVal(PN_PLOT_ORIENT);
  else
    orient = SG_PLOT_NORMAL;

  if (ParDefined(PN_PLOT_GAMMA))
    gamma = (float) ParGetDoubleVal(PN_PLOT_GAMMA);
  else
    gamma = 1.0f;

  res = SgStartPlot(dev, DStrToStr(fileName), orient, gamma,
      0.5f * (paperW - plotW), 0.5f * (paperH - plotH),
      plotW, plotH);

  DStrFree(fileName);

  if (res != SG_RES_OK) {
    CipSetError("could not open file");
    return EC_ERROR;
  }

  if (ParDefined(PN_PLOT_PREC)) {
    oldPrec = ParGetIntVal(PN_DRAW_PREC);
    prec = ParGetIntVal(PN_PLOT_PREC);
    ParSetIntVal(PN_DRAW_PREC, prec);
    SgSetPrecision(prec);
  }

  oldMode = ParGetIntVal(PN_STEREO_MODE);
  oldStereo = SgGetStereo();

  if (ParDefined(PN_PLOT_STEREO) && ParGetIntVal(PN_PLOT_STEREO)) {
    ParSetIntVal(PN_STEREO_MODE, SM_HARDWARE);
    SgSetStereo(TRUE);
  }

  if (ParDefined(PN_PLOT_SHADE) && ParGetIntVal(PN_PLOT_SHADE) == 0) {
    SgSetFeature(SG_FEATURE_BACKFACE, TRUE);
    SgSetFeature(SG_FEATURE_DEPTH_SORT, FALSE);
    SgSetFeature(SG_FEATURE_HIDDEN, FALSE);
    SgSetFeature(SG_FEATURE_SHADE, FALSE);
  } else {
    SgSetFeature(SG_FEATURE_FAST_SPHERE, FALSE);
    if (! ParGetIntArrVal(PN_RENDERING, SG_FEATURE_TWO_SIDE))
      SgSetFeature(SG_FEATURE_BACKFACE, TRUE);
    SgSetFeature(SG_FEATURE_DEPTH_SORT, TRUE);
    SgSetFeature(SG_FEATURE_HIDDEN,
        ParDefined(PN_PLOT_HIDDEN) && ParGetIntVal(PN_PLOT_HIDDEN) == 1);
    SgSetFeature(SG_FEATURE_SHADE, TRUE);
  }

  GraphDraw();
  SgEndPlot();

  if (ParDefined(PN_PLOT_PREC)) {
    ParSetIntVal(PN_DRAW_PREC, oldPrec);
    SgSetPrecision(oldPrec);
  }

  ParSetIntVal(PN_STEREO_MODE, oldMode);
  SgSetStereo(oldStereo);

  SgSetFeature(SG_FEATURE_FAST_SPHERE,
      ParGetIntArrVal(PN_RENDERING, SG_FEATURE_FAST_SPHERE));
  SgSetFeature(SG_FEATURE_BACKFACE,
      ParGetIntArrVal(PN_RENDERING, SG_FEATURE_BACKFACE));
  SgSetFeature(SG_FEATURE_DEPTH_SORT,
      ParGetIntArrVal(PN_RENDERING, SG_FEATURE_DEPTH_SORT));
  SgSetFeature(SG_FEATURE_HIDDEN,
      ParGetIntArrVal(PN_RENDERING, SG_FEATURE_HIDDEN));
  SgSetFeature(SG_FEATURE_SHADE,
      ParGetIntArrVal(PN_RENDERING, SG_FEATURE_SHADE));

  return EC_OK;
}

ErrCode
ExPlotDump(char *cmd)
{
  DSTR fileName;
  ErrCode errCode;
  char *pattern, *format;
  int resolX, resolY;
  float vpX, vpY, vpW, vpH;
  SgPlotOrient orient;
  float gamma;
  int quality;
  int oldPrec, prec;
  IORes res;

  if (strcmp(cmd, "PlotTiff") == 0) {
    pattern = "*.tif";
    format = "TIFF";
  } else if (strcmp(cmd, "PlotJpeg") == 0) {
    pattern = "*.jpg";
    format = "JPEG";
  } else if (strcmp(cmd, "PlotPng") == 0) {
    pattern = "*.png";
    format = "PNG";
  } else if (strcmp(cmd, "PlotBmp") == 0) {
    pattern = "*.bmp";
    format = "BMP";
  } else {
    pattern = "*.emf";
    format = "EMF";
  }

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), pattern, FALSE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  if (ParDefined(PN_PLOT_RESOL))
    resolX = ParGetIntVal(PN_PLOT_RESOL);
  else
    resolX = DEFAULT_RESOL;

  SgGetViewport(&vpX, &vpY, &vpW, &vpH);

  resolY = (int) (resolX * vpH / vpW);

  if (ParDefined(PN_PLOT_ORIENT))
    orient = ParGetIntVal(PN_PLOT_ORIENT);
  else
    orient = SG_PLOT_NORMAL;

  if (ParDefined(PN_PLOT_GAMMA))
    gamma = (float) ParGetDoubleVal(PN_PLOT_GAMMA);
  else
    gamma = 1.0f;

  if (ParDefined(PN_PLOT_QUALITY))
    quality = ParGetIntVal(PN_PLOT_QUALITY);
  else
    quality = DEFAULT_QUALITY;

  res = IOStartDump(format, DStrToStr(fileName),
      orient, resolX, resolY, quality, gamma);

  DStrFree(fileName);

  if (res != IO_RES_OK)
    /* error message is set by IOStartDump() */
    return EC_ERROR;

  if (ParDefined(PN_PLOT_PREC)) {
    oldPrec = ParGetIntVal(PN_DRAW_PREC);
    prec = ParGetIntVal(PN_PLOT_PREC);
    ParSetIntVal(PN_DRAW_PREC, prec);
    SgSetPrecision(prec);
  }

  GraphDraw();
  res = IOEndDump();

  if (ParDefined(PN_PLOT_PREC)) {
    ParSetIntVal(PN_DRAW_PREC, oldPrec);
    SgSetPrecision(oldPrec);
  }

  if (res != IO_RES_OK)
    return EC_ERROR;

  return EC_OK;
}
