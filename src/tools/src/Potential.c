/*
************************************************************************
*
*   Potential.c - calculation of electrostatic potential
*
*   Copyright (c) 1996
*
*   ETH Zuerich
*   Institut fuer Molekularbiologie und Biophysik
*   ETH-Hoenggerberg
*   CH-8093 Zuerich
*
*   All Rights Reserved
*
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.Potential.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

/* Code originally written by Lian Shen. */

#include <potential.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <values.h>

#include <break.h>
#include <surf_check.h>

#define SQRT8 2.8284271248f	  /* sqrt(8) */

#define ACCURACY ((float) 1.0e-9) /* If the difference of two steps is less
				   * than ACCURACY we stop the iteration. */
#define MAXSTEPS 200		  /* max. iteration steps */
#define CMXDEG   2		  /* degree of CMX calculation */

/***********************************************************************
 * Normally we are in room temperature, so we choose the most simple 
 * form of 300 Kelvin.
 ***********************************************************************/
#define TEMPERATURE 300.0f

#define OUT      0u		/* out of protein */
#define SFIN     1u		/* self in        */
#define XNIN     2u		/* X-negative in  */
#define XPIN     4u		/* X-positive in  */
#define YNIN     8u		/* Y-negative in  */
#define YPIN     16u		/* Y-positive in  */
#define ZNIN     32u		/* Z-negative in  */
#define ZPIN     64u		/* Z-positive in  */
#define ATBOUND 128u		/* At Boundary    */

typedef struct {
  float charge;
  float d;			/* equation (5) of Nicholls and Honig */
  unsigned char status;		/* definition see the SFIN, XPIN etc. */
} Lattice;

static int
faculty(int n)
{
  int j;
  int ret;

  ret = 1;
  for (j = 2; j <= n; j++)
    ret *= j;

  return ret;
}

/***********************************************************************
 * We calculate the kappa value, the Debye-Huckel parameter. Since in 
 * the whole calculation, we only need kappa^2, we calculate also only
 * kappa^2.
 *
 * kappa has a unit of Length^{-1}, we must change it to 1/Anstrom to
 * suit our purpose.
 ***********************************************************************/

static float
calcKappaSqr(float saltConcentr)
{
/***********************************************************************
 * Factor 2.529e3 = 8*PI*e^2*N_A/(1000*k_B*1e16)
 * e: elementry charge (4.8032e-10) (in CGS unit)
 * N_A: Avagadro const. (6.022e23)
 * k_B: Boltzmann const. (1.3806e-16) (in CGS unit)
 * 1e16: for the conversion from cm to Anstrom in square
 ***********************************************************************/

  return ((float) 2.529e3 * saltConcentr / TEMPERATURE);
}

static BOOL
isBoundary(PotGridDescr *gridP, int jA[3])
{
  return (jA[0] == 0 || jA[0] == (gridP->pointNo[0] - 1) ||
      jA[1] == 0 || jA[1] == (gridP->pointNo[1] - 1) ||
      jA[2] == 0 || jA[2] == (gridP->pointNo[2] - 1));
}

