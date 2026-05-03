/*
************************************************************************
*
*   psgamma.c - do gamma correction on PostScript file
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
*   Date of last modification : 95/07/13
*   Pathname of SCCS file     : /sgiext/molmol/aux/SCCS/s.psgamma.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef int BOOL;
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define KEY_WORD "setrgbcolor"
#define TOKEN_LEN 1000
#define DEF_NO 10
#define DEF_LEN 50

typedef char Token[TOKEN_LEN];

static int DefNo = 0;
static char Def[DEF_NO][DEF_LEN];

static void
addDef(char *name)
{
  int len;

  if (DefNo >= DEF_NO)
    return;

  len = strlen(name);
  if (len >= DEF_LEN)
    return;
  
  (void) strcpy(Def[DefNo], name);

  while (isspace(Def[DefNo][len - 1])) {
    len--;
    Def[DefNo][len] = '\0';
  }

  DefNo++;
}

static BOOL
strEq(char *tok, char *str)
{
  int len;

  len = strlen(str);
  return strncmp(tok, str, len) == 0 && 
      (tok[len] == '\0' || isspace(tok[len]));
}

static BOOL
isKey(char *name)
{
  int i;

  if (strEq(name, KEY_WORD))
    return TRUE;

  for (i = 0; i < DefNo; i++)
    if (strEq(name, Def[i]))
      return TRUE;
  
  return FALSE;
}

static BOOL
isNumber(char *str)
{
  return isdigit(str[0]);
}

static void
advTok(Token *tok, int *firstIP, int *currIP)
{
  int firstI = *firstIP;
  int currI = *currIP;

  if ((currI + 1) % 4 == firstI) {
    (void) printf("%s", tok[firstI]);
    firstI = (firstI + 1) % 4;
  }

  currI = (currI + 1) % 4;

  *firstIP = firstI;
  *currIP = currI;
}

int
main(int argc, char *argv[])
{
  float expo;
  Token tok[4];
  int firstI, currI;
  int len, parLev;
  BOOL esc;
  int ch;
  char *name;
  int i;

  if (argc < 2 || argc > 2 && isdigit(argv[2][0])) {
    (void) fprintf(stderr, "usage: psgamma exponent [ keyword ...]\n");
    return 1;
  }

  expo = atof(argv[1]);

  for (i = 2; i < argc; i++)
    addDef(argv[i]);

  firstI = 0;
  currI = 0;
  len = 0;
  parLev = 0;
  esc = FALSE;

  for (;;) {
    ch = getchar();
    if (ch == EOF)
      break;

    tok[currI][len++] = ch;
    if (len == TOKEN_LEN - 1) {
      tok[currI][len] = '\0';
      advTok(tok, &firstI, &currI);
      len = 0;
    }

    if (ch == '\\') {
      esc = TRUE;
      continue;
    }

    if (ch == '(')
      parLev++;
    else if (ch == ')')
      parLev--;

    if (parLev > 0)
      continue;

    if (isspace(ch)) {
      tok[currI][len] = '\0';

      if (isKey(tok[currI])) {
	if (isNumber(tok[(currI + 1) % 4]) &&
	    isNumber(tok[(currI + 2) % 4]) &&
	    isNumber(tok[(currI + 3) % 4])) {
	  (void) printf("%5.3f ", pow(atof(tok[(currI + 1) % 4]), expo));
	  (void) printf("%5.3f ", pow(atof(tok[(currI + 2) % 4]), expo));
	  (void) printf("%5.3f ", pow(atof(tok[(currI + 3) % 4]), expo));
	  (void) printf("%s", tok[currI]);
	  currI = 0;
	  firstI = 0;
	} else {
	  while (firstI != currI) {
	    (void) printf("%s", tok[firstI]);
	    firstI = (firstI + 1) % 4;
	  }
	  (void) printf("%5.3f exp ", expo);
	  (void) printf("3 1 roll ");
	  (void) printf("%5.3f exp ", expo);
	  (void) printf("3 1 roll ");
	  (void) printf("%5.3f exp ", expo);
	  (void) printf("3 1 roll ");
	  (void) printf("%s", tok[currI]);
	}
      } else {
        if (strEq(tok[currI], "def") && strEq(tok[(currI + 3) % 4], "load")) {
	  name = tok[(currI + 2) % 4];
	  if (name[0] == '/' && strEq(name + 1, KEY_WORD)) {
	    name = tok[(currI + 1) % 4];
	    if (name[0] == '/')
	      addDef(name + 1);
	  }
	}
	advTok(tok, &firstI, &currI);
      }

      len = 0;
    }

    esc = FALSE;
  }

  while (firstI != currI) {
    (void) printf("%s", tok[firstI]);
    firstI = (firstI + 1) % 4;
  }

  return 0;
}
