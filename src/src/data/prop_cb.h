/*
************************************************************************
*
*   prop_cb.h - callbacks for property access
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
*   Date of last modification : 95/11/03
*   Pathname of SCCS file     : /sgiext/molmol/src/data/SCCS/s.prop_cb.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <prop_def.h>

typedef void (* PropAccessCB) (char *, PropRefP, void *);

extern void PropAddAccessCB(char *, PropAccessCB, void *);

extern void PropRemoveAccessCB(char *, PropAccessCB, void *);