static void
setStatus(Lattice *lattice, PotAtomDescr *atomA, int atomNo,
    PotGridDescr *gridP, float solvRad, int prec)
{
  SurfaceCheckDescrP surfDescrP;
  int i, k;
  Vec3 x;
  int jA[3];

  /* our grid width is about twice as large as the one that
     we normally use for surface calculation, so we reduce
     the precision by 1 */
  if (prec > 3)
    prec--;
  surfDescrP = SurfaceCheckPrepareContact(atomA, atomNo, solvRad, prec);

  i = 0;
  for (jA[2] = 0; jA[2] < gridP->pointNo[2]; jA[2]++) {
    if (BreakCheck(1))
      break;

    for (jA[1] = 0; jA[1] < gridP->pointNo[1]; jA[1]++) {
      for (jA[0] = 0; jA[0] < gridP->pointNo[0]; jA[0]++) {
	lattice[i].status = 0;

	/* this point is at boundary */
	if (isBoundary(gridP, jA)) {
	  lattice[i].status |= ATBOUND;
	} else {		/* boundary point cannot be in protein */
	  for (k = 0; k < 3; k++)
	    x[k] = gridP->origin[k] + jA[k] * gridP->spacing;

	  /* point is in protein by itself */
	  if (SurfaceCheckPoint(surfDescrP, x))
	    lattice[i].status |= SFIN;
	  else
	    lattice[i].status |= OUT;

	  /* x negative direction */
	  x[0] -= 0.5f * gridP->spacing;
	  if (SurfaceCheckPoint(surfDescrP, x))
	    lattice[i].status |= XNIN;

	  /* x positive direction */
	  x[0] += gridP->spacing;
	  if (SurfaceCheckPoint(surfDescrP, x))
	    lattice[i].status |= XPIN;

	  /* y negative direction */
	  x[0] -= 0.5f * gridP->spacing;
	  x[1] -= 0.5f * gridP->spacing;
	  if (SurfaceCheckPoint(surfDescrP, x))
	    lattice[i].status |= YNIN;

	  /* y positive direction */
	  x[1] += gridP->spacing;
	  if (SurfaceCheckPoint(surfDescrP, x))
	    lattice[i].status |= YPIN;

	  /* z negative direction */
	  x[1] -= 0.5f * gridP->spacing;
	  x[2] -= 0.5f * gridP->spacing;
	  if (SurfaceCheckPoint(surfDescrP, x))
	    lattice[i].status |= ZNIN;

	  /* z positive direction */
	  x[2] += gridP->spacing;
	  if (SurfaceCheckPoint(surfDescrP, x))
	    lattice[i].status |= ZPIN;
	}

	i++;
      }
    }
  }

  SurfaceCheckFree(surfDescrP);
}

/***********************************************************************
 *
 * In this function, we build the lattice, assign each grid point a 
 * charge, d value. Returns number of points inside molecule or near
 * the surface.
 *
 ************************************************************************/

