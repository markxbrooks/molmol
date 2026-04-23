/*
************************************************************************
*
*   Callback.c - callback handling for opaque container types
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
*   Date of last modification : 95/07/20
*   Pathname of SCCS file     : /sgiext/molmol/tools/src/SCCS/s.Callback.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <callback.h>

#include <stdio.h>
#include <stdlib.h>

/* realloc increment for callback lists */
#define INCR    10


/*----------------------------------------------------------------------
DESCRIPTION %[
This module provides a couple of callback support routines which should
be used by generic modules (e.g. Table, Linlist, Stack, Hash ...) in
order to share code and reduce object size.
!! Applications should not call these functions directly. !!
%]
----------------------------------------------------------------------*/

/**********************************************************************/
void *
_AddDestroyCB(DestroyCBInfo *dci, DestroyCB callback,
    void *clientdta, FreeCldta freecldta)
/*----------------------------------------------------------------------
Add destroy callback support function.
!! Intended to be used by generic modules (e.g. Linlist, Table) only. !!
----------------------------------------------------------------------*/
{
  DestroyCBRec *p;                      /* temp ptr for realloc */

  if (dci->destroyLst == NULL) {
    p = malloc(INCR * sizeof(DestroyCBRec));
    if (p != NULL)
      dci->destroySiz = INCR;
  } else if (dci->destroyNum == dci->destroySiz) {
    p = realloc(dci->destroyLst,
        (dci->destroySiz + INCR) * sizeof(DestroyCBRec));
    if (p != NULL)
      dci->destroySiz += INCR;
  } else {
    p = dci->destroyLst;              /* there is enough room in list */
  }

  if (p) {                            /* we can add it now */
    dci->destroyLst = p;
    p += dci->destroyNum;             /* skip to next free entry */
    p->callback = callback;
    p->clientdta = clientdta;
    p->freecldta = freecldta;
    dci->destroyNum++;
    return clientdta;
  }
  return NULL;
}


/**********************************************************************/
void
_RemoveDestroyCB(DestroyCBInfo *dci, DestroyCB callback,
    void *clientdta)
/*----------------------------------------------------------------------
Remove the specified callback and clientdta.
!! Intended to be used by generic modules (e.g. Linlist, Table) only. !!
----------------------------------------------------------------------*/
{
  int     i;
  DestroyCBRec *p = dci->destroyLst;

  /* find the callback in the list */
  for (i = 0; i < dci->destroyNum; i++, p++) {

    if (p->callback != callback || p->clientdta != clientdta) {
      continue;
    } else {

      if (p->freecldta && p->clientdta)    /* auto free the clientdta */
        (*p->freecldta) (p->clientdta);

      /* remove the callback by rippling all others one down */
      for (i += 1; i < dci->destroyNum; i++)
        dci->destroyLst[i - 1] = dci->destroyLst[i];

      --dci->destroyNum;                   /* now we have one less */
      /* Note: destroySiz remains the same !! */

      /* fill the old last with zeros */
      p = &dci->destroyLst[dci->destroyNum];
      p->callback = NULL;
      p->clientdta = NULL;
      p->freecldta = NULL;
    }
  }
}


/**********************************************************************/
void
_RemoveAllDestroyCB(DestroyCBInfo *dci)
/*--------------------------------------------------------------
Remove all registered destroy callbacks and free associated
clientdata.
--------------------------------------------------------------*/
{
  int     i;
  DestroyCBRec *p = dci->destroyLst;

  for (i = 0; i < dci->destroyNum; i++, p++) {
    if (p->freecldta && p->clientdta)
      (*p->freecldta) (p->clientdta);
    p->callback = NULL;
    p->clientdta = NULL;
    p->freecldta = NULL;
  }
  if (dci->destroyLst != NULL) {
    free(dci->destroyLst);
  }
  dci->destroyLst = NULL;
  dci->destroyNum = 0;
  dci->destroySiz = 0;
}


/**********************************************************************/
void
_ExecAllDestroyCB(DestroyCBInfo *dci, void *pdata)
/*----------------------------------------------------------------------
Execute all destroy callbacks with clientdta on pdata.
----------------------------------------------------------------------*/
{
  int     i;
  DestroyCBRec *p = dci->destroyLst;

  for (i = 0; i < dci->destroyNum; i++, p++)
    (*p->callback) (pdata, p->clientdta);
}
