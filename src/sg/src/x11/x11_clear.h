/*
************************************************************************
*
*   x11_clear.h - X11 clear window
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/x11/SCCS/s.x11_clear.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <sg_types.h>

extern void SgX11Clear(void);

extern void SgX11SetBuffer(SgBuffer);

extern void SgX11FlushFrame(void);

extern void SgX11EndFrame(void);

extern BOOL SgX11Refresh(void);
