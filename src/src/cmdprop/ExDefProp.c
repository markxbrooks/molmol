/*
************************************************************************
*
*   ExDefProp.c - DefProp* commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/cmdprop/SCCS/s.ExDefProp.c
*   SCCS identification       : 1.15
*
************************************************************************
*/

#include <cmd_prop.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <pu.h>
#include <arg.h>
#include <expr.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <graph_draw.h>

typedef struct {
  ExprP exprP;
  BOOL val;
  PropRefP refP;
  int no;
} EvalData;

static DataEntityType
getEntType(char *name)
{
  if (strcmp(name, "Mol") == 0)
    return DE_MOL;
  else if (strcmp(name, "Res") == 0)
    return DE_RES;
  else if (strcmp(name, "Atom") == 0)
    return DE_ATOM;
  else if (strcmp(name, "Bond") == 0)
    return DE_BOND;
  else if (strcmp(name, "Angle") == 0)
    return DE_ANGLE;
  else if (strcmp(name, "Dist") == 0)
    return DE_DIST;
  else
    return DE_PRIM;
}

static char *
getEntName(DataEntityType entType)
{
  switch(entType) {
    case DE_MOL:
      return "molecule";
    case DE_RES:
      return "residue";
    case DE_ATOM:
      return "atom";
    case DE_BOND:
      return "bond";
    case DE_ANGLE:
      return "angle";
    case DE_DIST:
      return "distance";
    default:
      return "primitive";
  }
}

static ExprP
getExpr(DataEntityType entType, char *str, ExprResType *resTypeP)
{
  ExprP exprP;
  DSTR fullStr, partStr, valStr;
  BOOL prevShort, inStr, isNum;

  fullStr = DStrNew();
  partStr = DStrNew();
  valStr = DStrNew();

  prevShort = FALSE;
  inStr = FALSE;

  while (*str != '\0') {
    if (*str == '#' && ! inStr) {
      str++;
      DStrAppChar(partStr, '(');

      for (;;) {
	DStrAssignStr(valStr, "");
	isNum = TRUE;

	while (strchr(":@, )", *str) == NULL) {
	  if (isNum && *str == '-') {
	    DStrAppStr(valStr, "..");
	  } else {
	    if (! isdigit(*str))
	      isNum = FALSE;
	    DStrAppChar(valStr, *str);
	  }
	  str++;
	}

	if (isNum) {
	  DStrAppStr(partStr, "mol.num = ");
	  DStrAppDStr(partStr, valStr);
	} else {
	  DStrAppStr(partStr, "mol.name = ");
	  DStrAppChar(partStr, '"');
	  DStrAppDStr(partStr, valStr);
	  DStrAppChar(partStr, '"');
	}

	if (*str == ',') {
	  DStrAppStr(partStr, " || ");
	  str++;
	} else {
	  break;
	}
      }

      DStrAppChar(partStr, ')');
    }

    if (*str == ':' && ! inStr) {
      if (DStrLen(partStr) > 0)
	DStrAppStr(partStr, " && ");

      str++;
      DStrAppChar(partStr, '(');

      for (;;) {
	DStrAssignStr(valStr, "");
	isNum = TRUE;

	while (strchr("@, )", *str) == NULL) {
	  if (isNum && *str == '-') {
	    DStrAppStr(valStr, "..");
	  } else {
	    if (! isdigit(*str))
	      isNum = FALSE;
	    DStrAppChar(valStr, *str);
	  }
	  str++;
	}

	if (isNum) {
	  DStrAppStr(partStr, "res.num = ");
	  DStrAppDStr(partStr, valStr);
	} else {
	  DStrAppStr(partStr, "res.name = ");
	  DStrAppChar(partStr, '"');
	  DStrAppDStr(partStr, valStr);
	  DStrAppChar(partStr, '"');
	}

	if (*str == ',') {
	  DStrAppStr(partStr, " || ");
	  str++;
	} else {
	  break;
	}
      }

      DStrAppChar(partStr, ')');
    }

    if (*str == '@' && ! inStr) {
      if (DStrLen(partStr) > 0)
	DStrAppStr(partStr, " && ");

      str++;
      DStrAppChar(partStr, '(');

      for (;;) {
	if (entType == DE_BOND)
	  DStrAppStr(partStr, "atom2.name = ");
	else
	  DStrAppStr(partStr, "name = ");
	DStrAppChar(partStr, '"');
	while (strchr(", )", *str) == NULL) {
	  DStrAppChar(partStr, *str);
	  str++;
	}
	DStrAppChar(partStr, '"');

	if (*str == ',') {
	  DStrAppStr(partStr, " || ");
	  str++;
	} else {
	  break;
	}
      }

      DStrAppChar(partStr, ')');
    }

    if (DStrLen(partStr) > 0) {
      if (prevShort)
	DStrAppStr(fullStr, " || ");
      else
	DStrAppStr(fullStr, " (");

      DStrAppDStr(fullStr, partStr);
      DStrAssignStr(partStr, "");

      prevShort = TRUE;

      while (*str == ' ')
	str++;
    } else {
      if (prevShort)
	DStrAppStr(fullStr, ") ");

      if (*str == '"')
	inStr = ! inStr;

      DStrAppChar(fullStr, *str);
      str++;

      prevShort = FALSE;
    }
  }

  if (prevShort)
    DStrAppStr(fullStr, ")");

  exprP = ExprCompile(entType, DStrToStr(fullStr), resTypeP);

  DStrFree(fullStr);
  DStrFree(partStr);
  DStrFree(valStr);

  return exprP;
}

