/*
************************************************************************
*
*   PuDial.c - Pu wrapper functions for dialog boxes
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/src/pudev/SCCS/s.PuDial.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <pu_dev.h>

PuGizmo
PuCreateFileSelectionBox(char *name,
    char *dir, char *pattern, PuFileAccess acc)
{
  return CurrPuDevP->createFileSelectionBox(name, dir, pattern, acc);
}

PuGizmo
PuCreateDialog(char *name, int xBase, int yBase)
{
  return CurrPuDevP->createDialog(name, xBase, yBase);
}

PuGizmo
PuCreateLabel(PuGizmo dial, char *name)
{
  return CurrPuDevP->createLabel(dial, name);
}

PuGizmo
PuCreateButton(PuGizmo dial, char *name)
{
  return CurrPuDevP->createButton(dial, name);
}

PuGizmo
PuCreateColorField(PuGizmo dial, char *name)
{
  return CurrPuDevP->createColorField(dial, name);
}

PuGizmo
PuCreateSlider(PuGizmo dial, char *name,
    float minVal, float maxVal, int digits, float initVal)
{
  return CurrPuDevP->createSlider(dial, name,
      minVal, maxVal, digits, initVal);
}

PuGizmo
PuCreateTextField(PuGizmo dial, char *name, char *text)
{
  return CurrPuDevP->createTextField(dial, name, text);
}

PuGizmo
PuCreateText(PuGizmo dial, char *name, char *text)
{
  return CurrPuDevP->createText(dial, name, text);
}

PuGizmo
PuCreateFileViewer(PuGizmo dial, char *fileName, BOOL readOnly)
{
  return CurrPuDevP->createFileViewer(dial, fileName, readOnly);
}

PuGizmo
PuCreateList(PuGizmo dial, char *name)
{
  return CurrPuDevP->createList(dial, name);
}

void
PuAddListEntry(PuGizmo list, char *name, BOOL onOff)
{
  CurrPuDevP->addListEntry(list, name, onOff);
}

void
PuRemoveListEntries(PuGizmo list, int pos, int no)
{
  CurrPuDevP->removeListEntries(list, pos, no);
}

PuGizmo
PuCreateRadioBox(PuGizmo dial, char *name)
{
  return CurrPuDevP->createRadioBox(dial, name);
}

PuGizmo
PuCreateCheckBox(PuGizmo dial, char *name)
{
  return CurrPuDevP->createCheckBox(dial, name);
}

void
PuAddToggle(PuGizmo box, char *name, BOOL onOff)
{
  CurrPuDevP->addToggle(box, name, onOff);
}

void
PuSetBool(PuGizmo gizmo, PuBoolChoice choice, BOOL val)
{
  CurrPuDevP->setBool(gizmo, choice, val);
}

void
PuSetInt(PuGizmo gizmo, PuIntChoice choice, int val)
{
  CurrPuDevP->setInt(gizmo, choice, val);
}

void
PuSetStr(PuGizmo gizmo, PuStrChoice choice, char *val)
{
  CurrPuDevP->setStr(gizmo, choice, val);
}

void
PuSetColor(PuGizmo gizmo, float r, float g, float b)
{
  CurrPuDevP->setColor(gizmo, r, g, b);
}

void
PuSetConstraints(PuGizmo gizmo, PuConstraints constr)
{
  CurrPuDevP->setConstraints(gizmo, constr);
}

void
PuSwitchGizmo(PuGizmo gizmo, BOOL onOff)
{
  CurrPuDevP->switchGizmo(gizmo, onOff);
}

void
PuDestroyGizmo(PuGizmo gizmo)
{
  CurrPuDevP->destroyGizmo(gizmo);
}

void
PuAddGizmoCB(PuGizmo gizmo, PuGizmoCBType type,
    PuGizmoCB gizmoCB, void *clientData, PuFreeCB freeCB)
{
  CurrPuDevP->addGizmoCB(gizmo, type, gizmoCB, clientData, freeCB);
}
