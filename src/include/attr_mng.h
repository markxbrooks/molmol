/*
************************************************************************
*
*   attr_mng.h - attribute manager
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.attr_mng.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <bool.h>
#include <g_file.h>
#include <attr_type.h>

extern AttrP AttrGetInit(void);

extern AttrP AttrGet(AttrP);

extern void AttrCopy(AttrP, AttrP);

extern void AttrSetInit(AttrP);

extern void AttrReturn(AttrP);

extern MolAttrP MolAttrGet(void);

extern void MolAttrReturn(MolAttrP);

extern BOOL AttrDump(GFile);

extern BOOL AttrUndumpStart(GFile, int);

extern AttrP AttrFindIndex(int);

extern void AttrUndumpEnd(void);
