/*
************************************************************************
*
*   AttrMng.c - attribute manager
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/attr/SCCS/s.AttrMng.c
*   SCCS identification       : 1.19
*
************************************************************************
*/

#include <attr_mng.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include <hashtab.h>
#include <attr_struc.h>

typedef struct {
  struct AttrS attr;  /* must be first field (type cast) */
  int refCount;
  int idx;
} TabEntry;

typedef struct {
  GFile gf;
  BOOL ok;
} DumpData;

static HASHTABLE AttrTab = NULL;
static BOOL AttrInitDone = FALSE;
static struct AttrS AttrInitS;
static AttrP AttrInitP = NULL;
static BOOL IdxValid;
static struct AttrS *UndumpTab = NULL;

static unsigned
hashFunc(void *p, unsigned size)
{
  int *intP, i;
  unsigned idx;

  intP = p;
  idx = 0;
  for (i = 0; i < sizeof(struct AttrS) / sizeof(int); i++)
    idx = 37 * idx + intP[i];
  
  return idx % size;
}

static int
compFunc(void *p1, void *p2)
{
  return memcmp(p1, p2, sizeof(struct AttrS));
}

void
setAttrInitS(void)
{
  if (AttrInitDone)
    return;

  /* make sure that all padding bytes are 0, we will make binary
     comparisons later */
  (void) memset(&AttrInitS, 0, sizeof(AttrInitS));

  AttrInitS.colR = 0.0f;
  AttrInitS.colG = 0.0f;
  AttrInitS.colB = 1.0f;
  AttrInitS.secColR = 1.0f;
  AttrInitS.secColG = 0.0f;
  AttrInitS.secColB = 0.0f;
  AttrInitS.ambCoeff = 0.2f;
  AttrInitS.diffCoeff = 0.8f;
  AttrInitS.specCoeff = 0.4f;
  AttrInitS.shininess = 30.0f;
  AttrInitS.reflect = 0.0f;
  AttrInitS.alpha = 1.0f;
  AttrInitS.refract = 1.0f;
  AttrInitS.texture[0] = '\0';
  AttrInitS.textureScale = 1.0f;
  AttrInitS.bumpDepth = 0.0f;
  AttrInitS.bumpScale = 1.0f;
  AttrInitS.atomStyle = AS_INVISIBLE;
  AttrInitS.bondStyle = BS_LINE;
  AttrInitS.distStyle = DS_LINE;
  AttrInitS.distLabel = DL_INVISIBLE;
  AttrInitS.labelFormat[0] = '\0';
  AttrInitS.radius = 0.2f;
  AttrInitS.lineWidth = 0;
  AttrInitS.lineStyle = SG_LINE_SOLID;
  AttrInitS.shadeModel = -1;
  AttrInitS.fontSize = 0.03f;
  AttrInitS.nearClip = 0.0f;

  AttrInitDone = TRUE;
}

AttrP
AttrGetInit(void)
{
  if (AttrInitP == NULL)
    AttrInitP = AttrGet(&AttrInitS);
  else
    ((TabEntry *) AttrInitP)->refCount++;

  return AttrInitP;
}

AttrP
AttrGet(AttrP attrP)
{
  TabEntry entry, *entryP;

  if (AttrTab == NULL) {
    AttrTab = HashtabOpen(137, sizeof(TabEntry), hashFunc, compFunc);
    IdxValid = FALSE;
    setAttrInitS();
  }

  entryP = HashtabSearch(AttrTab, attrP);
  if (entryP != NULL) {
    entryP->refCount++;
    return (AttrP) entryP;
  }

  (void) memcpy(&entry, attrP, sizeof(*attrP));
  entry.refCount = 1;

  IdxValid = FALSE;
  return HashtabInsert(AttrTab, &entry, FALSE);
}

void
AttrCopy(AttrP attr1P, AttrP attr2P)
{
  (void) memcpy(attr1P, attr2P, sizeof(*attr1P));
}

void
AttrSetInit(AttrP attrP)
{
  AttrInitP = NULL;
  AttrInitS = *attrP;
}

void
AttrReturn(AttrP attrP)
{
  TabEntry *entryP;

  entryP = HashtabSearch(AttrTab, attrP);
  entryP->refCount--;
  if (entryP->refCount == 0) {
    HashtabRemove(AttrTab, entryP);
    if ((AttrP) entryP == AttrInitP)
      AttrInitP = NULL;
    IdxValid = FALSE;
  }
}

MolAttrP
MolAttrGet(void)
{
  MolAttrP attrP;

  attrP = malloc(sizeof(*attrP));
  attrP->objDefined = FALSE;

  return attrP;
}

void
MolAttrReturn(MolAttrP attrP)
{
  free(attrP);
}

#define CHECK_RES(s) if (s != GF_RES_OK) {dumpP->ok = FALSE; return;}

static void
dumpAttrStruc(void *p, void *clientData)
{
  AttrP attrP = (AttrP) p;
  DumpData *dumpP = clientData;
  GFile gf = dumpP->gf;

  if (! dumpP->ok)
    return;

  CHECK_RES(GFileWriteFLOAT32(gf, attrP->colR));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->colG));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->colB));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->secColR));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->secColG));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->secColB));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteFLOAT32(gf, attrP->ambCoeff));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->diffCoeff));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->specCoeff));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->shininess));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->reflect));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->alpha));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->refract));
  CHECK_RES(GFileWriteQuotedStr(gf, attrP->texture));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->textureScale));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->bumpDepth));
  CHECK_RES(GFileWriteFLOAT32(gf, attrP->bumpScale));

  CHECK_RES(GFileWriteINT16(gf, (INT16) attrP->atomStyle));
  CHECK_RES(GFileWriteINT16(gf, (INT16) attrP->bondStyle));
  CHECK_RES(GFileWriteINT16(gf, (INT16) attrP->distStyle));
  CHECK_RES(GFileWriteINT16(gf, (INT16) attrP->distLabel));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteQuotedStr(gf, attrP->labelFormat));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteFLOAT32(gf, attrP->radius));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteFLOAT32(gf, attrP->lineWidth));
  CHECK_RES(GFileWriteINT16(gf, (INT16) attrP->lineStyle));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteINT16(gf, attrP->shadeModel));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteFLOAT32(gf, attrP->fontSize));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteFLOAT32(gf, attrP->nearClip));
  CHECK_RES(GFileWriteNL(gf));

  CHECK_RES(GFileWriteNL(gf));
}

