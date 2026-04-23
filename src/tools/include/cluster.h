/*
************************************************************************
*
*   cluster.h - clustering algorithm
*
*   Copyright (c) 1995-96
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
*   Date of last modification : 96/07/21
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.cluster.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _CLUSTER_H_
#define _CLUSTER_H_

typedef void *ClusterEntryP;

typedef void (*ClusterReportF) (int, int, void *, float, float, void *);

extern ClusterEntryP ClusterAddEntry(void *);

extern void ClusterAddPair(ClusterEntryP, ClusterEntryP, float);

extern void ClusterCalc(int, float, ClusterReportF, void *);

#endif  /* _CLUSTER_H_ */