static void
countNo(EvalData *evalP, BOOL val)
{
  if (val)
    evalP->no++;
}

static void
evalMol(DhMolP molP, void *clientData)
{
  EvalData *evalP = clientData;
  ExprRes exprRes;

  if (evalP->exprP == NULL) {
    DhMolSetProp(evalP->refP, molP, evalP->val);
    countNo(evalP, evalP->val);
  } else {
    ExprEval(molP, evalP->exprP, &exprRes);
    DhMolSetProp(evalP->refP, molP, exprRes.u.boolVal);
    countNo(evalP, exprRes.u.boolVal);
  }
}

static void
evalRes(DhResP resP, void *clientData)
{
  EvalData *evalP = clientData;
  ExprRes exprRes;

  if (evalP->exprP == NULL) {
    DhResSetProp(evalP->refP, resP, evalP->val);
    countNo(evalP, evalP->val);
  } else {
    ExprEval(resP, evalP->exprP, &exprRes);
    DhResSetProp(evalP->refP, resP, exprRes.u.boolVal);
    countNo(evalP, exprRes.u.boolVal);
  }
}

static void
evalAtom(DhAtomP atomP, void *clientData)
{
  EvalData *evalP = clientData;
  ExprRes exprRes;

  if (evalP->exprP == NULL) {
    DhAtomSetProp(evalP->refP, atomP, evalP->val);
    countNo(evalP, evalP->val);
  } else {
    ExprEval(atomP, evalP->exprP, &exprRes);
    DhAtomSetProp(evalP->refP, atomP, exprRes.u.boolVal);
    countNo(evalP, exprRes.u.boolVal);
  }
}

static void
evalBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  EvalData *evalP = clientData;
  ExprRes exprRes;

  if (evalP->exprP == NULL) {
    DhBondSetProp(evalP->refP, bondP, evalP->val);
    countNo(evalP, evalP->val);
  } else {
    ExprEval(bondP, evalP->exprP, &exprRes);
    DhBondSetProp(evalP->refP, bondP, exprRes.u.boolVal);
    countNo(evalP, exprRes.u.boolVal);
  }
}

static void
evalAngle(DhAngleP angleP, void *clientData)
{
  EvalData *evalP = clientData;
  ExprRes exprRes;

  if (evalP->exprP == NULL) {
    DhAngleSetProp(evalP->refP, angleP, evalP->val);
    countNo(evalP, evalP->val);
  } else {
    ExprEval(angleP, evalP->exprP, &exprRes);
    DhAngleSetProp(evalP->refP, angleP, exprRes.u.boolVal);
    countNo(evalP, exprRes.u.boolVal);
  }
}

static void
evalDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  EvalData *evalP = clientData;
  ExprRes exprRes;

  if (evalP->exprP == NULL) {
    DhDistSetProp(evalP->refP, distP, evalP->val);
    countNo(evalP, evalP->val);
  } else {
    ExprEval(distP, evalP->exprP, &exprRes);
    DhDistSetProp(evalP->refP, distP, exprRes.u.boolVal);
    countNo(evalP, exprRes.u.boolVal);
  }
}

static void
evalPrim(PrimObjP primP, void *clientData)
{
  EvalData *evalP = clientData;
  ExprRes exprRes;

  if (evalP->exprP == NULL) {
    PrimSetProp(evalP->refP, primP, evalP->val);
    countNo(evalP, evalP->val);
  } else {
    ExprEval(primP, evalP->exprP, &exprRes);
    PrimSetProp(evalP->refP, primP, exprRes.u.boolVal);
    countNo(evalP, exprRes.u.boolVal);
  }
}

