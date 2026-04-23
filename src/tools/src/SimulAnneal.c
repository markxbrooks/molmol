/*
************************************************************************
*
*   SimulAnneal.c - minimization by simulated annealing
*
*   Copyright (c) 1994-95
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.SimulAnneal.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <simul_anneal.h>

#include <stdlib.h>
#include <math.h>

#include <break.h>
#include <rand_num.h>

#define DEBUG 0

#define T_START 100.0f
#define T_FACT 0.9f
#define T_STEP_NO 100
#define MOVE_FACT 100
#define SUCC_FACT 10
#define RAND_EXP 10.0f

void
SimulAnnealGenStart(float *xStart, int dim, float *xMin, float *xMax)
{
  int i;

  for (i = 0; i < dim; i++)
    xStart[i] = xMin[i] + RandFloat() * (xMax[i] - xMin[i]);
}

BOOL
SimulAnnealMin(MinFunc func, int dim, float *xCurr, void *clientData,
    float *xMin, float *xMax, float *xInc, int prec)
{
  float randFact, randOffs;
  float *xNew;
  float temp;
  float valCurr, valNew, valDiff, dist;
  int tStep, succNo, moveI, i;
  BOOL ok;

  randFact = 1.0f / (1.0f - expf(- RAND_EXP));
  randOffs = randFact * expf(- RAND_EXP);

  xNew = malloc(dim * sizeof(*xNew));

  temp = T_START;
  (void) func(&valCurr, xCurr, dim, clientData);

  for (tStep = 0; tStep < T_STEP_NO; tStep++) {
#if DEBUG
    (void) printf("temp: %9.5f\n", temp);
#endif

    succNo = 0;

    for (moveI = 0; moveI < MOVE_FACT * dim; moveI++) {
#if DEBUG
      (void) printf("%3d:", moveI);
#endif

      if (BreakCheck(1000))
	break;

      for (i = 0; i < dim; i++) {
	dist = randFact * expf(RAND_EXP * (RandFloat() - 1.0f)) - randOffs;
	if (RandFloat() < 0.5f)
	  xNew[i] = xCurr[i] + dist * xInc[i];
	else
	  xNew[i] = xCurr[i] - dist * xInc[i];
      }

#if DEBUG
      for (i = 0; i < dim; i++)
	(void) printf("%8.3f", xNew[i]);
#endif

      for (i = 0; i < dim; i++)
	if (xNew[i] < xMin[i] || xNew[i] > xMax[i])
	  break;

      if (i < dim) {
#if DEBUG
	(void) printf(" outside\n");
#endif
	continue;
      }

      ok = func(&valNew, xNew, dim, clientData);
#if DEBUG
      (void) printf(" ->%10.4f", valNew);
#endif

      if (BreakInterrupted())
	break;

      if (! ok) {
#if DEBUG
	(void) printf(" illegal\n");
#endif
	continue;
      }

      valDiff = valNew - valCurr;
      if (valDiff < 0.0f || RandFloat() < expf(- valDiff / temp)) {
	for (i = 0; i < dim; i++)
	  xCurr[i] = xNew[i];
	valCurr = valNew;
#if DEBUG
	(void) printf(" accepted\n");
#endif

	succNo++;
	if (succNo >= SUCC_FACT * dim)
	  break;
      } else {
#if DEBUG
	(void) printf(" rejected\n");
#endif
      }
    }

#if DEBUG
    (void) printf("moves: %d, succ: %d\n", moveI, succNo);
#endif

    if (BreakInterrupted())
      break;

    if (succNo == 0) {
      free(xNew);
      return TRUE;
    }

    temp *= T_FACT;
  }

  free(xNew);

  return FALSE;
}
