/*
************************************************************************
*
*   ExDump.c - ReadDump and WriteDump commands
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
*   Date of last modification : 99/10/17
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdio/SCCS/s.ExDump.c
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>
#include <string.h>

#include <sg.h>
#include <g_file.h>
#include <prog_vers.h>
#include <curr_dir.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <graph_draw.h>

static void
addMol(DhMolP molP, void *clientData)
{
  GraphMolAdd(molP);
}

ErrCode
ExReadDump(char *cmd)
{
  DSTR fileName;
  ErrCode errCode;
  GFile gf;
  INT16 vers;
  char currDev[20];
  BOOL oldFeatures[SG_FEATURE_NO];
  BOOL ok;
  int i;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.mml", TRUE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenRead(DStrToStr(fileName));
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  ok = (GFileReadINT16(gf, &vers) == GF_RES_OK);
  if (ok && vers < VERS_DUMP_MIN) {
    CipSetError("too old version of dump file");
    return EC_ERROR;
  }
  if (ok && vers > VERS_DUMP) {
    CipSetError("too new version of dump file (corrupted?)");
    return EC_ERROR;
  }

  (void) strcpy(currDev, ParGetStrVal(PN_GRAPH_DEV));
  for (i = 0; i < SG_FEATURE_NO; i++)
    oldFeatures[i] = ParGetIntArrVal(PN_RENDERING, i);

  ok = (ok && ParUndump(gf) == GF_RES_OK);
  if (ok) {
    GraphDrawInit();  /* initialize with new parameters */

    if (strcmp(ParGetStrVal(PN_GRAPH_DEV), currDev) == 0 &&
	ParGetArrSize(PN_RENDERING) == SG_FEATURE_NO) {
      /* same device, use values from dump file */
      for (i = 0; i < SG_FEATURE_NO; i++)
	SgSetFeature(i, ParGetIntArrVal(PN_RENDERING, i));
    } else {
      /* different device, restore previous values */
      for (i = 0; i < SG_FEATURE_NO; i++)
	ParSetIntArrVal(PN_RENDERING, i, oldFeatures[i]);
    }

    DhDestroyAll();
    PrimDestroyAll();
  }

  ok = ok && DhUndump(gf, vers);
  ok = ok && PrimUndump(gf, vers);

  GFileClose(gf);

  ParSetStrVal(PN_GRAPH_DEV, currDev);

  if (! ok)
    return EC_ERROR;

  DhApplyMol(PropGetRef(PROP_ALL, FALSE), addMol, NULL);
  GraphRedrawNeeded();

  return EC_OK;
}

ErrCode
ExWriteDump(char *cmd)
{
  GFile gf;
  DSTR fileName;
  ErrCode errCode;
  BOOL ok;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.mml", FALSE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenNew(DStrToStr(fileName), GF_FORMAT_BINARY);
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  ok = (GFileWriteINT16(gf, VERS_DUMP) == GF_RES_OK);
  ok = (ok && ParDump(gf) == GF_RES_OK);
  ok = ok && DhDump(gf);
  ok = ok && PrimDump(gf);

  GFileClose(gf);

  if (! ok)
    return EC_ERROR;

  return EC_OK;
}
