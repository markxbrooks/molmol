/*
************************************************************************
*
*   Tree.c - implement TREE type
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
*   Pathname of SCCS file     : /sgiext/molmol/tools/src/SCCS/s.Tree.c
*   SCCS identification       : 1.2
*
************************************************************************
*/
#include <tree.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef double ALIGN;

/*----------------------------------------------------------------------
DESCRIPTION %[
  This package implements a tree manager. The user (client) of
  this package can put any data items (of equal size) into a tree
  without bothering how such a tree is implemented.
  All TreeXxxxx(tr, ... ) functions take as first argument a tree
  (TREE) that has been returned by TreeOpen(). Therefore it is
  possible to have a virtually unlimited number of trees at the
  same time, each identified by a unique TREE. If a NULL pointer
  is passed instead of a TREE, this is considered to be an empty tree.
  The tree always allocates and maintains an internal copy of the
  data which is supplied by the client, therefore a single template
  variable can be used to put data into the tree. Manipulations of that
  template afterwards will not change the data in the tree.

  Data is always put into and retrieved from the tree via pointers,
  because earlier compiler versions did not allow to pass and return
  structures from functions.

  --------------------  example  --------------------
  struct data {
        char    name[20];
        int     age;
  } template, *pdata;
  static BOOL
  compFunc(data1P, data2P)
  struct data *data1P, *data2P;
  {
    return strcmp(data1P->name, data2P->name);
  }
  /\* open the tree *\/
  TREE tr = TreeOpen(sizeof(struct data), compFunc);
  /\*
   * fill the tree with data
   *\/
  for(;;) {
    (void) printf("name age ? ");
    n = scanf("%s %d\\n", template.name, &template.age);
    if( n == EOF )
      break;
    else if( n == 2 )
      (void) TreeInsert(tr, &template, TIB_OVERWRITE);
    else
      continue;         /\* ignore *\/
  };
  (void) printf("\\n");
  /\*
   * find the entry with name "Reto"
   *\/
  template.name = "Reto";
  pdata = TreeSearch(tr, &template);
  if (pdata == NULL)
    (void) printf("not found\\n");
  else
    (void) printf("age: %d\\n", pdata->age);
  TreeClose(tr);

  ------------------  end example  ------------------
%]
----------------------------------------------------------------------*/


/* macro to round up nodesize to multiples of ALIGN-size in bytes */
#define NODSIZ  \
  ( ((sizeof(T_NODE)-1) / sizeof(ALIGN) + 1) * sizeof(ALIGN) )

/* make a listnode pointer from data pointer */
#define MK_TNODP(p)     ( (p) ? (T_NODE*) ((char*)(p) - NODSIZ) : NULL )

/* make a data pointer from listnode pointer */
#define MK_DATAP(p)     ( (p) ? (void *) ((char*)(p) + NODSIZ) : NULL )

/***********************************************************************
        TYPES
***********************************************************************/

typedef union treenode {
  struct {
    union treenode *left, *right;
  }       s;
  ALIGN   x;
}       T_NODE;                       /* Tree node */


struct treeStruc {                    /* opaque exported as TREE */
  T_NODE *root;
  unsigned dataSize;                  /* rounded up data-size */
  TreeCompareFunc compFunc;
  unsigned elemNo;
  DestroyCBInfo dci;
};


/**********************************************************************/
TREE
TreeOpen(
unsigned dataSize,                    /* data size */
TreeCompareFunc compFunc)
/*----------------------------------------------------------------------
Open a tree, return tree or NULL on error.
compFunc will be called with two pointers to structrues as parameters.
It has to return:
  -1  if first key < second key
  0   if first key == second key
  1   if first key > second key
Example:
  static BOOL
  myCompFunc(void *ptr1, void *ptr2)
  {
    struct myEntryStruct *entry1P =  ptr1;
    struct myEntryStruct *entry2P =  ptr2;
    return strcmp(entry1P->key, entry2P->key);
  }
----------------------------------------------------------------------*/
{
  TREE tr;

  tr = malloc(sizeof(struct treeStruc));
  if (tr == NULL)
    return NULL;

  tr->root = NULL;
  tr->dataSize = dataSize;
  tr->compFunc = compFunc;
  tr->elemNo = 0;
  tr->dci.destroySiz = 0;
  tr->dci.destroyNum = 0;
  tr->dci.destroyLst = NULL;

  return tr;
}


