/*
************************************************************************
*
*   Linlist.c - implementation of LINLIST type
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.Linlist.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <linlist.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef double ALIGN;

/*----------------------------------------------------------------------
DESCRIPTION %[
  This package implements a list manager. The user (client) of
  this package can put any data items (of equal size) into a list
  without bothering how such a list is implemented.
  All ListXxxxx(ls, ... ) functions take as first argument a list
  (LINLIST) that has been returned by ListOpen(). Therefore it is
  possible to have a virtually unlimited number of lists at the
  same time, each identified by a unique LINLIST.  If a NULL pointer
  is passed instead of a LINLIST, this is considered to be an
  empty list.
  The list always allocates and maintains an internal copy of the
  data which is supplied by the client, therefore a single template
  variable can be used to put data into the list. Manipulations of that
  template afterwards will not change the data in the list.

  Data is always put into and retrieved from the list via pointers,
  because earlier compiler versions did not allow to pass and return
  structures from functions.

  --------------------  example  --------------------
  struct data {
        char    name[20];
        int     age;
  } template, *pdata;
  /\* open the list *\/
  LINLIST ls = ListOpen(sizeof(struct data));
  /\*
   * fill the list with data
   *\/
  for(;;) {
    (void) printf("name age ? ");
    n = scanf("%s %d\\n", template.name, &template.age);
    if( n == EOF )
      break;
    else if( n == 2 )
      (void) ListInsertLast(ls, &template);
    else
      continue;         /\* ignore *\/
  };
  (void) printf("\\n");
  /\*
   * find the entry with name "Reto"
   *\/
  pdata = ListFirst(ls, &template);
  while (pdata != NULL) {
    (void) printf("%s %d\\n", pdata->name, pdata->age);
    pdata = ListNext(ls, pdata);
  }
  ListClose(ls);

  ------------------  end example  ------------------
%]
----------------------------------------------------------------------*/


/* macro to round up nodesize to multiples of ALIGN-size in bytes */
#define NODSIZ  \
  ( ((sizeof(L_NODE)-1) / sizeof(ALIGN) + 1) * sizeof(ALIGN) )

/* make a listnode pointer from data pointer */
#define MK_LNODP(p)     ( (p) ? (L_NODE*) ((char*)(p) - NODSIZ) : NULL )

/* make a data pointer from listnode pointer */
#define MK_DATAP(p)     ( (p) ? (void *) ((char*)(p) + NODSIZ) : NULL )

/***********************************************************************
        TYPES
***********************************************************************/

typedef union lstnode {
  struct {
    union lstnode *prev, *next;
  }       s;
  ALIGN   x;
}       L_NODE;                       /* List node */


struct listStruc {                    /* opaque exported as LINLIST */
  L_NODE *first, *last;
  unsigned dataSize;                  /* rounded up data-size */
  unsigned elemNo;
  DestroyCBInfo dci;
};


/**********************************************************************/
LINLIST
ListOpen(
unsigned dataSize)                    /* data size */
/*----------------------------------------------------------------------
Open a list, return list or NULL on error.
----------------------------------------------------------------------*/
{
  LINLIST ls;

  ls = malloc(sizeof(struct listStruc));
  if (ls == NULL)
    return NULL;

  ls->first = NULL;
  ls->last = NULL;
  ls->dataSize = dataSize;
  ls->elemNo = 0;
  ls->dci.destroySiz = 0;
  ls->dci.destroyNum = 0;
  ls->dci.destroyLst = NULL;

  return ls;
}


/**********************************************************************/
void
ListAddDestroyCB(
LINLIST ls,               /* specify list */
DestroyCB callback,       /* specify destroy callback function */
void   *clientdta,        /* specify clientdata for callback */
FreeCldta freecldta)      /* specify free clientdata function or NULL */
/*----------------------------------------------------------------------
Add Destroy callback function.
The destroy callback which will be called for every entry at
ListRemove() and ListClose(). This can be used to free data allocated
externally by the user and referenced in the entry etc.
If (freecldta != NULL) it will be called to free the clientdta at
ListRemoveDestroyCB() or after the callback function has been called at
ListClose().
----------------------------------------------------------------------*/
{
  (void) _AddDestroyCB(&ls->dci, callback, clientdta, freecldta);
}


