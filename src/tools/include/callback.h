/*
************************************************************************
*
*   callback.h - callback handling for opaque container types
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.callback.h
*   SCCS identification       : 1.1
*
************************************************************************
*/
#ifndef _CALLBACK_H_
#define _CALLBACK_H_

/* define callback function types */ 
typedef void (*DestroyCB) (void *, void *);
typedef void (*FreeCldta) (void *);

typedef struct {
  DestroyCB callback;
  void     *clientdta;
  FreeCldta freecldta;
} DestroyCBRec;			/* destroy callback record */

typedef struct {
  int           destroySiz;     /* size of destroy list */
  int           destroyNum;     /* number of callbacks in list */
  DestroyCBRec *destroyLst;     /* list (array) of DestoyCBRec */ 
} DestroyCBInfo;		/* destroy callback info */

extern void *_AddDestroyCB(DestroyCBInfo *, DestroyCB, void *, FreeCldta);
extern void _RemoveDestroyCB(DestroyCBInfo *, DestroyCB, void *);
extern void _RemoveAllDestroyCB(DestroyCBInfo *);
extern void _ExecAllDestroyCB(DestroyCBInfo *, void *);

#endif /* _CALLBACK_H_ */
