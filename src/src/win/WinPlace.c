/*
************************************************************************
*
*   WinPlace.c - Windows dialog placement
*
*   Copyright (c) 1997
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
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.WinPlace.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "win_place.h"

#include "win_main_w.h"

typedef enum {
  DP_LEFT,
  DP_RIGHT
} DialogPlace;

static BOOL FirstPlaced = FALSE;
static int ScreenW, ScreenH;
static DialogPlace Place;
static int PlaceX, PlaceY;
static BOOL Moving = FALSE;

static void
placeInside(HWND w)
{
  HWND mainW;
  RECT mainRect;
  int mw, mh;
  RECT rect;
  int dx, dy, dw, dh;

  mainW = WinGetMainW();
  GetWindowRect(mainW, &mainRect);
  mw = mainRect.right - mainRect.left;
  mh = mainRect.bottom - mainRect.top;

  GetWindowRect(w, &rect);
  dw = rect.right - rect.left;
  dh = rect.bottom - rect.top;

  dx = mainRect.left + mw / 2 - dw / 2;
  dy = mainRect.top + mh / 2 - dh / 2;

  MoveWindow(w, dx, dy, dw, dh, TRUE);
}

static void
placeOutside(HWND w)
{
  HWND mainW;
  RECT mainRect;
  RECT rect;
  int dx, dy, dw, dh;

  if (! FirstPlaced) {
    ScreenW = GetSystemMetrics(SM_CXFULLSCREEN);
    ScreenH = GetSystemMetrics(SM_CYFULLSCREEN);

    mainW = WinGetMainW();
    GetWindowRect(mainW, &mainRect);

    if (mainRect.left > ScreenW - mainRect.right) {
      Place = DP_LEFT;
      PlaceX = mainRect.left;
    } else {
      Place = DP_RIGHT;
      PlaceX = mainRect.right;
    }

    PlaceY = mainRect.top;

    FirstPlaced = TRUE;
  }

  GetWindowRect(w, &rect);
  dw = rect.right - rect.left;
  dh = rect.bottom - rect.top;

  if (Place == DP_LEFT) {
    dx = PlaceX - dw;
    if (dx < 0)
      dx = 0;
  } else {
    dx = PlaceX;
    if (dx + dw > ScreenW)
      dx = ScreenW - dw;
  }

  dy = PlaceY;
  if (dy + dh > ScreenH)
    dy = ScreenH - dh;
  
  Moving = TRUE;
  MoveWindow(w, dx, dy, dw, dh, TRUE);
  Moving = FALSE;

  PlaceY = dy + dh;
}

void
WinPlace(HWND w, BOOL outside)
{
  if (outside)
    placeOutside(w);
  else
    placeInside(w);

  SendMessage(w, WM_SETICON, ICON_SMALL,
      GetClassLong(WinGetMainW(), GCL_HICONSM));
}

void
WinPlaceChanged(HWND w)
{
  RECT rect;

  if (Moving)
    return;

  GetWindowRect(w, &rect);

  if (Place == DP_LEFT)
    PlaceX = rect.right;
  else
    PlaceX = rect.left;

  PlaceY = rect.bottom;
}
