/*
************************************************************************
*
*   PropTab.c - manage property tables
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/data/SCCS/s.PropTab.c
*   SCCS identification       : 1.13
*
************************************************************************
*/

#include <prop_tab.h>
#include "prop_cb.h"

#include <stdio.h>
#include <stdlib.h>

#include <dstr.h>
#include <strtab.h>
#include <linlist.h>
#include <hashtab.h>
#include <prop_def.h>

#define MAX_PROP_NO_5 128
#define MAX_PROP_NO 2048
#define MAX_NAME_LEN 100
#define HEX_LEN 20

#define BITS_PER_FIELD (8 * sizeof(unsigned))
#define FIELD_NO_5 ((MAX_PROP_NO_5 + BITS_PER_FIELD - 1) / BITS_PER_FIELD)
#define FIELD_NO ((MAX_PROP_NO + BITS_PER_FIELD - 1) / BITS_PER_FIELD)
#define HASH_FIELD_NO FIELD_NO_5

typedef struct {
  unsigned fields[FIELD_NO];  /* must be first field (type cast) */
  int refCount;
  int idx;
} TabEntry;

typedef struct {
  PropAccessCB accessCB;
  void *clientData;
} PropCBData;

typedef struct {
  char *str;
  BOOL sticky;
  BOOL used;
  int newIdx;
} DumpTabEntry;

typedef struct {
  GFile gf;
  BOOL ok;
  DumpTabEntry *tabP;
  int idx;
} DumpData;

static STRTAB StrTab = NULL;
static HASHTABLE ValTab = NULL;
static struct PropRefS PropRefTab[MAX_PROP_NO];
static struct PropRefS PropRefZero = {0, 0};
static TabEntry *NewTabVisibleP = NULL, *NewTabInvisibleP = NULL;
static LINLIST PropCBTab[MAX_PROP_NO];
static TabEntry *UndumpTab = NULL;

static void
initStrTab(void)
{
  int i;

  StrTab = StrTabOpen(MAX_PROP_NO);

  (void) StrTabDefine(StrTab, PROP_ALL, TRUE);
  (void) StrTabDefine(StrTab, PROP_SELECTED, TRUE);
  (void) StrTabDefine(StrTab, PROP_DISPLAYED, TRUE);
  (void) StrTabDefine(StrTab, PROP_MOVABLE, TRUE);

  /* put these masks into static memory to avoid the mess
     with allocating/freeing */
  for (i = 0; i < MAX_PROP_NO; i++) {
    PropRefTab[i].index = i / BITS_PER_FIELD;
    PropRefTab[i].mask = 1 << (i % BITS_PER_FIELD);
    PropCBTab[i] = NULL;
  }
}

static void
initVisible(void)
{
  /* can't init this with other standard properties to
     maintain compatibility of dump file */
  (void) StrTabDefine(StrTab, PROP_VISIBLE, TRUE);
}

static unsigned
hashFunc(void *p, unsigned size)
{
  TabEntry *entryP = p;
  unsigned idx;
  int i;

  idx = 0;
  for (i = 0; i < HASH_FIELD_NO; i++)
    idx = 37 * idx + entryP->fields[i];

  return idx % size;
}

static int
compFunc(void *p1, void *p2)
{
  TabEntry *entry1P = p1, *entry2P = p2;
  int i;

  for (i = 0; i < FIELD_NO; i++)
    if (entry1P->fields[i] != entry2P->fields[i])
      return 1;
  
  return 0;
}

static void
initValTab(void)
{
  ValTab = HashtabOpen(137, sizeof(TabEntry), hashFunc, compFunc);
}

static TabEntry *
valTabEntry(TabEntry *valP)
{
  TabEntry *entryP;

  entryP = HashtabSearch(ValTab, valP);
  if (entryP == NULL) {
    valP->refCount = 1;
    entryP = HashtabInsert(ValTab, valP, FALSE);
  } else {
    entryP->refCount++;
  }

  return entryP;
}

