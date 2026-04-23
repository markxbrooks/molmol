/*
************************************************************************
*
*   graph_input.h - graphical display, input
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
*   Date of last modification : 95/02/13
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.graph_input.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _GRAPH_INPUT_H_
#define _GRAPH_INPUT_H_

#include <bool.h>
#include <pu_types.h>

typedef BOOL (* GraphInputF) (char *, float, float, PuMouseCBStruc *, void *);

extern void GraphInputInit(void);

extern void GraphInputAdd(GraphInputF, void *);

extern void GraphInputGetClick(float *, float *);

#endif  /* _GRAPH_INPUT_H_ */
