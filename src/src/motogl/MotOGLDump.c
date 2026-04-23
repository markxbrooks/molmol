/*
************************************************************************
*
*   MotOGLDump.c - Motif/OpenGL image dump
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
*   Date of last modification : 01/07/07
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/motogl/SCCS/s.MotOGLDump.c
*   SCCS identification       : 1.15
*
************************************************************************
*/

#include "motogl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define BOOL X11_BOOL  /* hack to avoid naming conflict */
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <GL/gl.h>
#include <GL/glx.h>
#undef BOOL

#ifdef TIFF_SUPPORT
#include <tiffio.h>
#endif
#ifdef JPEG_SUPPORT
#ifndef PNG_SUPPORT
#include <setjmp.h>
#endif
#include <jpeglib.h>
#endif
#ifdef PNG_SUPPORT
#include <png.h>
#endif

#include "motogl_int.h"
#include <sg.h>
#include <sg_get.h>
#include <sg_attr.h>
#include <io_error.h>

/* X servers often grow bigger and bigger when allocating/freeing
   many pixmaps, so it's better to keep and reuse them if possible. */
#define KEEP_PIXMAP 1

typedef enum {
  DIF_TIFF,
  DIF_JPEG,
  DIF_PNG,
  DIF_UNKNOWN
} DumpImageFormat;

#ifdef JPEG_SUPPORT
typedef struct {
  struct jpeg_error_mgr pub;
  jmp_buf setjmpBuf;
} JpegErr, *JpegErrP;
#endif

static DumpImageFormat ImgFormat;
static FILE *ImgFileP;
static SgPlotOrient Orient;
static int PixmapW, PixmapH;
static int Quality;
static float Gamma;
static BOOL OutOfMemory;
static Display *Dpy;
static Pixmap XPix = 0;
static GLXPixmap GPix = 0;
static GLXContext OldCtx, Ctx;
static float OldVpX, OldVpY, OldVpW, OldVpH;

static void
destroyPixmap(void)
{
  glXDestroyGLXPixmap(Dpy, GPix);
  GPix = 0;
  XFreePixmap(Dpy, XPix);
  XPix = 0;
}

static int
xErrorHandler(Display *dpy, XErrorEvent *evtP)
{
  OutOfMemory = TRUE;

  return 0;
}

IORes
IOMotifOGLStartDump(char *format, char *fileName,
    SgPlotOrient orient, int w, int h, int quality, float gamma)
{
  Widget drawW = MotifOGLGetDrawW();
  XErrorHandler oldHandler;
  int attrList[20];
  int n;
  XVisualInfo *visP;
  GLfloat v[4];
  int i;

  ImgFormat = DIF_UNKNOWN;
#ifdef TIFF_SUPPORT
  if (strcmp(format, "TIFF") == 0)
    ImgFormat = DIF_TIFF;
#endif
#ifdef JPEG_SUPPORT
  if (strcmp(format, "JPEG") == 0)
    ImgFormat = DIF_JPEG;
#endif
#ifdef PNG_SUPPORT
  if (strcmp(format, "PNG") == 0)
    ImgFormat = DIF_PNG;
#endif

  if (ImgFormat == DIF_UNKNOWN) {
    IORaiseError("unsupported image format");
    return IO_RES_ERR;
  }

  ImgFileP = fopen(fileName, "w");
  if (ImgFileP == NULL) {
    IORaiseError("could not open output file");
    return IO_RES_ERR;
  }

#if KEEP_PIXMAP
  if (GPix != 0 && (w != PixmapW || h != PixmapH))
    destroyPixmap();
#endif

  Orient = orient;
  PixmapW = w;
  PixmapH = h;
  Quality = quality;
  Gamma = gamma;

  Dpy = XtDisplay(drawW);

  n = 0;
  attrList[n++] = GLX_RGBA;
  attrList[n++] = GLX_RED_SIZE; attrList[n++] = 4;
  attrList[n++] = GLX_GREEN_SIZE; attrList[n++] = 4;
  attrList[n++] = GLX_BLUE_SIZE; attrList[n++] = 4;
  attrList[n++] = GLX_DEPTH_SIZE; attrList[n++] = 1;
  attrList[n++] = None;
  visP = glXChooseVisual(Dpy,
      XScreenNumberOfScreen(XtScreen(drawW)), attrList);
  if (visP == NULL) {
    IORaiseError("no 24-bit true color visual available");
    return IO_RES_ERR;
  }

  /* catch BadAlloc error */
  OutOfMemory = FALSE;
  oldHandler = XSetErrorHandler(xErrorHandler);

  if (XPix == 0) {
    XPix = XCreatePixmap(Dpy, XtWindow(drawW), w, h, visP->depth);
    XSync(Dpy, FALSE);  /* error comes too late otherwise */
    if (OutOfMemory) {
      XPix = 0;
      XSetErrorHandler(oldHandler);
      IORaiseError("could not allocate Pixmap");
      return IO_RES_ERR;
    }
  }

  if (GPix == 0) {
    GPix = glXCreateGLXPixmap(Dpy, visP, XPix);
    XSync(Dpy, FALSE);
    XSetErrorHandler(oldHandler);
    if (OutOfMemory) {
      GPix = 0;
      XFreePixmap(Dpy, XPix);
      XPix = 0;
      IORaiseError("could not allocate Pixmap");
      return IO_RES_ERR;
    }
  }

  Ctx = glXCreateContext(Dpy, visP, NULL, False);
  if (Ctx == NULL) {
    destroyPixmap();
    IORaiseError("could not create rendering context");
    return IO_RES_ERR;
  }

  XFree((void *) visP);

  OldCtx = glXGetCurrentContext();
  (void) glXMakeCurrent(Dpy, GPix, Ctx);

  SgSetAttr();
  SgUpdateView();

  SgGetViewport(&OldVpX, &OldVpY, &OldVpW, &OldVpH);
  SgSetViewport(0.0f, 0.0f, (float) w, (float) h);

  for (i = 0; i < SG_FEATURE_NO; i++)
    SgSetFeature(i, SgGetFeature(i));

  /* SgOGLInit() is not called for new context! */
  glEnable(GL_NORMALIZE);
  v[0] = 1.0f;
  v[1] = 1.0f;
  v[2] = 1.0f;
  v[3] = 1.0f;
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, v);

  (void) MotifOGLPrepareFont(Dpy);

  return IO_RES_OK;
}

