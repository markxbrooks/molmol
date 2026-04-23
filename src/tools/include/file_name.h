/*
************************************************************************
*
*   file_name.h - file name utility functions
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
*   Date of last modification : 98/03/19
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/tools/include/SCCS/s.file_name.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _FILE_NAME_H_
#define _FILE_NAME_H_

#include <bool.h>
#include <dstr.h>

extern void FileNamePath(DSTR);

extern void FileNameBase(DSTR);

extern void FileNameExt(DSTR);

extern void FileNameStrip(DSTR);

extern BOOL FileNameIsAbs(DSTR);

#endif  /* _FILE_NAME_H_ */
