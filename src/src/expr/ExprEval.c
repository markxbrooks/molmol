/*
************************************************************************
*
*   ExprEval.c - expression evaluation
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/expr/SCCS/s.ExprEval.c
*   SCCS identification       : 1.21
*
************************************************************************
*/

#include <expr.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <string.h>
#include <math.h>

#include <mat_vec.h>
#include <dstr.h>
#include <strmatch.h>
#include <data_hand.h>
#include <data_ref.h>
#include <prim_hand.h>
#include "expr_scan.h"

#define INIT_CODE_SIZE 10

typedef enum {
  OC_NOP,

  OC_PLUS_INT,
  OC_MINUS_INT,
  OC_MUL_INT,
  OC_DIV_INT,
  OC_REM,
  OC_NEG_INT,

  OC_PLUS_FLOAT,
  OC_MINUS_FLOAT,
  OC_MUL_FLOAT,
  OC_DIV_FLOAT,
  OC_NEG_FLOAT,
  OC_SQRT,
  OC_LOG,

  OC_DUP,
  OC_CAST,
  OC_CAST_1,

  OC_EQU_INT,
  OC_NEQ_INT,
  OC_LEQ_INT,
  OC_LSS_INT,
  OC_GEQ_INT,
  OC_GTR_INT,
  OC_INSET_INT,
  OC_OUTSET_INT,

  OC_EQU_FLOAT,
  OC_NEQ_FLOAT,
  OC_LEQ_FLOAT,
  OC_LSS_FLOAT,
  OC_GEQ_FLOAT,
  OC_GTR_FLOAT,
  OC_INSET_FLOAT,
  OC_OUTSET_FLOAT,

  OC_EQU_STR,
  OC_NEQ_STR,

  OC_OR,
  OC_AND,
  OC_NOT,

  OC_GET_INT,
  OC_GET_FLOAT,
  OC_GET_STR,
  OC_GET_PROP,

  OC_EXISTS,
  OC_ALL,

  OC_ENT_MOL,
  OC_ENT_MOL1,
  OC_ENT_MOL2,
  OC_ENT_RES,
  OC_ENT_RES1,
  OC_ENT_RES2,
  OC_ENT_ATOM,
  OC_ENT_ATOM1,
  OC_ENT_ATOM2,
  OC_ENT_ATOM3,
  OC_ENT_ATOM4,
  OC_ENT_BOND,
  OC_ENT_ANGLE,
  OC_ENT_DIST,
  OC_ENT_PRIM,

  OC_GET_UPL,
  OC_GET_LOL,
  OC_GET_HBOND,

  OC_GET_NUMBER,
  OC_GET_NAME,
  OC_GET_SHIFT,
  OC_GET_BFACTOR,
  OC_GET_VDW,
  OC_GET_CHARGE,
  OC_GET_HEAVYCHARGE,
  OC_GET_AVGCHARGE,
  OC_GET_SIMPLECHARGE,
  OC_GET_LEN,
  OC_GET_VAL,
  OC_GET_LIMIT,
  OC_GET_VIOL,
  OC_GET_ATTR,
  OC_GET_D,

  OC_END
} OpCode;

struct ExprS {
  int sp;
  int maxStackSize;
  int codeSize;
  OpCode *code;
  int pcStart, pcEnd;
  DataEntityType entType;
  DhMolP molP;
  DhResP resP;
  DhAtomP atomP;
  DhBondP bondP;
  DhAngleP angleP;
  DhDistP distP;
  PrimObjP primP;
  ExprResType resType;
  OpCode predOp;
  BOOL predResult;
};

typedef struct {
  ExprResType resType;
  int rangeNo;
} Ident;

typedef union {
  BOOL b;
  int i;
  float f;
  char *s;
} StackElem;

#define OP_CODE_NO(size) (((size) + sizeof(OpCode) - 1) / sizeof(OpCode))

static StackElem *Stack = NULL;
static int StackSize = 0;

static ExprP CurrExprP;
static ExprSym Sym;
static char *ErrorMsg;

static DhAtomP DistAtomP;

static void
getSym(void)
{
  Sym = ExprScanGetSym();
}

static void
setError(char *errMsg)
{
  if (ErrorMsg == NULL)
    ErrorMsg = errMsg;
}

static void
checkCodeSpace(void)
{
  if (CurrExprP->pcEnd > CurrExprP->codeSize) {
    CurrExprP->codeSize *= 2;
    CurrExprP->code = realloc(CurrExprP->code,
        CurrExprP->codeSize * sizeof(OpCode));
  }
}

static void
addOpCode(OpCode oc)
{
  CurrExprP->pcEnd++;
  checkCodeSpace();
  CurrExprP->code[CurrExprP->pcEnd - 1] = oc;
}

static void
addBytes(void *p, int size)
{
  int oldLen = CurrExprP->pcEnd;

  CurrExprP->pcEnd += OP_CODE_NO(size);
  checkCodeSpace();
  (void) memcpy(CurrExprP->code + oldLen, p, size);
}

static void
typeCast(Ident *id1P, Ident *id2P)
{
  if (id1P->resType == id2P->resType)
    return;
  
  if (id1P->resType == ER_INT && id2P->resType == ER_FLOAT) {
    addOpCode(OC_CAST_1);
    id1P->resType = ER_FLOAT;
  } else if (id1P->resType == ER_FLOAT && id2P->resType == ER_INT) {
    addOpCode(OC_CAST);
    id2P->resType = ER_FLOAT;
  } else {
    setError("different types for binary operation");
  }
}

static void
incStack(void)
{
  CurrExprP->sp++;
  if (CurrExprP->sp > CurrExprP->maxStackSize)
    CurrExprP->maxStackSize = CurrExprP->sp;
}

static void
decStack(void)
{
  CurrExprP->sp--;
}

static void parseExpr(Ident *);