/**********************************************************************/
void
ListRemoveDestroyCB(
LINLIST ls,
DestroyCB callback,
void   *clientdta)
/*----------------------------------------------------------------------
Remove destroy callback function and free clientdta.
----------------------------------------------------------------------*/
{
  _RemoveDestroyCB(&ls->dci, callback, clientdta);
}


/**********************************************************************/
void
ListClose(
LINLIST ls)               /* specify list to be closed */
/*----------------------------------------------------------------------
Close the list and destroy all entries (freeing the memory).
No action if NULL is passed.
Note: if the nodes contain ptr's to additional data allocated by the
      user, it is the user's responsibility to free all that data,
      otherwise the memory is lost. This can be avoided by registering
      a destroy callback function using ListAddDestroyCB().
----------------------------------------------------------------------*/
{
  L_NODE *p1, *p2;

  if (ls == NULL)
    return;

  p1 = ls->first;
  while (p1 != NULL) {
    p2 = p1->s.next;
    _ExecAllDestroyCB(&ls->dci, MK_DATAP(p1));
    free(p1);
    p1 = p2;
  }

  _RemoveAllDestroyCB(&ls->dci);

  free(ls);
}


/**********************************************************************/
void
ListApply(
LINLIST ls,               /* specify list */
ListApplyFunc applyfunc,  /* function applied for each element */
void   *cl_dta)           /* second arg to applyfunc */
/*----------------------------------------------------------------------
Apply the function 'applyfunc' to every element in the list.
It is legal to pass NULL as list, this is equivalent to an empty list.
Applyfunc's 1st argument is a ptr to the data in the list, the second
argument is cl_dta.
      applyfunc( void *pdata, void* cl_dta );
----------------------------------------------------------------------*/
{
  L_NODE *p1, *p2;

  if (ls == NULL)
    return;

  p1 = ls->first;
  while (p1 != NULL) {
    p2 = p1->s.next;  /* in case function removes p1 !! */
    applyfunc(MK_DATAP(p1), cl_dta);
    p1 = p2;
  }
}

/**********************************************************************/
void *
ListInsertFirst(
LINLIST ls,
void   *pdata)
/*----------------------------------------------------------------------
Insert *pdata at beginning of list.
----------------------------------------------------------------------*/
{
  L_NODE *newP;
  void   *newDtaP;

  /* allocate memory for new node */
  newP = malloc((unsigned) NODSIZ + ls->dataSize);
  if (newP == NULL)
    return NULL;

  if (ls->first == NULL) {
    newP->s.prev = NULL;
    newP->s.next = NULL;
    ls->first = newP;
    ls->last = newP;
  } else {
    newP->s.prev = NULL;
    newP->s.next = ls->first;
    ls->first->s.prev = newP;
    ls->first = newP;
  }

  ls->elemNo++;

  /* transfer data to new location */
  newDtaP = MK_DATAP(newP);
  (void) memcpy(newDtaP, pdata, (int) ls->dataSize);

  return newDtaP;
}

/**********************************************************************/
void *
ListInsertLast(
LINLIST ls,
void   *pdata)
/*----------------------------------------------------------------------
Insert *pdata at end of list.
----------------------------------------------------------------------*/
{
  L_NODE *newP;
  void   *newDtaP;

  /* allocate memory for new node */
  newP = malloc((unsigned) NODSIZ + ls->dataSize);
  if (newP == NULL)
    return NULL;

  if (ls->first == NULL) {
    newP->s.prev = NULL;
    newP->s.next = NULL;
    ls->first = newP;
    ls->last = newP;
  } else {
    newP->s.prev = ls->last;
    newP->s.next = NULL;
    ls->last->s.next = newP;
    ls->last = newP;
  }

  ls->elemNo++;

  /* transfer data to new location */
  newDtaP = MK_DATAP(newP);
  (void) memcpy(newDtaP, pdata, (int) ls->dataSize);

  return newDtaP;
}

