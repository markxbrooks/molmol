/*
************************************************************************
*
*   data_calc.h - calculation functions
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
*   Date of last modification : 96/12/05
*   Pathname of SCCS file     : /local/home/kor/molmol/src/data/SCCS/s.data_calc.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <data_hand.h>

extern void DhDistCalc(DhDistP distP);

extern void DhRotMatCalc(Mat4, Mat4, Vec3, Vec3, Vec3, Vec3, float);

extern void DhCalcPseudo(DhResP);
