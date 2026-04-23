/*
************************************************************************
*
*   cmd_dial.h - commands for switching dialog boxes
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
*   Date of last modification : 98/03/31
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/include/SCCS/s.cmd_dial.h
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <cmd.h>

extern ErrCode ExUserInterface(char *);

extern ErrCode ExDialColor(char *);
extern ErrCode ExDialMol(char *);
extern ErrCode ExDialRes(char *);
extern ErrCode ExDialMeasure(char *);
extern ErrCode ExDialRmsd(char *);
extern ErrCode ExDialSelect(char *);
extern ErrCode ExDialStyle(char *);
extern ErrCode ExRecordMac(char *);
