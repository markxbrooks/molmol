/*
************************************************************************
*
*   break.h - management for program/command break
*
*   Copyright (c) 1995
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
*   Date of last modification : 95/11/17
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.break.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#ifndef _BREAK_H_
#define _BREAK_H_

#include <bool.h>

typedef void (* BreakActivateF) (BOOL);
typedef BOOL (* BreakCheckF) (void);

extern void BreakSetActivate(BreakActivateF);

extern void BreakSetCheck(BreakCheckF);

extern void BreakReset(void);

extern void BreakActivate(BOOL);

extern BOOL BreakCheck(int);
/* call break check after (approximately):
   performing 500,000 floating point operations
   drawing     20,000 lines (hardware accelerated)
   drawing     10,000 polygons (hardware accelerated)
   drawing      1,000 circles (hardware accelerated)
*/

extern BOOL BreakInterrupted(void);

#endif  /* _BREAK_H_ */
