/*
************************************************************************
*
*   fm_init.h - init FrameMaker device
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
*   Date of last modification : 95/05/16
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/fm/SCCS/s.fm_init.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <sg_types.h>

extern SgRes SgFM3Init(int *, char *[]);

extern SgRes SgFM4Init(int *, char *[]);

extern SgRes SgFMCleanup(void);
