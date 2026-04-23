/*
************************************************************************
*
*   MotifDial.c - build Motif dialog boxes
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/motif/SCCS/s.MotifDial.c
*   SCCS identification       : 1.22
*
************************************************************************
*/

#include "motif_dial.h"

#include <string.h>
#include <stdlib.h>

#include <X11/keysym.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ArrowB.h>
#include <Xm/Scale.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>
#include <Xm/List.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>

#include <linlist.h>
#include "motif_place.h"
#include "motif_text.h"
#include "motif_access.h"
#include "motif_busy.h"

#define LABEL_OK     "OK"
#define LABEL_CLOSE  "Close"
#define LABEL_APPLY  "Apply"
#define LABEL_RESET  "Reset"
#define LABEL_CANCEL "Cancel"
#define LABEL_HELP   "Help"

#define FLOAT2SHORT(f) ((short) (f * ((1 << (8 * sizeof(short))) - 1.0E-4)))

typedef enum {
  GIZMO_FILE_SELECTION_BOX,
  GIZMO_DIALOG,
  GIZMO_LABEL,
  GIZMO_BUTTON,
  GIZMO_COLOR_FIELD,
  GIZMO_SLIDER,
  GIZMO_TEXT_FIELD,
  GIZMO_TEXT,
  GIZMO_FILE_VIEWER,
  GIZMO_LIST,
  GIZMO_RADIO_BOX,
  GIZMO_CHECK_BOX,
  GIZMO_TOGGLE
} GizmoType;

typedef struct {
  GizmoType type;
  char *name;
  union {
    struct {
      Widget buttonFormW, okW, closeW, applyW, resetW, cancelW, helpW;
      int buttonNo;
      int xBase, yBase;
      LINLIST childList;
    } dialog;
    struct {
      int factor;
      float lastVal;
    } slider;
    struct {
      int historySize;
      LINLIST historyList;
      char **historyCurr;
      BOOL firstMod;
    } text;
    struct {
      int size;
      BOOL autoDesel;
      BOOL showBottom;
      BOOL *sel;
    } list;
    struct {
      Widget labelW, boxW;
      BOOL lineNoSet;
      int toggleNo;
    } box;
  } u;
} WidgetInfo;

typedef struct {
  PuGizmoCB gizmoCB;
  char *name;
  void *clientData;
  PuFreeCB freeCB;
} GizmoCBInfo;

static WidgetInfo *
getWidgetInfo(Widget w)
{
  WidgetInfo *infoP;

  XtVaGetValues(w, XmNuserData, &infoP, NULL);

  return infoP;
}

static Widget
getFormChild(Widget w)
{
  if (getWidgetInfo(w)->type == GIZMO_TEXT_FIELD)
    w = XtParent(w);

  while (XtClass(XtParent(w)) != xmFormWidgetClass)
    w = XtParent(w);

  return w;
}

static WidgetInfo *
getDialogInfo(Widget w)
{
  return getWidgetInfo(XtParent(getFormChild(w)));
}

static void
freeWidgetInfoCB(Widget w, XtPointer clientData, XtPointer callData)
{
  WidgetInfo *infoP = clientData;

  free(infoP->name);
  if (infoP->type == GIZMO_DIALOG)
    ListClose(infoP->u.dialog.childList);
  else if (infoP->type == GIZMO_TEXT_FIELD && infoP->u.text.historySize > 0)
    ListClose(infoP->u.text.historyList);
  else if (infoP->type == GIZMO_LIST && infoP->u.list.sel != NULL)
    free(infoP->u.list.sel);
  free(infoP);
}

static WidgetInfo *
addWidgetInfo(Widget w, GizmoType type, char *name)
{
  WidgetInfo *infoP, *dialInfoP;
  Widget *prevWP;

  infoP = malloc(sizeof(*infoP));
  infoP->type = type;
  infoP->name = malloc(strlen(name) + 1);
  (void) strcpy(infoP->name, name);

  XtVaSetValues(w,
      XmNuserData, infoP,
      NULL);

  XtAddCallback(w, XmNdestroyCallback, freeWidgetInfoCB, infoP);

  if (type != GIZMO_FILE_SELECTION_BOX && type != GIZMO_DIALOG &&
      type != GIZMO_TOGGLE) {
    dialInfoP = getDialogInfo(w);

    if (dialInfoP->u.dialog.yBase == 0) {
      /* default attachments */
      prevWP = ListLast(dialInfoP->u.dialog.childList);
      if (prevWP == NULL) {
	XtVaSetValues(getFormChild(w),
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM,
	    NULL);
      } else {
	XtVaSetValues(*prevWP,
	    XmNbottomAttachment, XmATTACH_NONE,
	    NULL);
	XtVaSetValues(getFormChild(w),
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, *prevWP,
	    XmNbottomAttachment, XmATTACH_FORM,
	    NULL);
      }
    }

    (void) ListInsertLast(dialInfoP->u.dialog.childList, &w);
  }

  return infoP;
}

static void
sliderCB(Widget w, XtPointer clientData, XtPointer callData)
{
  GizmoCBInfo *infoP = clientData;
  WidgetInfo *wInfoP = getWidgetInfo(w);
  XmScaleCallbackStruct *callP = callData;
  float floatVal;
  PuFloatCBStruc floatStruc;

  floatVal = (float) callP->value / wInfoP->u.slider.factor;
  if (floatVal != wInfoP->u.slider.lastVal) {
    floatStruc.oldVal = wInfoP->u.slider.lastVal;
    floatStruc.newVal = floatVal;
    wInfoP->u.slider.lastVal = floatVal;
    infoP->gizmoCB(w, infoP->name, infoP->clientData, &floatStruc);
  }
}

