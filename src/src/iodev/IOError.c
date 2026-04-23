/*
************************************************************************
*
*   IOError.c - I/O error handling
*
*   Copyright (c) 1994-95
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
*   Date of last modification : 95/09/12
*   Pathname of SCCS file     : /sgiext/molmol/src/iodev/SCCS/s.IOError.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <io_error.h>
#include <io.h>

#include <stdio.h>

static void
defaultErrorHandler(char *errMsg)
{
  (void) fprintf(stderr, "IO ERROR : %s\n", errMsg);
}

static IOErrorHandler ErrorHandler = defaultErrorHandler;

void
IORaiseError(char *errMsg)
{
  ErrorHandler(errMsg);
}

void
IOSetErrorHandler(IOErrorHandler errHandler)
{
  if (errHandler == NULL)
    ErrorHandler = defaultErrorHandler;
  else
    ErrorHandler = errHandler;
}
