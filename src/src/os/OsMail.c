/*
************************************************************************
*
*   OsMail.c - send E-mail
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
*   Date of last modification : 97/11/07
*   Pathname of SCCS file     : /tmp_mnt/files/kor/molmol/src/os/SCCS/s.OsMail.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <os_mail.h>

#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#include <mapi.h>
#endif

#include <dstr.h>
#include <prog_vers.h>
#include <contact_addr.h>

/* missing on IRIX 5.3 */
extern FILE *popen(const char *, const char *);
extern int pclose(FILE *);

BOOL
OsMail(char *subj, char *msg)
{
#ifdef WIN32
  HINSTANCE inst;
  LPMAPILOGON MAPILogon;
  LPMAPILOGOFF MAPILogoff;
  LPMAPISENDMAIL MAPISendMail;
  LHANDLE sess;
  MapiMessage msgS;
  MapiRecipDesc recA[1];
  ULONG res;
  DSTR subjStr;

  if (GetProfileInt("Mail", "MAPI", 0) == 0)
    return FALSE;

  inst = LoadLibrary("MAPI32.DLL");
  if (inst == NULL)
    return FALSE;

  MAPILogon = (LPMAPILOGON) GetProcAddress(inst, "MAPILogon");
  MAPILogoff = (LPMAPILOGOFF) GetProcAddress(inst, "MAPILogoff");
  MAPISendMail = (LPMAPISENDMAIL) GetProcAddress(inst, "MAPISendMail");

  res = MAPILogon(0, NULL, NULL, MAPI_LOGON_UI, 0, &sess);
  if (res == SUCCESS_SUCCESS) {
    recA[0].ulReserved = 0;
    recA[0].ulRecipClass = MAPI_TO;
    recA[0].lpszName = E_MAIL_ADDR;
    recA[0].lpszAddress = E_MAIL_ADDR;
    recA[0].ulEIDSize = 0;
    recA[0].lpEntryID = 0;

    subjStr = DStrNew();
    DStrAssignStr(subjStr, PROG_NAME);
    DStrAppStr(subjStr, " ");
    DStrAppStr(subjStr, subj);

    memset(&msgS, 0, sizeof(msgS));
    msgS.lpszSubject = DStrToStr(subjStr);
    msgS.lpszNoteText = msg;
    msgS.nRecipCount = 1;
    msgS.lpRecips = recA;

    res = MAPISendMail(sess, 0, &msgS, 0, 0);

    DStrFree(subjStr);

    MAPILogoff(sess, 0, 0, 0);
  }

  FreeLibrary(inst);

  return res == SUCCESS_SUCCESS;
#else
  DSTR cmd = DStrNew();
  FILE *fp;

  DStrAssignStr(cmd, "mail ");

#ifdef __sun
  DStrAppStr(cmd, "-s '");
  DStrAppStr(cmd, PROG_NAME);
  DStrAppStr(cmd, " ");
  DStrAppStr(cmd, subj);
  DStrAppStr(cmd, "' ");
#endif

  DStrAppStr(cmd, E_MAIL_ADDR);

  fp = popen(DStrToStr(cmd), "w");
  if (fp == NULL) {
    DStrFree(cmd);
    return FALSE;
  }

#ifndef __sun
  (void) fprintf(fp, "Subject: %s %s\n\n", PROG_NAME, subj);
#endif

  (void) fprintf(fp, "%s\n", msg);

  (void) pclose(fp);

  DStrFree(cmd);

  return TRUE;
#endif
}