static void
parseQuantExpr(Ident *idP)
{
  DataEntityType oldType;
  OpCode ocQuant, oc;

  if (CurrExprP->entType == DE_NONE)
    setError("quantor not allowed");

  if (Sym == ES_EXISTS)
    ocQuant = OC_EXISTS;
  else
    ocQuant = OC_ALL;

  getSym();
  oldType = CurrExprP->entType;

  if (Sym == ES_MOL) {
    oc = OC_ENT_MOL;
    CurrExprP->entType = DE_MOL;
  } else if (Sym == ES_RES) {
    oc = OC_ENT_RES;
    CurrExprP->entType = DE_RES;
  } else if (Sym == ES_ATOM) {
    oc = OC_ENT_ATOM;
    CurrExprP->entType = DE_ATOM;
  } else if (Sym == ES_BOND) {
    oc = OC_ENT_BOND;
    CurrExprP->entType = DE_BOND;
  } else if (Sym == ES_ANGLE) {
    oc = OC_ENT_ANGLE;
    CurrExprP->entType = DE_ANGLE;
  } else if (Sym == ES_DIST) {
    oc = OC_ENT_DIST;
    CurrExprP->entType = DE_DIST;
  } else if (Sym == ES_PRIM) {
    oc = OC_ENT_PRIM;
    CurrExprP->entType = DE_PRIM;
  } else if (Sym == ES_MOL1 || Sym == ES_MOL2 ||
      Sym == ES_RES1 || Sym == ES_RES2 ||
      Sym == ES_ATOM1 || Sym == ES_ATOM2 ||
      Sym == ES_ATOM3 || Sym == ES_ATOM4) {
    setError("unallowed qualifier");
  } else {
    oc = OC_NOP;
  }

  if (oc != OC_NOP) {
    addOpCode(oc);
    getSym();
    if (Sym == ES_LPAR)
      getSym();
    else
      setError("( expected");
  }

  addOpCode(ocQuant);

  parseExpr(idP);
  CurrExprP->entType = oldType;
  addOpCode(OC_END);
  if (idP->resType != ER_BOOL)
    setError("boolean expression expected");

  if (Sym == ES_RPAR)
    getSym();
  else
    setError(") expected");
}

static DataEntityType
parseQual(void)
{
  OpCode oc;
  DataEntityType entType;

  if (Sym == ES_MOL) {
    oc = OC_ENT_MOL;
    entType = DE_MOL;
  } else if (Sym == ES_MOL1) {
    oc = OC_ENT_MOL1;
    entType = DE_MOL;
    if (CurrExprP->entType != DE_BOND &&
	CurrExprP->entType != DE_DIST)
      setError("mol1 only valid for bond and dist");
  } else if (Sym == ES_MOL2) {
    oc = OC_ENT_MOL2;
    entType = DE_MOL;
    if (CurrExprP->entType != DE_BOND &&
	CurrExprP->entType != DE_DIST)
      setError("mol2 only valid for bond and dist");
  } else if (Sym == ES_RES) {
    oc = OC_ENT_RES;
    entType = DE_RES;
    if (CurrExprP->entType != DE_ATOM &&
        CurrExprP->entType != DE_BOND &&
	CurrExprP->entType != DE_ANGLE &&
	CurrExprP->entType != DE_RES)
      setError("res only valid for atom, bond, angle and res");
  } else if (Sym == ES_RES1) {
    oc = OC_ENT_RES1;
    entType = DE_RES;
    if (CurrExprP->entType != DE_BOND &&
	CurrExprP->entType != DE_DIST)
      setError("res1 only valid for bond and dist");
  } else if (Sym == ES_RES2) {
    oc = OC_ENT_RES2;
    entType = DE_RES;
    if (CurrExprP->entType != DE_BOND &&
	CurrExprP->entType != DE_DIST)
      setError("res2 only valid for bond and dist");
  } else if (Sym == ES_ATOM) {
    oc = OC_ENT_ATOM;
    entType = DE_ATOM;
    if (CurrExprP->entType != DE_ATOM)
      setError("atom only valid for atom");
  } else if (Sym == ES_ATOM1) {
    oc = OC_ENT_ATOM1;
    entType = DE_ATOM;
    if (CurrExprP->entType != DE_BOND &&
	CurrExprP->entType != DE_ANGLE &&
	CurrExprP->entType != DE_DIST)
      setError("atom1 only valid for bond, angle and dist");
  } else if (Sym == ES_ATOM2) {
    oc = OC_ENT_ATOM2;
    entType = DE_ATOM;
    if (CurrExprP->entType != DE_BOND &&
	CurrExprP->entType != DE_ANGLE &&
	CurrExprP->entType != DE_DIST)
      setError("atom2 only valid for bond, angle and dist");
  } else if (Sym == ES_ATOM3) {
    oc = OC_ENT_ATOM3;
    entType = DE_ATOM;
    if (CurrExprP->entType != DE_ANGLE)
      setError("atom3 only valid for angle");
  } else if (Sym == ES_ATOM4) {
    oc = OC_ENT_ATOM4;
    entType = DE_ATOM;
    if (CurrExprP->entType != DE_ANGLE)
      setError("atom4 only valid for angle");
  } else if (Sym == ES_BOND) {
    oc = OC_ENT_BOND;
    entType = DE_BOND;
    if (CurrExprP->entType != DE_BOND)
      setError("bond only valid for bond");
  } else if (Sym == ES_ANGLE) {
    oc = OC_ENT_ANGLE;
    entType = DE_ANGLE;
    if (CurrExprP->entType != DE_ANGLE)
      setError("angle only valid for angle");
  } else if (Sym == ES_DIST) {
    oc = OC_ENT_DIST;
    entType = DE_DIST;
    if (CurrExprP->entType != DE_DIST)
      setError("dist only valid for dist");
  } else if (Sym == ES_PRIM) {
    oc = OC_ENT_PRIM;
    entType = DE_PRIM;
    if (CurrExprP->entType != DE_PRIM)
      setError("prim only valid for prim");
  } else {
    oc = OC_NOP;
    entType = CurrExprP->entType;
  }
  
  if (oc != OC_NOP) {
    addOpCode(oc);
    getSym();
    if (Sym == ES_DOT)
      getSym();
    else
      setError(". expected");
  }

  return entType;
}

