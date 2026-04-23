/*
************************************************************************
*
*   EllInteg.c - complete elliptical integrals
*
*   Copyright (c) 1996
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
*   Date of last modification : 01/05/26
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.EllInteg.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <ell_integ.h>

#include <math.h>
#include <values.h>

#define CA 0.0003f

float
EllInteg(float qc, float p, float a, float b)
{
  float e, f, g, em, q;

  qc = fabsf(qc);
  e = qc;
  em = 1.0f;

  if (p > 0.0f) {
    p = sqrtf(p);
    b /= p;
  } else {
    f = qc * qc;
    q = 1.0f - f;
    g = 1.0f - p;
    f -= p;
    q *= b - a * p;
    p = sqrtf(f / g);
    a = (a - b) / g;
    b = - q / (g * g * p) + a * p;
  }

  for (;;) {
    f = a;
    a += b / p;
    g = e / p;
    b = 2.0f * (b + f * g);
    p += g;
    g = em;
    em += qc;
    if (fabsf(g - qc) <= g * CA)
      break;
    
    qc = sqrtf(e);
    qc *= 2.0f;
    e = qc * em;
  }

  return 0.5f * (float) M_PI * (b + a * em) / (em * (em + p));
}

float
EllInteg1(float k)
{
  return EllInteg(sqrtf(1.0f - k * k), 1.0f, 1.0f, 1.0f);
}

float
EllInteg2(float k)
{
  float kc;

  kc = sqrtf(1.0f - k * k);
  return EllInteg(kc, 1.0f, 1.0f, kc * kc);
}
