/*
************************************************************************
*
*   WinInit.c - init Windows device
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
*   Date of last modification : 97/03/10
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.WinInit.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "win_init.h"

#include <stdio.h>
#include <windows.h>

#include "win_event.h"
#include "win_main_w.h"

/* HACK: this declaration must be identical to the one in the main program! */
typedef struct {
  HINSTANCE hInstance;
  int nCmdShow;
} WinArg;

PuRes
PuWinInit(char *appName, int *argcP, char *argv[])
{
  WinArg *winArgP;
  WNDCLASSEX wndClass;

  winArgP = (WinArg *) argv[0];

  wndClass.cbSize        = sizeof(wndClass);
  wndClass.style         = CS_HREDRAW | CS_VREDRAW;
  wndClass.lpfnWndProc   = WinMainProc;
  wndClass.cbClsExtra    = 0;
  wndClass.cbWndExtra    = 0;
  wndClass.hInstance     = winArgP->hInstance;
  wndClass.hIcon         = LoadIcon(winArgP->hInstance, "molmolIcon");
  wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wndClass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
  wndClass.lpszMenuName  = NULL;
  wndClass.lpszClassName = "main";
  wndClass.hIconSm       = LoadIcon(winArgP->hInstance, "molmolIcon");

  RegisterClassEx (&wndClass);

  wndClass.lpfnWndProc   = WinDrawProc;
  wndClass.hIcon         = NULL;
  wndClass.lpszClassName = "draw";
  wndClass.hIconSm       = NULL;

  RegisterClassEx (&wndClass);

  WinCreateMainW(appName, winArgP->hInstance, winArgP->nCmdShow);

  return PU_RES_OK;
}

PuRes
PuWinCleanup(void)
{
  return PU_RES_OK;
}