/**********************************************************************/
void *
ListInsertBefore(
LINLIST ls,
void   *elem,
void   *pdata)
/*----------------------------------------------------------------------
Insert pdata before elem.
----------------------------------------------------------------------*/
{
  L_NODE *newP, *nextP;
  void   *newDtaP;

  /* allocate memory for new node */
  newP = malloc((unsigned) NODSIZ + ls->dataSize);
  if (newP == NULL)
    return NULL;

  nextP = MK_LNODP(elem);

  newP->s.prev = nextP->s.prev;
  newP->s.next = nextP;

  if (nextP->s.prev == NULL) {
    ls->first = newP;
  } else {
    nextP->s.prev->s.next = newP;
  }

  nextP->s.prev = newP;

  ls->elemNo++;

  /* transfer data to new location */
  newDtaP = MK_DATAP(newP);
  (void) memcpy(newDtaP, pdata, (int) ls->dataSize);

  return newDtaP;
}

/**********************************************************************/
void *
ListInsertAfter(
LINLIST ls,
void   *elem,
void   *pdata)
/*----------------------------------------------------------------------
Insert pdata after elem.
----------------------------------------------------------------------*/
{
  L_NODE *newP, *prevP;
  void   *newDtaP;

  /* allocate memory for new node */
  newP = malloc((unsigned) NODSIZ + ls->dataSize);
  if (newP == NULL)
    return NULL;

  prevP = MK_LNODP(elem);

  newP->s.next = prevP->s.next;
  newP->s.prev = prevP;

  if (prevP->s.next == NULL) {
    ls->last = newP;
  } else {
    prevP->s.next->s.prev = newP;
  }

  prevP->s.next = newP;

  ls->elemNo++;

  /* transfer data to new location */
  newDtaP = MK_DATAP(newP);
  (void) memcpy(newDtaP, pdata, (int) ls->dataSize);

  return newDtaP;
}

/**********************************************************************/
void
ListMoveFirst(
LINLIST ls,
void   *pdata)
/*----------------------------------------------------------------------
Move elem to the start of the list.
----------------------------------------------------------------------*/
{
  L_NODE *p = MK_LNODP(pdata);

  if (p->s.prev == NULL)  /* already first */
    return;

  p->s.prev->s.next = p->s.next;
  if (p->s.next == NULL)
    ls->last = p->s.prev;
  else
    p->s.next->s.prev = p->s.prev;

  p->s.prev = NULL;
  p->s.next = ls->first;
  ls->first->s.prev = p;
  ls->first = p;
}

/**********************************************************************/
void
ListSwap(
LINLIST ls,
void   *elem1,
void   *elem2)
/*----------------------------------------------------------------------
Swap the position of the two list elements.
----------------------------------------------------------------------*/
{
  L_NODE *p1, *p2, *p;

  p1 = MK_LNODP(elem1);
  p2 = MK_LNODP(elem2);

  p = p1->s.next;
  p1->s.next = p2->s.next;
  p2->s.next = p;

  p = p1->s.prev;
  p1->s.prev = p2->s.prev;
  p2->s.prev = p;

  if (p1->s.prev == NULL)
    ls->first = p1;
  else
    p1->s.prev->s.next = p1;

  if (p1->s.next == NULL)
    ls->last = p1;
  else
    p1->s.next->s.prev = p1;

  if (p2->s.prev == NULL)
    ls->first = p2;
  else
    p2->s.prev->s.next = p2;

  if (p2->s.next == NULL)
    ls->last = p2;
  else
    p2->s.next->s.prev = p2;
}

/**********************************************************************/
void
ListRemove(
LINLIST ls,
void   *pdata)
/*----------------------------------------------------------------------
Remove *pdata from list.
Returns TRUE if pdata could be removed, FALSE otherwise.
----------------------------------------------------------------------*/
{
  L_NODE *p = MK_LNODP(pdata);

  if (p->s.prev == NULL)
    ls->first = p->s.next;
  else
    p->s.prev->s.next = p->s.next;
  if (p->s.next == NULL)
    ls->last = p->s.prev;
  else
    p->s.next->s.prev = p->s.prev;

  ls->elemNo--;

  _ExecAllDestroyCB(&ls->dci, pdata);
  free(p);
}


