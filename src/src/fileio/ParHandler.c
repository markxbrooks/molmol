/*
************************************************************************
*
*   ParHandler.c - parameter handler
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
*   Date of last modification : 95/09/01
*   Pathname of SCCS file     : /sgiext/molmol/src/fileio/SCCS/s.ParHandler.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <par_hand.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tree.h>

#define PAR_MAX_NAME_LEN 32
#define PAR_MAX_VAL_LEN 4096

typedef enum {
  PT_INT,
  PT_DOUBLE,
  PT_STR,
  PT_INT_ARR,
  PT_DOUBLE_ARR,
  PT_STR_ARR
} ParType;

typedef struct {
  char *name;
  ParType type;
  int arrSize;
  union {
    int intVal;
    double doubleVal;
    char *strVal;
    int *intArr;
    double *doubleArr;
    char **strArr;
  } u;
} ParEntry;

static TREE ParTab = NULL;
static GFileRes CurrRes;

static void
raiseError(char *name, char *msg)
{
  (void) fprintf(stderr, "PARAMETER ERROR: %s - %s\n", name, msg);
}

static int
compFunc(void *p1, void *p2)
{
  ParEntry *entry1P = p1;
  ParEntry *entry2P = p2;

  return strcmp(entry1P->name, entry2P->name);
}

static ParEntry *
find(ParEntry *entryP)
{
  return TreeSearch(ParTab, entryP);
}

static ParEntry *
insert(ParEntry *entryP)
{
  if (ParTab == NULL)
    ParTab = TreeOpen(sizeof(ParEntry), compFunc);

  return TreeInsert(ParTab, entryP, TIB_RETURN_NULL);
}

static ParEntry *
getEntry(char *name, ParType entryType, BOOL createIt)
{
  ParEntry entry, *entryP;

  if (strlen(name) > PAR_MAX_NAME_LEN) {
    raiseError(name, "name too long");
    return NULL;
  }

  entry.name = name;
  entryP = find(&entry);
  if (entryP == NULL) {
    if (createIt) {
      entry.name = malloc(strlen(name) + 1);
      (void) strcpy(entry.name, name);
      entry.type = entryType;
      entry.arrSize = 0;

      if (entryType == PT_STR)
	entry.u.strVal = NULL;

      return insert(&entry);
    } else {
      raiseError(name, "not found");
      return NULL;
    }
  } else {
    if (entryP->type != entryType) {
      raiseError(name, "wrong type");
      return NULL;
    } else {
      return entryP;
    }
  }
}

BOOL
ParDefined(char *name)
{
  ParEntry entry;

  entry.name = name;
  return find(&entry) != NULL;
}

void
ParSetIntVal(char *name, int i)
{
  ParEntry *entryP = getEntry(name, PT_INT, TRUE);
  if (entryP != NULL)
    entryP->u.intVal = i;
}

void
ParSetDoubleVal(char *name, double d)
{
  ParEntry *entryP = getEntry(name, PT_DOUBLE, TRUE);
  if (entryP != NULL)
    entryP->u.doubleVal = d;
}

void
ParSetStrVal(char *name, char *s)
{
  ParEntry *entryP;

  if (strlen(s) > PAR_MAX_VAL_LEN) {
    raiseError(name, "value too long");
    return;
  }

  entryP = getEntry(name, PT_STR, TRUE);

  if (entryP != NULL) {
    if (entryP->u.strVal != NULL)
      entryP->u.strVal = realloc(entryP->u.strVal, strlen(s) + 1);
    else
      entryP->u.strVal = malloc(strlen(s) + 1);

    (void) strcpy(entryP->u.strVal, s);
  }
}

void
ParSetIntArrVal(char *name, int ind, int i)
{
  ParEntry *entryP = getEntry(name, PT_INT_ARR, TRUE);

  if (ind < 0) {
    raiseError(name, "index < 0");
    return;
  }

  if (entryP != NULL) {
    if (ind >= entryP->arrSize) {
      int newSize = (ind + 1) * sizeof(int);
      if (entryP->arrSize == 0)
	entryP->u.intArr = malloc(newSize);
      else
	entryP->u.intArr = realloc(entryP->u.intArr, newSize);
      if (entryP->u.intArr == NULL) {
	raiseError(name, "out of memory");
	return;
      }
      entryP->arrSize = ind + 1;
    }
    entryP->u.intArr[ind] = i;
  }
}

void
ParSetDoubleArrVal(char *name, int ind, double d)
{
  ParEntry *entryP = getEntry(name, PT_DOUBLE_ARR, TRUE);

  if (ind < 0) {
    raiseError(name, "index < 0");
    return;
  }

  if (entryP != NULL) {
    if (ind >= entryP->arrSize) {
      int newSize = (ind + 1) * sizeof(double);
      if (entryP->arrSize == 0)
	entryP->u.doubleArr = malloc(newSize);
      else
	entryP->u.doubleArr = realloc(entryP->u.doubleArr, newSize);
      if (entryP->u.doubleArr == NULL) {
	raiseError(name, "out of memory");
	return;
      }
      entryP->arrSize = ind + 1;
    }
    entryP->u.doubleArr[ind] = d;
  }
}

void
ParSetStrArrVal(char *name, int ind, char *s)
{
  ParEntry *entryP = getEntry(name, PT_STR_ARR, TRUE);
  int i;

  if (strlen(s) > PAR_MAX_VAL_LEN) {
    raiseError(name, "value too long");
    return;
  }

  if (ind < 0) {
    raiseError(name, "index < 0");
    return;
  }

  if (entryP != NULL) {
    if (ind >= entryP->arrSize) {
      int newSize = (ind + 1) * sizeof(char *);
      if (entryP->arrSize == 0)
	entryP->u.strArr = malloc(newSize);
      else
	entryP->u.strArr = realloc(entryP->u.strArr, newSize);
      if (entryP->u.strArr == NULL) {
	raiseError(name, "out of memory");
	return;
      }
      for (i = entryP->arrSize; i < ind; i++)
	entryP->u.strArr[i] = NULL;
      entryP->arrSize = ind + 1;
    }

    if (entryP->u.strArr[ind] != NULL)
      entryP->u.strArr[ind] = realloc(entryP->u.strArr[ind], strlen(s) + 1);
    else
      entryP->u.strArr[ind] = malloc(strlen(s) + 1);

    (void) strcpy(entryP->u.strArr[ind], s);
  }
}

void
ParUndefine(char *name)
{
  ParEntry entry, *entryP;
  int i;

  entry.name = name;
  entryP = find(&entry);
  if (entryP != NULL) {
    switch (entryP->type) {
      case PT_INT:
      case PT_DOUBLE:
	/* do nothing */
	break;
      case PT_STR:
	free(entryP->u.strVal);
	break;
      case PT_INT_ARR:
	if (entryP->arrSize > 0)
	  free(entryP->u.intArr);
	break;
      case PT_DOUBLE_ARR:
	if (entryP->arrSize > 0)
	  free(entryP->u.doubleArr);
	break;
      case PT_STR_ARR:
	if (entryP->arrSize > 0) {
	  for (i = 0; i < entryP->arrSize; i++)
	    if (entryP->u.strArr[i] != NULL)
	      free(entryP->u.strArr[i]);
	  free(entryP->u.strArr);
	}
	break;
    }
    TreeRemove(ParTab, entryP);
  }
}

