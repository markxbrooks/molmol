/*
************************************************************************
*
*   rib_world.h - RIB status management
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.rib_world.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <bool.h>

extern void RIBWorldStart(void);

extern void RIBWorldEnd(void);

extern BOOL RIBInWorld(void);