static void
gizmoGenCB(Widget w, XtPointer clientData, XtPointer callData)
{
  GizmoCBInfo *infoP = clientData;

  infoP->gizmoCB(w, infoP->name, infoP->clientData, NULL);
}

static void
addHistory(WidgetInfo *infoP, char *str)
{
  LINLIST list;
  char *strCpy;

  if (infoP->u.text.historySize == 0)
    return;

  list = infoP->u.text.historyList;
  strCpy = malloc(strlen(str) + 1);
  (void) strcpy(strCpy, str);
  (void) ListInsertLast(list, &strCpy);
  if (ListSize(list) > infoP->u.text.historySize)
    ListRemove(list, ListFirst(list));

  infoP->u.text.historyCurr = NULL;
}

static void
textVerifyCB(Widget w, XtPointer clientData, XtPointer callData)
{
  WidgetInfo *wInfoP = getWidgetInfo(w);
  PuTextCBStruc textStruc;
  GizmoCBInfo *infoP = clientData;

  textStruc.oldText = XmTextGetString(w);

  if (TextStrucMotifToPu(callData, &textStruc)) {
    infoP->gizmoCB(w, infoP->name, infoP->clientData, &textStruc);
    TextStrucPuToMotif(&textStruc, callData);

    if (textStruc.doIt &&
	wInfoP->u.text.historySize > 0 && wInfoP->u.text.firstMod) {
      addHistory(wInfoP, textStruc.oldText);
      wInfoP->u.text.firstMod = FALSE;
    }
  }

  XtFree(textStruc.oldText);
}

static void
textActivateCB(Widget w, XtPointer clientData, XtPointer callData)
{
  WidgetInfo *wInfoP = getWidgetInfo(w);
  GizmoCBInfo *infoP = clientData;

  infoP->gizmoCB(w, infoP->name, infoP->clientData, NULL);

  if (wInfoP->u.text.historySize > 0)
    wInfoP->u.text.firstMod = TRUE;
}

static void
listSelCB(Widget w, XtPointer clientData, XtPointer callData)
{
  GizmoCBInfo *infoP = clientData;
  WidgetInfo *wInfoP = getWidgetInfo(w);
  int listSize = wInfoP->u.list.size;
  PuSelectCBStruc selectStruc;
  BOOL *sel;
  int *posP, posCount;
  XmStringTable itemList;
  char **cbNames;
  BOOL *cbSel;
  int cbNo;
  int i;

  sel = malloc(listSize * sizeof(BOOL));
  for (i = 0; i < listSize; i++)
    sel[i] = FALSE;
  if (XmListGetSelectedPos(w, &posP, &posCount)) {
    for (i = 0; i < posCount; i++)
      sel[posP[i] - 1] = TRUE;
    XtFree((char *) posP);
  }

  XtVaGetValues(w, XmNitems, &itemList, NULL);

  cbNames = malloc(listSize * sizeof(*cbNames));
  cbSel = malloc(listSize * sizeof(*cbSel));
  cbNo = 0;

  for (i = 0; i < listSize; i++) {
    if (wInfoP->u.list.sel[i] == sel[i])
      continue;
    
    /* store name instead of calling callback immediately,
       because callback can modify list! */
    cbSel[cbNo] = sel[i];
    (void) XmStringGetLtoR(itemList[i],
	XmSTRING_DEFAULT_CHARSET, cbNames + cbNo);
    cbNo++;

    wInfoP->u.list.sel[i] = sel[i];
  }

  for (i = 0; i < cbNo; i++) {
    selectStruc.onOff = cbSel[i];
    selectStruc.last = (i == cbNo - 1);
    infoP->gizmoCB(w, cbNames[i], infoP->clientData, &selectStruc);
    XtFree(cbNames[i]);
  }

  if (wInfoP->u.list.autoDesel)
    for (i = 0; i < listSize; i++)
      if (sel[i]) {
	XmListDeselectPos(w, i + 1);
	wInfoP->u.list.sel[i] = FALSE;
      }

  free(sel);
  free(cbNames);
  free(cbSel);
}

static void
fileSelOkCB(Widget w, XtPointer clientData, XtPointer callData)
{
  GizmoCBInfo *infoP = clientData;
  XmFileSelectionBoxCallbackStruct *callP = callData;
  PuFileSelCBStruc fileSelStruc;

  (void) XmStringGetLtoR(callP->value,
      XmSTRING_DEFAULT_CHARSET, &fileSelStruc.fileName);
  infoP->gizmoCB(w, infoP->name, infoP->clientData, &fileSelStruc);
  XtFree(fileSelStruc.fileName);
}

static void
radioCB(Widget w, XtPointer clientData, XtPointer callData)
{
  GizmoCBInfo *infoP = clientData;
  XmToggleButtonCallbackStruct *callP = callData;
  PuSelectCBStruc selectStruc;
  XmString str;
  char *itemName;

  selectStruc.onOff = callP->set;
  XtVaGetValues(w, XmNlabelString, &str, NULL);
  (void) XmStringGetLtoR(str, XmSTRING_DEFAULT_CHARSET, &itemName);
  XmStringFree(str);
  /* Toggle -> RadioBox -> Form */
  infoP->gizmoCB(XtParent(XtParent(w)),
      itemName, infoP->clientData, &selectStruc);
  XtFree(itemName);
}