/**********************************************************************/
static L_NODE *
mergeLists(
L_NODE *lp1,
L_NODE *lp2,
ListEntryCompareFunc compfunc)
/*----------------------------------------------------------------------
Merge two lists, used for sorting.
----------------------------------------------------------------------*/
{
  L_NODE *startP, *tmpP;
  int res;

  res = compfunc(MK_DATAP(lp1), MK_DATAP(lp2));
  if (res == 1)
    startP = lp2;
  else
    startP = lp1;
  
  do {
    if (res == 1) {
      do {
	tmpP = lp2;
	lp2 = lp2->s.next;
	if (lp2 == NULL)
	  break;
	res = compfunc(MK_DATAP(lp1), MK_DATAP(lp2));
      } while (res == 1);
      tmpP->s.next = lp1;
      lp1->s.prev = tmpP;
    } else {
      do {
	tmpP = lp1;
	lp1 = lp1->s.next;
	if (lp1 == NULL)
	  break;
	res = compfunc(MK_DATAP(lp1), MK_DATAP(lp2));
      } while (res != 1);
      tmpP->s.next = lp2;
      lp2->s.prev = tmpP;
    }
  } while (lp1 != NULL && lp2 != NULL);

  return startP;
}


/**********************************************************************/
static L_NODE *
sortRec(
L_NODE *lp,
unsigned len,
ListEntryCompareFunc compfunc)
/*----------------------------------------------------------------------
Recursive function for sorting (merge sort).
----------------------------------------------------------------------*/
{
  unsigned len1, i;
  L_NODE *lp1, *lp2;

  if (len <= 1)
    return lp;

  len1 = len / 2;
  lp1 = lp;

  for (i = 0; i < len1 - 1; i++)
    lp = lp->s.next;
  lp2 = lp->s.next;

  lp->s.next = NULL;
  lp2->s.prev = NULL;

  return mergeLists(
      sortRec(lp1, len1, compfunc),
      sortRec(lp2, len - len1, compfunc),
      compfunc);
}

/**********************************************************************/
void
ListSort(
LINLIST ls,                       /* specify list */
ListEntryCompareFunc compfunc)    /* function used to compare entries */
/*----------------------------------------------------------------------
Sort the given list. compfunc must return -1 if the first argument
is smaller than the second, 0 if they are equal and 1 if it is larger.
No action if NULL is passed as list.
----------------------------------------------------------------------*/
{
  unsigned i;

  if (ls == NULL || ls->elemNo <= 1)
    return;

  ls->first = sortRec(ls->first, ls->elemNo, compfunc);

  ls->last = ls->first;
  for (i = 0; i < ls->elemNo - 1; i++)
    ls->last = ls->last->s.next;
}


/**********************************************************************/
void *
ListFirst(
LINLIST ls)
/*----------------------------------------------------------------------
Returns first element of list. NULL is considered as empty list.
----------------------------------------------------------------------*/
{
  if (ls == NULL)
    return NULL;

  return MK_DATAP(ls->first);
}


/**********************************************************************/
void *
ListLast(
LINLIST ls)
/*----------------------------------------------------------------------
Returns last element of list. NULL is considered as empty list.
----------------------------------------------------------------------*/
{
  if (ls == NULL)
    return NULL;

  return MK_DATAP(ls->last);
}


/**********************************************************************/
void *
ListNext(
LINLIST ls,
void   *pdata)
/*----------------------------------------------------------------------
Returns element after pdata of list ls.
----------------------------------------------------------------------*/
{
  L_NODE *p = MK_LNODP(pdata);

  return MK_DATAP(p->s.next);
}


/**********************************************************************/
void *
ListPrev(
LINLIST ls,
void   *pdata)
/*----------------------------------------------------------------------
Returns element before pdata of list ls.
----------------------------------------------------------------------*/
{
  L_NODE *p = MK_LNODP(pdata);

  return MK_DATAP(p->s.prev);
}


/**********************************************************************/
void *
ListPos(
LINLIST ls,
int     idx)
/*----------------------------------------------------------------------
Returns element with given index of list, NULL if no such element
exists. NULL is considered as empty list.
----------------------------------------------------------------------*/
{
  L_NODE *p;
  int i;

  if (ls == NULL)
    return NULL;

  if (idx < 0 || idx >= (int) ls->elemNo)
    return NULL;

  p = ls->first;
  for (i = 0; i < idx; i++)
    p = p->s.next;

  return MK_DATAP(p);
}


/**********************************************************************/
unsigned
ListSize(
LINLIST ls)
/*----------------------------------------------------------------------
Returns number of elements in list. Returns 0 if argument is NULL.
----------------------------------------------------------------------*/
{
  if (ls == NULL)
    return 0;

  return ls->elemNo;
}