unsigned *
PropNewTab(BOOL visible)
{
  TabEntry entry;
  int idx;
  int i;

  if (visible) {
    if (NewTabVisibleP != NULL) {
      NewTabVisibleP->refCount++;
      return NewTabVisibleP->fields;
    }
  } else {
    if (NewTabInvisibleP != NULL) {
      NewTabInvisibleP->refCount++;
      return NewTabInvisibleP->fields;
    }
  }

  if (StrTab == NULL) {
    initStrTab();
    initVisible();
  }

  if (ValTab == NULL)
    initValTab();
  
  for (i = 0; i < FIELD_NO; i++)
    entry.fields[i] = 0;
  
  idx = StrTabIndex(StrTab, PROP_ALL);
  entry.fields[PropRefTab[idx].index] = PropRefTab[idx].mask;
  idx = StrTabIndex(StrTab, PROP_SELECTED);
  entry.fields[PropRefTab[idx].index] |= PropRefTab[idx].mask;
  idx = StrTabIndex(StrTab, PROP_DISPLAYED);
  entry.fields[PropRefTab[idx].index] |= PropRefTab[idx].mask;
  idx = StrTabIndex(StrTab, PROP_MOVABLE);
  entry.fields[PropRefTab[idx].index] |= PropRefTab[idx].mask;

  if (visible) {
    idx = StrTabIndex(StrTab, PROP_VISIBLE);
    entry.fields[PropRefTab[idx].index] |= PropRefTab[idx].mask;
    NewTabVisibleP = valTabEntry(&entry);
    return NewTabVisibleP->fields;
  } else {
    NewTabInvisibleP = valTabEntry(&entry);
    return NewTabInvisibleP->fields;
  }
}

PropRefP
PropGetRef(char *prop, BOOL createIt)
{
  int idx;
  PropCBData *dataP, *nextDataP;

  if (StrTab == NULL) {
    initStrTab();
    initVisible();
  }

  if (ValTab == NULL)
    initValTab();

  if (createIt)
    idx = StrTabDefine(StrTab, prop, FALSE);
  else
    idx = StrTabIndex(StrTab, prop);

  if (idx == -1)
    return &PropRefZero;

  if (ListSize(PropCBTab[idx]) > 0) {
    dataP = ListFirst(PropCBTab[idx]);
    while (dataP != NULL) {
      /* entry can be removed in callback! */
      nextDataP = ListNext(PropCBTab[idx], dataP);
      dataP->accessCB(prop, PropRefTab + idx, dataP->clientData);
      dataP = nextDataP;
    }
  }

  return PropRefTab + idx;
}

unsigned *
PropGet(unsigned *propTab)
{
  TabEntry entry, *entryP;
  int i;
  
  for (i = 0; i < FIELD_NO; i++)
    entry.fields[i] = propTab[i];

  entryP = valTabEntry(&entry);

  return entryP->fields;
}

unsigned *
PropChange(unsigned *propTab, PropRefP refP, BOOL onOff)
{
  TabEntry entry, *entryP;
  int i;

  if (onOff == ((propTab[refP->index] & refP->mask) != 0))
    return propTab;
  
  for (i = 0; i < FIELD_NO; i++)
    entry.fields[i] = propTab[i];
  
  entryP = HashtabSearch(ValTab, &entry);

  entryP->refCount--;
  if (entryP->refCount == 0) {
    if (entryP == NewTabVisibleP)
      NewTabVisibleP = NULL;
    else if (entryP == NewTabInvisibleP)
      NewTabInvisibleP = NULL;
    HashtabRemove(ValTab, entryP);
  }
  
  if (onOff)
    entry.fields[refP->index] |= refP->mask;
  else
    entry.fields[refP->index] &= ~refP->mask;

  entryP = valTabEntry(&entry);

  return entryP->fields;
}