static int
buildLattice(Lattice *lattice, PotAtomDescr *atomA, int atomNo,
    float solvDiel, float saltRad, float molDiel, float kappaSqr,
    PotGridDescr *gridP)
{
  SurfaceCheckDescrP surfDescrP;
  int pointNo;
  Vec3 x, y;
  float laplaceIn, laplaceOut, temp;
  BOOL isIn;
  int ind;
  int jA[3];
  int i, k, l, m;

  /*
   ***********************************************************************
   * To save the calculation time, we change the definition of lattice.d *
   * if the grid point is *NOT* near protein. In this case, we only need *
   * to solve a differential equation, which is quite similar (at least  *
   * from the numerical view point) to Laplace equation.                 *
   * For these points, lattice.d is equal to the factor laplace, which   *
   * is equal the "former" lattice.d*solvDiel. Use this approch, we save *
   * one division per step.                                              *
   ***********************************************************************
   */

  /*
   ***********************************************************************
   * We also scaled the lattice.charge and lattice.d through omega. It   *
   * will save quite much CPU time.                                      *
   ***********************************************************************
   */

  surfDescrP = SurfaceCheckPrepareSolvent(atomA, atomNo, saltRad);

  pointNo = 0;

  laplaceIn = 1.0f / (6.0f * solvDiel);
  laplaceOut = 1.0f /
      (6.0f * solvDiel + kappaSqr * gridP->spacing * gridP->spacing);

  i = 0;
  for (jA[2] = 0; jA[2] < gridP->pointNo[2]; jA[2]++) {
    for (jA[1] = 0; jA[1] < gridP->pointNo[1]; jA[1]++) {
      for (jA[0] = 0; jA[0] < gridP->pointNo[0]; jA[0]++) {
	lattice[i].charge = 0.0f;

	/* turn off the highest bit for boundary condition */
	if (lattice[i].status & ATBOUND) {
	  lattice[i].d = 0.0f;
	  i++;
	  continue;
	}

	for (k = 0; k < 3; k++)
	  x[k] = gridP->origin[k] + jA[k] * gridP->spacing;
	isIn = SurfaceCheckPoint(surfDescrP, x);

	if (! lattice[i].status) {
	  if (isIn)
	    lattice[i].d = laplaceIn;
	  else
	    lattice[i].d = laplaceOut;
	  i++;
	  continue;
	}

	if (isIn)
	  temp = 0.0f;
	else
	  temp = kappaSqr * gridP->spacing * gridP->spacing;

	if (lattice[i].status & XNIN)
	  temp += molDiel;
	else
	  temp += solvDiel;

	if (lattice[i].status & XPIN)
	  temp += molDiel;
	else
	  temp += solvDiel;

	if (lattice[i].status & YNIN)
	  temp += molDiel;
	else
	  temp += solvDiel;

	if (lattice[i].status & YPIN)
	  temp += molDiel;
	else
	  temp += solvDiel;

	if (lattice[i].status & ZNIN)
	  temp += molDiel;
	else
	  temp += solvDiel;

	if (lattice[i].status & ZPIN)
	  temp += molDiel;
	else
	  temp += solvDiel;

	lattice[i].d = 1.0f / temp;

	pointNo++;
	i++;
      }
    }
  }

  SurfaceCheckFree(surfDescrP);

  for (i = 0; i < atomNo; i++) {
    if (atomA[i].charge == 0.0f)
      continue;

    for (k = 0; k < 3; k++) {
      x[k] = (atomA[i].cent[k] - gridP->origin[k]) / gridP->spacing;
      jA[k] = (int) x[k];
    }

    for (m = jA[2]; m <= jA[2] + 1; m++) {
      for (l = jA[1]; l <= jA[1] + 1; l++) {
	for (k = jA[0]; k <= jA[0] + 1; k++) {
	  /* the partial charge of a atom at a grid point, which is
	   * in the direct near of this atom is defined as
	   * charge*(1-a)(1-b)(1-c), with a, b, c the fraction of
	   * distance in x, y, z directions
	   */

	  y[0] = 1.0f - fabsf(x[0] - k);
	  y[1] = 1.0f - fabsf(x[1] - l);
	  y[2] = 1.0f - fabsf(x[2] - m);

	  ind = k + gridP->pointNo[0] * (l + m * gridP->pointNo[1]);

	  lattice[ind].charge += atomA[i].charge * (y[0] * y[1] * y[2]) *
	      (4.0f * (float) M_PI * lattice[ind].d / gridP->spacing) *
	      100.0f;
	}
      }
    }
  }

  return pointNo;
}

/***********************************************************************
 *
 * In the following function, we implement the idea of Cioslowski.
 * Reference: Physical Review Letters, 58, (2) 1986.
 *
 ***********************************************************************/

static void
initVector(PotGridDescr *gridP)
{
  int i, j, k;
  int jA[3];
  int lll;
  Vec3 fac;
  Vec3 arg;
  float temp;

  lll = 1;
  for (k = 0; k < 3; k++) {
    j = gridP->pointNo[k] - 1;
    fac[k] = (float) M_PI / j;
    lll *= j;
  }

  temp = SQRT8 / sqrtf(lll);

  i = 0;
  for (jA[2] = 0; jA[2] < gridP->pointNo[2]; jA[2]++) {
    for (jA[1] = 0; jA[1] < gridP->pointNo[1]; jA[1]++) {
      for (jA[0] = 0; jA[0] < gridP->pointNo[0]; jA[0]++) {

	for (k = 0; k < 3; k++)
	  arg[k] = fac[k] * jA[k];

	gridP->valueA[i]
	    = temp * sinf(arg[0]) * sinf(arg[1]) * sinf(arg[2]);

	i++;
      }
    }
  }
}

