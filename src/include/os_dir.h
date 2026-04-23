/*
************************************************************************
*
*   os_dir.h - create and read directory
*
*   Copyright (c) 1997-2001
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.os_dir.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _OS_DIR_H_
#define _OS_DIR_H_

extern void OsDirCreate(char *);

extern void OsDirGet(char *, char *, char ***, int *);

extern void OsDirFree(char **, int);

#endif  /* _OS_DIR_H_ */
