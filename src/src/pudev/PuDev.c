/*
************************************************************************
*
*   PuDev.c - management of Pu devices
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
*   Date of last modification : 96/05/09
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuDev.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <string.h>

#include <pu_dev.h>

#ifdef PU_DEV_TTY
extern void PuTTYSetDev(void);
#endif
#ifdef PU_DEV_MOTIF
extern void PuMotifSetDev(void);
#endif
#ifdef PU_DEV_WIN
extern void PuWinSetDev(void);
#endif

PuDev *CurrPuDevP;

typedef void (*DevSetF) (void);

typedef struct {
  char *name;
  DevSetF set;
} DevListEntry;

static DevListEntry DevList[] = {
#ifdef PU_DEV_TTY
  {"TTY", PuTTYSetDev},
#endif
#ifdef PU_DEV_MOTIF
  {"Motif", PuMotifSetDev},
#endif
#ifdef PU_DEV_WIN
  {"Windows", PuWinSetDev},
#endif
};

PuRes
PuSetDevice(char *devName)
{
  int listSize, i;

  listSize = sizeof(DevList) / sizeof(DevList[0]);

  for (i = 0; i < listSize; i++)
    if (strcmp(DevList[i].name, devName) == 0) {
      DevList[i].set();
      return PU_RES_OK;
    }
  
  return PU_RES_ERR;
}
