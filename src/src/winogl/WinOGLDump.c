/*
************************************************************************
*
*   WinOGLDump.c - Windows/OpenGL image dump
*
*   Copyright (c) 1996-2001
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/winogl/SCCS/s.WinOGLDump.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include "winogl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#ifdef TIFF_SUPPORT
#include <tiffio.h>
#endif
#ifdef JPEG_SUPPORT
#include <setjmp.h>
#include <jpeglib.h>
#endif
#ifdef PNG_SUPPORT
#include <png.h>
#include <pngpriv.h>
#endif

#include "winogl_int.h"
#include <sg.h>
#include <sg_get.h>
#include <sg_attr.h>
#include <io_error.h>

#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)

typedef enum {
  DIF_TIFF,
  DIF_JPEG,
  DIF_PNG,
  DIF_BMP,
  DIF_EMF,
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
static HANDLE ImgFileHandle;
static SgPlotOrient Orient;
static int PixmapW, PixmapH;
static int Quality;
static float Gamma;
static HBITMAP Bitmap;
static HGLRC OldCtx, Ctx;
static HDC OldDC, DumpDC;
static float OldVpX, OldVpY, OldVpW, OldVpH;

IORes
IOWinOGLStartDump(char *format, char *fileName,
    SgPlotOrient orient, int w, int h, int quality, float gamma)
{
  BITMAPINFOHEADER bih;
  PIXELFORMATDESCRIPTOR pfd;
  int depth;
  int pixelFormat;
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
  if (strcmp(format, "BMP") == 0)
    ImgFormat = DIF_BMP;
  if (strcmp(format, "EMF") == 0)
    ImgFormat = DIF_EMF;

  if (ImgFormat == DIF_UNKNOWN) {
    IORaiseError("unsupported image format");
    return IO_RES_ERR;
  }

  Orient = orient;
  PixmapW = w;
  PixmapH = h;
  Quality = quality;
  Gamma = gamma;

  OldCtx = wglGetCurrentContext();
  OldDC = wglGetCurrentDC();

  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

  if (ImgFormat == DIF_EMF) {
    DumpDC = CreateEnhMetaFile(NULL, fileName, NULL, "MOLMOL\0EMF Plot\0");
    if (DumpDC == NULL) {
      IORaiseError("could not open metafile");
      return IO_RES_ERR;
    }

    pfd.dwFlags = PFD_DRAW_TO_WINDOW;
    depth = 24;
    pfd.cColorBits = 24;
  } else {
    if (ImgFormat == DIF_TIFF) {
      ImgFileHandle = CreateFile(fileName, GENERIC_WRITE, 0, NULL,
          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if (ImgFileHandle == INVALID_HANDLE_VALUE) {
        IORaiseError("could not open output file");
        return IO_RES_ERR;
      }
    } else {
      ImgFileP = fopen(fileName, "wb");
      if (ImgFileP == NULL) {
        IORaiseError("could not open output file");
        return IO_RES_ERR;
      }
    }

    DumpDC = CreateCompatibleDC(OldDC);

    depth = GetDeviceCaps(OldDC, PLANES) *
        GetDeviceCaps(OldDC, BITSPIXEL);

    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = w;
    bih.biHeight = h;
    bih.biPlanes = 1;
    bih.biBitCount = depth;
    if (bih.biBitCount > 24)
      bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = WIDTHBYTES(w * bih.biBitCount) * h;
    Bitmap = CreateDIBitmap(OldDC, &bih, 0, NULL, NULL, DIB_RGB_COLORS);
    if (Bitmap == NULL) {
      IORaiseError("could not allocate Pixmap");
      return IO_RES_ERR;
    }

    SelectObject(DumpDC, Bitmap);

    pfd.dwFlags = PFD_DRAW_TO_BITMAP;  
  }

  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags |= PFD_SUPPORT_OPENGL;
  pfd.dwLayerMask = PFD_MAIN_PLANE;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = depth;
  pfd.cDepthBits = 16;
  pfd.cAccumBits = 0;
  pfd.cStencilBits = 0;
 
  pixelFormat = ChoosePixelFormat(DumpDC, &pfd);
  if (pixelFormat == 0) {
    IORaiseError("PIXELFORMAT not found");
    return IO_RES_ERR;
  }

  DescribePixelFormat(DumpDC, pixelFormat,
      sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  if (! SetPixelFormat(DumpDC, pixelFormat, &pfd)) {
    IORaiseError("could not set PIXELFORMAT");
    return IO_RES_ERR;
  }

  Ctx = wglCreateContext(DumpDC);
  if (Ctx == NULL) {
    IORaiseError("could not create rendering context");
    return IO_RES_ERR;
  }

  wglMakeCurrent(DumpDC, Ctx);

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

  (void) WinOGLPrepareFont(DumpDC);

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
  longjmp(pngStrucP->longjmp_buffer, 1);
}
#endif

static IORes
writeImg(void)
{
  GLenum pixFormat;
  int rowStart, rowEnd, rowInc, rowI;
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
  BITMAPFILEHEADER bfh;
  BITMAPINFOHEADER bih;
  int imgW, imgH;
  int bufSize, i;
  float *fBuf = NULL;
  unsigned char *cBuf = NULL;
  IORes res;

  if (Orient == SG_PLOT_NORMAL) {
    imgW = PixmapW;
    imgH = PixmapH;
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
  } else {
    imgW = PixmapH;
    imgH = PixmapW;
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
  }

  bufSize = 4 * ((3 * imgW + 3) / 4);

  pixFormat = GL_RGB;
  rowStart = 0;
  rowEnd = imgH;
  rowInc = 1;

#ifdef TIFF_SUPPORT
  if (ImgFormat == DIF_TIFF) {
    tif = TIFFFdOpen((int) ImgFileHandle, "output file", "wb");
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

    if (setjmp(pngStrucP->longjmp_buffer)) {
      if (cBuf != NULL)
	free(cBuf);
      if (fBuf != NULL)
	free(fBuf);
      png_destroy_write_struct(&pngStrucP, &pngInfoP);
      return IO_RES_ERR;
    }

    png_init_io(pngStrucP, ImgFileP);

    pngInfoP->width = imgW;
    pngInfoP->height = imgH;
    pngInfoP->bit_depth = 8;
    pngInfoP->color_type = PNG_COLOR_TYPE_RGB;
    png_write_info(pngStrucP, pngInfoP);
  }
#endif

  if (ImgFormat == DIF_BMP) {
    pixFormat = GL_BGR_EXT;
    rowStart = imgH - 1;
    rowEnd = -1;
    rowInc = -1;

    bfh.bfType = 0x4d42;
    bfh.bfSize = sizeof(bfh);
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
    (void) fwrite(&bfh, sizeof(bfh), 1, ImgFileP);

    bih.biSize = sizeof(bih);
    bih.biWidth = imgW;
    bih.biHeight = imgH;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = bufSize * imgH;
    bih.biXPelsPerMeter = 7874;  /* 200 dpi */
    bih.biYPelsPerMeter = 7874;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;
    (void) fwrite(&bih, sizeof(bih), 1, ImgFileP);
  }

  cBuf = malloc(bufSize * sizeof(*cBuf));
  if (Gamma != 1.0f)
    fBuf = malloc(bufSize * sizeof(*fBuf));

  res = IO_RES_OK;
  for (rowI = rowStart; rowI != rowEnd; rowI += rowInc) {
    if (Gamma == 1.0f) {
      if (Orient == SG_PLOT_NORMAL)
	glReadPixels(0, PixmapH - 1 - rowI, PixmapW, 1,
	    pixFormat, GL_UNSIGNED_BYTE, cBuf);
      else
	glReadPixels(rowI, 0, 1, PixmapH,
	    pixFormat, GL_UNSIGNED_BYTE, cBuf);
    } else {
      if (Orient == SG_PLOT_NORMAL)
	glReadPixels(0, PixmapH - 1 - rowI, PixmapW, 1,
	    pixFormat, GL_FLOAT, fBuf);
      else
	glReadPixels(rowI, 0, 1, PixmapH,
	    pixFormat, GL_FLOAT, fBuf);
      
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

    if (ImgFormat == DIF_BMP)
      (void) fwrite(cBuf, 1, bufSize, ImgFileP);
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
IOWinOGLEndDump(void)
{
  IORes res;
  HENHMETAFILE emf;

  if (ImgFormat == DIF_EMF) {
    emf = CloseEnhMetaFile(DumpDC);
    if (emf == NULL) {
      res = IO_RES_ERR;
    } else {
      DeleteEnhMetaFile(emf);
      res = IO_RES_OK;
    }
  } else {
    res = writeImg();
    if (ImgFormat != DIF_TIFF)
      (void) fclose(ImgFileP);
    DeleteObject(Bitmap);
    DeleteDC(DumpDC);
  }

  wglMakeCurrent(OldDC, OldCtx);

  SgSetAttr();
  SgSetViewport(OldVpX, OldVpY, OldVpW, OldVpH);

  wglDeleteContext(Ctx);

  return res;
}
