/*
************************************************************************
*
*   prog_vers.h - program version
*
*   Copyright (c) 1994-2001
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
*   Date of last modification : 03/01/20
*   Pathname of SCCS file     : /home/rkoradi/molmol-master/include/SCCS/s.prog_vers.h
*   SCCS identification       : 1.63
*
************************************************************************
*/

#ifndef _PROG_VERS_H_
#define _PROG_VERS_H_

#define PROG_NAME  "MOLMOL"
#define VERS_MAJOR "2K"
#define VERS_MINOR 2
#define VERS_REV   0
#define VERS_STATE ""

#define PROG_COPYRIGHT \
    "Version 2.1-2.6: Copyright (c) 1994-98 by\n" \
    "    Institut fuer Molekularbiologie und Biophysik, ETH Zurich\n" \
    "    Spectrospin AG, Faellanden, Switzerland\n\n" \
    "Version 2K.2: Custom version by Reto Koradi, 1999-2003"

#define VERS_DUMP     16
#define VERS_DUMP_MIN 1

/* dump version history:
    1, 950411: version number in file introduced
    2, 950426: PT_SHEET inserted in PrimType
    3, 950519: new Attr fields (material properties)
    4, 950523: new Attr field (reflection)
    5, 950606: bonds to pseudo atoms
    6, 951105: dump standard properties
    7, 951210: colored surface
    8, 960307: added new ribbon orientation (RO_SPACING)
    9, 960315: introduced groups
   10, 960411: introduced heavycharge and avgcharge
   11, 960416: introduced simplecharge
   12, 961128: introduced pseudoI field for atoms
   13, 970219: introduced alternative coordinates (LengthBond)
   14, 970717: new Attr field (near clipping plane)
   15, 970722: cylinder style
   16, 980721: modifiable default attributes
*/

#endif  /* _PROG_VERS_H_ */
