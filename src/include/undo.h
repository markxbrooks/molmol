/*
************************************************************************
*
*   undo.h - undo management
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
*   Date of last modification : 95/06/24
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.undo.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _UNDO_H_
#define _UNDO_H_

#include <bool.h>

typedef enum {
  US_LEAVE,
  US_NONE,
  US_PAR,
  US_ALL
} UndoState;

extern void UndoSave(UndoState);

#endif   /* _UNDO_H_ */