static void
parseDesignator(Ident *idP)
{
  DataEntityType entType;
  PropRefP refP;

  entType = parseQual();

  if (Sym == ES_UPL) {
    idP->resType = ER_BOOL;
    addOpCode(OC_GET_UPL);
    if (entType != DE_DIST)
      setError("upl only defined for dist");
  } else if (Sym == ES_LOL) {
    idP->resType = ER_BOOL;
    addOpCode(OC_GET_LOL);
    if (entType != DE_DIST)
      setError("lol only defined for dist");
  } else if (Sym == ES_HBOND) {
    idP->resType = ER_BOOL;
    addOpCode(OC_GET_HBOND);
    if (entType != DE_DIST)
      setError("hbond only defined for dist");
  } else if (Sym == ES_NUMBER) {
    idP->resType = ER_INT;
    addOpCode(OC_GET_NUMBER);
    if (entType != DE_MOL && entType != DE_RES && entType != DE_PRIM)
      setError("number only defined for mol, res and prim");
  } else if (Sym == ES_NAME) {
    idP->resType = ER_STR;
    addOpCode(OC_GET_NAME);
    if (entType != DE_MOL && entType != DE_RES &&
	entType != DE_ATOM && entType != DE_ANGLE)
      setError("name only defined for mol, res, atom and angle");
  } else if (Sym == ES_SHIFT) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_SHIFT);
    if (entType != DE_ATOM)
      setError("shift only defined for atom");
  } else if (Sym == ES_BFACTOR) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_BFACTOR);
    if (entType != DE_ATOM)
      setError("bfactor only defined for atom");
  } else if (Sym == ES_VDW) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_VDW);
    if (entType != DE_ATOM)
      setError("vdw only defined for atom");
  } else if (Sym == ES_CHARGE) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_CHARGE);
    if (entType != DE_ATOM)
      setError("charge only defined for atom");
  } else if (Sym == ES_HEAVYCHARGE) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_HEAVYCHARGE);
    if (entType != DE_ATOM)
      setError("heavycharge only defined for atom");
  } else if (Sym == ES_AVGCHARGE) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_AVGCHARGE);
    if (entType != DE_ATOM)
      setError("avgcharge only defined for atom");
  } else if (Sym == ES_SIMPLECHARGE) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_SIMPLECHARGE);
    if (entType != DE_ATOM)
      setError("simplecharge only defined for atom");
  } else if (Sym == ES_LEN) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_LEN);
    if (entType != DE_BOND)
      setError("len only defined for bond");
  } else if (Sym == ES_VAL) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_VAL);
    if (entType != DE_ANGLE && entType != DE_DIST)
      setError("val only defined for angle and dist");
  } else if (Sym == ES_LIMIT) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_LIMIT);
    if (entType != DE_DIST)
      setError("limit only defined for dist");
  } else if (Sym == ES_VIOL) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_VIOL);
    if (entType != DE_DIST)
      setError("viol only defined for dist");
  } else if (Sym == ES_ATTR) {
    idP->resType = ER_INT;
    addOpCode(OC_GET_ATTR);
    if (entType != DE_ATOM && entType != DE_BOND &&
	entType != DE_DIST && entType != DE_PRIM)
      setError("attr only defined for atom, bond, dist and prim");
  } else if (Sym == ES_D) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_D);
    if (entType != DE_ATOM)
      setError("dist only defined for atom");
  } else if (Sym == ES_IDENT) {
    idP->resType = ER_BOOL;
    addOpCode(OC_GET_PROP);
    refP = PropGetRef(ExprScanGetStr(), FALSE);
    if (refP == NULL)
      setError("unknown property");
    else
      addBytes(&refP, sizeof(refP));
    if (entType == DE_NONE)
      setError("no access to property");
  } else {
    setError("identifier expected");
  }
  
  getSym();
}

static void
parsePrimary(Ident *idP)
{
  int i;
  float f;
  char *s;

  if (Sym == ES_INT) {
    idP->resType = ER_INT;
    addOpCode(OC_GET_INT);
    i = ExprScanGetInt();
    addBytes(&i, sizeof(i));
    getSym();
  } else if (Sym == ES_FLOAT) {
    idP->resType = ER_FLOAT;
    addOpCode(OC_GET_FLOAT);
    f = ExprScanGetFloat();
    addBytes(&f, sizeof(f));
    getSym();
  } else if (Sym == ES_STR) {
    idP->resType = ER_STR;
    addOpCode(OC_GET_STR);
    s = ExprScanGetStr();
    addBytes(s, strlen(s) + 1);
    getSym();
  } else if (Sym == ES_LPAR) {
    getSym();
    parseExpr(idP);
    if (Sym == ES_RPAR)
      getSym();
    else
      setError(") expected");
  } else if (Sym == ES_EXISTS || Sym == ES_ALL) {
    parseQuantExpr(idP);
  } else {
    parseDesignator(idP);
  }

  incStack();
}

static void
parseFactor(Ident *idP)
{
  ExprSym opSym;

  opSym = Sym;
  if (opSym == ES_MINUS || opSym == ES_SQRT ||
      opSym == ES_LOG || opSym == ES_NOT)
    getSym();

  parsePrimary(idP);

  if (opSym == ES_MINUS) {
    if (idP->resType == ER_INT)
      addOpCode(OC_NEG_INT);
    else if (idP->resType == ER_FLOAT)
      addOpCode(OC_NEG_FLOAT);
    else
      setError("int or float expected for -");
  } else if (opSym == ES_SQRT) {
    if (idP->resType == ER_INT) {
      addOpCode(OC_CAST);
      idP->resType = ER_FLOAT;
    }
    if (idP->resType == ER_FLOAT)
      addOpCode(OC_SQRT);
    else
      setError("int of float expected for sqrt");
  } else if (opSym == ES_LOG) {
    if (idP->resType == ER_INT) {
      addOpCode(OC_CAST);
      idP->resType = ER_FLOAT;
    }
    if (idP->resType == ER_FLOAT)
      addOpCode(OC_LOG);
    else
      setError("int of float expected for log");
  } else if (opSym == ES_NOT) {
    if (idP->resType == ER_BOOL)
      addOpCode(OC_NOT);
    else
      setError("boolean expected for !");
  }
}

static void
parseTerm(Ident *idP)
{
  ExprSym opSym;
  Ident id2;

  parseFactor(idP);
  while (Sym == ES_MUL || Sym == ES_DIV || Sym == ES_REM) {
    opSym = Sym;
    getSym();
    parseFactor(&id2);
    typeCast(idP, &id2);
    if (opSym == ES_MUL) {
      if (idP->resType == ER_INT)
	addOpCode(OC_MUL_INT);
      else if (idP->resType == ER_FLOAT)
	addOpCode(OC_MUL_FLOAT);
      else
	setError("int or float expected for *");
    } else if (opSym == ES_DIV) {
      if (idP->resType == ER_INT)
	addOpCode(OC_DIV_INT);
      else if (idP->resType == ER_FLOAT)
	addOpCode(OC_DIV_FLOAT);
      else
	setError("int or float expected for /");
    } else {
      if (idP->resType == ER_INT)
	addOpCode(OC_REM);
      else
	setError("int expected for %");
    }
    decStack();
  }
}

