/*
************************************************************************
*
*   OsSleep.c - sleep
*
*   Copyright (c) 1994-97
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
*   Pathname of SCCS file     : /local/home/kor/molmol/src/os/SCCS/s.OsSleep.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <os_sleep.h>

#ifdef WIN32

#include <windows.h>

void
OsSleep(int delay)
{
  (void) SleepEx(delay, FALSE);
}

#else

#undef _POSIX_SOURCE  /* select is not a POSIX function */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#ifndef _AIX
#ifndef _HPUX_SOURCE
/* prototype missing in system header files */
extern int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
#endif
#endif

void
OsSleep(int delay)
{
  struct timeval timeS;

  timeS.tv_sec = delay / 1000;
  timeS.tv_usec = (delay % 1000) * 1000;

  (void) select(0, NULL, NULL, NULL, &timeS);
}

#endif
