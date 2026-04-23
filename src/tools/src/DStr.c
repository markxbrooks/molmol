/*
************************************************************************
*
*   DStr.c - abstract type for dynamic strings
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
*   Date of last modification : 95/07/20
*   Pathname of SCCS file     : /sgiext/molmol/tools/src/SCCS/s.DStr.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <dstr.h>

#include <stdlib.h>
#include <string.h>
#include <memory.h>

#define REALLOC_DSTR(dstr) \
    dstr->str = realloc(dstr->str, dstr->size);

#define ALLOC_INC 16

struct dstr {
  char   *str;
  int     len;
  int     size;
};

/**********************************************************************/
DSTR
DStrNew(void)
/*----------------------------------------------------------------------
Create and return a new DSTR.
----------------------------------------------------------------------*/
{
  DSTR    dstr;

  dstr = malloc(sizeof(*dstr));
  dstr->size = 1;
  dstr->str = malloc(dstr->size);
  dstr->str[0] = '\0';
  dstr->len = 0;

  return dstr;
}

/**********************************************************************/
void
DStrFree(DSTR dstr)
/*----------------------------------------------------------------------
Free the memory used by dstr. Passing a NULL pointer is legal.
----------------------------------------------------------------------*/
{
  if (dstr == NULL)
    return;

  free(dstr->str);
  free(dstr);
}

/**********************************************************************/
char   *
DStrToStr(DSTR dstr)
/*----------------------------------------------------------------------
Convert dstr to a char*. Returns a pointer to internal data, must not
be freed.
----------------------------------------------------------------------*/
{
  return dstr->str;
}

/**********************************************************************/
static void
strAssign(DSTR dstr, char *str, int len)
/*----------------------------------------------------------------------
----------------------------------------------------------------------*/
{
  if (dstr->size <= len) {
    dstr->size = len + 1;
    REALLOC_DSTR(dstr);
  } else if (dstr->size > 2 * len) {
    dstr->size = len + 1;
    REALLOC_DSTR(dstr);
  }
  (void) memcpy(dstr->str, str, len + 1);
  dstr->len = len;
}

/**********************************************************************/
void
DStrAssignStr(DSTR dstr, char *str)
/*----------------------------------------------------------------------
Assign the string str to dstr.
----------------------------------------------------------------------*/
{
  strAssign(dstr, str, strlen(str));
}

/**********************************************************************/
void
DStrAssignDStr(DSTR dstr1, DSTR dstr2)
/*----------------------------------------------------------------------
Assign the string dstr2 to dstr1.
----------------------------------------------------------------------*/
{
  strAssign(dstr1, dstr2->str, dstr2->len);
}

/**********************************************************************/
void
DStrAppChar(DSTR dstr, char ch)
/*----------------------------------------------------------------------
Append the character ch to dstr.
----------------------------------------------------------------------*/
{
  if (dstr->len == dstr->size - 1) {
    dstr->size += ALLOC_INC;
    REALLOC_DSTR(dstr);
  }
  dstr->str[dstr->len++] = ch;
  dstr->str[dstr->len] = '\0';
}

/**********************************************************************/
static void
strAppend(DSTR dstr, char *str, int len)
/*----------------------------------------------------------------------
----------------------------------------------------------------------*/
{
  int     totLen;

  totLen = dstr->len + len;
  if (dstr->size <= totLen) {
    dstr->size = totLen + ALLOC_INC;
    REALLOC_DSTR(dstr);
  }
  (void) memcpy(dstr->str + dstr->len, str, len + 1);
  dstr->len = totLen;
}

/**********************************************************************/
void
DStrAppStr(DSTR dstr, char *str)
/*----------------------------------------------------------------------
Append the string str to dstr.
----------------------------------------------------------------------*/
{
  strAppend(dstr, str, strlen(str));
}

/**********************************************************************/
void
DStrAppDStr(DSTR dstr1, DSTR dstr2)
/*----------------------------------------------------------------------
Append the string dstr2 to dstr1.
----------------------------------------------------------------------*/
{
  strAppend(dstr1, dstr2->str, dstr2->len);
}

/**********************************************************************/
void
DStrPart(DSTR dstr, int startPos, int endPos)
/*----------------------------------------------------------------------
Take part of dstr, from position startPos to endPos.
----------------------------------------------------------------------*/
{
  int newLen, i;

  if (startPos < 0)
    startPos = 0;
  if (endPos >= dstr->len)
    endPos = dstr->len - 1;

  newLen = endPos - startPos + 1;
  if (newLen <= 0)
    newLen = 0;

  /* source and destination can overlap, do not use memcpy */
  for (i = 0; i < newLen; i++)
    dstr->str[i] = dstr->str[startPos + i];

  dstr->str[newLen] = '\0';
  dstr->len = newLen;
}

/**********************************************************************/
int
DStrLen(DSTR dstr)
/*----------------------------------------------------------------------
Return the length of dstr.
----------------------------------------------------------------------*/
{
  return dstr->len;
}

/**********************************************************************/
int
DStrCmp(DSTR dstr1, DSTR dstr2)
/*----------------------------------------------------------------------
Compare dstr1 and dstr2. Result like strcmp.
----------------------------------------------------------------------*/
{
  return strcmp(dstr1->str, dstr2->str);
}
