/*
************************************************************************
*
*   ExWriteAng.c - WriteAng command
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
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdio/SCCS/s.ExWriteAng.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_io.h>

#include <stdio.h>

#include <break.h>
#include <g_file.h>
#include <prog_vers.h>
#include <curr_dir.h>
#include <arg.h>
#include <data_hand.h>

#define LINE_LEN 100

typedef struct {
  GFile gf;
  int resNum;
  int pos;
  BOOL ok;
} WriteData;

#define CHECK_RES(s) if (s != GF_RES_OK) {dataP->ok = FALSE; return;}

static void
writeAng(DhAngleP angleP, void *clientData)
{
  WriteData *dataP = clientData;
  GFile gf = dataP->gf;
  char buf[30];
  DhResP resP;
  int resNum;

  if (! dataP->ok)
    return;

  resP = DhAngleGetRes(angleP);
  resNum = DhResGetNumber(resP);

  if (resNum != dataP->resNum) {
    CHECK_RES(GFileWriteNL(gf));
    (void) sprintf(buf, "%3d %-5.5s",
	resNum, DStrToStr(DhResGetName(resP)));
    CHECK_RES(GFileWriteStr(gf, buf));
    dataP->resNum = resNum;
    dataP->pos = 0;
  }

  if (dataP->pos == 4) {
    CHECK_RES(GFileWriteNL(gf));
    CHECK_RES(GFileWriteStr(gf, "         "));   /* 9 blanks */
    dataP->pos = 0;
  }

  (void) sprintf(buf, "%-5.5s%#9.3f",
      DStrToStr(DhAngleGetName(angleP)), DhAngleGetVal(angleP));
  CHECK_RES(GFileWriteStr(gf, buf));

  dataP->pos++;
}

static void
writeMol(DhMolP molP, void *clientData)
{
  WriteData *dataP = clientData;
  GFile gf = dataP->gf;

  if (BreakCheck(1) || ! dataP->ok)
    return;

  CHECK_RES(GFileWriteStr(gf, "# Structure of"));
  CHECK_RES(GFileWriteStr(gf, DStrToStr(DhMolGetName(molP))));
  CHECK_RES(GFileWriteStr(gf, "from"));
  CHECK_RES(GFileWriteStr(gf, PROG_NAME));

  dataP->resNum = 0;
  DhMolApplyAngle(PropGetRef(PROP_ALL, FALSE), molP, writeAng, dataP);

  CHECK_RES(GFileWriteNL(gf));

}

ErrCode
ExWriteAng(char *cmd)
{
  GFile gf;
  DSTR fileName;
  ErrCode errCode;
  WriteData writeData;

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, CurrDirGet(), "*.ang", FALSE);
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
  BreakActivate(TRUE);

  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), writeMol, &writeData);

  BreakActivate(FALSE);
  GFileClose(gf);

  if (writeData.ok)
    return EC_OK;
  else
    return EC_ERROR;
}
