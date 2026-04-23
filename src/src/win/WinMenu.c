/*
************************************************************************
*
*   WinMenu.c - Windows menus
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
*   Date of last modification : 97/11/05
*   Pathname of SCCS file     : /tmp_mnt/files/kor/molmol/src/win/SCCS/s.WinMenu.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "win_menu.h"

#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "win_main_w.h"

#define MENU_BASE 500

typedef struct {
  char *name;
  PuMenuCB menuCB;
  void *clientData;
} EntryInfo;

static int EntryNo = 0, EntryListSize;
static EntryInfo *EntryList;
static int AccelNo = 0, AccelListSize;
static ACCEL *AccelList;
static HACCEL AccelTable = NULL;
static HMENU HelpMenu = NULL, PopupMenu = NULL;
static BOOL MenuBarOn = FALSE, PopupOn = FALSE;

static char *
putMnemonic(char *name, char mnemonic, char *accText)
/* windows requires an ampersand before the mnemonic */
{
  int len;
  char *outName;
  int i;

  len = strlen(name);
  outName = malloc(len + 2);
  outName[len] = '\0';
  for (i = 0; i < len; i++)
    if (name[i] == mnemonic) {
      outName[i] = '&';
      (void) strcpy(outName + i + 1, name + i);
      break;
    } else {
      outName[i] = name[i];
    }

  if (accText != NULL && accText[0] != '\0') {
    len = strlen(outName);
    outName = realloc(outName, len + strlen(accText) + 2);
    outName[len] = '\t';
    (void) strcpy(outName + len + 1, accText);
  }

  return outName;
}


PuMenubar
PuWinCreateMenubar(void)
{
  HMENU menubar;

  menubar = CreateMenu();

  return (PuMenubar) menubar;
}

PuMenu
PuWinCreatePulldown(PuMenubar menubar, char *name, char mnemonic)
{
  HMENU menu;
  char *outName;

  menu = CreateMenu();
  outName = putMnemonic(name, mnemonic, NULL);
  if (HelpMenu == NULL)
    AppendMenu((HMENU) menubar, MF_POPUP, (UINT) menu, outName);
  else
    InsertMenu((HMENU) menubar, (UINT) HelpMenu,
        MF_POPUP, (UINT) menu, outName);
  free(outName);

  if (MenuBarOn)
    DrawMenuBar(WinGetMainW());

  if (strcmp(name, "Help") == 0)
    HelpMenu = menu;

  return (PuMenu) menu;
}

PuMenu
PuWinCreatePopup(char *name)
{
  HMENU popup;

  if (GetSystemMetrics(SM_CMOUSEBUTTONS) < 3)
    /* only create popup menu for three-button mouse */
    return NULL;

  popup = CreatePopupMenu();

  return (PuMenu) popup;
}

PuMenu
PuWinCreatePullright(PuMenu menu, char *name, char mnemonic)
{
  HMENU pullright;
  char *outName;

  pullright = CreateMenu();
  outName = putMnemonic(name, mnemonic, NULL);
  AppendMenu((HMENU) menu, MF_POPUP, (UINT) pullright, outName);
  free(outName);

  return (PuMenu) pullright;
}

PuMenuEntry
PuWinCreateMenuEntry(PuMenu menu, char *name,
    char mnemonic, char *acc, char *accText,
    PuMenuCB menuCB, void *clientData, PuFreeCB freeCB)
{
  char *outName;
  EntryInfo *entryP;

  outName = putMnemonic(name, mnemonic, accText);
  AppendMenu((HMENU) menu, MF_STRING, MENU_BASE + EntryNo, outName);
  free(outName);

  EntryNo++;

  if (EntryNo == 1) {
    EntryListSize = 10;
    EntryList = malloc(EntryListSize * sizeof(*EntryList));
  } else if (EntryNo > EntryListSize) {
    EntryListSize *= 2;
    EntryList = realloc(EntryList, EntryListSize * sizeof(*EntryList));
  }

  entryP = EntryList + EntryNo - 1;
  entryP->name = malloc(strlen(name) + 1);
  (void) strcpy(entryP->name, name);
  entryP->menuCB = menuCB;
  entryP->clientData = clientData;

  if (acc != NULL && acc[0] != '\0') {
    if (AccelNo == 0) {
      AccelListSize = 10;
      AccelList = malloc(AccelListSize * sizeof(*AccelList));
    } else if (AccelNo >= AccelListSize) {
      AccelListSize *= 2;
      AccelList = realloc(AccelList, AccelListSize * sizeof(*AccelList));
    }

    AccelList[AccelNo].fVirt = 0;
    AccelList[AccelNo].cmd = MENU_BASE + EntryNo - 1;

    while (acc[0] != '\0') {
      if (strncmp(acc, "Ctrl", 4) == 0) {
        /* AccelList[AccelNo].fVirt |= FCONTROL; */
	acc += 4;
      } else if (strncmp(acc, "Alt", 3) == 0) {
        AccelList[AccelNo].fVirt |= FALT;
	acc += 3;
      } else if (strncmp(acc, "<Key>", 5) == 0) {
        acc += 5;
      } else {
	if (acc[0] >= 'a' && acc[0] <= 'z')
	  AccelList[AccelNo].key = acc[0] - 'a' + 1;
	else if (acc[0] >= 'A' && acc[0] <= 'Z')
	  AccelList[AccelNo].key = acc[0] - 'A' + 1;
	else
	  AccelList[AccelNo].key = acc[0];

	acc++;
      }
    }

    AccelNo++;
  }

  return (PuMenuEntry) EntryNo;
}

void
PuWinSwitchMenubar(PuMenubar menubar, BOOL onOff)
{
  HWND mainW;

  if (onOff == MenuBarOn)
    return;

  mainW = WinGetMainW();
  if (onOff) {
    SetMenu(mainW, (HMENU) menubar);
    if (AccelTable == NULL)
      AccelTable = CreateAcceleratorTable(AccelList, AccelNo);
  } else {
    SetMenu(mainW, NULL);
  }

  MenuBarOn = onOff;
}

void
PuWinSwitchPopup(PuMenu menu, BOOL onOff)
{
  if (onOff)
    PopupMenu = (HMENU) menu;
  else
    PopupMenu = NULL;
}

void
WinMenuActivate(int childId, PuMenuAction act)
{
  EntryInfo *entryP;
  PuMenuCBStruc menuStruc;

  entryP = EntryList + childId - MENU_BASE;
  if (entryP->menuCB == NULL)
    return;

  menuStruc.act = act;
  menuStruc.name = entryP->name;
  entryP->menuCB((PuMenuEntry) childId, entryP->clientData, &menuStruc);
}

HACCEL
WinGetAccelTable(void)
{
  return AccelTable;
}

HMENU
WinGetPopup(void)
{
  return PopupMenu;
}