static void
parseSimpleExpr(Ident *idP)
{
  ExprSym opSym;
  Ident id2;

  parseTerm(idP);
  while (Sym == ES_PLUS || Sym == ES_MINUS) {
    opSym = Sym;
    getSym();
    parseTerm(&id2);
    typeCast(idP, &id2);
    if (idP->resType == ER_INT)
      if (opSym == ES_PLUS)
	addOpCode(OC_PLUS_INT);
      else
	addOpCode(OC_MINUS_INT);
    else if (idP->resType == ER_FLOAT)
      if (opSym == ES_PLUS)
	addOpCode(OC_PLUS_FLOAT);
      else
	addOpCode(OC_MINUS_FLOAT);
    else
      setError("int or float expected for + or -");
    decStack();
  }
}

static void
parseRangeExpr(Ident *idP)
{
  Ident id2;

  parseSimpleExpr(idP);
  if (Sym == ES_RANGE) {
    getSym();
    parseSimpleExpr(&id2);
    typeCast(idP, &id2);

    if (idP->resType == ER_INT)
      idP->resType = ER_INT_SET;
    else if (idP->resType == ER_FLOAT)
      idP->resType = ER_FLOAT_SET;
    else
      setError("int or float expected for ..");
  }
}

static void
parseSetExpr(Ident *idP)
{
  Ident id2;

  parseRangeExpr(idP);
  if (Sym == ES_COMMA) {
    if (idP->resType == ER_INT) {
      addOpCode(OC_DUP);
      incStack();
      idP->resType = ER_INT_SET;
    } else if (idP->resType == ER_FLOAT) {
      addOpCode(OC_DUP);
      incStack();
      idP->resType = ER_FLOAT_SET;
    } else if (idP->resType != ER_INT_SET && idP->resType != ER_FLOAT_SET) {
      setError("int or float expected for ,");
    }
  }

  idP->rangeNo = 1;

  while (Sym == ES_COMMA) {
    idP->rangeNo++;

    getSym();
    parseRangeExpr(&id2);
    if (idP->resType == ER_INT_SET) {
      if (id2.resType == ER_INT) {
	addOpCode(OC_DUP);
	incStack();
      } else if (idP->resType != ER_INT_SET) {
	setError("int expected for ,");
      }
    } else {
      if (id2.resType == ER_INT) {
	addOpCode(OC_CAST);
	addOpCode(OC_DUP);
	incStack();
      } else if (id2.resType == ER_FLOAT) {
	addOpCode(OC_DUP);
	incStack();
      } else if (id2.resType == ER_INT_SET) {
	addOpCode(OC_CAST_1);
	addOpCode(OC_CAST);
      } else if (id2.resType != ER_INT_SET) {
	setError("int or float expected for ,");
      }
    }
  }
}

static void
parseRelExpr(Ident *idP)
{
  ExprSym opSym;
  Ident id2;
  int i;

  parseSimpleExpr(idP);
  if (Sym == ES_EQU || Sym == ES_NEQ) {
    opSym = Sym;
    getSym();
    parseSetExpr(&id2);
    if (id2.resType == ER_INT_SET) {
      if (idP->resType == ER_INT) {
	if (opSym == ES_EQU)
	  addOpCode(OC_INSET_INT);
	else
	  addOpCode(OC_OUTSET_INT);
	addBytes(&id2.rangeNo, sizeof(id2.rangeNo));
      } else {
	setError("int expected for int set");
      }

      for (i = 0; i < 2 * id2.rangeNo; i++)
	decStack();
    } else if (id2.resType == ER_FLOAT_SET) {
      if (idP->resType == ER_FLOAT) {
	if (opSym == ES_EQU)
	  addOpCode(OC_INSET_FLOAT);
	else
	  addOpCode(OC_OUTSET_FLOAT);
	addBytes(&id2.rangeNo, sizeof(id2.rangeNo));
      } else {
	setError("float expected for float set");
      }

      for (i = 0; i < 2 * id2.rangeNo; i++)
	decStack();
    } else {
      typeCast(idP, &id2);

      if (idP->resType == ER_INT) {
	if (opSym == ES_EQU)
	  addOpCode(OC_EQU_INT);
	else
	  addOpCode(OC_NEQ_INT);
      } else if (idP->resType == ER_FLOAT) {
	if (opSym == ES_EQU)
	  addOpCode(OC_EQU_FLOAT);
	else
	  addOpCode(OC_NEQ_FLOAT);
      } else if (idP->resType == ER_STR) {
	if (opSym == ES_EQU)
	  addOpCode(OC_EQU_STR);
	else
	  addOpCode(OC_NEQ_STR);
      } else {
	if (opSym == ES_EQU)
	  setError("int, float or string expected for =");
	else
	  setError("int, float or string expected for !=");
      }

      decStack();
    }
    idP->resType = ER_BOOL;
  } else if (Sym == ES_LEQ) {
    getSym();
    parseSimpleExpr(&id2);
    typeCast(idP, &id2);
    if (idP->resType == ER_INT)
      addOpCode(OC_LEQ_INT);
    else if (idP->resType == ER_FLOAT)
      addOpCode(OC_LEQ_FLOAT);
    else
      setError("int or float expected for <=");
    idP->resType = ER_BOOL;
    decStack();
  } else if (Sym == ES_LSS) {
    getSym();
    parseSimpleExpr(&id2);
    typeCast(idP, &id2);
    if (idP->resType == ER_INT)
      addOpCode(OC_LSS_INT);
    else if (idP->resType == ER_FLOAT)
      addOpCode(OC_LSS_FLOAT);
    else
      setError("int or float expected for <");
    idP->resType = ER_BOOL;
    decStack();
  } else if (Sym == ES_GEQ) {
    getSym();
    parseSimpleExpr(&id2);
    typeCast(idP, &id2);
    if (idP->resType == ER_INT)
      addOpCode(OC_GEQ_INT);
    else if (idP->resType == ER_FLOAT)
      addOpCode(OC_GEQ_FLOAT);
    else
      setError("int or float expected for >=");
    idP->resType = ER_BOOL;
    decStack();
  } else if (Sym == ES_GTR) {
    getSym();
    parseSimpleExpr(&id2);
    typeCast(idP, &id2);
    if (idP->resType == ER_INT)
      addOpCode(OC_GTR_INT);
    else if (idP->resType == ER_FLOAT)
      addOpCode(OC_GTR_FLOAT);
    else
      setError("int or float expected for >");
    idP->resType = ER_BOOL;
    decStack();
  }
}

