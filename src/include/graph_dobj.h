/*
************************************************************************
*
*   graph_dobj.h - interaction with draw objects
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
*   Date of last modification : 95/06/16
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.graph_dobj.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _GRAPH_DOBJ_H_
#define _GRAPH_DOBJ_H_

typedef void (* GraphDrawobjModifyF) (int, float, float);

extern void GraphDrawobjInit(void);

extern void GraphDrawobjSetModifyFunc(GraphDrawobjModifyF);

#endif  /* _GRAPH_DOBJ_H_ */
