/*
************************************************************************
*
*   XGLTransf.c - XGL transformations
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
*   Date of last modification : 95/01/05
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLTransf.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "xgl_transf.h"

#include <stdio.h>
#include <xgl/xgl.h>

#include "xgl_access.h"

static Xgl_trans TmpTrans = NULL;

void
SgXGLRotateX(float ang)
{
  Xgl_object trans;

  xgl_object_get(XGLGetContext3D(), XGL_CTX_LOCAL_MODEL_TRANS, &trans);
  xgl_transform_rotate(trans, ang, XGL_AXIS_X, XGL_TRANS_PRECONCAT);
}

void
SgXGLRotateY(float ang)
{
  Xgl_object trans;

  xgl_object_get(XGLGetContext3D(), XGL_CTX_LOCAL_MODEL_TRANS, &trans);
  xgl_transform_rotate(trans, ang, XGL_AXIS_Y, XGL_TRANS_PRECONCAT);
}

void
SgXGLRotateZ(float ang)
{
  Xgl_object trans;

  xgl_object_get(XGLGetContext3D(), XGL_CTX_LOCAL_MODEL_TRANS, &trans);
  xgl_transform_rotate(trans, ang, XGL_AXIS_Z, XGL_TRANS_PRECONCAT);
}

void
SgXGLTranslate(float dx[3])
{
  Xgl_object trans;
  Xgl_pt pt;
  Xgl_pt_f3d f3d;

  xgl_object_get(XGLGetContext3D(), XGL_CTX_LOCAL_MODEL_TRANS, &trans);
  pt.pt_type = XGL_PT_F3D;
  pt.pt.f3d = &f3d;
  f3d.x = dx[0];
  f3d.y = dx[1];
  f3d.z = dx[2];
  xgl_transform_translate(trans, &pt, XGL_TRANS_PRECONCAT);
}

void
SgXGLScale(float sx, float sy, float sz)
{
  Xgl_object trans;
  Xgl_pt pt;
  Xgl_pt_f3d f3d;

  xgl_object_get(XGLGetContext3D(), XGL_CTX_LOCAL_MODEL_TRANS, &trans);
  pt.pt_type = XGL_PT_F3D;
  pt.pt.f3d = &f3d;
  f3d.x = sx;
  f3d.y = sy;
  f3d.z = sz;
  xgl_transform_scale(trans, &pt, XGL_TRANS_PRECONCAT);
}

void
SgXGLSetMatrix(float m[4][4])
{
  Xgl_object trans;

  xgl_object_get(XGLGetContext3D(), XGL_CTX_LOCAL_MODEL_TRANS, &trans);
  xgl_transform_write_specific(trans, m, XGL_TRANS_MEMBER_LENGTH_PRESERV);
}

void
SgXGLMultMatrix(float m[4][4])
{
  Xgl_object trans;

  if (TmpTrans == NULL)
    TmpTrans = xgl_object_create(XGLGetSysState(), XGL_TRANS, NULL, NULL);
  
  xgl_object_get(XGLGetContext3D(), XGL_CTX_LOCAL_MODEL_TRANS, &trans);
  xgl_transform_write_specific(TmpTrans, m, XGL_TRANS_MEMBER_LENGTH_PRESERV);
  xgl_transform_multiply(trans, TmpTrans, trans);
}

void
SgXGLPushMatrix(void)
{
  xgl_context_update_model_trans(XGLGetContext3D(),
      XGL_MTR_LOCAL_TRANS | XGL_MTR_PUSH);
}

void
SgXGLPopMatrix(void)
{
  xgl_context_update_model_trans(XGLGetContext3D(),
      XGL_MTR_LOCAL_TRANS | XGL_MTR_POP);
}