#ifdef JPEG_SUPPORT
static void
jpegErrHand(j_common_ptr jpegInfoP)
{
  JpegErrP errP = (JpegErrP) jpegInfoP->err;
  char buf[JMSG_LENGTH_MAX];

  errP->pub.format_message(jpegInfoP, buf);
  IORaiseError(buf);

  longjmp(errP->setjmpBuf, 1);
}
#endif

#ifdef PNG_SUPPORT
static void
pngErrHand(png_structp pngStrucP, char *errMsg)
{
  IORaiseError(errMsg);
  longjmp(png_jmpbuf(pngStrucP), 1);
}
#endif

static IORes
writeImg(void)
{
#ifdef TIFF_SUPPORT
  TIFF *tif;
  int rowsPerStrip;
#endif
#ifdef JPEG_SUPPORT
  struct jpeg_compress_struct jpegInfo;
  JpegErr jpegErr;
#endif
#ifdef PNG_SUPPORT
  png_structp pngStrucP;
  png_infop pngInfoP;
#endif
  int imgW, imgH;
  int bufSize, rowI, i;
  float *fBuf = NULL;
  unsigned char *cBuf = NULL;
  IORes res;

  if (Orient == SG_PLOT_NORMAL) {
    imgW = PixmapW;
    imgH = PixmapH;
    bufSize = 4 * ((3 * imgW + 3) / 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
  } else {
    imgW = PixmapH;
    imgH = PixmapW;
    bufSize = 3 * imgW;
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
  }

#ifdef TIFF_SUPPORT
  if (ImgFormat == DIF_TIFF) {
    tif = TIFFFdOpen(fileno(ImgFileP), "output file", "w");
    if (tif == NULL) {
      IORaiseError("could not create TIFF file");
      return IO_RES_ERR;
    }

    rowsPerStrip = (8 * 1024) / (3 * imgW);
    if (rowsPerStrip == 0)
      rowsPerStrip = 1;

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, imgW);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, imgH);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);

    if (Quality < 100)
      TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    else
      TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
    TIFFSetField(tif, TIFFTAG_DOCUMENTNAME, "MOLMOL Plot");
    TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "MOLMOL Plot");
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsPerStrip);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  }
#endif

#ifdef JPEG_SUPPORT
  if (ImgFormat == DIF_JPEG) {
    jpegInfo.err = jpeg_std_error(&jpegErr.pub);
    jpegErr.pub.error_exit = jpegErrHand;
    if (setjmp(jpegErr.setjmpBuf)) {
      if (cBuf != NULL)
	free(cBuf);
      if (fBuf != NULL)
	free(fBuf);
      jpeg_destroy_compress(&jpegInfo);
      return IO_RES_ERR;
    }
    jpeg_create_compress(&jpegInfo);
    jpeg_stdio_dest(&jpegInfo, ImgFileP);

    jpegInfo.image_width = imgW;
    jpegInfo.image_height = imgH;
    jpegInfo.input_components = 3;
    jpegInfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&jpegInfo);
    jpeg_set_quality(&jpegInfo, Quality, TRUE);
    jpeg_start_compress(&jpegInfo, TRUE);
  }
