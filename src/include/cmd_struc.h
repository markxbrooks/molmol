/*
************************************************************************
*
*   cmd_struc.h - commands to modify structures
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/include/SCCS/s.cmd_struc.h
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include <cmd.h>

extern ErrCode ExAddAco(char *);
extern ErrCode ExAddAngle(char *);
extern ErrCode ExAddAtom(char *);
extern ErrCode ExAddBond(char *);
extern ErrCode ExAddDist(char *);
extern ErrCode ExAddLimit(char *);
extern ErrCode ExAddPseudo(char *);
extern ErrCode ExAddRes(char *);
extern ErrCode ExChangeRes(char *);
extern ErrCode ExFirstMol(char *);
extern ErrCode ExFlipAtom(char *);
extern ErrCode ExMeanMol(char *);
extern ErrCode ExNameMol(char *);
extern ErrCode ExNewMol(char *);
extern ErrCode ExRemoveAco(char *);
extern ErrCode ExRemoveAngle(char *);
extern ErrCode ExRemoveAtom(char *);
extern ErrCode ExRemoveBond(char *);
extern ErrCode ExRemoveDist(char *);
extern ErrCode ExRemoveMol(char *);
extern ErrCode ExRemoveRes(char *);
extern ErrCode ExRotateAngle(char *);
extern ErrCode ExSetAngle(char *);
extern ErrCode ExSetNeigh(char *);