static void
destroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
  GizmoCBInfo *infoP = clientData;

  if (infoP->freeCB != NULL)
    infoP->freeCB(infoP->clientData);
  /* don't free name here, it's either a pointer to the name in
     WidgetInfo or static data */
  free(infoP);
}

PuGizmo
PuMotifCreateFileSelectionBox(char *name,
    char *dir, char *pattern, PuFileAccess acc)
{
  Widget dialogW, fileW;
  XmString nameStr, dirStr, patternStr;

  dialogW = XmCreateDialogShell(MotifGetTopW(),
      "Dialog", NULL, 0);
  XtVaSetValues(dialogW,
      XmNtitle, name,
      XmNdeleteResponse, XmDO_NOTHING,
      NULL);
  MotifAddDialog(dialogW);

  fileW = XmCreateFileSelectionBox(dialogW, "FileSel", NULL, 0);
  nameStr = XmStringCreateSimple(name);
  dirStr = XmStringCreateSimple(dir);
  patternStr = XmStringCreateSimple(pattern);
  XtVaSetValues(fileW,
      XmNdirListLabelString, nameStr,
      XmNdirectory, dirStr,
      XmNpattern, patternStr,
      NULL);
  XmStringFree(nameStr);
  XmStringFree(dirStr);
  XmStringFree(patternStr);

  (void) addWidgetInfo(fileW, GIZMO_FILE_SELECTION_BOX, name);
  
  return fileW;
}

PuGizmo
PuMotifCreateDialog(char *name, int xBase, int yBase)
{
  Widget dialogW, dummyW, paneW, formW;
  int base;
  WidgetInfo *infoP;

  dialogW = XmCreateDialogShell(MotifGetTopW(),
      "Dialog", NULL, 0);
  XtVaSetValues(dialogW,
      XmNtitle, name,
      XmNdeleteResponse, XmDO_NOTHING,
      NULL);
  MotifAddDialog(dialogW);
  
  /* Dialogs don't get properly placed when a PanedWindow is the
     child of the DialogShell. Put a Form in between. */
  dummyW = XmCreateForm(dialogW, "DialogForm", NULL, 0);

  paneW = XmCreatePanedWindow(dummyW, "PanedWindow", NULL, 0);
  XtVaSetValues(paneW,
      XmNsashWidth, 1,
      XmNsashHeight, 1,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);

  formW = XmCreateForm(paneW, "Form", NULL, 0);
  base = xBase * yBase;
  if (base == 0)
    base = 1;
  XtVaSetValues(formW,
      XmNautoUnmanage, False,
      XmNfractionBase, base,
      NULL);
  
  infoP = addWidgetInfo(formW, GIZMO_DIALOG, name);
  infoP->u.dialog.xBase = xBase;
  infoP->u.dialog.yBase = yBase;
  infoP->u.dialog.childList = ListOpen(sizeof(Widget));
  infoP->u.dialog.okW = NULL;
  infoP->u.dialog.closeW = NULL;
  infoP->u.dialog.applyW = NULL;
  infoP->u.dialog.resetW = NULL;
  infoP->u.dialog.cancelW = NULL;
  infoP->u.dialog.helpW = NULL;
  infoP->u.dialog.buttonNo = 0;
  infoP->u.dialog.buttonFormW = XmCreateForm(paneW, "Form", NULL, 0);

  return formW;
}

PuGizmo
PuMotifCreateLabel(PuGizmo dial, char *name)
{
  Widget labelW;
  XmString str;

  labelW = XmCreateLabel(dial, "Label", NULL, 0);
  str = XmStringCreateSimple(name);
  XtVaSetValues(labelW,
      XmNalignment, XmALIGNMENT_BEGINNING,
      XmNlabelString, str,
      NULL);
  XmStringFree(str);
  
  (void) addWidgetInfo(labelW, GIZMO_LABEL, name);
  
  return labelW;
}

PuGizmo
PuMotifCreateButton(PuGizmo dial, char *name)
{
  Widget buttonW;
  XmString str;

  buttonW = XmCreatePushButton(dial, "Button", NULL, 0);
  str = XmStringCreateSimple(name);
  XtVaSetValues(buttonW,
      XmNlabelString, str,
      NULL);
  XmStringFree(str);
  
  (void) addWidgetInfo(buttonW, GIZMO_BUTTON, name);
  
  return buttonW;
}

PuGizmo
PuMotifCreateColorField(PuGizmo dial, char *name)
{
  Widget colorW;
  Display *dpy;
  int screen;
  Pixel pix;

  colorW = XtCreateWidget("ColorField", xmPrimitiveWidgetClass,
      dial, NULL, 0);
  XtVaSetValues(colorW,
      XmNborderWidth, 1,
      NULL);

  dpy = XtDisplay(colorW);
  screen = DefaultScreen(dpy);
  if (XAllocColorCells(dpy, DefaultColormap(dpy, screen),
      False, NULL, 0, &pix, 1) == 0)
    pix = BlackPixel(dpy, screen);

  XtVaSetValues(colorW,
      XmNbackground, pix,
      NULL);

  (void) addWidgetInfo(colorW, GIZMO_COLOR_FIELD, name);
  
  return colorW;
}