int
ParGetIntVal(char *name)
{
  ParEntry *entryP = getEntry(name, PT_INT, FALSE);
  if (entryP != NULL)
    return entryP->u.intVal;
  else
    return 0;
}

double
ParGetDoubleVal(char *name)
{
  ParEntry *entryP = getEntry(name, PT_DOUBLE, FALSE);
  if (entryP != NULL)
    return entryP->u.doubleVal;
  else
    return 0.0;
}

char *
ParGetStrVal(char *name)
{
  ParEntry *entryP = getEntry(name, PT_STR, FALSE);
  if (entryP != NULL)
    return entryP->u.strVal;
  else
    return NULL;
}

int
ParGetIntArrVal(char *name, int ind)
{
  ParEntry *entryP = getEntry(name, PT_INT_ARR, FALSE);

  if (entryP == NULL)
    return 0;

  if (ind < 0 || ind >= entryP->arrSize) {
    raiseError(name, "index out of bounds");
    return 0;
  }

  return entryP->u.intArr[ind];
}

double
ParGetDoubleArrVal(char *name, int ind)
{
  ParEntry *entryP = getEntry(name, PT_DOUBLE_ARR, FALSE);

  if (entryP == NULL)
    return 0.0;

  if (ind < 0 || ind >= entryP->arrSize) {
    raiseError(name, "index out of bounds");
    return 0.0;
  }

  return entryP->u.doubleArr[ind];
}

char *
ParGetStrArrVal(char *name, int ind)
{
  ParEntry *entryP = getEntry(name, PT_STR_ARR, FALSE);

  if (entryP == NULL)
    return NULL;

  if (ind < 0 || ind >= entryP->arrSize) {
    raiseError(name, "index out of bounds");
    return NULL;
  }

  return entryP->u.strArr[ind];
}

int
ParGetArrSize(char *name)
{
  ParEntry entry, *entryP;

  entry.name = name;
  entryP = find(&entry);

  if (entryP == NULL) {
    raiseError(name, "not found");
    return 0;
  }

  if (entryP->type == PT_INT_ARR ||
      entryP->type == PT_DOUBLE_ARR ||
      entryP->type == PT_STR_ARR) {
    return entryP->arrSize;
  } else {
    raiseError(name, "not array type");
    return 0;
  }
}

#undef CHECK_RES
#define CHECK_RES if (CurrRes != GF_RES_OK) return

