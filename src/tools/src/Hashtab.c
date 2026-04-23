/*
************************************************************************
*
*   Hashtab.c - implementation of HASHTAB type
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.Hashtab.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <hashtab.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef double ALIGN;

/*----------------------------------------------------------------------
DESCRIPTION %[
  This package implements a hash table manager. The user (client) of
  this package can put any data items (of equal size) into a table
  without bothering how such a table is implemented.
  All HashtabXxxxx(ht, ... ) functions take as first argument a hash
  table (HASHTABLE) that has been returned by HashtabOpen(). Therefore
  it is possible to have a virtually unlimited number of hash tables at
  the same time, each identified by a unique HASHTABLE. If a NULL
  pointer is passed instead of a HASHTABLE, this is considered to be
  an empty table.
  The hash table always allocates and maintains an internal copy of the
  data which is supplied by the client, therefore a single template
  variable can be used to put data into the table. Manipulations of that
  template afterwards will not change the data in the table.

  Data is always put into and retrieved from the table via pointers,
  because earlier compiler versions did not allow to pass and return
  structures from functions.

  --------------------  example  --------------------

  struct data {
        char    name[20];
        int     age;
  } template, *pdata;

  /\*
     example hash function, similar to HashString()
  *\/
  static unsigned
  hashFunc(void *dataP, unsigned size)
  {
    char c;
    unsigned idx = 0;
    char *key = ((struct data*)dataP)->name;

    while ( (c = *key++) != '\\0')
      idx = 37 * idx + c;
    return idx % size;
  }


  /\*
     example compare function
  *\/
  static int
  compFunc(void *p1, void *p2)
  {
    struct data *p1 = dataP1;
    struct data *p2 = dataP2;
    return strcmp(p1->name, p2->name);
  }


  /\* open the table *\/
  HASHTABLE ht = HashtabOpen(137, sizeof(struct data),
      hashFunc, compFunc);

  /\* fill the table with data *\/
  for(;;) {
    (void) printf("name age ? ");
    n = scanf("%s %d\\n", template.name, &template.age);
    if( n == EOF )
      break;
    else if( n == 2 )
      (void) HashtabInsert(ht, &template, TRUE);
    else
      continue;         /\* ignore *\/
  };
  (void) printf("\\n");

  /\* find the entry with name "Reto" *\/
  template.name = "Reto";
  pdata = HashtabSearch(ht, &template);
  if (pdata == NULL)
    (void) printf("not found\\n");
  else
    (void) printf("age: %d\\n", pdata->age);
  HashtabClose(ht);

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
    union lstnode *nextnode;
  }       s;
  ALIGN   x;
}       L_NODE;                       /* List node */


struct hashTabStruc {                 /* opaque exported as HASHTAB */
  unsigned tabSize;
  L_NODE **tab;
  unsigned dataSize;                  /* rounded up data-size */
  HashFunc hashFunc;
  HashCompareFunc compFunc;
  unsigned elemNo;
  DestroyCBInfo dci;
};

/**********************************************************************/
HASHTABLE
HashtabOpen(
unsigned tabSize,                     /* table size */
unsigned dataSize,                    /* data size */
HashFunc hashFunc,
HashCompareFunc compFunc)
/*----------------------------------------------------------------------
Open a hash table, return hash table or NULL on error.
hashFunc will be called with a pointer to an entry and the hash table
size as parameters. It has to return an unsigned between 0 and size-1.
So the hash function should look like this:
  static unsigned
  myHashFunc(void *ptr, unsigned size)
  {
    struct myEntryStruct *entryP = ptr;
    unsigned hashVal;
    /\* calculate hashVal *\/
    return hashVal % size;
  }
compFunc will be called with two pointers to structrues as parameters.
It has to return 0 if the keys are equal (like strcmp).
  static int
  myCompFunc(void *ptr1, void *ptr2)
  {
    struct myEntryStruct *entry1P = ptr1;
    struct myEntryStruct *entry2P = ptr2;
    return entry1P->key - entry2P->key;
  }
----------------------------------------------------------------------*/
{
  HASHTABLE ht;
  unsigned i;

  ht = malloc(sizeof(struct hashTabStruc));
  if (ht == NULL)
    return NULL;

  ht->tab = malloc(tabSize * sizeof(L_NODE *));
  if (ht->tab == NULL)
    return NULL;

  for (i = 0; i < tabSize; i++)
    ht->tab[i] = NULL;

  ht->tabSize = tabSize;
  ht->dataSize = dataSize;
  ht->hashFunc = hashFunc;
  ht->compFunc = compFunc;
  ht->elemNo = 0;
  ht->dci.destroySiz = 0;
  ht->dci.destroyNum = 0;
  ht->dci.destroyLst = NULL;

  return ht;
}


