/*
************************************************************************
*
*   os_browse.h - display files in web browser
*
*   Copyright (c) 1997-99
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
*   Date of last modification : 99/10/30
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.os_browse.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#ifndef _OS_BROWSE_H_
#define _OS_BROWSE_H_

#include <bool.h>

typedef enum {
  OBR_OK,
  OBR_STARTUP_FAILED,
  OBR_DISPLAY_FAILED
} OsBrowseRes;

extern char *OsBrowseGetSpecPrompt(void);

extern int OsBrowseGetListSize(void);

extern void OsBrowseGetListEntry(int, char **, char **, char **);

extern OsBrowseRes OsBrowseShow(char *, char *, char *);

extern void OsBrowseCleanup(void);

#endif  /* _OS_BROWSE_H_ */
