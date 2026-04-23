/*
************************************************************************
*
*   ExWriteSeq.c - WriteSeq command
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdio/SCCS/s.ExWriteSeq.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>

#include <g_file.h>
#include <curr_dir.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>

typedef struct {
  GFile gf;
  BOOL ok;
} WriteData;

static void
writeRes(DhResP resP, void *clientData)
{
  WriteData *dataP = clientData;

  if (! dataP->ok)
    return;

  dataP->ok = (GFileWriteLine(dataP->gf,
      DStrToStr(DhResGetName(resP))) == GF_RES_OK);
}

ErrCode
ExWriteSeq(char *cmd)
{
  GFile gf;
  DSTR fileName;
  ErrCode errCode;
  DhMolP molP;
  PropRefP refP;
  WriteData writeData;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.seq", FALSE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  if (SelMolGet(&molP, 1) != 1) {
    CipSetError("exactly 1 molecule must be selected");
    return EC_ERROR;
  }

  gf = GFileOpenNew(DStrToStr(fileName), GF_FORMAT_ASCII);
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  refP = PropGetRef(PROP_ALL, FALSE);

  writeData.gf = gf;
  writeData.ok = TRUE;
  DhMolApplyRes(refP, molP, writeRes, &writeData);

  GFileClose(gf);

  if (! writeData.ok)
    return EC_ERROR;

  return EC_OK;
}
