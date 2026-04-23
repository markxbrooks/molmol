/*
************************************************************************
*
*   par_hand.h - parameter handling
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.par_hand.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _PAR_HANDLER_H_
#define _PAR_HANDLER_H_

#include <g_file.h>

extern BOOL ParDefined(char *);

extern void ParSetIntVal(char *, int);

extern void ParSetDoubleVal(char *, double);

extern void ParSetStrVal(char *, char *);

extern void ParSetIntArrVal(char *, int, int);

extern void ParSetDoubleArrVal(char *, int, double);

extern void ParSetStrArrVal(char *, int, char *);

extern void ParUndefine(char *);

extern int ParGetIntVal(char *);

extern double ParGetDoubleVal(char *);

extern char *ParGetStrVal(char *);

extern int ParGetIntArrVal(char *, int);

extern double ParGetDoubleArrVal(char *, int);

extern char *ParGetStrArrVal(char *, int);

extern int ParGetArrSize(char *);

extern GFileRes ParDump(GFile);

extern GFileRes ParUndump(GFile);

#endif  /* _PAR_HANDLER_H_ */
