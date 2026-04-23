/*
************************************************************************
*
*   win_text_w.h - Windows text windows
*
*   Copyright (c) 1996
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
*   Date of last modification : 96/05/09
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.win_text_w.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_types.h>

extern PuTextWindow PuWinCreateTextWindow(char *);

extern void PuWinWriteStr(PuTextWindow, char *);
