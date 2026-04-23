/*
************************************************************************
*
*   cmd_prop.h - commands for modifying properties
*
*   Copyright (c) 1994-99
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.cmd_prop.h
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd.h>

extern ErrCode ExDefProp(char *);
extern ErrCode ExListProp(char *);
extern ErrCode ExListSelected(char *);
extern ErrCode ExSelect(char *);
extern ErrCode ExSetRef(char *);
extern ErrCode ExUndefProp(char *);