void
writePar(void *p, void *cd)
{
  ParEntry *entryP = p;
  GFile gf = cd;
  int i;

  CHECK_RES;

  CurrRes = GFileWriteStr(gf, entryP->name); CHECK_RES;

  switch (entryP->type) {
    case PT_INT:
      CurrRes = GFileWriteStr(gf, "i"); CHECK_RES;
      CurrRes = GFileWriteINT32(gf, entryP->u.intVal); CHECK_RES;
      break;
    case PT_DOUBLE:
      CurrRes = GFileWriteStr(gf, "d"); CHECK_RES;
      CurrRes = GFileWriteFLOAT64(gf, entryP->u.doubleVal); CHECK_RES;
      break;
    case PT_STR:
      CurrRes = GFileWriteStr(gf, "s"); CHECK_RES;
      CurrRes = GFileWriteQuotedStr(gf, entryP->u.strVal); CHECK_RES;
      break;
    case PT_INT_ARR:
      CurrRes = GFileWriteStr(gf, "ia"); CHECK_RES;
      CurrRes = GFileWriteINT32(gf, entryP->arrSize); CHECK_RES;
      for (i = 0; i < entryP->arrSize; i++) {
        CurrRes = GFileWriteNL(gf); CHECK_RES;
	CurrRes = GFileWriteINT32(gf, entryP->u.intArr[i]); CHECK_RES;
      }
      break;
    case PT_DOUBLE_ARR:
      CurrRes = GFileWriteStr(gf, "da"); CHECK_RES;
      CurrRes = GFileWriteINT32(gf, entryP->arrSize); CHECK_RES;
      for (i = 0; i < entryP->arrSize; i++) {
        CurrRes = GFileWriteNL(gf); CHECK_RES;
	CurrRes = GFileWriteFLOAT64(gf, entryP->u.doubleArr[i]); CHECK_RES;
      }
      break;
    case PT_STR_ARR:
      CurrRes = GFileWriteStr(gf, "sa"); CHECK_RES;
      CurrRes = GFileWriteINT32(gf, entryP->arrSize); CHECK_RES;
      for (i = 0; i < entryP->arrSize; i++) {
        CurrRes = GFileWriteNL(gf); CHECK_RES;
	CurrRes = GFileWriteQuotedStr(gf, entryP->u.strArr[i]); CHECK_RES;
      }
      break;
  }

  CurrRes = GFileWriteNL(gf); CHECK_RES;
}

#undef CHECK_RES
#define CHECK_RES if (res != GF_RES_OK) return res

GFileRes
ParDump(GFile gf)
{
  GFileRes res;

  res = GFileWriteINT32(gf, TreeSize(ParTab)); CHECK_RES;
  res = GFileWriteNL(gf); CHECK_RES;
  res = GFileWriteNL(gf); CHECK_RES;

  CurrRes = GF_RES_OK;
  TreeApply(ParTab, writePar, gf);
  if (CurrRes != GF_RES_OK)
    return CurrRes;

  res = GFileWriteNL(gf); CHECK_RES;

  return GF_RES_OK;
}

GFileRes
ParUndump(GFile gf)
{
  GFileRes res;
  int parNo, parIdx;
  char name[PAR_MAX_NAME_LEN + 1];
  char type[16];
  INT32 i32, size;
  FLOAT64 f64;
  char s[PAR_MAX_VAL_LEN + 1];
  int i;

  res = GFileReadINT32(gf, &i32); CHECK_RES;
  parNo = i32;

  for (parIdx = 0; parIdx < parNo; parIdx++) {
    res = GFileReadStr(gf, name, sizeof(name)); CHECK_RES;
    res = GFileReadStr(gf, type, sizeof(type)); CHECK_RES;
    if (strcmp(type, "i") == 0) {
      res = GFileReadINT32(gf, &i32); CHECK_RES;
      ParSetIntVal(name, i32);
    } else if (strcmp(type, "d") == 0) {
      res = GFileReadFLOAT64(gf, &f64); CHECK_RES;
      ParSetDoubleVal(name, f64);
    } else if (strcmp(type, "s") == 0) {
      res = GFileReadQuotedStr(gf, s, sizeof(s)); CHECK_RES;
      ParSetStrVal(name, s);
    } else if (strcmp(type, "ia") == 0) {
      res = GFileReadINT32(gf, &size); CHECK_RES;
      for (i = 0; i < size; i++) {
        res = GFileReadINT32(gf, &i32); CHECK_RES;
        ParSetIntArrVal(name, i, i32);
      }
      getEntry(name, PT_INT_ARR, FALSE)->arrSize = size;
    } else if (strcmp(type, "da") == 0) {
      res = GFileReadINT32(gf, &size); CHECK_RES;
      for (i = 0; i < size; i++) {
        res = GFileReadFLOAT64(gf, &f64); CHECK_RES;
        ParSetDoubleArrVal(name, i, f64);
      }
      getEntry(name, PT_DOUBLE_ARR, FALSE)->arrSize = size;
    } else if (strcmp(type, "sa") == 0) {
      res = GFileReadINT32(gf, &size); CHECK_RES;
      for (i = 0; i < size; i++) {
        res = GFileReadQuotedStr(gf, s, sizeof(s)); CHECK_RES;
        ParSetStrArrVal(name, i, s);
      }
      getEntry(name, PT_STR_ARR, FALSE)->arrSize = size;
    } else {
      raiseError(name, "unknown type");
      return GF_RES_ERR;
    }
  }
  return GF_RES_OK;
}
