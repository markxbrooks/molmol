/*
************************************************************************
*
*   ExXMac.c - XMac command
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 98/03/19
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/cmdmisc/SCCS/s.ExXMac.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_misc.h>

#include <string.h>

#include <dstr.h>
#include <file_name.h>
#include <arg.h>
#include <prog_dir.h>
#include <curr_dir.h>
#include <cip.h>

#define NAME_PATTERN "*.mac"

ErrCode
ExXMac(char *cmd)
{
  DSTR fileName, dirName, fullName;
  ErrCode errCode;

  dirName = DStrNew();
  if (strcmp(cmd, "XMacUser") == 0) {
    DStrAssignStr(dirName, CurrDirGet());
  } else {
    DStrAssignStr(dirName, ProgDirGet());
    DStrAppStr(dirName, "/macros");
  }

  fileName = DStrNew();
  errCode = ArgGetFilename(fileName, DStrToStr(dirName), NAME_PATTERN, TRUE);

  if (errCode != EC_OK) {
    DStrFree(dirName);
    DStrFree(fileName);
    return errCode;
  }

  fullName = DStrNew();
  if (FileNameIsAbs(fileName)) {
    DStrAssignDStr(fullName, fileName);
  } else {
    DStrAssignDStr(fullName, dirName);
    DStrAppChar(fullName, '/');
    DStrAppDStr(fullName, fileName);
  }

  errCode = CipExecFile(DStrToStr(fullName));

  DStrFree(dirName);
  DStrFree(fileName);
  DStrFree(fullName);

  return errCode;
}