static void
setIdx(void *p, void *clientData)
{
  TabEntry *entryP = p;
  int *idxP = clientData;

  entryP->idx = *idxP;
  *idxP += 1;
}

static void
updateIdx(void)
{
  int idx;

  if (IdxValid)
    return;
  
  idx = 0;
  HashtabApply(AttrTab, setIdx, &idx);

  IdxValid = TRUE;
}

#undef CHECK_RES
#define CHECK_RES(s) if (s != GF_RES_OK) return FALSE

BOOL
AttrDump(GFile gf)
{
  DumpData dumpS;

  dumpS.gf = gf;
  dumpS.ok = TRUE;

  setAttrInitS();
  dumpAttrStruc(&AttrInitS, &dumpS);

  updateIdx();

  CHECK_RES(GFileWriteINT32(gf, HashtabSize(AttrTab)));
  CHECK_RES(GFileWriteNL(gf));
  CHECK_RES(GFileWriteNL(gf));

  HashtabApply(AttrTab, dumpAttrStruc, &dumpS);

  return dumpS.ok;
}

int
AttrGetIndex(AttrP attrP)
{
  TabEntry *entryP = (TabEntry *) attrP;

  updateIdx();

  return entryP->idx;
}

#undef CHECK_RES
#define CHECK_RES(s) if (s != GF_RES_OK) {dumpP->ok = FALSE; return;}

static void
undumpAttrStruc(AttrP attrP, DumpData *dumpP, int vers)
{
  GFile gf = dumpP->gf;
  INT16 i16;
  FLOAT32 f32;

  if (! dumpP->ok)
    return;

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  attrP->colR = f32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  attrP->colG = f32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  attrP->colB = f32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  attrP->secColR = f32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  attrP->secColG = f32;
  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  attrP->secColB = f32;

  if (vers >= 3) {
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->ambCoeff = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->diffCoeff = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->specCoeff = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->shininess = f32;
    if (vers >= 4) {
      CHECK_RES(GFileReadFLOAT32(gf, &f32));
      attrP->reflect = f32;
    }
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->alpha = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->refract = f32;
    CHECK_RES(GFileReadQuotedStr(gf,
	attrP->texture, sizeof(attrP->texture)));
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->textureScale = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->bumpDepth = f32;
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->bumpScale = f32;
  }

  CHECK_RES(GFileReadINT16(gf, &i16));
  attrP->atomStyle = i16;
  CHECK_RES(GFileReadINT16(gf, &i16));
  attrP->bondStyle = i16;
  CHECK_RES(GFileReadINT16(gf, &i16));
  attrP->distStyle = i16;
  CHECK_RES(GFileReadINT16(gf, &i16));
  attrP->distLabel = i16;

  CHECK_RES(GFileReadQuotedStr(gf,
      attrP->labelFormat, sizeof(attrP->labelFormat)));

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  attrP->radius = f32;

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  attrP->lineWidth = f32;
  CHECK_RES(GFileReadINT16(gf, &i16));
  attrP->lineStyle = i16;

  CHECK_RES(GFileReadINT16(gf, &i16));
  attrP->shadeModel = i16;

  CHECK_RES(GFileReadFLOAT32(gf, &f32));
  attrP->fontSize = f32;

  if (vers >= 14) {
    CHECK_RES(GFileReadFLOAT32(gf, &f32));
    attrP->nearClip = f32;
  }
}

#undef CHECK_RES
#define CHECK_RES(s) if (s != GF_RES_OK) return FALSE

BOOL
AttrUndumpStart(GFile gf, int vers)
{
  int tabSize, idx;
  DumpData dumpS;
  struct AttrS attr;
  AttrP attrP;
  INT32 i32;

  dumpS.gf = gf;
  dumpS.ok = TRUE;

  setAttrInitS();
  if (vers >= 16) {
    /* MOLMOL 2K.1 would save blank AttrInitS under certain
       conditions, only restore if values are reasonable. */
    undumpAttrStruc(&attr, &dumpS, vers);
    if (attr.ambCoeff + attr.diffCoeff > 0.0f)
      (void) memcpy(&AttrInitS, &attr, sizeof(attr));
  }

  CHECK_RES(GFileReadINT32(gf, &i32));
  tabSize = i32;

  if (tabSize == 0)
    return TRUE;

  if (UndumpTab != NULL)  /* should already be freed, but maybe... */
    free(UndumpTab);

  UndumpTab = malloc(tabSize * sizeof(struct AttrS));

  for (idx = 0; idx < tabSize; idx++) {
    attrP = UndumpTab + idx;
    *attrP = AttrInitS;
    undumpAttrStruc(attrP, &dumpS, vers);
  }

  return dumpS.ok;
}

AttrP
AttrFindIndex(int idx)
{
  return UndumpTab + idx;
}

void
AttrUndumpEnd(void)
{
  free(UndumpTab);
  UndumpTab = NULL;
}
