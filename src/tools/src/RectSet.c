/*
************************************************************************
*
*   RectSet.c - implementation of RECTSET type
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
*   Pathname of SCCS file     : /sgiext/molmol/tools/src/SCCS/s.RectSet.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <rect_set.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bool.h>

#define REPORT_PERF 0

typedef double ALIGN;

/*----------------------------------------------------------------------
DESCRIPTION %[
  This package implements a manager for a set of rectangles. It's main
  purpose is to allow for efficient search of rectangles in a big set
  that overlap a given rectangle. The user (client) of this package
  can store any data items (of equal size) together with the rectangles.
  The data items MUST have a Rect at the beginning, they will normally
  be structs with a field of type Rect as a first field.
  The rectangle set manager always allocates and maintains an internal
  copy of the data which is supplied by the client.
%]
----------------------------------------------------------------------*/

/* macro to round up nodesize to multiples of ALIGN-size in bytes */
#define NODSIZ  \
  ( ((sizeof(T_NODE)-1) / sizeof(ALIGN) + 1) * sizeof(ALIGN) )

/* make a listnode pointer from data pointer */
#define MK_TNODP(p)     ( (p) ? (T_NODE*) ((char*)(p) - NODSIZ) : NULL )

/* make a data pointer from listnode pointer */
#define MK_DATAP(p)     ( (p) ? (void *) ((char*)(p) + NODSIZ) : NULL )

typedef union treenode {
  struct {
    union treenode *left, *right, *parent;
    int coordInd;
  }       s;
  ALIGN   x;
}       T_NODE;                       /* Tree node */

struct rectSetStruc {                 /* opaque exported as RCTSET */
  T_NODE *root;
  unsigned dataSize;
  unsigned elemNo;
  DestroyCBInfo dci;
};

#if REPORT_PERF
static int InsertOps;
static int IntersectOps;
static int IntersectHits;
#endif

/**********************************************************************/
RECTSET
RectSetOpen(
unsigned dataSize)                    /* data size */
/*----------------------------------------------------------------------
Open a rectangle set, return rectangle set or NULL on error.
----------------------------------------------------------------------*/
{
  RECTSET rs;

  rs = malloc(sizeof(struct rectSetStruc));
  if (rs == NULL)
    return NULL;
  
  rs->root = 0;
  rs->dataSize = dataSize;
  rs->elemNo = 0;
  rs->dci.destroySiz = 0;
  rs->dci.destroyNum = 0;
  rs->dci.destroyLst = NULL;

  return rs;
}

static void
freeTree(
RECTSET rs,
T_NODE *nodeP)
{
  if (nodeP == NULL)
    return;

  freeTree(rs, nodeP->s.left);
  freeTree(rs, nodeP->s.right);

  _ExecAllDestroyCB(&rs->dci, MK_DATAP(nodeP));
  free(nodeP);
}

/**********************************************************************/
void
RectSetClose(
RECTSET rs)               /* specify rectangle set to be closed */
/*----------------------------------------------------------------------
Close the rectangle set and destroy all entries (freeing the memory).
Note: if the nodes contain ptr's to additional data allocated by the
      user, it is the user's responsibility to free all that data,
      otherwise the memory is lost. This can be avoided by registering
      a destroy callback function using RectSetAddDestroyCB().
----------------------------------------------------------------------*/
{
  freeTree(rs, rs->root);

  _RemoveAllDestroyCB(&rs->dci);

  free(rs);
}

static void
applyTree(
T_NODE *nodeP,
RectSetApplyFunc applyfunc,  /* function applied for each element */
void   *cl_dta)              /* second arg to applyfunc */
{
  if (nodeP == NULL)
    return;

  applyTree(nodeP->s.left, applyfunc, cl_dta);
  applyTree(nodeP->s.right, applyfunc, cl_dta);
  applyfunc(MK_DATAP(nodeP), cl_dta);
}

/**********************************************************************/
void
RectSetApply(
RECTSET rs,                  /* specify rectangle set */
RectSetApplyFunc applyfunc,  /* function applied for each element */
void *cl_dta)                /* second arg to applyfunc */
/*----------------------------------------------------------------------
Apply the function 'applyfunc' to every element in the tree.
Applyfunc's 1st argument is a ptr to the data in the tree, the second
argument is cl_dta.
      applyfunc( void *pdata, void* cl_dta );
----------------------------------------------------------------------*/
{
  applyTree(rs->root, applyfunc, cl_dta);
}

