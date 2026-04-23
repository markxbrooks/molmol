/*
************************************************************************
*
*   RandNum.c - random number generation
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.RandNum.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <rand_num.h>

#include <stdio.h>
#include <math.h>

#define IA 16807
#define IM 2147483647
#define IQ 127773
#define IR 2836

static int Val = IM / 2;

void
RandSetSeed(int seed)
{
  Val = seed;
}

int
RandInt32(void)
{
  int k;

  k = Val / IQ;
  Val = IA * (Val - k * IQ) - IR * k;
  if (Val < 0)
    Val += IM;

  return Val;
}

float
RandFloat(void)
{
  return (float) RandInt32() / IM;
}

float
RandGauss(void)
/* Random number with normal distribution, average 0, deviation 1.
   From Numerical Recipes. */
{
  static int iSet = 0;
  static float gSet;
  float fac, r, v1, v2;

  if (iSet == 0) {
    do {
      v1 = 2.0f * RandFloat() - 1.0f;
      v2 = 2.0f * RandFloat() - 1.0f;
      r = v1 * v1 + v2 * v2;
    } while (r >= 1.0f);

    fac = sqrtf(-2.0f * logf(r) / r);
    gSet = v1 * fac;
    iSet = 1;

    return v2 * fac;
  } else {
    iSet = 0;
    return gSet;
  }
}
