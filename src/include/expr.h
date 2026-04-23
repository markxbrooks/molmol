/*
************************************************************************
*
*   expr.h - expression compilation and evaluation
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
*   Date of last modification : 94/09/20
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.expr.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _EXPR_H_
#define _EXPR_H_

#include <bool.h>
#include <data_ent.h>

typedef struct ExprS *ExprP;

typedef enum {
  ER_BOOL,
  ER_INT,
  ER_FLOAT,
  ER_INT_SET,
  ER_FLOAT_SET,
  ER_STR
} ExprResType;

typedef struct {
  ExprResType resType;
  union {
    BOOL boolVal;
    int intVal;
    float floatVal;
  } u;
} ExprRes;

extern ExprP ExprCompile(DataEntityType, char *, ExprResType *);

extern char *ExprGetErrorMsg(void);

extern void ExprEval(void *, ExprP, ExprRes *);

extern void ExprFree(ExprP);

#endif  /* _EXPR_H_ */