/**********************************************************************/
void
HashtabAddDestroyCB(
HASHTABLE ht,             /* specify hash table */
DestroyCB callback,       /* specify destroy callback function */
void   *clientdta,        /* specify clientdata for callback */
FreeCldta freecldta)      /* specify free clientdata function or NULL */
/*----------------------------------------------------------------------
Add Destroy callback function.
The destroy callback which will be called for every entry at
HashtabRemove() and HashtabClose(). This can be used to free data
allocated externally by the user and referenced in the entry etc.
If (freecldta != NULL) it will be called to free the clientdta at
HashtabRemoveDestroyCB() or after the callback function has been called
at HashtabClose().
----------------------------------------------------------------------*/
{
  (void) _AddDestroyCB(&ht->dci, callback, clientdta, freecldta);
}


/**********************************************************************/
void
HashtabRemoveDestroyCB(
HASHTABLE ht,
DestroyCB callback,
void   *clientdta)
/*----------------------------------------------------------------------
Remove destroy callback function and free clientdta.
----------------------------------------------------------------------*/
{
  _RemoveDestroyCB(&ht->dci, callback, clientdta);
}


/**********************************************************************/
void
HashtabClose(
HASHTABLE ht)             /* specify hash table to be closed */
/*----------------------------------------------------------------------
Close the table and destroy all entries (freeing the memory).
No action if NULL is passed.
Note: if the nodes contain ptr's to additional data allocated by the
      user, it is the user's responsibility to free all that data,
      otherwise the memory is lost. This can be avoided by registering
      a destroy callback function using HashtabAddDestroyCB().
----------------------------------------------------------------------*/
{
  unsigned tabIdx;
  L_NODE *p1, *p2;

  if (ht == NULL)
    return;

  for (tabIdx = 0; tabIdx < ht->tabSize; tabIdx++) {
    p1 = ht->tab[tabIdx];
    while (p1 != NULL) {
      p2 = p1->s.nextnode;              /* remember next node */
      _ExecAllDestroyCB(&ht->dci, MK_DATAP(p1));
      free(p1);
      p1 = p2;
    }
  }

  free(ht->tab);

  _RemoveAllDestroyCB(&ht->dci);

  free(ht);
}

/**********************************************************************/
void
HashtabApply(
HASHTABLE ht,                    /* specify hash table to be closed */
HashApplyCB applyfunc,           /* function applied for each element */
void   *cl_dta)                  /* second arg to applyfunc */
/*----------------------------------------------------------------------
Apply the function 'applyfunc' to every element in the hash table.
It is legal to pass NULL as hash table, this is equivalent to an
empty table.
Applyfunc's 1st argument is a ptr to the data in the table, the second
argument is cl_dta.
      applyfunc( void *pdata, void* cl_dta );

Note: Applyfunc can safely remove the data (*pdata) using
      HashtabRemove(). But it may not remove any other data from the
      hash list that has the same hashindex as (*pdata).
----------------------------------------------------------------------*/
{
  unsigned tabIdx;
  L_NODE *p1, *p2;

  if (ht == NULL)
    return;

  for (tabIdx = 0; tabIdx < ht->tabSize; tabIdx++) {
    p1 = ht->tab[tabIdx];
    while (p1 != NULL) {
      p2 = p1->s.nextnode;       /* in case function removes p1 !! */
      (*applyfunc) (MK_DATAP(p1), cl_dta);
      p1 = p2;                   /* remember next node */
    }
  }
}


