/*
************************************************************************
*
*   OGLStereo.c - OpenGL stereo
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.OGLStereo.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include "ogl_stereo.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <sg.h>

#ifdef OPENGL_SGI_STEREO
#include <stdlib.h>

#define YMAXSTEREO 492
#define YOFFSET 532
#endif

static BOOL StereoSwitch = FALSE, StereoState = FALSE, StereoInWindow;

BOOL
OGLFullscreenStereo(void)
{
  return StereoState && ! StereoInWindow;
}

void
SgOGLSetStereo(BOOL onOff)
{
  GLboolean support;

  if (onOff == StereoSwitch)
    return;
  
  StereoSwitch = onOff;

  glGetBooleanv(GL_STEREO, &support);
  StereoInWindow = support;

  if (! StereoInWindow) {
#ifdef OPENGL_SGI_STEREO
    if (onOff)
      (void) system("/usr/gfx/setmon -n STR_RECT");
    else
      (void) system("/usr/gfx/setmon -n $MONITOR");
#else
    return;
#endif
  }

  if (StereoInWindow && ! onOff)
    glDrawBuffer(GL_BACK);

  StereoState = onOff;
}

void
SgOGLSetEye(SgEye eye)
{
#ifdef OPENGL_SGI_STEREO
  float x, y, w, h;
  int xi, wi;
#endif

  if (! StereoState)
    return;

/* The stereo buffers are exchanged for Indigo2 Impact running
   IRIX 5.3. It's supposed to be fixed in IRIX 6.2 with patch #1333 */

  if (StereoInWindow) {
#ifdef OPENGL_STEREO_BUG
    if (eye == SG_EYE_LEFT)
      glDrawBuffer(GL_BACK_RIGHT);
    else
      glDrawBuffer(GL_BACK_LEFT);
#else
    if (eye == SG_EYE_LEFT)
      glDrawBuffer(GL_BACK_LEFT);
    else
      glDrawBuffer(GL_BACK_RIGHT);
#endif

    glClear(GL_DEPTH_BUFFER_BIT);
#ifdef OPENGL_SGI_STEREO
  } else {
    SgGetViewport(&x, &y, &w, &h);
    xi = (int) (x + 0.5f);
    wi = (int) (w + 0.5f);
    if (eye == SG_EYE_LEFT)
      glViewport(xi, YOFFSET, wi, YMAXSTEREO);
    else
      glViewport(xi, 0, wi, YMAXSTEREO);
#endif
  }
}