/**********************************************************************/
void
RectSetAddDestroyCB(
RECTSET rs,               /* specify rectangle set */
DestroyCB callback,       /* specify destroy callback function */
void   *clientdta,        /* specify clientdata for callback */
FreeCldta freecldta)      /* specify free clientdata function or NULL */
/*----------------------------------------------------------------------
Add Destroy callback function.
The destroy callback which will be called for every entry at
RectSetRemove() and RectSetClose(). This can be used to free data
allocated externally by the user and referenced in the entry etc.
If (freecldta != NULL) it will be called to free the clientdta at
RectSetRemoveDestroyCB() or after the callback function has been called
at RectSetClose().
----------------------------------------------------------------------*/
{
  (void) _AddDestroyCB(&rs->dci, callback, clientdta, freecldta);
}

/**********************************************************************/
void
RectSetRemoveDestroyCB(
RECTSET rs,
DestroyCB callback,
void   *clientdta)
/*----------------------------------------------------------------------
Remove destroy callback function and free clientdta.
----------------------------------------------------------------------*/
{
  _RemoveDestroyCB(&rs->dci, callback, clientdta);
}

/**********************************************************************/
static void
insertNode(
T_NODE *parentP,
T_NODE *newP)
/*----------------------------------------------------------------------
Insert *pdata into rectangle set.
----------------------------------------------------------------------*/
{
  T_NODE *childP;
  Rect *newRectP, *rectP;
  BOOL goLeft;

  newRectP = MK_DATAP(newP);

  for (;;) {

#if REPORT_PERF
    InsertOps++;
#endif

    rectP = MK_DATAP(parentP);
    switch (parentP->s.coordInd) {
      case 0:
	goLeft = (newRectP->xMin < rectP->xMin);
	break;
      case 1:
	goLeft = (newRectP->xMax < rectP->xMax);
	break;
      case 2:
	goLeft = (newRectP->yMin < rectP->yMin);
	break;
      case 3:
	goLeft = (newRectP->yMax < rectP->yMax);
	break;
    }

    if (goLeft)
      childP = parentP->s.left;
    else
      childP = parentP->s.right;

    if (childP == NULL)
      break;
    
    parentP = childP;
  }

  if (goLeft)
    parentP->s.left = newP;
  else
    parentP->s.right = newP;
  
  newP->s.left = newP->s.right = NULL;
  newP->s.parent = parentP;
  newP->s.coordInd = (parentP->s.coordInd + 1) % 4;
}

/**********************************************************************/
void *
RectSetInsert(
RECTSET rs,
void   *pdata)
/*----------------------------------------------------------------------
Insert *pdata into rectangle set.
----------------------------------------------------------------------*/
{
  T_NODE *newP;
  void *newDtaP;

  newP = malloc(NODSIZ + rs->dataSize);
  if (newP == NULL)
    return NULL;
  newDtaP = MK_DATAP(newP);
  (void) memcpy(newDtaP, pdata, rs->dataSize);

#if REPORT_PERF
    InsertOps = 0;
#endif

  if (rs->root == NULL) {
    rs->root = newP;
    newP->s.left = newP->s.right = NULL;
    newP->s.parent = NULL;
    newP->s.coordInd = 0;
  } else {
    insertNode(rs->root, newP);
  }

  rs->elemNo++;

#if REPORT_PERF
  (void) printf("RectSetInsert: %d elements, %d ops\n",
      rs->elemNo, InsertOps);
#endif

  return newDtaP;
}

/**********************************************************************/
static void
insertTree(
T_NODE *parentP,
T_NODE *insertP)
{
  T_NODE *leftP, *rightP;

  if (insertP == NULL)
    return;

  leftP = insertP->s.left;
  rightP = insertP->s.right;

  insertNode(parentP, insertP);
  insertTree(parentP, leftP);
  insertTree(parentP, rightP);
}

