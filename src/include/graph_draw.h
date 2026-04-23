/*
************************************************************************
*
*   graph_draw.h - graphical display, drawing
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
*   Date of last modification : 95/06/24
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.graph_draw.h
*   SCCS identification       : 1.8
*
************************************************************************
*/

#ifndef _GRAPH_DRAW_H_
#define _GRAPH_DRAW_H_

#include <data_hand.h>
#include <attr_type.h>

typedef enum {
  SM_OFF,
  SM_LEFT,
  SM_RIGHT,
  SM_SIDE_BY_SIDE,
  SM_CROSS_EYE,
  SM_HARDWARE
} StereoMode;

typedef void (* GraphDrawF) (void *);

extern void GraphDrawInit(void);

extern void GraphDelaySet(int);

extern void GraphZoomSet(float);

extern void GraphMolAdd(DhMolP);

extern void GraphMolChanged(char *);

extern void GraphRedrawEnable(BOOL);

extern void GraphRedrawNeeded(void);

extern void GraphRefreshNeeded(void);

extern void GraphRedraw(void);

extern void GraphDraw(void);

extern void GraphSetAttr(AttrP);

extern void GraphDrawOverlay(GraphDrawF, void *);

extern void GraphAddOverlay(GraphDrawF, void *);

extern void GraphShowAlt(GraphDrawF, void *);

#endif  /* _GRAPH_DRAW_H_ */
