/*
************************************************************************
*
*   cmd_io.h - commands for data I/O
*
*   Copyright (c) 1994-2001
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
*   Date of last modification : 01/07/04
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.cmd_io.h
*   SCCS identification       : 1.13
*
************************************************************************
*/

#include <cmd.h>

extern ErrCode ExReadAco(char *);
extern ErrCode ExReadAng(char *);
extern ErrCode ExReadDg(char *);
extern ErrCode ExReadDump(char *);
extern ErrCode ExReadLib(char *);
extern ErrCode ExReadLimit(char *);
extern ErrCode ExReadListAng(char *);
extern ErrCode ExReadListDg(char *);
extern ErrCode ExReadListPdb(char *);
extern ErrCode ExReadOldPdb(char *);
extern ErrCode ExReadOmap(char *);
extern ErrCode ExReadPdb(char *);
extern ErrCode ExReadPot(char *);
extern ErrCode ExReadSeq(char *);
extern ErrCode ExReadShift(char *);
extern ErrCode ExReadSybyl(char *);
extern ErrCode ExReadXyz(char *);
extern ErrCode ExWriteAng(char *);
extern ErrCode ExWriteDg(char *);
extern ErrCode ExWriteDump(char *);
extern ErrCode ExWriteLib(char *);
extern ErrCode ExWriteLimit(char *);
extern ErrCode ExWritePdb(char *);
extern ErrCode ExWriteSeq(char *);
extern ErrCode ExWriteTransform(char *);
extern ErrCode ExWriteSecondary(char *);
