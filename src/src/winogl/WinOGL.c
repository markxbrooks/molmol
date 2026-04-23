/*
************************************************************************
*
*   WinOGL.c - Windows/OpenGL device
*
*   Copyright (c) 1996-99
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
*   Date of last modification : 99/10/24
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/winogl/SCCS/s.WinOGL.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "winogl.h"
#include "winogl_int.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <windows.h>
#include <GL/gl.h>

#include <sg.h>
#include <win_p.h>

static HWND CurrW = NULL;
static HDC CurrDC = NULL;

IORes
WinOGLPrepareFont(HDC dc)
{
  int base;

  SelectObject(dc, GetStockObject(SYSTEM_FONT));

  base = glGenLists(256);
  if (base == 0)
    return IO_RES_ERR;

  if (! wglUseFontBitmaps(dc, 0, 255, base))
    return IO_RES_ERR;

  glListBase(base);

  return IO_RES_OK;
}

IORes
IOWinOGLInit(char *appName, int *argcP, char *argv[])
{
  BOOL stereo;
  HWND drawW;
  HDC dc;
  PIXELFORMATDESCRIPTOR pfd; 
  int pixelFormat; 
  int i;

  (void) PuInit(appName, argcP, argv);

  stereo = FALSE;
  for (i = 1; i < *argcP; i++)
    if (strcmp(argv[i], "-stereo") == 0) {
      stereo = TRUE;
      break;
    }

  drawW = WinGetDrawW();
  dc = GetDC(drawW);

  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |  
      PFD_DOUBLEBUFFER;
  if (stereo)
    pfd.dwFlags |= PFD_STEREO;
  pfd.dwLayerMask = PFD_MAIN_PLANE;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pfd.cAccumBits = 0;
  pfd.cStencilBits = 0;
 
  pixelFormat = ChoosePixelFormat(dc, &pfd);
  if (pixelFormat == 0)
    return IO_RES_ERR;

  if (! SetPixelFormat(dc, pixelFormat, &pfd))
    return IO_RES_ERR;
  
  DescribePixelFormat(dc, pixelFormat,  
        sizeof(PIXELFORMATDESCRIPTOR), &pfd); 

  IOSetWindow(drawW);

  (void) SgInit(argcP, argv);

  return WinOGLPrepareFont(dc);
}

IORes
IOWinOGLCleanup(void)
{
  (void) SgCleanup();
  (void) PuCleanup();

  return IO_RES_OK;
}

IORes
IOWinOGLSetWindow(PuWindow win)
{
  HWND w = (HWND) win;
  HDC dc;
  HGLRC wglContext;

  if (CurrW != NULL)
    ReleaseDC(CurrW, CurrDC);

  dc = GetDC(w);
  wglContext = (HGLRC) GetWindowLong(w, GWL_USERDATA);
  if (wglContext == NULL) {
    wglContext = wglCreateContext(dc); 
    (void) SetWindowLong(w, GWL_USERDATA, (LONG) wglContext);
  }
 
  wglMakeCurrent(dc, wglContext);

  CurrW = w;
  CurrDC = dc;
  
  return IO_RES_OK;
}

void
IOWinOGLEndFrame(void)
{
  SwapBuffers(CurrDC);
}
