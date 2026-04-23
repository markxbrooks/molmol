/*
************************************************************************
*
*   pov_init.h - init POV device
*
*   Copyright (c) 1994-2000
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
*   Date of last modification : 00/02/26
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.pov_init.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <sg_types.h>

extern SgRes SgPOVInit(int *, char *[]);

extern SgRes SgMPOVInit(int *, char *[]);

extern SgRes SgPOVCleanup(void);
