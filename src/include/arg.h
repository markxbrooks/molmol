/*
************************************************************************
*
*   arg.h - argument input for commands
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/include/SCCS/s.arg.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#ifndef _ARG_H_
#define _ARG_H_

#include <bool.h>
#include <dstr.h>
#include <expr.h>
#include <cmd.h>

typedef enum {
  AT_INT,
  AT_DOUBLE,
  AT_STR,
  AT_ENUM,
  AT_MULT_ENUM,
  AT_LIST,
  AT_MULT_LIST
} ArgType;

typedef struct ArgDescrS *ArgDescrP;

typedef struct {
  ExprP exprP;
  int intVal;
  double doubleVal;
  DSTR strVal;
  BOOL boolVal;
} ArgVal;

typedef BOOL (* ArgVerifyFunc) (ArgDescrP);

typedef struct {
  char *str;
  BOOL onOff;
} EnumEntryDescr;

typedef struct {
  float minVal, maxVal;
  int digits;
} SliderDescr;

typedef struct {
  int n;
  int lineNo;
  EnumEntryDescr *entryP;
} EnumDescr;

typedef struct ArgDescrS {
  ArgType type;
  char *prompt;
  BOOL useSlider;
  BOOL optional;
  DataEntityType entType;
  ArgVerifyFunc verifyFunc;
  void *userData;
  union {
    SliderDescr sliderD;
    EnumDescr enumD;
  } u;
  ArgVal v;
} ArgDescr;

extern void ArgEnable(BOOL);

extern void ArgInit(ArgDescrP, int);

extern void ArgCleanup(ArgDescrP, int);

extern ErrCode ArgGet(ArgDescrP, int);

extern ErrCode ArgGetFilename(DSTR, char *, char *, BOOL);

#endif  /* _ARG_H_ */
