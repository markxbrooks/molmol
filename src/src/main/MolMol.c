/*
************************************************************************
*
*   MolMol.c - UNIX main program
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
*   Date of last modification : 97/03/03
*   Pathname of SCCS file     : /local/home/kor/molmol/src/main/SCCS/s.MolMol.c
*   SCCS identification       : 1.39
*
************************************************************************
*/

#include "mol_init.h"

#include <stdio.h>

int
main(int argc, char *argv[])
{
  /* Must match a name in iodev/IODev.c DevList for the IO_DEV_* set in makedef. */
#if defined(IO_DEV_MOTIF_OGL)
  return MolInit("Motif/OpenGL", argc, argv, NULL);
#elif defined(IO_DEV_MOTIF_XGL)
  return MolInit("Motif/XGL", argc, argv, NULL);
#elif defined(IO_DEV_MOTIF_GL)
  return MolInit("Motif/GL", argc, argv, NULL);
#elif defined(IO_DEV_MOTIF_X11)
  return MolInit("Motif/X11", argc, argv, NULL);
#elif defined(IO_DEV_WIN_OGL)
  return MolInit("Windows/OpenGL", argc, argv, NULL);
#elif defined(IO_DEV_TTY_NO)
  return MolInit("TTY/NO", argc, argv, NULL);
#else
  return MolInit("Motif/OpenGL", argc, argv, NULL);
#endif
}
