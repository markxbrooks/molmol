/*
************************************************************************
*
*   SgError.c - Sg error handling
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/sgdev/SCCS/s.SgError.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_error.h>

#include <stdio.h>

#include <sg.h>

static void
defaultErrorHandler(SgError errCode, char *errMsg)
{
  char *errStr;

  switch (errCode) {
    case SG_ERR_ILLDEV:
      errStr = "unknown device";
      break;
    case SG_ERR_ILLMODE:
      errStr = "wrong mode";
      break;
    case SG_ERR_ILLOBJ:
      errStr = "unknown object";
      break;
    case SG_ERR_ILLVAL:
      errStr = "illegal value";
      break;
    case SG_ERR_NOMEM:
      errStr = "out of memory";
      break;
    case SG_ERR_STACKEMPTY:
      errStr = "matrix stack empty";
      break;
    case SG_ERR_STACKFULL:
      errStr = "matrix stack full";
      break;
  }
  (void) fprintf(stderr, "SG ERROR : %s : %s\n", errStr, errMsg);
}

static SgErrorHandler ErrorHandler = defaultErrorHandler;

void
SgRaiseError(SgError errCode, char *errMsg)
{
  ErrorHandler(errCode, errMsg);
}

void
SgSetErrorHandler(SgErrorHandler errHandler)
{
  if (errHandler == NULL)
    ErrorHandler = defaultErrorHandler;
  else
    ErrorHandler = errHandler;
}
