/*
************************************************************************
*
*   rect_set.h - manage sets of rectangles
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
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.rect_set.h
*   SCCS identification       : 1.1
*
************************************************************************
*/
#ifndef _RECT_SET_H_
#define _RECT_SET_H_

#include <callback.h>

typedef struct rectSetStruc *RECTSET;

typedef struct {
  float xMin, xMax, yMin, yMax;
} Rect;

typedef void (*RectSetApplyFunc) (void *, void *);

extern RECTSET RectSetOpen(unsigned);
extern void RectSetClose(RECTSET);
extern void RectSetApply(RECTSET, RectSetApplyFunc, void *);
extern void RectSetAddDestroyCB(RECTSET, DestroyCB, void *, FreeCldta);
extern void RectSetRemoveDestroyCB(RECTSET, DestroyCB, void *);
extern void *RectSetInsert(RECTSET, void *);
extern void RectSetRemove(RECTSET, void *);
extern void RectSetFindOverlap(RECTSET, Rect *, RectSetApplyFunc, void *);
extern unsigned RectSetSize(RECTSET);
extern void RectSetPrint(RECTSET);

#endif  /* _RECT_SET_H_ */