/**********************************************************************/
void *
HashtabInsert(
HASHTABLE ht,
void   *pdata,
BOOL    testUnique)
/*----------------------------------------------------------------------
Insert *pdata into hash table.
If testUnique is TRUE, the function tests whether an entry with the
same key is already in the table and returns a pointer to this entry
in this case.
If testUnique is FALSE, no test is done.
Use FALSE only if you are sure that it is impossible that the entry is
already in the table.
Returns ptr to new data location, or NULL if error.
----------------------------------------------------------------------*/
{
  unsigned tabIdx;
  L_NODE *firstP, *p, *newP;
  void   *newDtaP;

  tabIdx = (*ht->hashFunc) (pdata, ht->tabSize);

  firstP = ht->tab[tabIdx];
  if (testUnique) {
    p = firstP;
    while (p != NULL && ht->compFunc(pdata, MK_DATAP(p)))
      p = p->s.nextnode;
    if (p != NULL)
      return MK_DATAP(p);
  }
  /* allocate memory for new node */
  newP = malloc((unsigned) NODSIZ + ht->dataSize);
  if (newP == NULL)
    return NULL;

  newP->s.nextnode = firstP;
  ht->tab[tabIdx] = newP;

  ht->elemNo++;

  /* transfer data to new location */
  newDtaP = MK_DATAP(newP);
  (void) memcpy(newDtaP, pdata, (int) ht->dataSize);
  return newDtaP;
}


/**********************************************************************/
void *
HashtabSearch(
HASHTABLE ht,
void   *pdata)
/*----------------------------------------------------------------------
Search *pdata in hash table. Passing NULL as table is legal, this is
equivalent to an empty table, the data will never be found in this case.
Returns ptr to location of found data, or NULL if not found.
----------------------------------------------------------------------*/
{
  unsigned tabIdx;
  L_NODE *p;

  if (ht == NULL)
    return NULL;

  tabIdx = (*ht->hashFunc) (pdata, ht->tabSize);

  p = ht->tab[tabIdx];
  while (p != NULL && ht->compFunc(pdata, MK_DATAP(p)))
    p = p->s.nextnode;

  return MK_DATAP(p);
}


/**********************************************************************/
void
HashtabRemove(
HASHTABLE ht,
void   *pdata)
/*----------------------------------------------------------------------
Remove *pdata from hash table.
----------------------------------------------------------------------*/
{
  unsigned tabIdx;
  L_NODE *p, *nextP;

  tabIdx = (*ht->hashFunc) (pdata, ht->tabSize);

  p = ht->tab[tabIdx];
  if (p == NULL)
    return;

  if (ht->compFunc(pdata, MK_DATAP(p)) == 0)  /* delete first element */
    ht->tab[tabIdx] = p->s.nextnode;
  else {
    for (;;) {
      nextP = p->s.nextnode;
      if (nextP == NULL)
        return;
      if (ht->compFunc(pdata, MK_DATAP(nextP)) == 0)
        break;
      p = nextP;
    }
    p->s.nextnode = nextP->s.nextnode;
    p = nextP;
  }

  ht->elemNo--;

  _ExecAllDestroyCB(&ht->dci, MK_DATAP(p));
  free(p);
}

/**********************************************************************/
unsigned
HashtabSize(
HASHTABLE ht)
/*----------------------------------------------------------------------
Returns number of elements in hash table. Returns 0 if argument is NULL.
----------------------------------------------------------------------*/
{
  if (ht == NULL)
    return 0;

  return ht->elemNo;
}
