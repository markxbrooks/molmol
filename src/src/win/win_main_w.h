/*
************************************************************************
*
*   win_main_w.h - Windows main window
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
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.win_main_w.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <windows.h>

#include <pu_types.h>

extern void PuWinSwitchFullscreen(BOOL);

extern void PuWinSetDrawSize(int, int);

extern void PuWinSetTextField(PuTextFieldChoice, char *);

extern void PuWinSwitchTextField(PuTextFieldChoice, BOOL);

extern PuCmdField PuWinCreateCmdField(char *,
    PuCmdFieldCB, void *, PuFreeCB);

extern void PuWinSwitchCmdFields(BOOL);

extern void PuWinSetBusyCursor(BOOL);

extern void WinCreateMainW(char *, HINSTANCE hInstance, int nCmdShow);

extern HWND WinGetMainW(void);

extern void WinSetFocus(void);

extern HCURSOR WinGetCursor(void);

extern LRESULT CALLBACK WinMainProc(HWND, UINT, WPARAM, LPARAM);