void
PropFreeTab(unsigned *propTab)
{
  TabEntry entry, *entryP;
  int i;

  for (i = 0; i < FIELD_NO; i++)
    entry.fields[i] = propTab[i];
  
  entryP = HashtabSearch(ValTab, &entry);

  entryP->refCount--;
  if (entryP->refCount == 0) {
    if (entryP == NewTabVisibleP)
      NewTabVisibleP = NULL;
    else if (entryP == NewTabInvisibleP)
      NewTabInvisibleP = NULL;
    HashtabRemove(ValTab, entryP);
  }
}

static void
fillStrList(int idx, char *str, BOOL sticky, void *clientData)
{
  char **strPA = clientData;

  strPA[idx] = str;
}

void
PropList(unsigned *propTab, PropListFunc listFunc, void *clientData)
{
  char *strPA[MAX_PROP_NO];
  int i;

  StrTabApply(StrTab, fillStrList, strPA);

  for (i = 0; i < MAX_PROP_NO; i++)
    if (propTab[PropRefTab[i].index] & PropRefTab[i].mask)
      listFunc(strPA[i], clientData);
}

void
PropAddAccessCB(char *prop, PropAccessCB accessCB, void *clientData)
{
  int idx;
  PropCBData data;

  idx = StrTabIndex(StrTab, prop);

  if (PropCBTab[idx] == NULL)
    PropCBTab[idx] = ListOpen(sizeof(PropCBData));
  
  data.accessCB = accessCB;
  data.clientData = clientData;
  (void) ListInsertLast(PropCBTab[idx], &data);
}

void
PropRemoveAccessCB(char *prop, PropAccessCB accessCB, void *clientData)
/* It is legal to remove a callback more than once. */
{
  int idx;
  PropCBData *dataP;

  idx = StrTabIndex(StrTab, prop);
  if (idx == -1)
    /* this will not happen normally, but if StrTab is overfilled,
       an entry can be replaced with another one */
    return;

  dataP = ListFirst(PropCBTab[idx]);
  while (dataP != NULL) {
    if (dataP->accessCB == accessCB && dataP->clientData == clientData) {
      ListRemove(PropCBTab[idx], dataP);
      break;
    }
    dataP = ListNext(PropCBTab[idx], dataP);
  }
}

static void
orFields(void *p, void *clientData)
{
  TabEntry *entryP = p;
  unsigned *fields = clientData;
  int i;

  for (i = 0; i < FIELD_NO; i++)
    fields[i] |= entryP->fields[i];
}

static void
fillDumpTab(int idx, char *str, BOOL sticky, void *clientData)
{
  DumpTabEntry *dumpTabP = clientData;

  dumpTabP[idx].str = str;
  dumpTabP[idx].sticky = sticky;
}

#define CHECK_RES(s) if (s != GF_RES_OK) {dumpP->ok = FALSE; return;}

static void
dumpEntry(void *p, void *clientData)
{
  TabEntry *entryP = p;
  DumpData *dumpP = clientData;
  GFile gf = dumpP->gf;
  unsigned fields[FIELD_NO];
  char buf[HEX_LEN];
  int newIdx, i;

  if (! dumpP->ok)
    return;

  for (i = 0; i < FIELD_NO; i++)
    fields[i] = 0;

  for (i = 0; i < MAX_PROP_NO; i++) {
    if (entryP->fields[PropRefTab[i].index] & PropRefTab[i].mask) {
      newIdx = dumpP->tabP[i].newIdx;
      fields[newIdx / BITS_PER_FIELD] |= 1 << (newIdx % BITS_PER_FIELD);
    }
  }

  for (i = 0; i < FIELD_NO; i++) {
    (void) sprintf(buf, "%x", fields[i]);
    CHECK_RES(GFileWriteStr(gf, buf));
  }

  CHECK_RES(GFileWriteNL(gf));

  entryP->idx = dumpP->idx;
  dumpP->idx++;
}

#undef CHECK_RES
#define CHECK_RES(s) if (s != GF_RES_OK) return FALSE

