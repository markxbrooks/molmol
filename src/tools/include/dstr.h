/*
************************************************************************
*
*   dstr.h - abstract type for dynamic strings
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
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.dstr.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _DSTR_H_
#define _DSTR_H_

typedef struct dstr *DSTR;

extern DSTR DStrNew(void);  /* returns new DSTR */

extern void DStrFree(DSTR);  /* free dstr */

extern char *DStrToStr(DSTR);

extern void DStrAssignStr(DSTR, char *);  /* dstr = str */

extern void DStrAssignDStr(DSTR, DSTR);  /* dstr1 = dstr2 */

extern void DStrAppChar(DSTR, char);  /* append ch to dstr */

extern void DStrAppStr(DSTR, char *);  /* append str to dstr */

extern void DStrAppDStr(DSTR, DSTR);  /* append dstr2 to dstr1 */

extern void DStrPart(DSTR, int, int);  /* take part of dstr */

extern int DStrLen(DSTR);  /* returns length of dstr */

extern int DStrCmp(DSTR, DSTR);  /* strcmp for DSTR */

#endif  /* _DSTR_H_ */
