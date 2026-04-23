/*
************************************************************************
*
*   win_event.h - Windows functions for handling events
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
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.win_event.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <windows.h>

#include <pu_types.h>

extern void PuWinEventLoop(void);

extern void PuWinProcessEvent(void);

extern void PuWinAddTimeOut(int, PuTimeOut, void *);

extern LRESULT CALLBACK WinDrawProc(HWND, UINT, WPARAM, LPARAM);

extern void WinAddDialog(HWND);

extern void WinRemoveDialog(HWND);