static void
parseAndExpr(Ident *idP)
{
  Ident id2;

  parseRelExpr(idP);
  while (Sym == ES_AND) {
    getSym();
    parseRelExpr(&id2);
    if (idP->resType != ER_BOOL || id2.resType != ER_BOOL)
      setError("bool expected for &");
    addOpCode(OC_AND);
    decStack();
  }
}

static void
parseExpr(Ident *idP)
{
  Ident id2;

  parseAndExpr(idP);
  while (Sym == ES_OR) {
    getSym();
    parseAndExpr(&id2);
    if (idP->resType != ER_BOOL || id2.resType != ER_BOOL)
      setError("bool expected for |");
    addOpCode(OC_OR);
    decStack();
  }
}

ExprP
ExprCompile(DataEntityType entType, char *exprStr, ExprResType *resTypeP)
{
  Ident id;

  CurrExprP = malloc(sizeof(*CurrExprP));
  CurrExprP->sp = 0;
  CurrExprP->maxStackSize = 0;
  CurrExprP->codeSize = INIT_CODE_SIZE;
  CurrExprP->code = malloc(CurrExprP->codeSize * sizeof(OpCode));
  CurrExprP->pcEnd = 0;
  CurrExprP->entType = entType;

  ErrorMsg = NULL;

  ExprScanStart(exprStr);
  getSym();
  parseExpr(&id);
  addOpCode(OC_END);
  if (Sym != ES_END)
    setError("end expected");

  if (ErrorMsg != NULL) {
    ExprFree(CurrExprP);
    return NULL;
  }

  CurrExprP->resType = id.resType;
  CurrExprP->codeSize = CurrExprP->pcEnd;
  CurrExprP->code = realloc(CurrExprP->code,
      CurrExprP->codeSize * sizeof(OpCode));

  *resTypeP = id.resType;
  if (Stack == NULL) {
    Stack = malloc(CurrExprP->maxStackSize * sizeof(StackElem));
    StackSize = CurrExprP->maxStackSize;
  } else if (CurrExprP->maxStackSize > StackSize) {
    Stack = realloc(Stack, CurrExprP->maxStackSize * sizeof(StackElem));
    StackSize = CurrExprP->maxStackSize;
  }

  return CurrExprP;
}

char *
ExprGetErrorMsg(void)
{
  return ErrorMsg;
}

static void evalOneExpr(ExprP);

static void
predMol(DhMolP molP, void *clientData)
{
  ExprP exprP = clientData;

  if (exprP->predOp == OC_EXISTS) {
    if (exprP->predResult == TRUE)
      return;
  } else {
    if (exprP->predResult == FALSE)
      return;
  }

  exprP->molP = molP;
  evalOneExpr(exprP);
  exprP->predResult = Stack[exprP->sp + 1].b;
}

static void
predRes(DhResP resP, void *clientData)
{
  ExprP exprP = clientData;

  if (exprP->predOp == OC_EXISTS) {
    if (exprP->predResult == TRUE)
      return;
  } else {
    if (exprP->predResult == FALSE)
      return;
  }

  exprP->resP = resP;
  evalOneExpr(exprP);
  exprP->predResult = Stack[exprP->sp + 1].b;
}

static void
predAtom(DhAtomP atomP, void *clientData)
{
  ExprP exprP = clientData;

  if (exprP->predOp == OC_EXISTS) {
    if (exprP->predResult == TRUE)
      return;
  } else {
    if (exprP->predResult == FALSE)
      return;
  }

  exprP->atomP = atomP;
  exprP->resP = DhAtomGetRes(atomP);
  evalOneExpr(exprP);
  exprP->predResult = Stack[exprP->sp + 1].b;
}

static void
predBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  ExprP exprP = clientData;

  if (exprP->predOp == OC_EXISTS) {
    if (exprP->predResult == TRUE)
      return;
  } else {
    if (exprP->predResult == FALSE)
      return;
  }

  exprP->bondP = bondP;
  exprP->resP = DhBondGetRes(bondP);
  evalOneExpr(exprP);
  exprP->predResult = Stack[exprP->sp + 1].b;
}

static void
predAngle(DhAngleP angleP, void *clientData)
{
  ExprP exprP = clientData;

  if (exprP->predOp == OC_EXISTS) {
    if (exprP->predResult == TRUE)
      return;
  } else {
    if (exprP->predResult == FALSE)
      return;
  }

  exprP->angleP = angleP;
  exprP->resP = DhAngleGetRes(angleP);
  evalOneExpr(exprP);
  exprP->predResult = Stack[exprP->sp + 1].b;
}

static void
predDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  ExprP exprP = clientData;

  if (exprP->predOp == OC_EXISTS) {
    if (exprP->predResult == TRUE)
      return;
  } else {
    if (exprP->predResult == FALSE)
      return;
  }

  exprP->distP = distP;
  evalOneExpr(exprP);
  exprP->predResult = Stack[exprP->sp + 1].b;
}

static void
predPrim(PrimObjP primP, void *clientData)
{
  ExprP exprP = clientData;

  if (exprP->predOp == OC_EXISTS) {
    if (exprP->predResult == TRUE)
      return;
  } else {
    if (exprP->predResult == FALSE)
      return;
  }

  exprP->primP = primP;
  evalOneExpr(exprP);
  exprP->predResult = Stack[exprP->sp + 1].b;
}

