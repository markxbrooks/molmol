/*
************************************************************************
*
*   graph_text.h - interaction with texts
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
*   Date of last modification : 95/02/14
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.graph_text.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _GRAPH_TEXT_H_
#define _GRAPH_TEXT_H_

typedef void (* GraphTextMoveF) (float, float);
typedef void (* GraphTextResizeF) (float);

extern void GraphTextInit(void);

extern void GraphTextSetMoveFunc(GraphTextMoveF);

extern void GraphTextSetResizeFunc(GraphTextResizeF);

#endif  /* _GRAPH_TEXT_H_ */
