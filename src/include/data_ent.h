/*
************************************************************************
*
*   data_ent.h - enumeration type for data entities
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
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.data_ent.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _DATA_ENT_H_
#define _DATA_ENT_H_

typedef enum {
  DE_NONE,
  DE_MOL,
  DE_RES,
  DE_ATOM,
  DE_BOND,
  DE_ANGLE,
  DE_DIST,
  DE_PRIM
} DataEntityType;

#endif  /* _DATA_ENT_H_ */