PuGizmo
PuMotifCreateSlider(PuGizmo dial, char *name,
    float minVal, float maxVal, int digits, float initVal)
{
  int factor, i;
  Widget sliderW;
  WidgetInfo *infoP;

  factor = 1;
  for (i = 0; i < digits; i++)
    factor *= 10;

  sliderW = XmCreateScale(dial, "Slider", NULL, 0);
  XtVaSetValues(sliderW,
      XmNorientation, XmHORIZONTAL,
      XmNprocessingDirection, XmMAX_ON_RIGHT,
      XmNdecimalPoints, digits,
      XmNminimum, (int) (minVal * factor),
      XmNmaximum, (int) (maxVal * factor),
      XmNscaleMultiple, 1,
      XmNvalue, (int) (initVal * factor),
      XmNshowValue, True,
      NULL);
  
  infoP = addWidgetInfo(sliderW, GIZMO_SLIDER, name);
  infoP->u.slider.factor = factor;
  infoP->u.slider.lastVal = initVal;
  
  return sliderW;
}

PuGizmo
PuMotifCreateTextField(PuGizmo dial, char *name, char *text)
{
  Widget formW, textW;
  WidgetInfo *infoP;

  /* put into a Form widget so that arrows for the history
     can be added */
  formW = XmCreateForm(dial, "TextForm", NULL, 0);
  XtVaSetValues(formW,
      XmNfractionBase, 2,
      NULL);

  textW = XmCreateText(formW, "TextField", NULL, 0);
  XtVaSetValues(textW,
      XmNvalue, text,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);
  
  infoP = addWidgetInfo(textW, GIZMO_TEXT_FIELD, name);
  infoP->u.text.historySize = 0;
  
  XtManageChild(textW);

  return textW;
}

PuGizmo
PuMotifCreateText(PuGizmo dial, char *name, char *text)
{
  Widget textW;
  WidgetInfo *infoP;

  textW = XmCreateText(dial, "Text", NULL, 0);
  XtVaSetValues(textW,
      XmNeditMode, XmMULTI_LINE_EDIT,
      XmNvalue, text,
      NULL);
  
  infoP = addWidgetInfo(textW, GIZMO_TEXT, name);
  infoP->u.text.historySize = 0;
  
  return textW;
}

PuGizmo
PuMotifCreateFileViewer(PuGizmo dial, char *fileName, BOOL readOnly)
{
  return NULL;
}

PuGizmo
PuMotifCreateList(PuGizmo dial, char *name)
{
  Widget listW;
  WidgetInfo *infoP;

  listW = XmCreateScrolledList(dial, "List", NULL, 0);
  XtVaSetValues(listW,
      XmNselectionPolicy, XmBROWSE_SELECT,
      XmNvisibleItemCount, 5,
      NULL);
  
  infoP = addWidgetInfo(listW, GIZMO_LIST, name);
  infoP->u.list.size = 0;
  infoP->u.list.autoDesel = FALSE;
  infoP->u.list.showBottom = FALSE;
  infoP->u.list.sel = NULL;
  
  return listW;
}

void
PuMotifAddListEntry(PuGizmo list, char *name, BOOL onOff)
{
  XmString str;
  WidgetInfo *infoP;
  unsigned char selPolicy;

  str = XmStringCreateSimple(name);
  XmListAddItemUnselected(list, str, 0);

  infoP = getWidgetInfo(list);

  infoP->u.list.size++;
  if (infoP->u.list.sel == NULL)
    infoP->u.list.sel = malloc(sizeof(BOOL));
  else
    infoP->u.list.sel = realloc(infoP->u.list.sel,
	infoP->u.list.size * sizeof(BOOL));

  infoP->u.list.sel[infoP->u.list.size - 1] = onOff;

  if (onOff) {
    /* temporarily change selectionPolicy, see Heller */
    XtVaGetValues(list, XmNselectionPolicy, &selPolicy, NULL);
    if (selPolicy == XmEXTENDED_SELECT) {
      XtVaSetValues(list,
	  XmNselectionPolicy, XmMULTIPLE_SELECT,
	  NULL);
      XmListSelectItem(list, str, False);
      XtVaSetValues(list,
	  XmNselectionPolicy, XmEXTENDED_SELECT,
	  NULL);
    } else {
      XmListSelectItem(list, str, False);
    }
  }

  XmStringFree(str);

  if (infoP->u.list.showBottom)
    XmListSetBottomPos(list, 0);
}

void
PuMotifRemoveListEntries(PuGizmo list, int pos, int no)
{
  WidgetInfo *infoP;
  int i;

  infoP = getWidgetInfo(list);

  if (pos + no > infoP->u.list.size)
    no = infoP->u.list.size - pos;
  if (no < 1)
    return;

  for (i = pos + no; i < infoP->u.list.size; i++)
    infoP->u.list.sel[i - no] = infoP->u.list.sel[i];

  infoP->u.list.size -= no;

  XmListDeleteItemsPos(list, no, pos + 1);
}

PuGizmo
PuMotifCreateRadioBox(PuGizmo dial, char *name)
{
  Widget formW;
  WidgetInfo *infoP;
  XmString str;

  formW = XmCreateForm(dial, "RadioForm", NULL, 0);
  XtVaSetValues(formW,
      XmNfractionBase, 2,
      NULL);

  infoP = addWidgetInfo(formW, GIZMO_RADIO_BOX, name);
  infoP->u.box.lineNoSet = TRUE;
  infoP->u.box.toggleNo = 0;

  infoP->u.box.labelW = XmCreateLabel(formW, "Label", NULL, 0);
  str = XmStringCreateSimple(name);
  XtVaSetValues(infoP->u.box.labelW,
      XmNalignment, XmALIGNMENT_BEGINNING,
      XmNlabelString, str,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_POSITION,
      XmNbottomPosition, 1,
      NULL);
  XmStringFree(str);

  infoP->u.box.boxW = XmCreateRadioBox(formW, "RadioBox", NULL, 0);
  XtVaSetValues(infoP->u.box.boxW,
      XmNorientation, XmHORIZONTAL,
      XmNpacking, XmPACK_TIGHT,
      XmNborderWidth, 1,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNtopPosition, 1,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);
  
  XtManageChild(infoP->u.box.labelW);
  XtManageChild(infoP->u.box.boxW);

  return formW;
}

