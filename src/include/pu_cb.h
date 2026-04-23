/*
************************************************************************
*
*   pu_cb.h - callback management, only used inside Pu
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.pu_cb.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <pu_types.h>

extern void PuInitCallbacks(void);

extern void PuCallExposeCB(PuWindow, int, int, int, int);

extern void PuCallResizeCB(PuWindow, int, int);

extern void PuCallKeyCB(PuWindow, char);

extern void PuCallButtonPressCB(PuWindow, PuMouseButton, BOOL, int, int);

extern void PuCallButtonReleaseCB(PuWindow, PuMouseButton, BOOL, int, int);

extern void PuCallDragCB(PuWindow, PuMouseButton, int, int);

extern void PuCallMoveCB(PuWindow, int, int);

extern void PuCallCmdCB(PuWindow, PuTextCBStruc *);

extern void PuCallDropCB(PuWindow, char **, int);

extern void PuCallQuitCB(void);
