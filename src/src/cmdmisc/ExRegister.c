/*
************************************************************************
*
*   ExRegister.c - Register command
*
*   Copyright (c) 1996-2000
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
*   Date of last modification : 00/03/11
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdmisc/SCCS/s.ExRegister.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_misc.h>

#include <stdio.h>

#include <dstr.h>
#include <os_mail.h>
#include <prog_vers.h>
#include <contact_addr.h>
#include <pu.h>
#include <arg.h>

#define ARG_NUM 5

static char *FieldNames[] = {
  "Name",
  "Organisation",
  "Address",
  "E-mail",
  "Program"
};

ErrCode
ExRegister(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  DSTR msg, errMsg;
  char buf[30];
  int i;

  for (i = 0; i < ARG_NUM; i++)
    arg[i].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  for (i = 0; i < ARG_NUM; i++)
    arg[i].prompt = FieldNames[i];

  DStrAssignStr(arg[4].v.strVal, PROG_NAME);
  (void) sprintf(buf, " %s.%d.%d", VERS_MAJOR, VERS_MINOR, VERS_REV);
  DStrAppStr(arg[4].v.strVal, buf);
  arg[4].optional = TRUE;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  msg = DStrNew();

  for (i = 0; i < ARG_NUM; i++) {
    DStrAppStr(msg, FieldNames[i]);
    DStrAppStr(msg, ": ");
    DStrAppDStr(msg, arg[i].v.strVal);
    DStrAppStr(msg, "\n");
  }

  if (OsMail("registration", DStrToStr(msg))) {
    PuShowMessage(PU_MT_INFO, cmd, "E-mail sent");
  } else {
    errMsg = DStrNew();
    DStrAppStr(errMsg, "E-mail could not be sent.\n");
    DStrAppStr(errMsg, "Please use your E-mail program\n");
    DStrAppStr(errMsg, "to send the following information:\n\n");
    DStrAppDStr(errMsg, msg);
    DStrAppStr(errMsg, "\nto ");
    DStrAppStr(errMsg, E_MAIL_ADDR);
    PuShowMessage(PU_MT_ERROR, cmd, DStrToStr(errMsg));
    DStrFree(errMsg);
  }

  DStrFree(msg);

  ArgCleanup(arg, ARG_NUM);

  return EC_OK;
}
