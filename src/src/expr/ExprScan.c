/*
************************************************************************
*
*   ExprScan.c - expression scanner
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/expr/SCCS/s.ExprScan.c
*   SCCS identification       : 1.14
*
************************************************************************
*/

#include "expr_scan.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <bool.h>
#include <dstr.h>

static int IntVal;
static float FloatVal;
static DSTR StrVal = NULL;

static char *ExprStr;

void
ExprScanStart(char *exprStr)
{
  ExprStr = exprStr - 1;  /* will be incremented before first use */

  if (StrVal == NULL)
    StrVal = DStrNew();
}

static ExprSym
lookupIdent(void)
{
  char *s = DStrToStr(StrVal);

  if (strcmp(s, "sqrt") == 0)
    return ES_SQRT;
  if (strcmp(s, "log") == 0)
    return ES_LOG;
  if (strcmp(s, "not") == 0)
    return ES_NOT;
  if (strcmp(s, "or") == 0)
    return ES_OR;
  if (strcmp(s, "and") == 0)
    return ES_AND;

  if (strcmp(s, "exists") == 0)
    return ES_EXISTS;
  if (strcmp(s, "all") == 0)
    return ES_ALL;

  if (strcmp(s, "mol") == 0)
    return ES_MOL;
  if (strcmp(s, "mol1") == 0)
    return ES_MOL1;
  if (strcmp(s, "mol2") == 0)
    return ES_MOL2;
  if (strcmp(s, "res") == 0)
    return ES_RES;
  if (strcmp(s, "res1") == 0)
    return ES_RES1;
  if (strcmp(s, "res2") == 0)
    return ES_RES2;
  if (strcmp(s, "atom") == 0)
    return ES_ATOM;
  if (strcmp(s, "atom1") == 0)
    return ES_ATOM1;
  if (strcmp(s, "atom2") == 0)
    return ES_ATOM2;
  if (strcmp(s, "atom3") == 0)
    return ES_ATOM3;
  if (strcmp(s, "atom4") == 0)
    return ES_ATOM4;
  if (strcmp(s, "bond") == 0)
    return ES_BOND;
  if (strcmp(s, "angle") == 0)
    return ES_ANGLE;
  if (strcmp(s, "dist") == 0)
    return ES_DIST;
  if (strcmp(s, "prim") == 0)
    return ES_PRIM;

  if (strcmp(s, "upl") == 0)
    return ES_UPL;
  if (strcmp(s, "lol") == 0)
    return ES_LOL;
  if (strcmp(s, "hbond") == 0)
    return ES_HBOND;

  if (strcmp(s, "num") == 0)
    return ES_NUMBER;
  if (strcmp(s, "number") == 0)
    return ES_NUMBER;
  if (strcmp(s, "name") == 0)
    return ES_NAME;
  if (strcmp(s, "shift") == 0)
    return ES_SHIFT;
  if (strcmp(s, "bfactor") == 0)
    return ES_BFACTOR;
  if (strcmp(s, "vdw") == 0)
    return ES_VDW;
  if (strcmp(s, "charge") == 0)
    return ES_CHARGE;
  if (strcmp(s, "heavycharge") == 0)
    return ES_HEAVYCHARGE;
  if (strcmp(s, "avgcharge") == 0)
    return ES_AVGCHARGE;
  if (strcmp(s, "simplecharge") == 0)
    return ES_SIMPLECHARGE;
  if (strcmp(s, "len") == 0)
    return ES_LEN;
  if (strcmp(s, "val") == 0)
    return ES_VAL;
  if (strcmp(s, "limit") == 0)
    return ES_LIMIT;
  if (strcmp(s, "attr") == 0)
    return ES_ATTR;
  if (strcmp(s, "viol") == 0)
    return ES_VIOL;
  if (strcmp(s, "d") == 0)
    return ES_D;

  return ES_IDENT;
}

ExprSym
ExprScanGetSym(void)
{
  char ch;
  BOOL isFloat;

  do
    ExprStr++;
  while (ExprStr[0] == ' ');

  switch (ExprStr[0]) {
    case '+':
      return ES_PLUS;
    case '-':
      return ES_MINUS;
    case '*':
      return ES_MUL;
    case '/':
      return ES_DIV;
    case '%':
      return ES_REM;
    case '=':
      if (ExprStr[1] == '=')
	ExprStr++;
      return ES_EQU;
    case '!':
      if (ExprStr[1] == '=') {
	ExprStr++;
	return ES_NEQ;
      }
      return ES_NOT;
    case '<':
      if (ExprStr[1] == '=') {
	ExprStr++;
	return ES_LEQ;
      } else if (ExprStr[1] == '>') {
	ExprStr++;
	return ES_NEQ;
      }
      return ES_LSS;
    case '>':
      if (ExprStr[1] == '=') {
	ExprStr++;
	return ES_GEQ;
      }
      return ES_GTR;
    case '|':
      if (ExprStr[1] == '|')
	ExprStr++;
      return ES_OR;
    case '&':
      if (ExprStr[1] == '&')
	ExprStr++;
      return ES_AND;
    case '.':
      if (ExprStr[1] == '.') {
	ExprStr++;
	return ES_RANGE;
      }
      return ES_DOT;
    case ':':
      return ES_COLON;
    case '(':
      return ES_LPAR;
    case ')':
      return ES_RPAR;
    case ',':
      return ES_COMMA;
    case '\0':
      return ES_END;
    default:
      ch = ExprStr[0];
      DStrAssignStr(StrVal, "");
      if (isdigit(ch)) {
        isFloat = FALSE;
	for (;;) {
	  if (isdigit(ch)) {
	    DStrAppChar(StrVal, ch);
	  } else if ((ch == '.' && ExprStr[1] != '.') ||
	      ch == 'e' || ch == 'E') {
	    DStrAppChar(StrVal, ch);
	    isFloat = TRUE;
	  } else {
	    break;
	  }
	  ExprStr++;
	  ch = ExprStr[0];
	}
	ExprStr--;
	if (isFloat) {
	  FloatVal = (float) atof(DStrToStr(StrVal));
	  return ES_FLOAT;
	} else {
	  IntVal = atoi(DStrToStr(StrVal));
	  return ES_INT;
	}
      } else if (isalpha(ch)) {
	while (isalnum(ch) || ch == '_') {
	  DStrAppChar(StrVal, ch);
	  ExprStr++;
	  ch = ExprStr[0];
	}
	ExprStr--;
	return lookupIdent();
      } else if (ch == '"') {
	ExprStr++;
	ch = ExprStr[0];
	while (ch != '"' && ch != '\0') {
	  if (ch == '\\' && ExprStr[1] == '"') {
	    ExprStr++;
	    ch = '"';
	  }
	  DStrAppChar(StrVal, ch);
	  ExprStr++;
	  ch = ExprStr[0];
	}
	if (ch == '"')
	  return ES_STR;
      }
      return ES_UNKNOWN;
  }  /* switch */
}

int
ExprScanGetInt(void)
{
  return IntVal;
}

float
ExprScanGetFloat(void)
{
  return FloatVal;
}

char *
ExprScanGetStr(void)
{
  return DStrToStr(StrVal);
}
