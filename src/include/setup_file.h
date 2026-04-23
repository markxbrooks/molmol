/*
************************************************************************
*
*   setup_file.h - management of setup files
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
*   Date of last modification : 95/03/02
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.setup_file.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _SETUP_FILE_H_
#define _SETUP_FILE_H_

#include <g_file.h>

extern void SetupSetDir(char *);

extern char *SetupGetDir(void);

extern char *SetupGetName(char *, char *, BOOL);

extern GFile SetupOpen(char *, char *, BOOL);

#endif  /* _SETUP_FILE_H_ */
