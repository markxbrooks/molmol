/*
************************************************************************
*
*   ui_menu.h - menu creation
*
*   Copyright (c) 1994
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
*   Date of last modification : 94/08/17
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.ui_menu.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _UI_MENU_H_
#define _UI_MENU_H_

#include <bool.h>

extern void UiMenuCreateBar(void);

extern void UiMenuSwitchBar(BOOL);

extern void UiMenuCreatePopup(void);

extern void UiMenuSwitchPopup(BOOL);

#endif  /* _UI_MENU_H_ */