/**********************************************************************/
void
TreeAddDestroyCB(
TREE    tr,               /* specify tree */
DestroyCB callback,       /* specify destroy callback function */
void   *clientdta,        /* specify clientdata for callback */
FreeCldta freecldta)      /* specify free clientdata function or NULL */
/*----------------------------------------------------------------------
Add Destroy callback function.
The destroy callback which will be called for every entry at
TreeRemove() and TreeClose(). This can be used to free data allocated
externally by the user and referenced in the entry etc.
If (freecldta != NULL) it will be called to free the clientdta at
TreeRemoveDestroyCB() or after the callback function has been called at
TreeClose().
----------------------------------------------------------------------*/
{
  (void) _AddDestroyCB(&tr->dci, callback, clientdta, freecldta);
}


/**********************************************************************/
void
TreeRemoveDestroyCB(
TREE    tr,
DestroyCB callback,
void   *clientdta)
/*----------------------------------------------------------------------
Remove destroy callback function and free clientdta.
----------------------------------------------------------------------*/
{
  _RemoveDestroyCB(&tr->dci, callback, clientdta);
}


static void
freeTree(
TREE    tr,
T_NODE *nodeP)
{
  if (nodeP == NULL)
    return;

  freeTree(tr, nodeP->s.left);
  freeTree(tr, nodeP->s.right);

  _ExecAllDestroyCB(&tr->dci, MK_DATAP(nodeP));
  free(nodeP);
}


/**********************************************************************/
void
TreeClose(
TREE    tr)               /* specify tree to be closed */
/*----------------------------------------------------------------------
Close the tree and destroy all entries (freeing the memory).
No action if NULL is passed.
Note: if the nodes contain ptr's to additional data allocated by the
      user, it is the user's responsibility to free all that data,
      otherwise the memory is lost. This can be avoided by registering
      a destroy callback function using TreeAddDestroyCB().
----------------------------------------------------------------------*/
{
  if (tr == NULL)
    return;

  freeTree(tr, tr->root);
  _RemoveAllDestroyCB(&tr->dci);
  free(tr);
}


static void
applyTree(
T_NODE *nodeP,
TreeApplyFunc applyfunc,  /* function applied for each element */
void   *cl_dta)           /* second arg to applyfunc */
{
  if (nodeP == NULL)
    return;

  applyTree(nodeP->s.left, applyfunc, cl_dta);
  applyfunc(MK_DATAP(nodeP), cl_dta);
  applyTree(nodeP->s.right, applyfunc, cl_dta);
}


/**********************************************************************/
void
TreeApply(
TREE    tr,               /* specify tree */
TreeApplyFunc applyfunc,  /* function applied for each element */
void   *cl_dta)           /* second arg to applyfunc */
/*----------------------------------------------------------------------
Apply the function 'applyfunc' to every element in the tree.
It is legal to pass NULL as tree, this is equivalent to an empty tree.
Applyfunc's 1st argument is a ptr to the data in the tree, the second
argument is cl_dta.
      applyfunc( void *pdata, void* cl_dta );
----------------------------------------------------------------------*/
{
  if (tr == NULL)
    return;

  applyTree(tr->root, applyfunc, cl_dta);
}


static T_NODE **
searchTree(
T_NODE **nodePP,
void    *pdata,
TreeCompareFunc compFunc)
{
  int     compRes;

  if (*nodePP == NULL)
    return nodePP;        /* not found */

  compRes = compFunc(pdata, MK_DATAP(*nodePP));

  if (compRes < 0)
    return searchTree(&(*nodePP)->s.left, pdata, compFunc);
  else if (compRes > 0)
    return searchTree(&(*nodePP)->s.right, pdata, compFunc);
  else
    return nodePP;        /* found */
}


