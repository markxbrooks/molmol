/*
************************************************************************
*
*   MolWin.c - Windows main program
*
*   Copyright (c) 1994-2001
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
*   Date of last modification : 01/07/04
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/main/SCCS/s.MolWin.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "mol_init.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <fcntl.h>

extern int _open_osfhandle(long, int);

/* HACK: this declaration must be identical to the one in WinInit.c! */
typedef struct {
  HINSTANCE hInstance;
  int nCmdShow;
} WinArg;

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  char **argv;
  int argc;
  WinArg winArg;
  char *s, *as;
  int len;
  char *dev;
  int hIn, hOut, hErr;
  FILE *fIn, *fOut, *fErr;

  argv = malloc(sizeof(*argv));
  argc = 1;
  
  /* pass important arguments in argv[0] */
  winArg.hInstance = hInstance;
  winArg.nCmdShow = nCmdShow;

  argv[0] = (char *) &winArg;

  s = lpCmdLine;
  while (*s == '-') {
    argc++;
    argv = realloc(argv, argc * sizeof(*argv));
    as = malloc(strlen(s) + 1);

    len = 0;
    while (*s != '\0' && *s != ' ') {
      as[len] = *s++;
      len++;
    }
    as[len] = '\0';

    as = realloc(as, len + 1);
    argv[argc - 1] = as;
  }

  while (*s == ' ')
    s++;

  dev = getenv("MOLMOLDEV");
  if (dev != NULL && strncmp(dev, "TTY", 3) == 0) {
    AllocConsole();

    hIn = _open_osfhandle(
        (long) GetStdHandle(STD_INPUT_HANDLE), _O_TEXT);
    fIn = _fdopen(hIn, "r");
    *stdin = *fIn;
    (void) setvbuf(stdin, NULL, _IONBF, 0);

    hOut = _open_osfhandle(
        (long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
    fOut = _fdopen(hOut, "w");
    *stdout = *fOut;
    (void) setvbuf(stdout, NULL, _IONBF, 0);

    hErr = _open_osfhandle(
        (long) GetStdHandle(STD_ERROR_HANDLE), _O_TEXT);
    fErr = _fdopen(hErr, "w");
    *stderr = *fErr;
    (void) setvbuf(stderr, NULL, _IONBF, 0);
  }

  if (*s == '\0')
    return MolInit("Windows/OpenGL", argc, argv, NULL);
  else
    return MolInit("Windows/OpenGL", argc, argv, s);
}
