/*
************************************************************************
*
*   prop_def.h - property definiton
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
*   Date of last modification : 96/03/15
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.prop_def.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#ifndef _PROP_DEF_H_
#define _PROP_DEF_H_

typedef struct PropRefS *PropRefP;

#define PROP_ALL       "all"
#define PROP_VISIBLE   "visible"
#define PROP_SELECTED  "selected"
#define PROP_DISPLAYED "displayed"
#define PROP_MOVABLE   "movable"

#define PROP_MEAN      "mean"

extern PropRefP PropGetRef(char *, BOOL);

#endif  /* _PROP_DEF_H_ */