PuGizmo
PuMotifCreateCheckBox(PuGizmo dial, char *name)
{
  Widget formW;
  WidgetInfo *infoP;
  XmString str;

  formW = XmCreateForm(dial, "CheckForm", NULL, 0);
  XtVaSetValues(formW,
      XmNfractionBase, 2,
      NULL);

  infoP = addWidgetInfo(formW, GIZMO_CHECK_BOX, name);
  infoP->u.box.lineNoSet = FALSE;
  infoP->u.box.toggleNo = 0;

  if (name[0] == '\0') {
    infoP->u.box.labelW = NULL;
  } else {
    infoP->u.box.labelW = XmCreateLabel(formW, "Label", NULL, 0);
    str = XmStringCreateSimple(name);
    XtVaSetValues(infoP->u.box.labelW,
	XmNalignment, XmALIGNMENT_BEGINNING,
	XmNlabelString, str,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_POSITION,
	XmNbottomPosition, 1,
	NULL);
    XmStringFree(str);
  }

  infoP->u.box.boxW = XmCreateSimpleCheckBox(formW, "CheckBox", NULL, 0);
  XtVaSetValues(infoP->u.box.boxW,
      XmNborderWidth, 1,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);
  if (infoP->u.box.labelW == NULL) {
    XtVaSetValues(infoP->u.box.boxW,
	XmNtopAttachment, XmATTACH_FORM,
	NULL);
  } else {
    XtVaSetValues(infoP->u.box.boxW,
	XmNtopAttachment, XmATTACH_POSITION,
	XmNtopPosition, 1,
	NULL);
  }
  
  if (infoP->u.box.labelW != NULL)
    XtManageChild(infoP->u.box.labelW);
  XtManageChild(infoP->u.box.boxW);

  return formW;
}

void
PuMotifAddToggle(PuGizmo box, char *name, BOOL onOff)
{
  WidgetInfo *infoP;
  Widget entryW;
  XmString str;

  infoP = getWidgetInfo(box);

  infoP->u.box.toggleNo++;
  if (! infoP->u.box.lineNoSet)
    XtVaSetValues(box,
	XmNfractionBase, infoP->u.box.toggleNo + 1,
	NULL);

  entryW = XmCreateToggleButton(infoP->u.box.boxW, "Toggle", NULL, 0);
  str = XmStringCreateSimple(name);
  XtVaSetValues(entryW,
      XmNlabelString, str,
      NULL);
  XmStringFree(str);
  XmToggleButtonSetState(entryW, onOff, False);

  (void) addWidgetInfo(entryW, GIZMO_TOGGLE, name);

  XtManageChild(entryW);
}

void
PuMotifSetBool(PuGizmo gizmo, PuBoolChoice choice, BOOL val)
{
  WidgetInfo *infoP = getWidgetInfo(gizmo);
  GizmoType gType;

  if (infoP != NULL)
    gType = infoP->type;

  switch (choice) {
    case PU_BC_PLACE_OUTSIDE:
      while (XtClass(gizmo) != xmDialogShellWidgetClass)
	gizmo = XtParent(gizmo);
      MotifPlace(gizmo);
      break;
    case PU_BC_MULT_SEL:
      if (gType == GIZMO_LIST) {
	if (val)
	  XtVaSetValues(gizmo,
	      XmNselectionPolicy, XmEXTENDED_SELECT,
	      NULL);
	else
	  XtVaSetValues(gizmo,
	      XmNselectionPolicy, XmBROWSE_SELECT,
	      NULL);
      }
      break;
    case PU_BC_AUTO_DESEL:
      if (gType == GIZMO_LIST) {
	infoP->u.list.autoDesel = val;
      }
    case PU_BC_SHOW_BOTTOM:
      if (gType == GIZMO_LIST) {
	infoP->u.list.showBottom = val;
      }
  }
}

static void
freeStr(void *p, void *clientData)
{
  char **strP = p;

  free(*strP);
}

static void
setStr(Widget w, char *str)
{
  XtVaSetValues(w, XmNvalue, str, NULL);
  XmTextSetInsertionPosition(w, (int) strlen(str));
}

static void
upHistory(Widget w)
{
  WidgetInfo *infoP = getWidgetInfo(w);
  LINLIST list;
  char **currP;

  list = infoP->u.text.historyList;
  currP = infoP->u.text.historyCurr;
  if (currP == NULL)
    currP = ListLast(list);
  else
    currP = ListPrev(list, currP);

  if (currP == NULL)
    setStr(w, "");
  else
    setStr(w, *currP);

  infoP->u.text.historyCurr = currP;
}

static void
downHistory(Widget w)
{
  WidgetInfo *infoP = getWidgetInfo(w);
  LINLIST list;
  char **currP;

  list = infoP->u.text.historyList;
  currP = infoP->u.text.historyCurr;
  if (currP != NULL)
    currP = ListNext(list, currP);

  if (currP == NULL)
    setStr(w, "");
  else
    setStr(w, *currP);

  infoP->u.text.historyCurr = currP;
}

