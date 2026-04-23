/*
************************************************************************
*
*   attr_type.h - opaque pointers to attribute structure
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
*   Date of last modification : 95/01/26
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.attr_type.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _ATTR_TYPE_H_
#define _ATTR_TYPE_H_

typedef struct AttrS *AttrP;
typedef struct MolAttrS *MolAttrP;

extern int AttrGetIndex(AttrP);

#endif  /* _ATTR_TYPE_H_ */
