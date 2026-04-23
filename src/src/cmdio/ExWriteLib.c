/*
************************************************************************
*
*   ExWriteLib.c - WriteLib command
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
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdio/SCCS/s.ExWriteLib.c
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

  dataP->ok = DhResDefWrite(dataP->gf, resP);
}

ErrCode
ExWriteLib(char *cmd)
{
  GFile gf;
  DSTR fileName;
  ErrCode errCode;
  WriteData writeData;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.lib", FALSE);
  if (errCode != EC_OK) {
    DStrFree(fileName);
    return errCode;
  }

  gf = GFileOpenNew(DStrToStr(fileName), GF_FORMAT_ASCII);
  DStrFree(fileName);
  if (gf == NULL)
    return EC_ERROR;

  writeData.gf = gf;
  writeData.ok = TRUE;
  DhApplyRes(PropGetRef(PROP_SELECTED, FALSE), writeRes, &writeData);

  GFileClose(gf);

  if (! writeData.ok)
    return EC_ERROR;

  return EC_OK;
}
