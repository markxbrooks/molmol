/*
************************************************************************
*
*   ExFirstMol.c - FirstMol command
*
*   Copyright (c) 1997
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
*   Date of last modification : 97/07/22
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdstruc/SCCS/s.ExFirstMol.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>
#include <stdlib.h>

#include <data_hand.h>
#include <data_sel.h>

ErrCode
ExFirstMol(char *cmd)
{
  int molNo, molI;
  DhMolP *molPA;

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0) {
    CipSetError("no molecule selected");
    return EC_WARNING;
  }

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  for (molI = molNo - 1; molI >= 0; molI--)
    DhMolMoveFirst(molPA[molI]);

  free(molPA);

  return EC_OK;
}
