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
  return MolInit("Motif/OpenGL", argc, argv, NULL);
}