static void
upHistoryCB(Widget w, XtPointer clientData, XtPointer callData)
{
  Widget textW = clientData;

  upHistory(textW);
}

static void
downHistoryCB(Widget w, XtPointer clientData, XtPointer callData)
{
  Widget textW = clientData;

  downHistory(textW);
}

static void
keyEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  char buf[10];
  KeySym sym;

  (void) XLookupString(&evtP->xkey, buf, sizeof(buf), &sym, NULL);
  if (sym == XK_Up) {
    upHistory(w);
  } else if (sym == XK_Down) {
    downHistory(w);
  }
}

static void
initHistory(Widget textW)
{
  WidgetInfo *infoP = getWidgetInfo(textW);
  Widget formW, upArrowW, downArrowW;

  formW = XtParent(textW);

  upArrowW = XmCreateArrowButton(formW, "up", NULL, 0);
  XtVaSetValues(upArrowW,
      XmNarrowDirection, XmARROW_UP,
      XmNwidth, 20,
      XmNheight, 1,
      XmNborderWidth, 0,
      XmNhighlightThickness, 0,
      XmNshadowThickness, 0,
      XmNtraversalOn, FALSE,
      XmNleftAttachment, XmATTACH_NONE,
      XmNrightAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_POSITION,
      XmNbottomPosition, 1,
      NULL);

  downArrowW = XmCreateArrowButton(formW, "down", NULL, 0);
  XtVaSetValues(downArrowW,
      XmNarrowDirection, XmARROW_DOWN,
      XmNwidth, 20,
      XmNheight, 1,
      XmNborderWidth, 0,
      XmNhighlightThickness, 0,
      XmNshadowThickness, 0,
      XmNtraversalOn, FALSE,
      XmNleftAttachment, XmATTACH_NONE,
      XmNrightAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNtopPosition, 1,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);

  XtVaSetValues(textW,
      XmNrightAttachment, XmATTACH_WIDGET,
      XmNrightWidget, upArrowW,
      NULL);

  XtAddCallback(upArrowW, XmNactivateCallback, upHistoryCB, textW);
  XtAddCallback(downArrowW, XmNactivateCallback, downHistoryCB, textW);
  XtAddEventHandler(textW, KeyReleaseMask, False, keyEH, NULL);

  XtManageChild(upArrowW);
  XtManageChild(downArrowW);

  infoP->u.text.historyList = ListOpen(sizeof(char *));
  ListAddDestroyCB(infoP->u.text.historyList, freeStr, NULL, NULL);
  infoP->u.text.historyCurr = NULL;
  infoP->u.text.firstMod = FALSE;
}

void
PuMotifSetInt(PuGizmo gizmo, PuIntChoice choice, int val)
{
  WidgetInfo *infoP = getWidgetInfo(gizmo);
  GizmoType gType = infoP->type;

  switch (choice) {
    case PU_IC_TEXT_WIDTH:
      if (gType == GIZMO_TEXT_FIELD || gType == GIZMO_TEXT)
	XtVaSetValues(gizmo,
	    XmNcolumns, val,
	    NULL);
      break;
    case PU_IC_TEXT_HEIGHT:
      if (gType == GIZMO_TEXT)
	XtVaSetValues(gizmo,
	    XmNrows, val,
	    NULL);
      break;
    case PU_IC_HISTORY_SIZE:
      if (gType == GIZMO_TEXT_FIELD) {
	if (infoP->u.text.historySize == 0)
	  initHistory(gizmo);
	infoP->u.text.historySize = val;
      }
      break;
    case PU_IC_ENTRY_NO:
      if (gType == GIZMO_LIST)
	XtVaSetValues(gizmo,
	    XmNvisibleItemCount, val,
	    NULL);
      break;
    case PU_IC_LINE_NO:
      infoP->u.box.lineNoSet = TRUE;
      XtVaSetValues(gizmo,
	  XmNfractionBase, val + 1,
	  NULL);
      if (gType == GIZMO_RADIO_BOX) {
	if (val == 1)
	  XtVaSetValues(infoP->u.box.boxW,
	      XmNpacking, XmPACK_TIGHT,
	      XmNnumColumns, 1,
	      NULL);
	else
	  XtVaSetValues(infoP->u.box.boxW,
	      XmNpacking, XmPACK_COLUMN,
	      XmNnumColumns, val,
	      NULL);
      }
      break;
  }
}

void
PuMotifSetStr(PuGizmo gizmo, PuStrChoice choice, char *val)
{
  GizmoType gType = getWidgetInfo(gizmo)->type;
  XmString str;

  str = XmStringCreateSimple(val);

  switch (choice) {
    case PU_SC_TEXT:
      if (gType == GIZMO_LABEL)
	XtVaSetValues(gizmo,
	    XmNlabelString, str,
	    NULL);
      else if (gType == GIZMO_TEXT_FIELD || gType == GIZMO_TEXT)
	XtVaSetValues(gizmo,
	    XmNvalue, val,
	    NULL);
      break;
  }

  XmStringFree(str);
}

