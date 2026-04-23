/*
************************************************************************
*
*   OsSeed.c - random number seed
*
*   Copyright (c) 1994-99
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
*   Date of last modification : 99/10/24
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/os/SCCS/s.OsSeed.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <os_sleep.h>

#include <stdio.h>
#ifdef WIN32
#include <time.h>
#include <process.h>
#else
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#endif

int
OsSeed(void)
{
  int t, p;

  t = time(NULL);

#ifdef WIN32
  p = _getpid();
#else
  p = getpid();
#endif

  return t + p;
}