/***********************************************************************
 * In this function we calculate the n-th moment the Hamiltonian.
 ***********************************************************************/

static BOOL
exponential(PotGridDescr *gridP, int totalNo, float solvDiel,
    float molDiel, Lattice *lattice,
    float *potA, int n)
{
  /************************************************************************
   * totalNo is the total number of lattice points.
   * lattice is the array of lattice properties (d valude, near protein...
   * potA is (1-T)^n Phi(0)
   * n is the grad of potential.
   ************************************************************************/

  /************************ ATTENTION!!! ************************
   * We can *NOT* use Gauss-Seidel update methods to calculate
   * the moment, with that method, we would only get very small
   * omegas. With Jacobi update methods, we can get something
   * more reasonable.
   **************************************************************/

  float temp;
  int j;
  int yincr, zincr;
  float *vecTmp;

  vecTmp = malloc(totalNo * sizeof(*vecTmp));
  if (vecTmp == NULL)
    return FALSE;

  yincr = gridP->pointNo[0];
  zincr = gridP->pointNo[0] * gridP->pointNo[1];

  for (j = 0; j < totalNo; j++) {
    potA[j] = gridP->valueA[j];
    /* make a copy of initialization to make moment */
    vecTmp[j] = potA[j];
  }

  while (n > 0) {
    for (j = 0; j < totalNo; j++) {
      if (lattice[j].status & ATBOUND)
	continue;

      if (! lattice[j].status) {
	/* Not at boundary *AND* not near protein */
	temp = potA[j - 1] + potA[j + 1] + potA[j - yincr] + potA[j + yincr]
	    + potA[j - zincr] + potA[j + zincr];

	vecTmp[j] = potA[j] - solvDiel * lattice[j].d * temp;
	/* Phi(n+1)=(1-T)*Phi(n) */

	continue;
      }

      /* not at boundary *BUT* near protein */
      temp = 0.0f;
      if (lattice[j].status & XNIN)
	temp += molDiel * potA[j - 1];
      else
	temp += solvDiel * potA[j - 1];

      if (lattice[j].status & XPIN)
	temp += molDiel * potA[j + 1];
      else
	temp += solvDiel * potA[j + 1];

      if (lattice[j].status & YNIN)
	temp += molDiel * potA[j - yincr];
      else
	temp += solvDiel * potA[j - yincr];

      if (lattice[j].status & YPIN)
	temp += molDiel * potA[j + yincr];
      else
	temp += solvDiel * potA[j + yincr];

      if (lattice[j].status & ZNIN)
	temp += molDiel * potA[j - zincr];
      else
	temp += solvDiel * potA[j - zincr];

      if (lattice[j].status & ZPIN)
	temp += molDiel * potA[j + zincr];
      else
	temp += solvDiel * potA[j + zincr];

      vecTmp[j] = potA[j] - lattice[j].d * temp;
      /* Phi(n+1)=Phi(n)-T*Phi(n) */
    }

    for (j = 0; j < totalNo; j++)
      potA[j] = vecTmp[j];

    n--;
  }

  free(vecTmp);

  return TRUE;
}

static float 
scalarProduct(float *vec1, float *vec2, int dim)
{
  float temp;
  int j;

  temp = 0.0f;
  for (j = 0; j < dim; j++)
    temp += vec1[j] * vec2[j];

  return temp;
}

static BOOL
calcMoment(PotGridDescr *gridP, int totalNo, float solvDiel,
    float molDiel, Lattice *lattice, float *potA,
    int n, float *momentP)
{
  if (! exponential(gridP, totalNo, solvDiel, molDiel, lattice, potA, n))
    return FALSE;

  *momentP = scalarProduct(potA, gridP->valueA, totalNo);

  return TRUE;
}

