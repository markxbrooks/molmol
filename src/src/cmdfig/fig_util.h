/*
************************************************************************
*
*   fig_util.h - utility functions for drawing figures
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
*   Date of last modification : 96/12/24
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdfig/SCCS/s.fig_util.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <data_hand.h>

typedef enum {
  AO_HORIZ,
  AO_VERT
} AxisOrient;

typedef enum {
  AT_LEFT,
  AT_CENT,
  AT_RIGHT,
  AT_NONE
} AxisTicks;

typedef enum {
  AL_NONE,
  AL_NOT_FIRST,
  AL_ALL
} AxisLabels;

extern int FigGetGreek(char *, char *);

extern void FigDrawAxis(float, float, float, AxisOrient,
    char *, float, float, float, float,
    AxisTicks, int, float, AxisLabels, int);

extern void FigSetAngleColor(DhAngleP);

extern void FigOff(void);
