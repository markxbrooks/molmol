/*
************************************************************************
*
*   MotifVal.c - Motif valuator box
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
*   Date of last modification : 95/07/20
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifVal.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "motif_val.h"

#include <string.h>
#include <stdlib.h>

#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>

#include "motif_access.h"
#include "motif_busy.h"

typedef struct {
  char *name;
  float minVal, maxVal;
  int factor;
  float lastVal;
  PuValuatorCB valuatorCB;
  void *clientData;
  PuFreeCB freeCB;
} ValuatorCBInfo;

static void
valuatorEnterEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  ValuatorCBInfo *infoP = clientData;
  PuValuatorCBStruc valuatorStruc;

  valuatorStruc.act = PU_MA_ENTER;
  valuatorStruc.name = infoP->name;
  infoP->valuatorCB(w, infoP->clientData, &valuatorStruc);
}

static void
valuatorLeaveEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  ValuatorCBInfo *infoP = clientData;
  PuValuatorCBStruc valuatorStruc;

  valuatorStruc.act = PU_MA_LEAVE;
  valuatorStruc.name = infoP->name;
  infoP->valuatorCB(w, infoP->clientData, &valuatorStruc);
}

static void
valuatorActCB(Widget w, XtPointer clientData, XtPointer callData)
{
  ValuatorCBInfo *infoP = clientData;
  XmScaleCallbackStruct *callP = callData;
  PuValuatorCBStruc valuatorStruc;
  float floatVal;

  floatVal = (float) callP->value / infoP->factor;
  if (floatVal != infoP->lastVal) {
    infoP->lastVal = floatVal;

    valuatorStruc.act = PU_MA_ACTIVATE;
    valuatorStruc.name = infoP->name;
    valuatorStruc.val = floatVal;
    infoP->valuatorCB(w, infoP->clientData, &valuatorStruc);
  }
}

static void
destroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
  ValuatorCBInfo *infoP = clientData;

  if (infoP->freeCB != NULL)
    infoP->freeCB(infoP->clientData);
  free(infoP->name);
  free(infoP);
}

PuValuatorBox
PuMotifCreateValuatorBox(char *title)
{
  Widget dialogW, formW;

  dialogW = XmCreateDialogShell(MotifGetTopW(),
      "ValuatorBox", NULL, 0);
  XtVaSetValues(dialogW,
      XmNtitle, title,
      XmNdeleteResponse, XmUNMAP,
      NULL);
  MotifAddDialog(dialogW);

  formW = XmCreateForm(dialogW, "ValuatorForm", NULL, 0);

  return formW;
}

PuValuator
PuMotifCreateValuator(PuValuatorBox box, char *name,
    float minVal, float maxVal, int digits, float initVal,
    PuValuatorCB valuatorCB, void *clientData, PuFreeCB freeCB)
{
  int factor, i;
  Cardinal childNo;
  Widget scaleW;
  XmString labelStr;
  ValuatorCBInfo *infoP;

  factor = 1;
  for (i = 0; i < digits; i++)
    factor *= 10;

  XtVaGetValues(box, XmNnumChildren, &childNo, NULL);

  scaleW = XmCreateScale(box, name, NULL, 0);
  labelStr = XmStringCreateSimple(name);
  XtVaSetValues(scaleW,
      XmNwidth, 200,
      XmNtitleString, labelStr,
      XmNorientation, XmHORIZONTAL,
      XmNprocessingDirection, XmMAX_ON_RIGHT,
      XmNdecimalPoints, digits,
      XmNminimum, (int) (minVal * factor),
      XmNmaximum, (int) (maxVal * factor),
      XmNvalue, (int) (initVal * factor),
      XmNshowValue, True,
      XmNtopAttachment, XmATTACH_POSITION,
      XmNtopPosition, childNo,
      XmNbottomAttachment, XmATTACH_POSITION,
      XmNbottomPosition, childNo + 1,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      NULL);
  XmStringFree(labelStr);

  if (valuatorCB != NULL) {
    infoP = malloc(sizeof(*infoP));
    infoP->name = malloc(strlen(name) + 1);
    (void) strcpy(infoP->name, name);
    infoP->minVal = minVal;
    infoP->maxVal = maxVal;
    infoP->factor = factor;
    infoP->lastVal = initVal;
    infoP->valuatorCB = valuatorCB;
    infoP->clientData = clientData;
    infoP->freeCB = freeCB;
    XtAddEventHandler(scaleW, LeaveWindowMask, False, valuatorLeaveEH, infoP);
    XtAddEventHandler(scaleW, EnterWindowMask, False, valuatorEnterEH, infoP);
    XtAddCallback(scaleW, XmNdragCallback, valuatorActCB, infoP);
    XtAddCallback(scaleW, XmNvalueChangedCallback, valuatorActCB, infoP);
    XtAddCallback(scaleW, XmNdestroyCallback, destroyCB, infoP);
  }

  XtManageChild(scaleW);

  return scaleW;
}

void
PuMotifSwitchValuatorBox(PuValuatorBox box, BOOL onOff)
{
  Cardinal childNo;
  Widget dialW;
  Window win;

  if (onOff) {
    XtVaGetValues(box, XmNnumChildren, &childNo, NULL);
    XtVaSetValues(box, XmNfractionBase, childNo, NULL);
    XtManageChild(box);

    /* bring it to top if it was already mapped */
    dialW = XtParent(box);
    win = XtWindow(XtParent(dialW));
    if (win != 0)
      XRaiseWindow(XtDisplay(dialW), win);
  } else {
    XtUnmanageChild(box);
  }
}