static void
evalOneExpr(ExprP exprP)
{
  OpCode *code = exprP->code;
  int pc = exprP->pcStart;
  int sp = exprP->sp;
  DataEntityType entType = exprP->entType;
  int iVal;
  float fVal;
  int rangeNo, i;
  BOOL setRes;
  PropRefP refP;
  struct ExprS exprS;
  DhAtomP oldDistAtomP;
  AttrP attrP;
  Vec3 coord1, coord2;

  while (code[pc] != OC_END) {
    switch (code[pc++]) {
      case OC_PLUS_INT:
	sp--;
	Stack[sp].i = Stack[sp].i + Stack[sp + 1].i;
	break;
      case OC_MINUS_INT:
	sp--;
	Stack[sp].i = Stack[sp].i - Stack[sp + 1].i;
	break;
      case OC_MUL_INT:
	sp--;
	Stack[sp].i = Stack[sp].i * Stack[sp + 1].i;
	break;
      case OC_DIV_INT:
	sp--;
	Stack[sp].i = Stack[sp].i / Stack[sp + 1].i;
	break;
      case OC_REM:
	sp--;
	Stack[sp].i = Stack[sp].i % Stack[sp + 1].i;
	break;
      case OC_NEG_INT:
	Stack[sp].i = - Stack[sp].i;
	break;
      case OC_PLUS_FLOAT:
	sp--;
	Stack[sp].f = Stack[sp].f + Stack[sp + 1].f;
	break;
      case OC_MINUS_FLOAT:
	sp--;
	Stack[sp].f = Stack[sp].f - Stack[sp + 1].f;
	break;
      case OC_MUL_FLOAT:
	sp--;
	Stack[sp].f = Stack[sp].f * Stack[sp + 1].f;
	break;
      case OC_DIV_FLOAT:
	sp--;
	Stack[sp].f = Stack[sp].f / Stack[sp + 1].f;
	break;
      case OC_NEG_FLOAT:
	Stack[sp].f = - Stack[sp].f;
	break;
      case OC_SQRT:
	Stack[sp].f = sqrtf(Stack[sp].f);
	break;
      case OC_LOG:
	Stack[sp].f = logf(Stack[sp].f);
	break;
      case OC_DUP:
	sp++;
	Stack[sp] = Stack[sp - 1];
	break;
      case OC_CAST:
	Stack[sp].f = (float) Stack[sp].i;
	break;
      case OC_CAST_1:
	Stack[sp - 1].f = (float) Stack[sp - 1].i;
	break;
      case OC_EQU_INT:
	sp--;
	Stack[sp].b = (Stack[sp].i == Stack[sp + 1].i);
	break;
      case OC_NEQ_INT:
	sp--;
	Stack[sp].b = (Stack[sp].i != Stack[sp + 1].i);
	break;
      case OC_LEQ_INT:
	sp--;
	Stack[sp].b = (Stack[sp].i <= Stack[sp + 1].i);
	break;
      case OC_LSS_INT:
	sp--;
	Stack[sp].b = (Stack[sp].i < Stack[sp + 1].i);
	break;
      case OC_GEQ_INT:
	sp--;
	Stack[sp].b = (Stack[sp].i >= Stack[sp + 1].i);
	break;
      case OC_GTR_INT:
	sp--;
	Stack[sp].b = (Stack[sp].i > Stack[sp + 1].i);
	break;
      case OC_INSET_INT:
	rangeNo = * (int *) (code + pc);
	pc += OP_CODE_NO(sizeof(int));
	iVal = Stack[sp - 2 * rangeNo].i;
	setRes = FALSE;
	for (i = 0; i < rangeNo; i++) {
	  if (iVal >= Stack[sp - 1].i && iVal <= Stack[sp].i)
	    setRes = TRUE;
	  sp -= 2;
	}
	Stack[sp].b = setRes;
	break;
      case OC_OUTSET_INT:
	rangeNo = * (int *) (code + pc);
	pc += OP_CODE_NO(sizeof(int));
	iVal = Stack[sp - 2 * rangeNo].i;
	setRes = TRUE;
	for (i = 0; i < rangeNo; i++) {
	  if (iVal >= Stack[sp - 1].i && iVal <= Stack[sp].i)
	    setRes = FALSE;
	  sp -= 2;
	}
	Stack[sp].b = setRes;
	break;
      case OC_EQU_FLOAT:
	sp--;
	Stack[sp].b = (Stack[sp].f == Stack[sp + 1].f);
	break;
      case OC_NEQ_FLOAT:
	sp--;
	Stack[sp].b = (Stack[sp].f != Stack[sp + 1].f);
	break;
      case OC_LEQ_FLOAT:
	sp--;
	Stack[sp].b = (Stack[sp].f <= Stack[sp + 1].f);
	break;
      case OC_LSS_FLOAT:
	sp--;
	Stack[sp].b = (Stack[sp].f < Stack[sp + 1].f);
	break;
      case OC_GEQ_FLOAT:
	sp--;
	Stack[sp].b = (Stack[sp].f >= Stack[sp + 1].f);
	break;
      case OC_GTR_FLOAT:
	sp--;
	Stack[sp].b = (Stack[sp].f > Stack[sp + 1].f);
	break;
      case OC_INSET_FLOAT:
	rangeNo = * (int *) (code + pc);
	pc += OP_CODE_NO(sizeof(int));
	fVal = Stack[sp - 2 * rangeNo].f;
	setRes = FALSE;
	for (i = 0; i < rangeNo; i++) {
	  if (fVal >= Stack[sp - 1].f && fVal <= Stack[sp].f)
	    setRes = TRUE;
	  sp -= 2;
	}
	Stack[sp].b = setRes;
	break;
      case OC_OUTSET_FLOAT:
	rangeNo = * (int *) (code + pc);
	pc += OP_CODE_NO(sizeof(int));
	fVal = Stack[sp - 2 * rangeNo].f;
	setRes = TRUE;
	for (i = 0; i < rangeNo; i++) {
	  if (fVal >= Stack[sp - 1].f && fVal <= Stack[sp].f)
	    setRes = FALSE;
	  sp -= 2;
	}
	Stack[sp].b = setRes;
	break;
      case OC_EQU_STR:
	sp--;
	Stack[sp].b = StrMatch(Stack[sp].s, Stack[sp + 1].s);
	break;
      case OC_NEQ_STR:
	sp--;
	Stack[sp].b = ! StrMatch(Stack[sp].s, Stack[sp + 1].s);
	break;
      case OC_OR:
	sp--;
	Stack[sp].b = Stack[sp].b || Stack[sp + 1].b;
	break;
      case OC_AND:
	sp--;
	Stack[sp].b = Stack[sp].b && Stack[sp + 1].b;
	break;
      case OC_NOT:
	Stack[sp].b = ! Stack[sp].b;
	break;
      case OC_GET_INT:
	sp++;
	Stack[sp].i = * (int *) (code + pc);
	pc += OP_CODE_NO(sizeof(int));
	break;
      case OC_GET_FLOAT:
	sp++;
	Stack[sp].f = * (float *) (code + pc);
	pc += OP_CODE_NO(sizeof(float));
	break;
      case OC_GET_STR:
	sp++;
	Stack[sp].s = (char *) (code + pc);
	pc += OP_CODE_NO(strlen(Stack[sp].s) + 1);
	break;
      case OC_GET_PROP:
	sp++;
	memcpy(&refP, code + pc, sizeof(refP));
	pc += OP_CODE_NO(sizeof(PropRefP));
	if (entType == DE_MOL)
	  if (exprP->molP == NULL)
	    Stack[sp].b = FALSE;
	  else
	    Stack[sp].b = DhMolGetProp(refP, exprP->molP);
	else if (entType == DE_RES)
	  Stack[sp].b = DhResGetProp(refP, exprP->resP);
	else if (entType == DE_ATOM)
	  Stack[sp].b = DhAtomGetProp(refP, exprP->atomP);
	else if (entType == DE_BOND)
	  Stack[sp].b = DhBondGetProp(refP, exprP->bondP);
	else if (entType == DE_ANGLE)
	  Stack[sp].b = DhAngleGetProp(refP, exprP->angleP);
	else if (entType == DE_DIST)
	  Stack[sp].b = DhDistGetProp(refP, exprP->distP);
	else
	  Stack[sp].b = PrimGetProp(refP, exprP->primP);
	entType = exprP->entType;
	break;
      case OC_EXISTS:
      case OC_ALL:
	oldDistAtomP = DistAtomP;
	DistAtomP = exprP->atomP;
	exprS = *exprP;
	exprS.pcStart = pc;
	exprS.pcEnd = -1;
	exprS.sp = sp;
	exprS.entType = entType;
	exprS.predOp = code[pc - 1];
	if (exprS.predOp == OC_EXISTS)
	  exprS.predResult = FALSE;
	else
	  exprS.predResult = TRUE;
	sp++;
	refP = PropGetRef(PROP_ALL, FALSE);
	if (entType == DE_MOL)
	  DhApplyMol(refP, predMol, &exprS);
	else if (entType == DE_RES)
	  DhMolApplyRes(refP, exprS.molP, predRes, &exprS);
	else if (entType == DE_ATOM)
	  if (exprP->entType == DE_MOL)
	    if (exprS.molP == NULL)
	      DhApplyAtom(refP, predAtom, &exprS);
	    else
	      DhMolApplyAtom(refP, exprS.molP, predAtom, &exprS);
	  else
	    DhResApplyAtom(refP, exprS.resP, predAtom, &exprS);
	else if (entType == DE_BOND)
	  if (exprP->entType == DE_MOL)
	    if (exprS.molP == NULL)
	      DhApplyBond(refP, predBond, &exprS);
	    else
	      DhMolApplyBond(refP, exprS.molP, predBond, &exprS);
	  else
	    DhResApplyBond(refP, exprS.resP, predBond, &exprS);
	else if (entType == DE_ANGLE)
	  if (exprP->entType == DE_MOL)
	    if (exprS.molP == NULL)
	      DhApplyAngle(refP, predAngle, &exprS);
	    else
	      DhMolApplyAngle(refP, exprS.molP, predAngle, &exprS);
	  else
	    DhResApplyAngle(refP, exprS.resP, predAngle, &exprS);
	else if (entType == DE_DIST)
	  DhMolApplyDist(refP, exprS.molP, predDist, &exprS);
	else
	  PrimApply(PT_ALL, refP, predPrim, &exprS);
	DistAtomP = oldDistAtomP;
	if (exprS.pcEnd >= 0)
	  pc = exprS.pcEnd + 1;
	else
	  while (code[pc++] != OC_END);
	break;
      case OC_ENT_MOL:
	entType = DE_MOL;
	break;
      case OC_ENT_MOL1:
	entType = DE_MOL;
	if (exprP->entType == DE_BOND)
	  exprP->molP =
	      DhResGetMol(DhAtomGetRes(DhBondGetAtom1(exprP->bondP)));
	else
	  exprP->molP =
	      DhResGetMol(DhAtomGetRes(DhDistGetAtom1(exprP->distP)));
	break;
      case OC_ENT_MOL2:
	entType = DE_MOL;
	if (exprP->entType == DE_BOND)
	  exprP->molP =
	      DhResGetMol(DhAtomGetRes(DhBondGetAtom2(exprP->bondP)));
	else
	  exprP->molP =
	      DhResGetMol(DhAtomGetRes(DhDistGetAtom2(exprP->distP)));
	break;
      case OC_ENT_RES:
	entType = DE_RES;
	break;
      case OC_ENT_RES1:
	entType = DE_RES;
	if (exprP->entType == DE_BOND)
	  exprP->resP = DhAtomGetRes(DhBondGetAtom1(exprP->bondP));
	else
	  exprP->resP = DhAtomGetRes(DhDistGetAtom1(exprP->distP));
	break;
      case OC_ENT_RES2:
	entType = DE_RES;
	if (exprP->entType == DE_BOND)
	  exprP->resP = DhAtomGetRes(DhBondGetAtom2(exprP->bondP));
	else
	  exprP->resP = DhAtomGetRes(DhDistGetAtom2(exprP->distP));
	break;
      case OC_ENT_ATOM:
	entType = DE_ATOM;
	break;
      case OC_ENT_ATOM1:
	entType = DE_ATOM;
	if (exprP->entType == DE_BOND)
	  exprP->atomP = DhBondGetAtom1(exprP->bondP);
	else if (exprP->entType == DE_ANGLE)
	  exprP->atomP = DhAngleGetAtom1(exprP->angleP);
	else
	  exprP->atomP = DhDistGetAtom1(exprP->distP);
	break;
      case OC_ENT_ATOM2:
	entType = DE_ATOM;
	if (exprP->entType == DE_BOND)
	  exprP->atomP = DhBondGetAtom2(exprP->bondP);
	else if (exprP->entType == DE_ANGLE)
	  exprP->atomP = DhAngleGetAtom2(exprP->angleP);
	else
	  exprP->atomP = DhDistGetAtom2(exprP->distP);
	break;
      case OC_ENT_ATOM3:
	entType = DE_ATOM;
        exprP->atomP = DhAngleGetAtom3(exprP->angleP);
	break;
      case OC_ENT_ATOM4:
	entType = DE_ATOM;
        exprP->atomP = DhAngleGetAtom4(exprP->angleP);
	break;
      case OC_ENT_BOND:
	entType = DE_BOND;
	break;
      case OC_ENT_ANGLE:
	entType = DE_ANGLE;
	break;
      case OC_ENT_DIST:
	entType = DE_DIST;
	break;
      case OC_ENT_PRIM:
	entType = DE_PRIM;
	break;
      case OC_GET_UPL:
	sp++;
	Stack[sp].b = (DhDistGetKind(exprP->distP) == DK_UPPER);
	break;
      case OC_GET_LOL:
	sp++;
	Stack[sp].b = (DhDistGetKind(exprP->distP) == DK_LOWER);
	break;
      case OC_GET_HBOND:
	sp++;
	Stack[sp].b = (DhDistGetKind(exprP->distP) == DK_HBOND);
	break;
      case OC_GET_NUMBER:
	sp++;
	if (entType == DE_MOL)
	  if (exprP->molP == NULL)
	    Stack[sp].i = 0;
	  else
	    /* molecule numbers start at 0, add 1 for user */
	    Stack[sp].i = DhMolGetNumber(exprP->molP) + 1;
	else if (entType == DE_RES)
	  Stack[sp].i = DhResGetNumber(exprP->resP);
	else
	  Stack[sp].i = PrimGetNumber(exprP->primP);
	entType = exprP->entType;
	break;
      case OC_GET_NAME:
	sp++;
	if (entType == DE_MOL)
	  if (exprP->molP == NULL)
	    Stack[sp].s = "";
	  else
	    Stack[sp].s = DStrToStr(DhMolGetName(exprP->molP));
	else if (entType == DE_RES)
	  Stack[sp].s = DStrToStr(DhResGetName(exprP->resP));
	else if (entType == DE_ATOM)
	  Stack[sp].s = DStrToStr(DhAtomGetName(exprP->atomP));
	else
	  Stack[sp].s = DStrToStr(DhAngleGetName(exprP->angleP));
	entType = exprP->entType;
	break;
      case OC_GET_SHIFT:
	sp++;
	Stack[sp].f = DhAtomGetShift(exprP->atomP);
	entType = exprP->entType;
	break;
      case OC_GET_BFACTOR:
	sp++;
	Stack[sp].f = DhAtomGetBFactor(exprP->atomP);
	entType = exprP->entType;
	break;
      case OC_GET_VDW:
	sp++;
	Stack[sp].f = DhAtomGetVdw(exprP->atomP);
	entType = exprP->entType;
	break;
      case OC_GET_CHARGE:
	sp++;
	Stack[sp].f = DhAtomGetCharge(exprP->atomP, CK_CHARGE);
	entType = exprP->entType;
	break;
      case OC_GET_HEAVYCHARGE:
	sp++;
	Stack[sp].f = DhAtomGetCharge(exprP->atomP, CK_HEAVY);
	entType = exprP->entType;
	break;
      case OC_GET_AVGCHARGE:
	sp++;
	Stack[sp].f = DhAtomGetCharge(exprP->atomP, CK_AVG);
	entType = exprP->entType;
	break;
      case OC_GET_SIMPLECHARGE:
	sp++;
	Stack[sp].f = DhAtomGetCharge(exprP->atomP, CK_SIMPLE);
	entType = exprP->entType;
	break;
      case OC_GET_LEN:
	sp++;
	DhAtomGetCoordTransf(DhBondGetAtom1(exprP->bondP), coord1);
	DhAtomGetCoordTransf(DhBondGetAtom2(exprP->bondP), coord2);
	Stack[sp].f = Vec3DiffAbs(coord1, coord2);
	entType = exprP->entType;
	break;
      case OC_GET_VAL:
	sp++;
	if (entType == DE_ANGLE)
	  Stack[sp].f = DhAngleGetVal(exprP->angleP);
	else
	  Stack[sp].f = DhDistGetVal(exprP->distP);
	entType = exprP->entType;
	break;
      case OC_GET_LIMIT:
	sp++;
	Stack[sp].f = DhDistGetLimit(exprP->distP);
	entType = exprP->entType;
	break;
      case OC_GET_VIOL:
	sp++;
	Stack[sp].f = DhDistGetViol(exprP->distP);
	entType = exprP->entType;
	break;
      case OC_GET_ATTR:
	sp++;
	if (entType == DE_ATOM)
	  attrP = DhAtomGetAttr(exprP->atomP);
	else if (entType == DE_BOND)
	  attrP = DhBondGetAttr(exprP->bondP);
	else if (entType == DE_DIST)
	  attrP = DhDistGetAttr(exprP->distP);
	else
	  attrP = PrimGetAttr(exprP->primP);
	Stack[sp].i = AttrGetIndex(attrP);
	entType = exprP->entType;
	break;
      case OC_GET_D:
	sp++;
	if (DistAtomP == NULL) {
	  DhAtomGetCoordTransf(exprP->atomP, coord2);
	  Stack[sp].f = RefGetDist(coord2);
	} else {
	  DhAtomGetCoord(DistAtomP, coord1);
	  DhAtomGetCoord(exprP->atomP, coord2);
	  Stack[sp].f = Vec3DiffAbs(coord1, coord2);
	}
	entType = exprP->entType;
	break;
    }  /* switch */
  }  /* for */

  exprP->pcEnd = pc;
}

