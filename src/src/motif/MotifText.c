/*
************************************************************************
*
*   MotifText.c - Motif text utility functions
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
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifText.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "motif_text.h"

#include <stdio.h>
#include <stdlib.h>
#include <Xm/Text.h>

static int TextPos;

void
TextValueChangedCB(Widget w, XtPointer clientData, XtPointer callData)
/* necessary for old Motif to set cursor after changing text in
   VerifyCallback */
{
  if (TextPos >= 0)
    XmTextSetInsertionPosition(w, TextPos);
}

BOOL
TextStrucMotifToPu(XmTextVerifyCallbackStruct *motifStrucP,
    PuTextCBStruc *puStrucP)
{
  int oldLen, newLen, oldPartLen, newPartLen;
  BOOL deleted, inserted;

  oldLen = strlen(puStrucP->oldText);
  puStrucP->oldLen = oldLen;

  deleted = (motifStrucP->startPos < motifStrucP->endPos);
  inserted = (motifStrucP->text->ptr != NULL &&
      motifStrucP->text->ptr[0] != '\0');

  if (! (inserted || deleted))
    return FALSE;

  if (inserted && deleted) 
    puStrucP->action = PU_CA_REPLACE;
  else if (deleted)
    puStrucP->action = PU_CA_DELETE;
  else if (motifStrucP->startPos == oldLen)
    puStrucP->action = PU_CA_APPEND;
  else
    puStrucP->action = PU_CA_INSERT;
  
  if (deleted) {
    oldPartLen = motifStrucP->endPos - motifStrucP->startPos;
    puStrucP->oldPartStart = motifStrucP->startPos;
    puStrucP->oldPartEnd = motifStrucP->endPos;
    puStrucP->oldPart = malloc(oldPartLen + 1);
    (void) strncpy(puStrucP->oldPart,
	puStrucP->oldText + puStrucP->oldPartStart, oldPartLen);
    puStrucP->oldPart[oldPartLen] = '\0';
  } else {
    oldPartLen = 0;
    puStrucP->oldPart = NULL;
  }

  if (inserted) {
    newPartLen = motifStrucP->text->length;
    puStrucP->newPartStart = motifStrucP->startPos;
    puStrucP->newPartEnd = motifStrucP->startPos + newPartLen;
    puStrucP->newPart = malloc(newPartLen + 1);
    (void) strncpy(puStrucP->newPart,
	motifStrucP->text->ptr, newPartLen);
    puStrucP->newPart[newPartLen] = '\0';
  } else {
    newPartLen = 0;
    puStrucP->newPart = NULL;
  }

  newLen = oldLen - oldPartLen + newPartLen;
  puStrucP->newText = malloc(newLen + 1);
  (void) strncpy(puStrucP->newText,
      puStrucP->oldText, motifStrucP->startPos);
  (void) strncpy(puStrucP->newText + motifStrucP->startPos,
      puStrucP->newPart, newPartLen);
  (void) strncpy(puStrucP->newText + motifStrucP->startPos + newPartLen,
      puStrucP->oldText + motifStrucP->endPos, oldLen - motifStrucP->endPos);
  puStrucP->newText[newLen] = '\0';
  puStrucP->newLen = newLen;

  puStrucP->doIt = TRUE;

  return TRUE;
}

void
TextStrucPuToMotif(PuTextCBStruc *puStrucP,
    XmTextVerifyCallbackStruct *motifStrucP)
{
  int newPartLen;

  if (puStrucP->doIt) {
    if (puStrucP->action == PU_CA_DELETE) {
      motifStrucP->startPos = puStrucP->oldPartStart;
      motifStrucP->endPos = puStrucP->oldPartEnd;
      TextPos = -1;
    } else {
      newPartLen = strlen(puStrucP->newPart);
      motifStrucP->text->ptr = XtRealloc(motifStrucP->text->ptr, newPartLen);
      (void) strncpy(motifStrucP->text->ptr, puStrucP->newPart, newPartLen);
      motifStrucP->text->length = newPartLen;
      TextPos = puStrucP->newPartStart + newPartLen;
    }
  }

  motifStrucP->doit = puStrucP->doIt;

  free(puStrucP->newText);
  if (puStrucP->oldPart != NULL)
    free(puStrucP->oldPart);
  if (puStrucP->newPart != NULL)
    free(puStrucP->newPart);
}
