/*
************************************************************************
*
*   win_menu.h - Windows menus
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.win_menu.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <windows.h>

#include <pu_types.h>

extern PuMenubar PuWinCreateMenubar(void);

extern PuMenu PuWinCreatePulldown(PuMenubar, char *, char);

extern PuMenu PuWinCreatePopup(char *);

extern PuMenu PuWinCreatePullright(PuMenu, char *, char);

extern PuMenuEntry PuWinCreateMenuEntry(PuMenu, char *,
    char, char *, char *,
    PuMenuCB, void *, PuFreeCB);

extern void PuWinSwitchMenubar(PuMenubar, BOOL);

extern void PuWinSwitchPopup(PuMenu, BOOL);

extern void WinMenuActivate(int, PuMenuAction);

HACCEL WinGetAccelTable(void);

extern HMENU WinGetPopup(void);
