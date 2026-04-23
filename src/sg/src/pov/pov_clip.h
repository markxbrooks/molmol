/*
************************************************************************
*
*   pov_clip.h - POV clipping
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
*   Date of last modification : 97/01/08
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/pov/SCCS/s.pov_clip.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <bool.h>

extern BOOL POVClipTest(float[3], float);

extern void POVClipOutput(void);

extern BOOL POVClip(float[3], float);
