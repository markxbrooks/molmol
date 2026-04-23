/*
************************************************************************
*
*   win_text.h - Windows text utility functions
*
*   Copyright (c) 1996-97
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
*   Date of last modification : 97/02/24
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.win_text.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <windows.h>

#include <pu_types.h>

typedef struct {
  char *currText;
  BOOL editing;
} WinTextStruc;

extern void WinTextStrucInit(WinTextStruc *);

extern BOOL TextStrucWinToPu(HWND w, WinTextStruc *, PuTextCBStruc *);

extern void TextStrucPuToWin(HWND w, PuTextCBStruc *, WinTextStruc *);