void
ExprEval(void *entP, ExprP exprP, ExprRes *resultP)
{
  DistAtomP = NULL;

  exprP->atomP = NULL;  /* may be assigned to DistAtomP */

  if (exprP->entType == DE_MOL) {
    exprP->molP = entP;
  } else if (exprP->entType == DE_RES) {
    exprP->resP = entP;
    exprP->molP = DhResGetMol(exprP->resP);
  } else if (exprP->entType == DE_ATOM) {
    exprP->atomP = entP;
    exprP->resP = DhAtomGetRes(exprP->atomP);
    exprP->molP = DhResGetMol(exprP->resP);
  } else if (exprP->entType == DE_BOND) {
    exprP->bondP = entP;
    exprP->resP = DhBondGetRes(exprP->bondP);
    exprP->molP = DhResGetMol(exprP->resP);
  } else if (exprP->entType == DE_ANGLE) {
    exprP->angleP = entP;
    exprP->resP = DhAngleGetRes(exprP->angleP);
    exprP->molP = DhResGetMol(exprP->resP);
  } else if (exprP->entType == DE_DIST) {
    exprP->distP = entP;
    exprP->molP = DhDistGetMol(exprP->distP);
  } else if (exprP->entType == DE_PRIM) {
    exprP->primP = entP;
    exprP->molP = PrimGetMol(exprP->primP);
  }

  exprP->pcStart = 0;
  exprP->sp = -1;
  evalOneExpr(exprP);

  resultP->resType = exprP->resType;

  if (exprP->resType == ER_BOOL)
    resultP->u.boolVal = Stack[0].b;
  else if (exprP->resType == ER_INT)
    resultP->u.intVal = Stack[0].i;
  else if (exprP->resType == ER_FLOAT)
    resultP->u.floatVal = Stack[0].f;
}

void
ExprFree(ExprP exprP)
{
  free(exprP->code);
  free(exprP);
}
