/*
************************************************************************
*
*   FileName.c - file name utility functions
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 98/03/19
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/tools/src/SCCS/s.FileName.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <file_name.h>

#include <ctype.h>

void
FileNamePath(DSTR name)
{
  char *s;
  int lastI;

  s = DStrToStr(name);
  lastI = DStrLen(name) - 1;
  while (lastI >= 0 && s[lastI] != '/')
    lastI--;
  if (lastI >= 0)
    DStrPart(name, 0, lastI);
  else
    DStrAssignStr(name, "");
}

void
FileNameBase(DSTR name)
{
  char *s;
  int firstI, lastI, i;

  s = DStrToStr(name);
  lastI = DStrLen(name) - 1;
  i = lastI;
  while (i >= 0 && s[i] != '.' && s[i] != '/')
    i--;
  if (i >= 0 && s[i] == '.') {
    lastI = i - 1;
    while (i >= 0 && s[i] != '/')
      i--;
  }
  firstI = i + 1;
  DStrPart(name, firstI, lastI);
}

void
FileNameExt(DSTR name)
{
  char *s;
  int len, i;

  s = DStrToStr(name);
  len = DStrLen(name) - 1;
  i = len;
  while (i >= 0 && s[i] != '.' && s[i] != '/')
    i--;
  if (i >= 0 && s[i] == '.')
    DStrPart(name, i + 1, len);
  else
    DStrAssignStr(name, "");
}

void
FileNameStrip(DSTR name)
{
  char *s;
  int len, startI, endI;

  s = DStrToStr(name);
  len = DStrLen(name);

  startI = 0;
  while (startI < len && isspace(s[startI]))
    startI++;

  endI = len - 1;
  while (endI >= startI && isspace(s[endI]))
    endI--;

  DStrPart(name, startI, endI);
}

BOOL
FileNameIsAbs(DSTR name)
{
  char *s;
  int len;

  s = DStrToStr(name);
  if (s[0] == '/')
    return TRUE;  /* UNIX absolute */

  len = DStrLen(name);
  if (len > 2 && s[1] == ':' && s[2] == '/')
    return TRUE;  /* Windows absolute */
  
  return FALSE;
}