void
PuMotifSetColor(PuGizmo gizmo, float r, float g, float b)
{
  Display *display;
  Pixel pixel;
  XColor color;
  Colormap colormap;

  display = XtDisplay(gizmo);
  XtVaGetValues(gizmo, XmNbackground, &pixel, NULL);
  colormap = DefaultColormapOfScreen(XtScreen(gizmo));

  color.pixel = pixel;
  color.red = FLOAT2SHORT(r);
  color.green = FLOAT2SHORT(g);
  color.blue = FLOAT2SHORT(b);
  color.flags = DoRed | DoGreen | DoBlue;

  XFreeColors(display, colormap, &pixel, 1, 0);
  XAllocColor(display, colormap, &color);
  XtVaSetValues(gizmo, XmNbackground, color.pixel, NULL);
}

void
PuMotifSetConstraints(PuGizmo gizmo, PuConstraints constr)
{
  WidgetInfo *infoP;
  int xBase, yBase;

  infoP = getDialogInfo(gizmo);
  xBase = infoP->u.dialog.xBase;
  yBase = infoP->u.dialog.yBase;

  XtVaSetValues(getFormChild(gizmo),
      XmNleftAttachment, XmATTACH_POSITION,
      XmNrightAttachment, XmATTACH_POSITION,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNbottomAttachment, XmATTACH_POSITION,
      XmNleftPosition, constr.x * yBase,
      XmNrightPosition, (constr.x + constr.w) * yBase,
      XmNtopPosition, constr.y * xBase,
      XmNbottomPosition, (constr.y + constr.h) * xBase,
      NULL);
}

void
closeGizmo(PuGizmo gizmo)
/* make dialog boxes disappear immediately */
{
  if (XtClass(gizmo) == xmDialogShellWidgetClass ||
      XtClass(XtParent(gizmo)) == xmDialogShellWidgetClass) {
    XSync(XtDisplay(gizmo), False);
    XmUpdateDisplay(MotifGetDrawW());
  }
}

void
switchOneGizmo(PuGizmo gizmo, BOOL onOff)
{
  if (onOff) {
    XtManageChild(gizmo);
  } else {
    XtUnmanageChild(gizmo);
    closeGizmo(gizmo);
  }
}

void
PuMotifSwitchGizmo(PuGizmo gizmo, BOOL onOff)
{
  WidgetInfo *infoP;
  Widget dialW;
  Window win;

  infoP = getWidgetInfo(gizmo);

  if (infoP->type == GIZMO_DIALOG) {
    /* manage everything up to child of DialogShell
       (hierarchy: Form -> PanedWindow -> Form -> DialogShell */
    if (onOff) {
      dialW = XtParent(XtParent(gizmo));

      switchOneGizmo(gizmo, TRUE);
      switchOneGizmo(XtParent(gizmo), TRUE);
      switchOneGizmo(dialW, TRUE);

      /* avoid resizes while it's visible */
      XtVaSetValues(gizmo, XmNresizePolicy, XmRESIZE_NONE, NULL);

      /* bring it to top if it was already mapped */
      win = XtWindow(XtParent(dialW));
      if (win != 0)
	XRaiseWindow(XtDisplay(dialW), win);
    } else {
      /* unmanage only child of DialogShell */
      switchOneGizmo(XtParent(XtParent(gizmo)), FALSE);

      /* allow resizes while it's not visible */
      XtVaSetValues(gizmo, XmNresizePolicy, XmRESIZE_ANY, NULL);
    }
  } else if (infoP->type == GIZMO_TEXT_FIELD) {
    switchOneGizmo(XtParent(gizmo), onOff);
  } else {
    switchOneGizmo(gizmo, onOff);
  }
}

void
PuMotifDestroyGizmo(PuGizmo gizmo)
{
  WidgetInfo *infoP, *dialInfoP;
  Widget *wP;

  infoP = getWidgetInfo(gizmo);
  if (infoP->type == GIZMO_DIALOG ||
      infoP->type == GIZMO_FILE_SELECTION_BOX) {
    /* destroy DialogShell */
    while (XtClass(gizmo) != xmDialogShellWidgetClass)
      gizmo = XtParent(gizmo);
  } else {
    dialInfoP = getDialogInfo(gizmo);
    wP = ListFirst(dialInfoP->u.dialog.childList);
    while (*wP != gizmo)
      wP = ListNext(dialInfoP->u.dialog.childList, wP);
    ListRemove(dialInfoP->u.dialog.childList, wP);
  }

  XtUnmapWidget(gizmo);
  closeGizmo(gizmo);
  XtDestroyWidget(gizmo);
}

static Widget
createButton(WidgetInfo *dialInfoP, char *name)
{
  Widget buttonW;
  Dimension height;

  dialInfoP->u.dialog.buttonNo++;
  XtVaSetValues(dialInfoP->u.dialog.buttonFormW,
      XmNfractionBase, 2 * dialInfoP->u.dialog.buttonNo - 1,
      NULL);
  
  buttonW = XmCreatePushButton(dialInfoP->u.dialog.buttonFormW,
      name, NULL, 0);
  XtVaSetValues(buttonW,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNleftPosition, 2 * dialInfoP->u.dialog.buttonNo - 2,
      XmNrightAttachment, XmATTACH_POSITION,
      XmNrightPosition, 2 * dialInfoP->u.dialog.buttonNo - 1,
      NULL);

  XtManageChild(buttonW);

  XtVaGetValues(buttonW, XmNheight, &height, NULL);
  XtVaSetValues(dialInfoP->u.dialog.buttonFormW,
      XmNpaneMaximum, height,
      XmNpaneMinimum, height,
      NULL);

  return buttonW;
}