BOOL
PropDump(GFile gf)
{
  unsigned usedFields[FIELD_NO];
  DumpTabEntry dumpTab[MAX_PROP_NO];
  int newIdx;
  DumpData dumpS;
  int i;

  /* find used fields */
  for (i = 0; i < FIELD_NO; i++)
    usedFields[i] = 0;
  HashtabApply(ValTab, orFields, usedFields);

  for (i = 0; i < MAX_PROP_NO; i++)
    dumpTab[i].str = NULL;

  StrTabApply(StrTab, fillDumpTab, dumpTab);

  newIdx = 0;
  for (i = 0; i < MAX_PROP_NO; i++) {
    if (dumpTab[i].str != NULL && (dumpTab[i].sticky ||
	usedFields[PropRefTab[i].index] & PropRefTab[i].mask)) {
      dumpTab[i].used = TRUE;
      dumpTab[i].newIdx = newIdx;
      newIdx++;
    } else {
      dumpTab[i].used = FALSE;
    }
  }

  CHECK_RES(GFileWriteINT32(gf, newIdx));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  for (i = 0; i < MAX_PROP_NO; i++) {
    if (dumpTab[i].used) {
      CHECK_RES(GFileWriteQuotedStr(gf, dumpTab[i].str));
      CHECK_RES(GFileWriteNL(gf));
    }
  }
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteINT32(gf, HashtabSize(ValTab)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  dumpS.gf = gf;
  dumpS.ok = TRUE;
  dumpS.tabP = dumpTab;
  dumpS.idx = 0;
  HashtabApply(ValTab, dumpEntry, &dumpS);

  if (! dumpS.ok)
    return FALSE;

  CHECK_RES(GFileWriteNL(gf));

  return TRUE;
}

int
PropGetIndex(unsigned *propTab)
{
  TabEntry *entryP = (TabEntry *) propTab;

  return entryP->idx;
}

BOOL
PropUndumpStart(GFile gf, int vers)
{
  int propNo, tabSize, idx;
  INT32 i32;
  char name[MAX_NAME_LEN];
  char buf[HEX_LEN];
  int i;

  if (StrTab != NULL) {
    StrTabClose(StrTab);
    for (i = 0; i < MAX_PROP_NO; i++)
      ListClose(PropCBTab[i]);
  }

  initStrTab();

  if (HashtabSize(ValTab) > 0) {
    (void) fprintf(stderr,
	"internal error: property table not empty after cleanup!\n");
    HashtabClose(ValTab);
    initValTab();
  }

  CHECK_RES(GFileReadINT32(gf, &i32));
  propNo = i32;

  for (i = 0; i < propNo; i++) {
    CHECK_RES(GFileReadQuotedStr(gf, name, sizeof(name)));
    (void) StrTabDefine(StrTab, name, FALSE);
  }

  initVisible();

  CHECK_RES(GFileReadINT32(gf, &i32));
  tabSize = i32;

  if (tabSize == 0)
    return TRUE;

  if (UndumpTab != NULL)  /* should already be freed, but maybe... */
    free(UndumpTab);

  UndumpTab = malloc(tabSize * sizeof(TabEntry));

  for (idx = 0; idx < tabSize; idx++) {
    if (vers < 6) {
      for (i = 0; i < FIELD_NO_5; i++) {
	CHECK_RES(GFileReadStr(gf, buf, sizeof(buf)));
	(void) sscanf(buf, "%x", &UndumpTab[idx].fields[i]);
      }
      for (i = FIELD_NO_5; i < FIELD_NO; i++)
	UndumpTab[idx].fields[i] = 0;
    } else {
      for (i = 0; i < FIELD_NO; i++) {
	CHECK_RES(GFileReadStr(gf, buf, sizeof(buf)));
	(void) sscanf(buf, "%x", &UndumpTab[idx].fields[i]);
      }
    }
  }

  return TRUE;
}

unsigned *
PropFindIndex(int idx)
{
  return UndumpTab[idx].fields;
}

void
PropUndumpEnd(void)
{
  free(UndumpTab);
  UndumpTab = NULL;
}
