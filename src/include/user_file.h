/*
************************************************************************
*
*   user_file.h - management of user files
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
*   Date of last modification : 94/12/15
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.user_file.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _USER_FILE_H_
#define _USER_FILE_H_

#include <g_file.h>

extern void UserFileSetDir(char *, char *);

extern char *UserFileGetName(char *);

extern GFile UserFileOpenRead(char *);

extern GFile UserFileOpenWrite(char *, GFileFormat);

extern GFile UserFileOpenAppend(char *);

#endif  /* _USER_FILE_H_ */