/**********************************************************************/
void *
TreeInsert(
TREE    tr,
void   *pdata,
TreeInsBehav_E behav)
/*----------------------------------------------------------------------
Insert *pdata into tree.
The behaviour if there is already an entry with the same key depends
on the parameter behav:
  TIB_RETURN_NULL :  return NULL
  TIB_RETURN_OLD  :  return the entry already in the tree
  TIB_OVERWRITE   :  overwrite the old entry and return the new one
----------------------------------------------------------------------*/
{
  T_NODE **foundPP, *newP;
  void   *newDtaP;

  foundPP = searchTree(&tr->root, pdata, tr->compFunc);

  if (*foundPP == NULL) {
    /* allocate memory for new node */
    newP = malloc((unsigned) NODSIZ + tr->dataSize);
    if (newP == NULL)
      return NULL;
    newP->s.left = newP->s.right = NULL;
    *foundPP = newP;
  } else if (behav == TIB_RETURN_NULL) {
    return NULL;
  } else if (behav == TIB_RETURN_OLD) {
    return MK_DATAP(*foundPP);
  } else {
    _ExecAllDestroyCB(&tr->dci, MK_DATAP(*foundPP));
    newP = *foundPP;
  }

  tr->elemNo++;

  /* transfer data to new location */
  newDtaP = MK_DATAP(newP);
  (void) memcpy(newDtaP, pdata, (int) tr->dataSize);
  return newDtaP;
}


/**********************************************************************/
void *
TreeSearch(
TREE    tr,
void   *pdata)
/*----------------------------------------------------------------------
Search *pdata in tree. Passing NULL as tree is legal, this is
equivalent to an empty tree, the data will never be found in this case.
Returns ptr to location of found data, or NULL if not found.
----------------------------------------------------------------------*/
{
  T_NODE **foundPP;

  if (tr == NULL)
    return NULL;

  foundPP = searchTree(&tr->root, pdata, tr->compFunc);

  return MK_DATAP(*foundPP);
}


/**********************************************************************/
void
TreeRemove(
TREE    tr,
void   *pdata)
/*----------------------------------------------------------------------
Remove *pdata from tree.
----------------------------------------------------------------------*/
{
  T_NODE **foundPP, *foundP, **movePP, *moveP;

  foundPP = searchTree(&tr->root, pdata, tr->compFunc);
  if (*foundPP == NULL)
    return;

  foundP = *foundPP;

  if (foundP->s.right == NULL)
    *foundPP = foundP->s.left;
  else if (foundP->s.left == NULL)
    *foundPP = foundP->s.right;
  else {
    /* replace foundP by rightmost entry of left subtree (moveP) */
    movePP = &foundP->s.left;
    while ((*movePP)->s.right != NULL)
      movePP = &(*movePP)->s.right;
    moveP = *movePP;
    *movePP = moveP->s.left;
    moveP->s.left = foundP->s.left;
    moveP->s.right = foundP->s.right;
    *foundPP = moveP;
  }

  tr->elemNo--;

  _ExecAllDestroyCB(&tr->dci, MK_DATAP(foundP));
  free(foundP);
}

/**********************************************************************/
unsigned
TreeSize(
TREE tr)
/*----------------------------------------------------------------------
Returns number of elements in tree. Returns 0 if argument is NULL.
----------------------------------------------------------------------*/
{
  if (tr == NULL)
    return 0;

  return tr->elemNo;
}

static void
printTree(
T_NODE *nodeP,
int     level,
TreePrintFunc printFunc)
{
  int     i;

  if (nodeP == NULL)
    return;

  printTree(nodeP->s.right, level + 1, printFunc);

  for (i = 0; i < level; i++)
    (void) printf("  ");
  printFunc(MK_DATAP(nodeP));
  (void) printf("\n");

  printTree(nodeP->s.left, level + 1, printFunc);
}


/**********************************************************************/
void
TreePrint(
TREE    tr,
TreePrintFunc printFunc)
/*----------------------------------------------------------------------
Used for debugging.
Print the given tree. printFunc is called with a pointer to each entry.
----------------------------------------------------------------------*/
{
  if (tr == NULL)
    return;

  printTree(tr->root, 0, printFunc);
}