/**********************************************************************/
void
RectSetRemove(
RECTSET rs,
void   *pdata)
/*----------------------------------------------------------------------
Remove *pdata from rectangle set.
----------------------------------------------------------------------*/
{
  T_NODE *childP, *parentP, **childPP;

  childP = MK_TNODP(pdata);
  parentP = childP->s.parent;
  if (parentP == NULL)
    childPP = &rs->root;
  else if (parentP->s.left == childP)
    childPP = &parentP->s.left;
  else
    childPP = &parentP->s.right;

#if REPORT_PERF
    InsertOps = 0;
#endif

  if (childP->s.right == NULL) {
    *childPP = childP->s.left;
  } else {
    *childPP = childP->s.right;
    insertTree(childP->s.right, childP->s.left);
  }

  if (*childPP != NULL)
    (*childPP)->s.parent = parentP;

  rs->elemNo--;

#if REPORT_PERF
  (void) printf("RectSetRemove: %d elements, %d ops\n",
      rs->elemNo, InsertOps);
#endif

  _ExecAllDestroyCB(&rs->dci, pdata);

  free(childP);
}

/**********************************************************************/
static void
findOverlap(
T_NODE *nodeP,
Rect   *searchRectP,
RectSetApplyFunc applyfunc,
void   *cl_dta)
{
  Rect *rectP;
  BOOL searchLeft, searchRight;

  if (nodeP == NULL)
    return;

#if REPORT_PERF
  IntersectOps++;
#endif

  rectP = MK_DATAP(nodeP);

  switch (nodeP->s.coordInd) {
    case 0:
      searchLeft = TRUE;
      searchRight = (searchRectP->xMax > rectP->xMin);
      break;
    case 1:
      searchLeft = (searchRectP->xMin < rectP->xMax);
      searchRight = TRUE;
      break;
    case 2:
      searchLeft = TRUE;
      searchRight = (searchRectP->yMax > rectP->yMin);
      break;
    case 3:
      searchLeft = (searchRectP->yMin < rectP->yMax);
      searchRight = TRUE;
      break;
  }

  if (searchLeft)
    findOverlap(nodeP->s.left, searchRectP, applyfunc, cl_dta);
  if (searchRight)
    findOverlap(nodeP->s.right, searchRectP, applyfunc, cl_dta);

  if (searchRectP->xMin < rectP->xMax &&
      searchRectP->xMax > rectP->xMin &&
      searchRectP->yMin < rectP->yMax &&
      searchRectP->yMax > rectP->yMin) {
#if REPORT_PERF
    IntersectHits++;
#endif
    applyfunc(MK_DATAP(nodeP), cl_dta);
  }
}

/**********************************************************************/
void
RectSetFindOverlap(
RECTSET rs,
Rect   *rectP,
RectSetApplyFunc applyfunc,
void   *cl_dta)
/*----------------------------------------------------------------------
Call applyfunc for all entries in the given rectangle set that overlap
with the given rectangle.
----------------------------------------------------------------------*/
{
#if REPORT_PERF
  IntersectOps = 0;
  IntersectHits = 0;
#endif

  findOverlap(rs->root, rectP, applyfunc, cl_dta);

#if REPORT_PERF
  (void) printf("RectSetFindOverlap: %d elements, %d ops, %d hits\n",
      rs->elemNo, IntersectOps, IntersectHits);
#endif
}

/**********************************************************************/
unsigned
RectSetSize(
RECTSET rs)
/*----------------------------------------------------------------------
Returns number of elements in the rectangle set.
----------------------------------------------------------------------*/
{
  return rs->elemNo;
}

static void
printTree(
T_NODE *nodeP,
int     level)
{
  int     i;
  Rect *rectP;

  if (nodeP == NULL)
    return;

  printTree(nodeP->s.right, level + 1);

  rectP = (MK_DATAP(nodeP));

  for (i = 0; i < level; i++)
    (void) printf("  ");
  (void) printf("xMin: %f xMax: %f yMin: %f yMax %f  ",
      rectP->xMin, rectP->xMax, rectP->yMin, rectP->yMax);
  switch (nodeP->s.coordInd) {
    case 0:
      (void) printf("split xMin\n");
      break;
    case 1:
      (void) printf("split xMax\n");
      break;
    case 2:
      (void) printf("split yMin\n");
      break;
    case 3:
      (void) printf("split yMax\n");
      break;
  }

  printTree(nodeP->s.left, level + 1);
}

/**********************************************************************/
void
RectSetPrint(
RECTSET rs)
/*----------------------------------------------------------------------
Used for debugging. Print the given rectangle set.
----------------------------------------------------------------------*/
{
  printTree(rs->root, 0);
}
