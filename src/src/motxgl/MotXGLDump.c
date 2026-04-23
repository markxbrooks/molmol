/*
************************************************************************
*
*   MotXGLDump.c - Motif/XGL image dump
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
*   Date of last modification : 96/06/21
*   Pathname of SCCS file     : /sgiext/molmol/src/motxgl/SCCS/s.MotXGLDump.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "motxgl.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xgl/xgl.h>

#ifdef TIFF_SUPPORT
#include <tiffio.h>
#endif

#include "motxgl_int.h"
#include <sg.h>
#include <sg_get.h>
#include <sg_xgl_p.h>
#include <io_error.h>

static FILE *TiffFileP;
static SgPlotOrient Orient;
static int ImgW, ImgH;
static float Gamma;
static Xgl_win_ras OldRas;
static Xgl_mem_ras Ras;
static float OldVpX, OldVpY, OldVpW, OldVpH;

IORes
IOMotifXGLStartDump(char *format, char *fileName,
    SgPlotOrient orient, int w, int h, int quality, float gamma)
{
#ifndef TIFF_SUPPORT
  IORaiseError("unsupported image format");
  return IO_RES_ERR;
#endif

  if (strcmp(format, "TIFF") != 0) {
    IORaiseError("unsupported image format");
    return IO_RES_ERR;
  }

  TiffFileP = fopen(fileName, "w");
  if (TiffFileP == NULL) {
    IORaiseError("could not open output file");
    return IO_RES_ERR;
  }

  Orient = orient;
  ImgW = w;
  ImgH = h;
  Gamma = gamma;

  Ras = xgl_object_create(MotifXGLGetSysState(), XGL_MEM_RAS, NULL,
      XGL_DEV_COLOR_TYPE, XGL_COLOR_RGB,
      XGL_RAS_DEPTH, 32,
      XGL_RAS_WIDTH, w,
      XGL_RAS_HEIGHT, h,
      NULL);

  if (Ras == NULL) {
    IORaiseError("could not create raster");
    return IO_RES_ERR;
  }

  OldRas = MotifXGLGetRaster();
  XGLSetRaster(Ras);
  /* z-buffer value must be set again after different raster
     is associated with context */
  if (SgGetFeature(SG_FEATURE_HIDDEN))
    SgSetFeature(SG_FEATURE_HIDDEN, TRUE);

  SgGetViewport(&OldVpX, &OldVpY, &OldVpW, &OldVpH);
  SgSetViewport(0.0, 0.0, w, h);

  return IO_RES_OK;
}

static IORes
writeTiff(void)
{
#ifdef TIFF_SUPPORT
  TIFF *tif;
  int tiffW, tiffH, rowsPerStrip;
  Xgl_usgn32 *imgP, pix;
  unsigned char *buf;
  float fact;
  IORes res;
  int rowI, i;

  tif = TIFFFdOpen(fileno(TiffFileP), "output file", "w");
  if (tif == NULL) {
    IORaiseError("could not create TIFF file");
    return IO_RES_ERR;
  }

  if (Orient == SG_PLOT_NORMAL) {
    tiffW = ImgW;
    tiffH = ImgH;
  } else {
    tiffW = ImgH;
    tiffH = ImgW;
  }

  rowsPerStrip = (8 * 1024) / (3 * tiffW);
  if (rowsPerStrip == 0)
    rowsPerStrip = 1;

  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, tiffW);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, tiffH);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
  TIFFSetField(tif, TIFFTAG_DOCUMENTNAME, "MOLMOL Plot");
  TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "MOLMOL Plot");
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsPerStrip);
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  xgl_object_get(Ras, XGL_MEM_RAS_IMAGE_BUFFER_ADDR, &imgP);

  buf = malloc(3 * tiffW * sizeof(*buf));
  fact = 1.0 / 255.0;

  res = IO_RES_OK;
  for (rowI = 0; rowI < tiffH; rowI++) {
    if (Orient == SG_PLOT_NORMAL)
      for (i = 0; i < ImgW; i++) {
	pix = imgP[rowI * ImgW + i];
	buf[3 * i] = pix & 0xff;
	buf[3 * i + 1] = (pix & 0xff00) >> 8;
	buf[3 * i + 2] = (pix & 0xff0000) >> 16;
      }
    else
      for (i = 0; i < ImgH; i++) {
	pix = imgP[(ImgH - 1 - i) * ImgW + rowI];
	buf[3 * i] = pix & 0xff;
	buf[3 * i + 1] = (pix & 0xff00) >> 8;
	buf[3 * i + 2] = (pix & 0xff0000) >> 16;
      }

    if (Gamma != 1.0)
      for (i = 0; i < 3 * tiffW; i++)
	buf[i] = (int) (255.99 * powf(fact * buf[i], Gamma));

    if (TIFFWriteScanline(tif, buf, rowI, 0) < 0) {
      IORaiseError("error while writing TIFF file");
      res = IO_RES_ERR;
      break;
    }
  }

  free(buf);

  TIFFFlushData(tif);
  TIFFClose(tif);

  return res;
#else
  return IO_RES_ERR;
#endif
}

IORes
IOMotifXGLEndDump(void)
{
  IORes res;

  res = writeTiff();

  (void) fclose(TiffFileP);

  XGLSetRaster(OldRas);
  /* z-buffer value must be set again after different raster
     is associated with context */
  if (SgGetFeature(SG_FEATURE_HIDDEN))
    SgSetFeature(SG_FEATURE_HIDDEN, TRUE);

  SgSetViewport(OldVpX, OldVpY, OldVpW, OldVpH);

  xgl_object_destroy(Ras);

  return res;
}
