/*
************************************************************************
*
*   Break.c - management for program/command break
*
*   Copyright (c) 1995
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
*   Date of last modification : 95/11/17
*   Pathname of SCCS file     : /sgiext/molmol/tools/src/SCCS/s.Break.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <break.h>

#include <stdio.h>

static int BreakLevel = 0;
static BOOL Interrupted = FALSE;

static BOOL
defaultCheck(void)
{
  return FALSE;
}

static BreakActivateF ActivateF = NULL;
static BreakCheckF CheckF = defaultCheck;

void
BreakSetActivate(BreakActivateF activateF)
{
  ActivateF = activateF;
}

void
BreakSetCheck(BreakCheckF checkF)
{
  if (checkF == NULL)
    CheckF = defaultCheck;
  else
    CheckF = checkF;
}

void
BreakReset(void)
{
  if (BreakLevel == 0)
    Interrupted = FALSE;
}

void
BreakActivate(BOOL onOff)
{
  /* allow nested calls, only perform action at "top level" */
  if (onOff) {
    BreakLevel++;
    if (BreakLevel == 1) {
      Interrupted = FALSE;
      if (ActivateF != NULL)
	ActivateF(TRUE);
    }
  } else {
    BreakLevel--;
    if (BreakLevel == 0) {
      if (ActivateF != NULL)
	ActivateF(FALSE);
    }
  }
}

BOOL
BreakCheck(int checkCount)
{
  static int callCount = 0;

  callCount++;
  if (callCount >= checkCount && ! Interrupted && BreakLevel > 0) {
    Interrupted = CheckF();
    callCount = 0;
  }

  return Interrupted;
}

BOOL
BreakInterrupted(void)
{
  return Interrupted;
}
