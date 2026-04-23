/*
************************************************************************
*
*   UiMenu.c - build menus
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/07/24
*   Pathname of SCCS file     : /local/home/kor/molmol/src/ui/SCCS/s.UiMenu.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <ui_menu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <dstr.h>
#include <g_file.h>
#include <prog_vers.h>
#include <pu.h>
#include <par_names.h>
#include <setup_file.h>
#include <cip.h>

#define K_PULLRIGHT "pullright"
#define K_ENTRY     "entry"
#define K_END       "end"
#define K_ACCEL     "accel"

#define LINE_LEN 200
#define KEY_WORD_LEN 20
#define MENU_STR_LEN 50
#define CMD_STR_LEN 100
#define ACC_LEN 50
#define ACC_STR_LEN 20

static PuMenu Popup = NULL;
static PuMenubar Menubar = NULL;
static PuMenu OptMenu = NULL;

static void
menuCB(PuMenuEntry entry, void *clientData, PuMenuCBStruc *callP)
{
  if (callP->act == PU_MA_ACTIVATE)
    (void) CipExecCmd(clientData);
  else if (callP->act == PU_MA_ENTER)
    CipShowHelpLine(clientData);
  else
    PuSetTextField(PU_TF_STATUS, " ");
}

static char *
skipWhite(char *s)
{
  while (isspace(*s))
    s++;

  return s;
}

static char *
getString(char *resStr, int resSize, char *line)
{
  char quoteCh;
  int i = 0;

  if (line[0] == '"' || line[0] == '\'') {
    quoteCh = line[0];
    line++;
    while (line[i] != '\0' && line[i] != quoteCh && i < resSize - 1) {
      resStr[i] = line[i];
      i++;
    }
    resStr[i] = '\0';
    line += i;
    if (line[0] == quoteCh)
      line++;
  } else {
    while (line[i] != '\0' && !isspace(line[i]) && i < resSize - 1) {
      resStr[i] = line[i];
      i++;
    }
    resStr[i] = '\0';
    line += i;
  }

  return line;
}

static void
makeOneMenu(PuMenu menu, DSTR name, GFile gf)
{
  GFileRes res;
  BOOL pendingEntry;
  char lineBuf[LINE_LEN];
  char keyWord[KEY_WORD_LEN];
  char mnemonic;
  char menuStr[MENU_STR_LEN];
  char cmdStr[CMD_STR_LEN], *cmdCopy;
  char acc[ACC_LEN];
  char accStr[ACC_STR_LEN];
  char *posP;
  PuMenu subMenu;
  DSTR subName;
  int i;

  pendingEntry = FALSE;

  while (! GFileEOF(gf)) {
    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK)
      return;

    if (lineBuf[0] == '#')
      continue;
    
    posP = skipWhite(lineBuf);

    if (*posP == '\0')
      continue;

    i = 0;
    while (isalnum(posP[i]) && i < sizeof(keyWord) - 1) {
      keyWord[i] = posP[i];
      i++;
    }
    keyWord[i] = '\0';
    posP += i;
    posP = skipWhite(posP);

    if (strcmp(keyWord, K_ACCEL) == 0) {
      posP = getString(acc, sizeof(acc), posP);
      posP = skipWhite(posP);
      posP = getString(accStr, sizeof(accStr), posP);
    }

    if (pendingEntry) {
      (void) PuCreateMenuEntry(menu, menuStr, mnemonic,
	  acc, accStr, menuCB, cmdCopy, free);
      if (name != NULL)
	CipSetMenu(cmdCopy, DStrToStr(name));
      pendingEntry = FALSE;
    }

    if (strcmp(keyWord, K_PULLRIGHT) == 0 ||
	strcmp(keyWord, K_ENTRY) == 0) {
      mnemonic = posP[0];
      posP = skipWhite(posP + 1);
      posP = getString(menuStr, sizeof(menuStr), posP);
      if (strcmp(keyWord, K_PULLRIGHT) == 0) {
	subMenu = PuCreatePullright(menu, menuStr, mnemonic);
	if (name == NULL) {
	  makeOneMenu(subMenu, NULL, gf);
	} else {
	  subName = DStrNew();
	  DStrAssignDStr(subName, name);
	  DStrAppStr(subName, "->");
	  DStrAppStr(subName, menuStr);
	  makeOneMenu(subMenu, subName, gf);
	  DStrFree(subName);
	}
      } else {
	posP = skipWhite(posP);
	posP = getString(cmdStr, sizeof(cmdStr), posP);
	if (cmdStr[0] == '\0') {
	  cmdCopy = malloc(strlen(menuStr) + 1);
	  (void) strcpy(cmdCopy, menuStr);
	} else {
	  cmdCopy = malloc(strlen(cmdStr) + 1);
	  (void) strcpy(cmdCopy, cmdStr);
	}
	acc[0] = '\0';
	accStr[0] = '\0';
	pendingEntry = TRUE;
      }
    } else if (strcmp(keyWord, K_END) == 0) {
      break;
    }
  }

  if (pendingEntry) {
    (void) PuCreateMenuEntry(menu, menuStr, mnemonic,
	acc, accStr, menuCB, cmdCopy, free);
    if (name != NULL)
      CipSetMenu(cmdCopy, DStrToStr(name));
  }
}

static void
menuAddCB(char *cmd, BOOL hasArg)
{
  DSTR str;
  char *cmdCopy;

  if (OptMenu == NULL)
    OptMenu = PuCreatePulldown(Menubar, "Opt", 'O');

  str = DStrNew();
  DStrAssignStr(str, cmd);
  if (hasArg)
    DStrAppStr(str, "...");

  cmdCopy = malloc(strlen(cmd) + 1);
  (void) strcpy(cmdCopy, cmd);

  (void) PuCreateMenuEntry(OptMenu, DStrToStr(str), '\0',
      "", "", menuCB, cmdCopy, free);
}

void
UiMenuCreateBar(void)
{
  GFile gf, gfSub;
  GFileRes res;
  PuMenu menu;
  char lineBuf[LINE_LEN];
  char *menuName;
  DSTR menuStr;
  char mnemonic;

  gf = SetupOpen(PN_MENU_DIR, "MenuBar", TRUE);
  if (gf == NULL) {
    PuShowMessage(PU_MT_WARNING, "Create Menu",
	"MenuBar file could not be opened,\n"
	"check installation (setting of " PROG_NAME "HOME)!");
    return;
  }

  Menubar = PuCreateMenubar();
  menuStr = DStrNew();

  while (! GFileEOF(gf)) {
    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK)
      /* error */
      break;

    mnemonic = lineBuf[0];

    if (! isalnum(mnemonic))
      continue;
    
    menuName = lineBuf + 1;
    while (isspace(*menuName))
      menuName++;

    menu = PuCreatePulldown(Menubar, menuName, mnemonic);

    DStrAssignStr(menuStr, menuName);
    DStrAppStr(menuStr, ".menu");
    gfSub = SetupOpen(PN_MENU_DIR, DStrToStr(menuStr), TRUE);

    if (gfSub != NULL) {
      DStrAssignStr(menuStr, menuName);
      makeOneMenu(menu, menuStr, gfSub);
    }

    GFileClose(gfSub);
  }

  GFileClose(gf);
  DStrFree(menuStr);

  CipSetMenuAddCB(menuAddCB);
  OptMenu = NULL;
}

void
UiMenuSwitchBar(BOOL onOff)
{
  if (Menubar != NULL)
    PuSwitchMenubar(Menubar, onOff);
}

void
UiMenuCreatePopup(void)
{
  GFile gf;

  gf = SetupOpen(PN_MENU_DIR, "Popup", TRUE);
  if (gf == NULL)
    return;

  Popup = PuCreatePopup("popup");
  makeOneMenu(Popup, NULL, gf);

  GFileClose(gf);
}

void
UiMenuSwitchPopup(BOOL onOff)
{
  if (Popup != NULL)
    PuSwitchPopup(Popup, onOff);
}