static void
setProp(DataEntityType entType, char *prop, EvalData *evalP)
{
  PropRefP allRefP;
  char numBuf[20];
  DSTR statStr;

  allRefP = PropGetRef(PROP_ALL, FALSE);

  evalP->refP = PropGetRef(prop, TRUE);
  evalP->no = 0;

  switch (entType) {
    case DE_MOL:
      DhApplyMol(allRefP, evalMol, evalP);
      break;
    case DE_RES:
      DhApplyRes(allRefP, evalRes, evalP);
      break;
    case DE_ATOM:
      DhApplyAtom(allRefP, evalAtom, evalP);
      break;
    case DE_BOND:
      DhApplyBond(allRefP, evalBond, evalP);
      break;
    case DE_ANGLE:
      DhApplyAngle(allRefP, evalAngle, evalP);
      break;
    case DE_DIST:
      DhApplyDist(allRefP, evalDist, evalP);
      break;
    case DE_PRIM:
      PrimApply(PT_ALL, allRefP, evalPrim, evalP);
      break;
  }

  statStr = DStrNew();
  (void) sprintf(numBuf, "%d", evalP->no);
  DStrAppStr(statStr, numBuf);
  DStrAppChar(statStr, ' ');
  DStrAppStr(statStr, getEntName(entType));
  if (evalP->no != 1)
    DStrAppChar(statStr, 's');
  DStrAppChar(statStr, ' ');
  DStrAppStr(statStr, prop);
  PuSetTextField(PU_TF_STATUS, DStrToStr(statStr));
  DStrFree(statStr);
}

#define ARG_NUM 2

ErrCode
ExDefProp(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  DataEntityType entType;
  char *exprStr;
  EvalData evalData;
  ExprResType resType;

  arg[0].type = AT_STR;
  arg[1].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Property";
  arg[1].prompt = "Definition";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  entType = getEntType(cmd + 7);
  exprStr = DStrToStr(arg[1].v.strVal);

  if (exprStr[0] == '\0') {
    evalData.exprP = NULL;
    evalData.val = TRUE;
  } else if (strcmp(exprStr, "0") == 0) {
    evalData.exprP = NULL;
    evalData.val = FALSE;
  } else {
    evalData.exprP = getExpr(entType, exprStr, &resType);
    if (evalData.exprP == NULL) {
      CipSetError(ExprGetErrorMsg());
      ArgCleanup(arg, ARG_NUM);
      return EC_ERROR;
    }
    if (resType != ER_BOOL) {
      CipSetError("expression must be boolean");
      ArgCleanup(arg, ARG_NUM);
      ExprFree(evalData.exprP);
      return EC_ERROR;
    }
  }

  setProp(entType, DStrToStr(arg[0].v.strVal), &evalData);

  if (strcmp(DStrToStr(arg[0].v.strVal), PROP_DISPLAYED) == 0) {
    GraphMolChanged(PROP_DISPLAYED);
    GraphRedrawNeeded();
  }

  ArgCleanup(arg, ARG_NUM);
  if (evalData.exprP != NULL)
    ExprFree(evalData.exprP);

  return EC_OK;
}

#undef ARG_NUM
#define ARG_NUM 1

ErrCode
ExUndefProp(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PropRefP refP;
  EvalData evalData;

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Property";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  refP = PropGetRef(PROP_ALL, FALSE);
  if (refP == NULL) {
    ArgCleanup(arg, ARG_NUM);
    return EC_OK;
  }

  evalData.exprP = NULL;
  evalData.val = FALSE;
  evalData.refP = PropGetRef(DStrToStr(arg[0].v.strVal), TRUE);

  ArgCleanup(arg, ARG_NUM);

  DhApplyMol(refP, evalMol, &evalData);
  DhApplyRes(refP, evalRes, &evalData);
  DhApplyAtom(refP, evalAtom, &evalData);
  DhApplyBond(refP, evalBond, &evalData);
  DhApplyAngle(refP, evalAngle, &evalData);
  DhApplyDist(refP, evalDist, &evalData);
  PrimApply(PT_ALL, refP, evalPrim, &evalData);

  return EC_OK;
}

#undef ARG_NUM
#define ARG_NUM 1

ErrCode
ExSelect(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  DataEntityType entType;
  char *exprStr;
  EvalData evalData;
  ExprResType resType;

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Expression";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  entType = getEntType(cmd + 6);
  exprStr = DStrToStr(arg[0].v.strVal);

  if (exprStr[0] == '\0') {
    evalData.exprP = NULL;
    evalData.val = TRUE;
  } else if (strcmp(exprStr, "0") == 0) {
    evalData.exprP = NULL;
    evalData.val = FALSE;
  } else {
    evalData.exprP = getExpr(entType, exprStr, &resType);
    if (evalData.exprP == NULL) {
      CipSetError(ExprGetErrorMsg());
      ArgCleanup(arg, ARG_NUM);
      return EC_ERROR;
    }
    if (resType != ER_BOOL) {
      CipSetError("expression must be boolean");
      ArgCleanup(arg, ARG_NUM);
      ExprFree(evalData.exprP);
      return EC_ERROR;
    }
  }

  setProp(entType, PROP_SELECTED, &evalData);
  ArgCleanup(arg, ARG_NUM);
  if (evalData.exprP != NULL)
    ExprFree(evalData.exprP);

  return EC_OK;
}
