/*
************************************************************************
*
*   pu_types.h - types used for Pu
*
*   Copyright (c) 1994-98
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.pu_types.h
*   SCCS identification       : 1.15
*
************************************************************************
*/

#ifndef _PU_TYPES_H_
#define _PU_TYPES_H_

#include <bool.h>

typedef void *PuWindow;

typedef enum {
  PU_RES_OK,
  PU_RES_ERR
} PuRes;

typedef void (* PuFreeCB) (void *);

#define PU_MOUSE_BUTTON_NO 5

typedef enum {
  PU_MB_1,
  PU_MB_2,
  PU_MB_3,
  PU_MB_4,
  PU_MB_5,
  PU_MB_NONE
} PuMouseButton;

typedef enum {
  PU_CA_INSERT,
  PU_CA_APPEND,
  PU_CA_DELETE,
  PU_CA_REPLACE,
  PU_CA_CR
} PuTextAction;

typedef struct {
  int x, y, w, h;
} PuExposeCBStruc;

typedef void (* PuExposeCB) (PuWindow, void *, PuExposeCBStruc *);

typedef struct {
  int w, h;
} PuResizeCBStruc;

typedef void (* PuResizeCB) (PuWindow, void *, PuResizeCBStruc *);

typedef struct {
  char ch;
} PuKeyCBStruc;

typedef void (* PuKeyCB) (PuWindow, void *, PuKeyCBStruc *);

typedef struct {
  PuMouseButton button;
  BOOL pressed;
  BOOL addSel;
  BOOL doSel;
  int x, y;
} PuMouseCBStruc;

typedef void (* PuMouseCB) (PuWindow, void *, PuMouseCBStruc *);

typedef struct {
  PuTextAction action;
  char *oldText;  /* complete text before change */
  int oldLen;
  char *oldPart;  /* for DELETE and REPLACE */
  int oldPartStart, oldPartEnd;
  char *newText;  /* complete text after change */
  int newLen;
  char *newPart;  /* for INSERT, APPEND and REPLACE */
  int newPartStart, newPartEnd;
  BOOL doIt;
} PuTextCBStruc;

typedef void (* PuCmdCB) (PuWindow, void *, PuTextCBStruc *);

typedef struct {
  char **fileNameA;
  int fileNo;
} PuDropCBStruc;

typedef void (* PuDropCB) (PuWindow, void *, PuDropCBStruc *);

typedef void (* PuQuitCB) (void *);

typedef void (* PuTimeOut) (void *);

typedef void (* PuExtInput) (int, void *);

typedef void *PuMenubar;

typedef void *PuMenu;

typedef void *PuMenuEntry;

typedef enum {
  PU_MA_ENTER,
  PU_MA_ACTIVATE,
  PU_MA_LEAVE
} PuMenuAction;

typedef struct {
  PuMenuAction act;
  char *name;
} PuMenuCBStruc;

typedef void (* PuMenuCB) (PuMenuEntry, void *, PuMenuCBStruc *);

typedef void *PuCmdField;

typedef struct {
  PuMenuAction act;
  char *name;
} PuCmdFieldCBStruc;

typedef void (* PuCmdFieldCB) (PuCmdField, void *, PuCmdFieldCBStruc *);

typedef void *PuValuatorBox;

typedef void *PuValuator;

typedef struct {
  PuMenuAction act;
  char *name;
  float val;
} PuValuatorCBStruc;

typedef void (* PuValuatorCB) (PuValuator, void *, PuValuatorCBStruc *);

typedef void *PuTextWindow;

typedef enum {
  PU_TF_TITLE,
  PU_TF_PROMPT,
  PU_TF_CMD,
  PU_TF_STATUS
} PuTextFieldChoice;

typedef enum {
  PU_MT_BUSY,
  PU_MT_INFO,
  PU_MT_QUESTION,
  PU_MT_WARNING,
  PU_MT_ERROR
} PuMessageType;

typedef enum {
  PU_FA_READ,
  PU_FA_WRITE
} PuFileAccess;

typedef void *PuGizmo;

typedef enum {
  PU_BC_PLACE_OUTSIDE,
  PU_BC_MULT_SEL,
  PU_BC_AUTO_DESEL,
  PU_BC_SHOW_BOTTOM
} PuBoolChoice;

typedef enum {
  PU_IC_TEXT_WIDTH,
  PU_IC_TEXT_HEIGHT,
  PU_IC_HISTORY_SIZE,
  PU_IC_ENTRY_NO,
  PU_IC_LINE_NO
} PuIntChoice;

typedef enum {
  PU_SC_TEXT
} PuStrChoice;

typedef struct {
  int x, y, w, h;
} PuConstraints;

typedef enum {
  PU_CT_ACTIVATE,
  PU_CT_MODIFY,
  PU_CT_SELECT,
  PU_CT_OK,
  PU_CT_CLOSE,
  PU_CT_APPLY,
  PU_CT_RESET,
  PU_CT_CANCEL,
  PU_CT_HELP,
  PU_CT_DESTROY
} PuGizmoCBType;

typedef struct {
  int oldVal, newVal;
} PuIntCBStruc;

typedef struct {
  float oldVal, newVal;
} PuFloatCBStruc;

typedef struct {
  BOOL onOff;
  BOOL last;
} PuSelectCBStruc;

typedef struct {
  char *fileName;
} PuFileSelCBStruc;

typedef void (* PuGizmoCB) (PuGizmo, char *, void *, void *);

#endif  /* _PU_TYPES_H_ */
