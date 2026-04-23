/*
************************************************************************
*
*   PuMainW.c - Pu wrapper functions for main window
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
*   Date of last modification : 96/02/06
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuMainW.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <pu_dev.h>

#define LOCK_CHAR '!'

static BOOL Fullscreen = FALSE;
static BOOL StatusLocked = FALSE;

void
PuSwitchFullscreen(BOOL onOff)
{
  if (onOff == Fullscreen)
    return;

  CurrPuDevP->switchFullscreen(onOff);
  Fullscreen = onOff;
}

void
PuSetDrawSize(int w, int h)
{
  CurrPuDevP->setDrawSize(w, h);
}

void
PuSetTextField(PuTextFieldChoice fieldChoice, char *text)
{
  if (fieldChoice == PU_TF_STATUS) {
    /* Status line can be locked/unlocked by a message starting with
       LOCK_CHAR. This is used to make sure that an important message
       is not overwritten by something less important. */
    if (StatusLocked) {
      if (text[0] != LOCK_CHAR)
	return;
      text++;
      if (text[0] == '\0' || (text[0] == ' ' && text[1] == '\0')) {
	StatusLocked = FALSE;
	text = " ";
      }
    } else if (text[0] == LOCK_CHAR) {
      text++;
      StatusLocked = TRUE;
    }
  }

  CurrPuDevP->setTextField(fieldChoice, text);
}

void
PuSwitchTextField(PuTextFieldChoice fieldChoice, BOOL onOff)
{
  CurrPuDevP->switchTextField(fieldChoice, onOff);
}
