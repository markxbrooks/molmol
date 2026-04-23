/*
************************************************************************
*
*   SgObj.c - Sg object (aka display list) management
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgObj.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <stdio.h>

#include <rand_num.h>
#include <hashtab.h>

#include <sg_dev.h>
#include <sg_error.h>
#include "sg_set.h"

#define OBJ_SUPPORTED (CurrSgDevP->beginObj != NULL)

typedef struct {
  SgObjId obj;
  SgDrawFunc drawF;
  void *clientData;
} ObjDescr;

#define TABLE_SIZE 2791

static HASHTABLE ObjTab = NULL;

static unsigned
hashFunc(void *p, unsigned size)
{
  ObjDescr *descrP = p;

  return descrP->obj % size;
}

static int
compFunc(void *p1, void *p2)
{
  ObjDescr *descr1P = p1;
  ObjDescr *descr2P = p2;

  return descr1P->obj != descr2P->obj;
}

SgObjId
SgCreateObj(SgDrawFunc drawF, void *clientData)
{
  ObjDescr descr;

  if (ObjTab == NULL)
    ObjTab = HashtabOpen(TABLE_SIZE, sizeof(ObjDescr), hashFunc, compFunc);

  if (OBJ_SUPPORTED) {
    descr.obj = CurrSgDevP->beginObj();
    SgSetAttr();
    drawF(clientData);
    CurrSgDevP->endObj();
  } else {
    for (;;) {
      descr.obj = RandInt32();
      if (HashtabSearch(ObjTab, &descr) == NULL)
	break;
    }
  }

  descr.drawF = drawF;
  descr.clientData = clientData;
  (void) HashtabInsert(ObjTab, &descr, FALSE);

  return descr.obj;
}
 
void
SgReplaceObj(SgObjId obj, SgDrawFunc drawF, void *clientData)
{
  ObjDescr descr, *descrP;

  descr.obj = obj;
  descrP = HashtabSearch(ObjTab, &descr);
  if (descrP == NULL) {
    SgRaiseError(SG_ERR_ILLOBJ, "SgReplaceObj");
    return;
  }
  descrP->drawF = drawF;
  descrP->clientData = clientData;

  if (OBJ_SUPPORTED) {
    CurrSgDevP->replaceObj(obj);
    SgSetAttr();
    drawF(clientData);
    CurrSgDevP->endObj();
  }
}
 
void
SgDestroyObj(SgObjId obj)
{
  ObjDescr descr, *descrP;

  descr.obj = obj;
  descrP = HashtabSearch(ObjTab, &descr);
  if (descrP == NULL) {
    SgRaiseError(SG_ERR_ILLOBJ, "SgDestroyObj");
    return;
  }
  HashtabRemove(ObjTab, descrP);

  if (OBJ_SUPPORTED)
    CurrSgDevP->destroyObj(obj);
}
 
void
SgDrawObj(SgObjId obj)
{
  ObjDescr descr, *descrP;

  if (OBJ_SUPPORTED) {
    CurrSgDevP->drawObj(obj);
  } else {
    descr.obj = obj;
    descrP = HashtabSearch(ObjTab, &descr);
    if (descrP != NULL)
      descrP->drawF(descrP->clientData);
  }
}
