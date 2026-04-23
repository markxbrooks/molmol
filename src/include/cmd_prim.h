/*
************************************************************************
*
*   cmd_prim.h - commands for primitives
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
*   Date of last modification : 99/10/23
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.cmd_prim.h
*   SCCS identification       : 1.21
*
************************************************************************
*/

#include <cmd.h>

extern ErrCode ExAddCircles(char *);
extern ErrCode ExAddCylinder(char *);
extern ErrCode ExAddDipole(char *);
extern ErrCode ExAddDrawobj(char *);
extern ErrCode ExAddIsosurface(char *);
extern ErrCode ExAddPlates(char *);
extern ErrCode ExAddRibbon(char *);
extern ErrCode ExAddSheet(char *);
extern ErrCode ExAddSolid(char *);
extern ErrCode ExAddSurface(char *);
extern ErrCode ExAddText(char *);
extern ErrCode ExAddTitle(char *);
extern ErrCode ExAddTrajec(char *);
extern ErrCode ExEditText(char *);
extern ErrCode ExLengthPrim(char *);
extern ErrCode ExMoveDrawobj(char *);
extern ErrCode ExMovePrim(char *);
extern ErrCode ExPaintRibbon(char *);
extern ErrCode ExPaintSurface(char *);
extern ErrCode ExRemovePrim(char *);
extern ErrCode ExSizePlate(char *);
extern ErrCode ExSizeRibbon(char *);
extern ErrCode ExSplitRibbon(char *);
extern ErrCode ExStyleCylinder(char *);
extern ErrCode ExStyleDrawobj(char *);
extern ErrCode ExStyleMap(char *);
extern ErrCode ExStyleRibbon(char *);
extern ErrCode ExStyleTrajec(char *);
extern ErrCode ExTrimSurface(char *);
extern ErrCode ExTypeRibbon(char *);