static BOOL
groundEnergy(int k, PotGridDescr *gridP, int totalNo, float solvDiel,
    float molDiel, Lattice *lattice, float *potA,
    float *energyP)
{
  /*********************************************************************
   * k: the order, to which it should be calculated
   * potA: (1-T)^n * Phi(0)
   *********************************************************************/

  float *iA;
  float *momA;
  float temp;
  int j, l;

  if (k <= 1)
    k = 3;
  else
    k = 5;

  iA = malloc((k + 1) * sizeof(*iA));
  if (iA == NULL)
    return FALSE;

  momA = malloc((k + 1) * sizeof(*momA));
  if (momA == NULL) {
    free(iA);
    return FALSE;
  }

  for (j = 1; j <= k; j++)
    if (! calcMoment(gridP, totalNo, solvDiel, molDiel,
	lattice, potA, j, &momA[j])) {
      free(iA);
      free(momA);
      return FALSE;
    }

  momA[0] = 0.0f;
  iA[0] = 0.0f;
  iA[1] = momA[1];

  for (l = 2; l <= k; l++) {
    temp = 0.0f;
    for (j = 0; j <= l - 2; j++)
      temp += faculty(l - 1) / (faculty(j) * faculty(l - 1 - j)) *
	  iA[j + 1] * momA[l - j - 1];
    iA[l] = momA[l] - temp;
  }
  /* compare (23) in Nicholls and Honig */

  *energyP = iA[1] - iA[2] * iA[2] / iA[3];

  if (*energyP < 0.0f || *energyP > 1.0f)
    /* invalid ground state energy, use the second order */
    k = 5;

  if (k > 3) {
    temp = iA[4] * iA[2] - iA[3] * iA[3];
    temp = *energyP - temp * temp / (iA[3] * (iA[5] * iA[3] - iA[4] * iA[4]));

    /* compare (24) in Nicholls and Honig */

    if (temp < 0.0f)
      temp = 0.0f;
    else if (temp > 1.0f)
      temp = 1.0f;

    *energyP = temp;
  }

  free(iA);
  free(momA);

  return TRUE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * We calculate the Omega value, which depends on the biggest eigenvalue 
 * of T. i.e. the ground state energy of (1-T).
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static BOOL
getOmega(PotGridDescr *gridP, int totalNo, float solvDiel,
    float molDiel, Lattice *lattice, float *omegaP)
{
  float energy, lambda;
  float *potA;

  potA = malloc(totalNo * sizeof(*potA));
  if (potA == NULL)
    return FALSE;

  if (! groundEnergy(CMXDEG, gridP, totalNo, solvDiel, molDiel,
      lattice, potA, &energy)) {
    free(potA);
    return FALSE;
  }

  /* energy is the ground state energy of (1-T) */
  free(potA);

  /* engergy = 1 - spectral radius of gaus-seidel relaxation */

  lambda = 1.0f + sqrtf(energy);	/* use lambda as help-variable */

  /************************************************************************
   * change the definition of energy. It's now the biggest eigenvalue of T
   * i.e. the spectral radius of T
   ************************************************************************/

  energy = 1.0f - energy;

  lambda = energy / (lambda * lambda);	/* (20) */
  *omegaP = 2.0f / (1.0f + sqrtf(1.0f - lambda));	/* (19) */

  return TRUE;
}

/*
 ***********************************************************************
 * We scaled the lattice.charge and lattice.d through omega. It will   *
 * save quite much CPU time.                                           *
 ***********************************************************************
 */

static void
scaleLattice(Lattice *lattice, PotGridDescr *gridP, float omega)
{
  int i;
  int totalNo;

  totalNo = gridP->pointNo[0] * gridP->pointNo[1] * gridP->pointNo[2];

  for (i = 0; i < totalNo; i++) {
    lattice[i].d *= omega;
    lattice[i].charge *= omega;
  }
}

static void
setBoundary(PotAtomDescr *atomA, int atomNo,
    PotGridDescr *gridP, Lattice *lattice,
    float solvDiel, PotBoundaryCond boundCond, float kappaSqr)
{
  int jA[3];
  int ind, i, k;
  Vec3 temp;
  float r;
  float kappa;

  kappa = sqrtf(kappaSqr);

  ind = 0;
  for (jA[2] = 0; jA[2] < gridP->pointNo[2]; jA[2]++) {
    if (BreakCheck(1))
      break;

    for (jA[1] = 0; jA[1] < gridP->pointNo[1]; jA[1]++) {
      for (jA[0] = 0; jA[0] < gridP->pointNo[0]; jA[0]++) {
	if (lattice[ind].status & ATBOUND) {
	  gridP->valueA[ind] = 0.0f;

	  if (boundCond == PBC_DEBYE) {
	    for (i = 0; i < atomNo; i++) {
	      for (k = 0; k < 3; k++)
		temp[k] = gridP->origin[k] + jA[k] * gridP->spacing;

	      r = Vec3DiffAbs(temp, atomA[i].cent);
	      if (kappa == 0.0f)
		gridP->valueA[ind] += atomA[i].charge / (solvDiel * r);
	      else
		gridP->valueA[ind] += atomA[i].charge *
		    expf(- kappa * r) / (solvDiel * r);
	    }
	  }
	}

	ind++;
      }
    }
  }
}

/*
**********************************************************************
*                                                                    *
*   This is the main iteration program to solve the PB equation.     *
*                                                                    *
**********************************************************************
*/

static void
iteration(PotGridDescr *gridP, PotAtomDescr *atomA, int atomNo,
    float solvDiel, float molDiel,
    Lattice *lattice, float omega,
    int totalNo, int insideNo)
{
  float temp;
  float *valA;
  int jA[3];
  int i, yincr, zincr;
  int step;
  float xi;
  float insideRmsd;

  xi = 1.0f - omega;

  yincr = gridP->pointNo[0];
  zincr = gridP->pointNo[0] * gridP->pointNo[1];

  valA = gridP->valueA;

  for (step = 0; step < MAXSTEPS; step++) {
    if (BreakCheck(1))
      break;

    insideRmsd = 0.0f;

    i = 0;
    for (jA[2] = 0; jA[2] < gridP->pointNo[2]; jA[2]++) {
      for (jA[1] = 0; jA[1] < gridP->pointNo[1]; jA[1]++) {
	for (jA[0] = 0; jA[0] < gridP->pointNo[0]; jA[0]++) {
	  if (lattice[i].status & ATBOUND) {
	    i++;
	    continue;
	  }

	  temp = valA[i];

	  if (! lattice[i].status) {
	    /***************************************************
	     * NOT at boundary and NOT near protein 
	     * where Charge is ZERO 
	     ***************************************************/
	    valA[i] = lattice[i].d * solvDiel *
		(valA[i - 1] + valA[i + 1] + valA[i - yincr] + valA[i + yincr]
		+ valA[i - zincr] + valA[i + zincr]);
	    valA[i] += xi * temp;

	    i++;
	    continue;
	  }

	  /* near or in protein */
	  if (lattice[i].status & XNIN)
	    valA[i] = molDiel * valA[i - 1];
	  else
	    valA[i] = solvDiel * valA[i - 1];

	  if (lattice[i].status & XPIN)
	    valA[i] += molDiel * valA[i + 1];
	  else
	    valA[i] += solvDiel * valA[i + 1];

	  if (lattice[i].status & YNIN)
	    valA[i] += molDiel * valA[i - yincr];
	  else
	    valA[i] += solvDiel * valA[i - yincr];

	  if (lattice[i].status & YPIN)
	    valA[i] += molDiel * valA[i + yincr];
	  else
	    valA[i] += solvDiel * valA[i + yincr];

	  if (lattice[i].status & ZNIN)
	    valA[i] += molDiel * valA[i - zincr];
	  else
	    valA[i] += solvDiel * valA[i - zincr];

	  if (lattice[i].status & ZPIN)
	    valA[i] += molDiel * valA[i + zincr];
	  else
	    valA[i] += solvDiel * valA[i + zincr];

	  valA[i] *= lattice[i].d;

	  valA[i] += xi * temp;
	  valA[i] += lattice[i].charge;

	  temp = temp - valA[i];
	  insideRmsd += temp * temp;
	  i++;
	}
      }
    }

    insideRmsd /= insideNo;
    if (insideRmsd < ACCURACY)
      break;
  }
}

BOOL
PotentialCalc(PotAtomDescr atomA[], int atomNo, float molDiel,
    float solvDiel, float solvRad,
    float saltConcentr, float saltRad,
    float boundSize, PotBoundaryCond boundCond, int prec,
    PotGridDescr *gridP)
{
  Lattice *lattice;
  int i, j;
  float kappaSqr;
  float omega;
  float tmp1, tmp2;
  Vec3 max, min;
  int totalNo;
  int insideNo;

  for (i = 0; i < 3; i++) {
    max[i] = - MAXFLOAT;
    min[i] = MAXFLOAT;
  }

  for (j = 0; j < atomNo; j++) {
    for (i = 0; i < 3; i++) {
      tmp1 = atomA[j].cent[i] + atomA[j].rad;
      if (tmp1 > max[i])
	max[i] = tmp1;

      tmp2 = atomA[j].cent[i] - atomA[j].rad;
      if (tmp2 < min[i])
	min[i] = tmp2;
    }
  }

  /*************************************************************************
   * Creating the lattice. The maximum linear dimension of the melecule is 
   * half of the box width. 
   *************************************************************************/

  gridP->spacing = 8.0f / (1 << prec);

  totalNo = 1;

  for (i = 0; i < 3; i++) {
    gridP->pointNo[i] = (int) ((max[i] - min[i] + 2.0f * boundSize) /
	gridP->spacing) + 1;
    if (gridP->pointNo[i] < 2)
      gridP->pointNo[i] = 2;

    gridP->origin[i] = 0.5f * (min[i] + max[i]) -
	0.5f * (gridP->pointNo[i] - 1) * gridP->spacing;

    totalNo *= gridP->pointNo[i];
  }

  gridP->valueA = malloc(totalNo * sizeof(*gridP->valueA));
  if (gridP->valueA == NULL)
    return FALSE;

  lattice = malloc(totalNo * sizeof(*lattice));
  if (lattice == NULL) {
    free(gridP->valueA);
    return FALSE;
  }

  /*****************************************************************
   * Hopely we can get further enough momory.                      *
   *****************************************************************/

  setStatus(lattice, atomA, atomNo, gridP, solvRad, prec);
  if (BreakInterrupted()) {
    free(gridP->valueA);
    free(lattice);
    return FALSE;
  }

  kappaSqr = calcKappaSqr(saltConcentr);

  insideNo = buildLattice(lattice, atomA, atomNo,
      solvDiel, saltRad, molDiel, kappaSqr, gridP);

  initVector(gridP);

  if (! getOmega(gridP, totalNo, solvDiel, molDiel, lattice, &omega)) {
    free(gridP->valueA);
    free(lattice);
    return FALSE;
  }

  scaleLattice(lattice, gridP, omega);

  setBoundary(atomA, atomNo, gridP, lattice, solvDiel, boundCond, kappaSqr);
  if (BreakInterrupted()) {
    free(gridP->valueA);
    free(lattice);
    return FALSE;
  }

  iteration(gridP, atomA, atomNo, solvDiel, molDiel,
      lattice, omega, totalNo, insideNo);

  free(lattice);

  return TRUE;
}
