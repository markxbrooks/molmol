/*
************************************************************************
*
*   StrMatch.c - string matching with wildcards
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
*   Date of last modification : 94/10/19
*   Pathname of SCCS file     : /sgiext/molmol/tools/src/SCCS/s.StrMatch.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <strmatch.h>

static BOOL
strMatchRec(char *strStart, char *strEnd, char *patStart, char *patEnd)
{
  BOOL match;

  for (;;) {
    if (patStart > patEnd)
      return strStart > strEnd;

    if (*patStart == '*')
      break;

    if (strStart > strEnd)
      return FALSE;
    
    if (*patStart == '[') {
      match = FALSE;
      patStart++;
      while (patStart <= patEnd && *patStart != ']') {
        if (*patStart == *strStart)
          match = TRUE;
        patStart++;
      }
      if (! match)
        return FALSE;
    } else if (*patStart != '?' && *patStart != *strStart) {
      return FALSE;
    }

    strStart++;
    patStart++;
  }  /* for (;;) */

  for (;;) {
    if (patStart > patEnd)
      return strStart > strEnd;

    if (*patEnd == '*')
      break;

    if (strStart > strEnd)
      return FALSE;
    
    if (*patEnd == ']') {
      match = FALSE;
      patEnd--;
      while (patEnd >= patStart && *patEnd != '[') {
      if (*patEnd == *strEnd)
        match = TRUE;
      patEnd--;
      }
      if (! match)
	return FALSE;
    } else if (*patEnd != '?' && *patEnd != *strEnd) {
      return FALSE;
    }

    strEnd--;
    patEnd--;
  }  /* for (;;) */

  if (patStart == patEnd)  /* pattern is a single '*' */
    return TRUE;
  
  /* pattern has '*' at beginning and at end */
  patStart++;  /* leave away first '*' */
  while (strStart <= strEnd) {
    if (strMatchRec(strStart, strEnd, patStart, patEnd))
      return TRUE;
    strStart++;
  }

  return FALSE;
}

BOOL
StrMatch(char *str, char *pat)
{
  char *strEnd, *patEnd;

  strEnd = str;
  while (*strEnd != '\0')
    strEnd++;
  strEnd--;

  patEnd = pat;
  while (*patEnd != '\0')
    patEnd++;
  patEnd--;

  return strMatchRec(str, strEnd, pat, patEnd);
}
