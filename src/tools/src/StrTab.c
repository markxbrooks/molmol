/*
************************************************************************
*
*   StrTab.c - implementation of STRTAB type
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
*   Date of last modification : 99/10/30
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.StrTab.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <strtab.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hashtab.h>

typedef struct {
  char *str;
  int index;
  BOOL sticky;
  long timeStamp;
} TabEntry;

struct strtab {
  int size;
  int entryNo;
  HASHTABLE entryTab;
};

typedef struct {
  StrTabApplyFunc func;
  void *clientData;
} ApplyData;

typedef struct {
  int timeStamp;
  TabEntry *entryP;
} FindData;

static long TimeStamp = 0;

static void
destroyCB(void *p, void *clientData)
{
  TabEntry *entryP = p;

  free(entryP->str);
}

static unsigned
hashFunc(void *p, unsigned size)
{
  TabEntry *entryP = p;
  unsigned idx = 0;
  char *str;

  str = entryP->str;
  while (*str != '\0') {
    idx = 37 * idx + *str;
    str++;
  }

  return idx % size;
}

static int
compFunc(void *p1, void *p2)
{
  TabEntry *entry1P = p1, *entry2P = p2;

  return strcmp(entry1P->str, entry2P->str);
}

STRTAB
StrTabOpen(int size)
{
  STRTAB tab;

  tab = malloc(sizeof(*tab));
  tab->size = size;
  tab->entryNo = 0;
  tab->entryTab = HashtabOpen(137, sizeof(TabEntry), hashFunc, compFunc);
  HashtabAddDestroyCB(tab->entryTab, destroyCB, NULL, NULL);

  return tab;
}

void
doApply(void *p, void *clientData)
{
  TabEntry *entryP = p;
  ApplyData *dataP = clientData;

  dataP->func(entryP->index, entryP->str, entryP->sticky, dataP->clientData);
}

void
StrTabApply(STRTAB tab, StrTabApplyFunc applyF, void *clientData)
{
  ApplyData applyData;

  applyData.func = applyF;
  applyData.clientData = clientData;
  HashtabApply(tab->entryTab, doApply, &applyData);
}

static void
findLRU(void *p, void *clientData)
{
  TabEntry *entryP = p;
  FindData *dataP = clientData;

  if (entryP->timeStamp < dataP->timeStamp && ! entryP->sticky) {
    dataP->timeStamp = entryP->timeStamp;
    dataP->entryP = entryP;
  }
}

int
getIndex(STRTAB tab, char *str, BOOL insertIt, BOOL sticky)
{
  TabEntry entry, *entryP;
  int len;
  FindData findData;

  entry.str = str;
  entryP = HashtabSearch(tab->entryTab, &entry);
  if (entryP == NULL) {
    if (! insertIt)
      return -1;
    
    len = strlen(str);
    entry.str = malloc(len + 1);
    entry.sticky = sticky;
    (void) strcpy(entry.str, str);

    if (tab->entryNo < tab->size) {
      entry.index = tab->entryNo;
      entryP = HashtabInsert(tab->entryTab, &entry, FALSE);
      tab->entryNo++;
    } else {  /* full, replace least recently used */
      findData.timeStamp = TimeStamp + 1;
      (void) HashtabApply(tab->entryTab, findLRU, &findData);
      entry.index = findData.entryP->index;
      entryP = HashtabInsert(tab->entryTab, &entry, FALSE);

      HashtabRemove(tab->entryTab, findData.entryP);
    }
  } else if (sticky && ! entryP->sticky) {
    entryP->sticky = TRUE;
  }

  TimeStamp++;
  entryP->timeStamp = TimeStamp;

  return entryP->index;
}

int
StrTabDefine(STRTAB tab, char *str, BOOL sticky)
{
  return getIndex(tab, str, TRUE, sticky);
}

int
StrTabIndex(STRTAB tab, char *str)
{
  return getIndex(tab, str, FALSE, FALSE);
}

void
StrTabClose(STRTAB tab)
{
  HashtabClose(tab->entryTab);
  free(tab);
}
