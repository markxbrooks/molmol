/*
************************************************************************
*
*   expr_scan.h - expression scanner
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/expr/SCCS/s.expr_scan.h
*   SCCS identification       : 1.12
*
************************************************************************
*/

typedef enum {
  ES_PLUS,
  ES_MINUS,
  ES_MUL,
  ES_DIV,
  ES_REM,
  ES_SQRT,
  ES_LOG,
  ES_EQU,
  ES_NEQ,
  ES_LEQ,
  ES_LSS,
  ES_GEQ,
  ES_GTR,
  ES_OR,
  ES_AND,
  ES_NOT,

  ES_DOT,
  ES_COLON,
  ES_LPAR,
  ES_RPAR,
  ES_COMMA,
  ES_RANGE,

  ES_END,
  ES_UNKNOWN,

  ES_INT,
  ES_FLOAT,
  ES_STR,
  ES_IDENT,

  ES_EXISTS,
  ES_ALL,

  ES_MOL,
  ES_MOL1,
  ES_MOL2,
  ES_RES,
  ES_RES1,
  ES_RES2,
  ES_ATOM,
  ES_ATOM1,
  ES_ATOM2,
  ES_ATOM3,
  ES_ATOM4,
  ES_BOND,
  ES_ANGLE,
  ES_DIST,
  ES_PRIM,

  ES_UPL,
  ES_LOL,
  ES_HBOND,

  ES_NUMBER,
  ES_NAME,
  ES_SHIFT,
  ES_BFACTOR,
  ES_VDW,
  ES_CHARGE,
  ES_HEAVYCHARGE,
  ES_AVGCHARGE,
  ES_SIMPLECHARGE,
  ES_LEN,
  ES_VAL,
  ES_LIMIT,
  ES_VIOL,
  ES_ATTR,
  ES_D
} ExprSym;

extern void ExprScanStart(char *);

extern ExprSym ExprScanGetSym(void);

extern int ExprScanGetInt(void);

extern float ExprScanGetFloat(void);

extern char *ExprScanGetStr(void);