static void
addButtonCB(Widget dialFormW, char *name, GizmoCBInfo *cbInfoP)
{
  WidgetInfo *wInfoP;
  Widget *wP;

  wInfoP = getWidgetInfo(dialFormW);

  if (strcmp(name, LABEL_OK) == 0)
    wP = &wInfoP->u.dialog.okW;
  else if (strcmp(name, LABEL_CLOSE) == 0)
    wP = &wInfoP->u.dialog.closeW;
  else if (strcmp(name, LABEL_APPLY) == 0)
    wP = &wInfoP->u.dialog.applyW;
  else if (strcmp(name, LABEL_RESET) == 0)
    wP = &wInfoP->u.dialog.resetW;
  else if (strcmp(name, LABEL_CANCEL) == 0)
    wP = &wInfoP->u.dialog.cancelW;
  else if (strcmp(name, LABEL_HELP) == 0)
    wP = &wInfoP->u.dialog.helpW;

  if (*wP == NULL)
    *wP = createButton(wInfoP, name);
  XtAddCallback(*wP, XmNactivateCallback, gizmoGenCB, cbInfoP);

  XtManageChild(wInfoP->u.dialog.buttonFormW);
}

void
PuMotifAddGizmoCB(PuGizmo gizmo, PuGizmoCBType type,
    PuGizmoCB gizmoCB, void *clientData, PuFreeCB freeCB)
{
  WidgetInfo *wInfoP = getWidgetInfo(gizmo);
  GizmoCBInfo *infoP;
  GizmoType gType = wInfoP->type;

  infoP = malloc(sizeof(*infoP));
  infoP->name = wInfoP->name;
  infoP->gizmoCB = gizmoCB;
  infoP->clientData = clientData;
  infoP->freeCB = freeCB;

  switch (type) {
  case PU_CT_ACTIVATE:
    if (gType == (GIZMO_TEXT_FIELD | GIZMO_TEXT))
      XtAddCallback(gizmo, XmNactivateCallback, textActivateCB, infoP);
    else if (gType == GIZMO_BUTTON)
      XtAddCallback(gizmo, XmNactivateCallback, gizmoGenCB, infoP);
    break;
  case PU_CT_MODIFY:
    if (gType == GIZMO_SLIDER) {
      XtAddCallback(gizmo, XmNdragCallback, sliderCB, infoP);
      XtAddCallback(gizmo, XmNvalueChangedCallback, sliderCB, infoP);
    } else if (gType == GIZMO_TEXT_FIELD || gType == GIZMO_TEXT) {
      XtAddCallback(gizmo, XmNmodifyVerifyCallback, textVerifyCB, infoP);
      XtAddCallback(gizmo, XmNvalueChangedCallback, TextValueChangedCB, infoP);
    }
    break;
  case PU_CT_SELECT:
    if (gType == GIZMO_LIST) {
      XtAddCallback(gizmo, XmNbrowseSelectionCallback, listSelCB, infoP);
      XtAddCallback(gizmo, XmNextendedSelectionCallback, listSelCB, infoP);
    } else if (gType == GIZMO_RADIO_BOX || gType == GIZMO_CHECK_BOX) {
      Widget *children;
      Cardinal numChildren;
      int i;

      XtVaGetValues(wInfoP->u.box.boxW,
	  XmNchildren, &children,
	  XmNnumChildren, &numChildren,
	  NULL);
      for (i = 0; i < numChildren; i++)
	XtAddCallback(children[i], XmNvalueChangedCallback,
	    radioCB, infoP);
    }
    break;
  case PU_CT_OK:
    infoP->name = LABEL_OK;
    if (gType == GIZMO_FILE_SELECTION_BOX)
      XtAddCallback(gizmo, XmNokCallback, fileSelOkCB, infoP);
    else if (gType == GIZMO_DIALOG)
      addButtonCB(gizmo, infoP->name, infoP);
    break;
  case PU_CT_CLOSE:
    infoP->name = LABEL_CLOSE;
    if (gType == GIZMO_DIALOG)
      addButtonCB(gizmo, infoP->name, infoP);
    break;
  case PU_CT_APPLY:
    infoP->name = LABEL_APPLY;
    if (gType == GIZMO_FILE_SELECTION_BOX)
      XtAddCallback(gizmo, XmNapplyCallback, gizmoGenCB, infoP);
    else if (gType == GIZMO_DIALOG)
      addButtonCB(gizmo, infoP->name, infoP);
    break;
  case PU_CT_RESET:
    infoP->name = LABEL_RESET;
    if (gType == GIZMO_DIALOG)
      addButtonCB(gizmo, infoP->name, infoP);
    break;
  case PU_CT_CANCEL:
    infoP->name = LABEL_CANCEL;
    if (gType == GIZMO_FILE_SELECTION_BOX)
      XtAddCallback(gizmo, XmNcancelCallback, gizmoGenCB, infoP);
    else if (gType == GIZMO_DIALOG)
      addButtonCB(gizmo, infoP->name, infoP);
    break;
  case PU_CT_HELP:
    infoP->name = LABEL_HELP;
    if (gType == GIZMO_FILE_SELECTION_BOX)
      XtAddCallback(gizmo, XmNhelpCallback, gizmoGenCB, infoP);
    else if (gType == GIZMO_DIALOG)
      addButtonCB(gizmo, infoP->name, infoP);
    break;
  case PU_CT_DESTROY:
    XtAddCallback(gizmo, XmNdestroyCallback, gizmoGenCB, infoP);
    break;
  }  /* switch (type) */

  XtAddCallback(gizmo, XmNdestroyCallback, destroyCB, infoP);
}