#endif

#ifdef PNG_SUPPORT
  if (ImgFormat == DIF_PNG) {
    pngStrucP = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	NULL, pngErrHand, pngErrHand);
    if (pngStrucP == NULL) {
      IORaiseError("could not create PNG file");
      return IO_RES_ERR;
    }

    pngInfoP = png_create_info_struct(pngStrucP);
    if (pngInfoP == NULL) {
      png_destroy_write_struct(&pngStrucP, NULL);
      IORaiseError("could not create PNG file");
      return IO_RES_ERR;
    }

    if (setjmp(png_jmpbuf(pngStrucP))) {
      if (cBuf != NULL)
	free(cBuf);
      if (fBuf != NULL)
	free(fBuf);
      png_destroy_write_struct(&pngStrucP, &pngInfoP);
      return IO_RES_ERR;
    }

    png_init_io(pngStrucP, ImgFileP);

    png_set_IHDR(pngStrucP, pngInfoP, imgW, imgH, 8, PNG_COLOR_TYPE_RGB, 0, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(pngStrucP, pngInfoP);
  }
#endif

  cBuf = malloc(bufSize * sizeof(*cBuf));
  if (Gamma != 1.0f)
    fBuf = malloc(bufSize * sizeof(*fBuf));

  res = IO_RES_OK;
  for (rowI = 0; rowI < imgH; rowI++) {
    if (Gamma == 1.0f) {
      if (Orient == SG_PLOT_NORMAL)
	glReadPixels(0, PixmapH - 1 - rowI, PixmapW, 1,
	    GL_RGB, GL_UNSIGNED_BYTE, cBuf);
      else
	glReadPixels(rowI, 0, 1, PixmapH,
	    GL_RGB, GL_UNSIGNED_BYTE, cBuf);
    } else {
      if (Orient == SG_PLOT_NORMAL)
	glReadPixels(0, PixmapH - 1 - rowI, PixmapW, 1,
	    GL_RGB, GL_FLOAT, fBuf);
      else
	glReadPixels(rowI, 0, 1, PixmapH,
	    GL_RGB, GL_FLOAT, fBuf);
      
      for (i = 0; i < 3 * imgW; i++)
	cBuf[i] = (int) (255.99f * powf(fBuf[i], Gamma));
    }

#ifdef TIFF_SUPPORT
    if (ImgFormat == DIF_TIFF)
      if (TIFFWriteScanline(tif, cBuf, rowI, 0) < 0) {
	IORaiseError("error while writing TIFF file");
	res = IO_RES_ERR;
	break;
      }
#endif

#ifdef JPEG_SUPPORT
    if (ImgFormat == DIF_JPEG)
      (void) jpeg_write_scanlines(&jpegInfo, &cBuf, 1);
#endif

#ifdef PNG_SUPPORT
    if (ImgFormat == DIF_PNG)
      png_write_row(pngStrucP, cBuf);
#endif
  }

  free(cBuf);
  if (Gamma != 1.0f)
    free(fBuf);

#ifdef TIFF_SUPPORT
  if (ImgFormat == DIF_TIFF) {
    TIFFFlushData(tif);
    TIFFClose(tif);
  }
#endif

#ifdef JPEG_SUPPORT
  if (ImgFormat == DIF_JPEG) {
    jpeg_finish_compress(&jpegInfo);
    jpeg_destroy_compress(&jpegInfo);
  }
#endif

#ifdef PNG_SUPPORT
  if (ImgFormat == DIF_PNG) {
    png_write_end(pngStrucP, pngInfoP);
    png_destroy_write_struct(&pngStrucP, &pngInfoP);
  }
#endif

  return res;
}

IORes
IOMotifOGLEndDump(void)
{
  IORes res;

  res = writeImg();

  (void) fclose(ImgFileP);

  (void) glXMakeCurrent(Dpy, XtWindow(MotifOGLGetDrawW()), OldCtx);
  SgSetAttr();
  SgSetViewport(OldVpX, OldVpY, OldVpW, OldVpH);

#if KEEP_PIXMAP
#else
  destroyPixmap();
#endif

  glXDestroyContext(Dpy, Ctx);

  return res;
}
