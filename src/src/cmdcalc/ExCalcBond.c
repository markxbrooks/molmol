/*
************************************************************************
*
*   ExCalcBond.c - CalcBond and CheckBond commands
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcBond.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <values.h>

#include <break.h>
#include <pu.h>
#include <arg.h>
#include <par_names.h>
#include <setup_file.h>
#include <data_hand.h>
#include <data_sel.h>
#include <calc_pair.h>
#include <graph_draw.h>

#define FIRST_LETTER 'A'
#define LAST_LETTER 'Z'
#define LETTER_NO (LAST_LETTER - FIRST_LETTER + 1)

typedef struct {
  int minNo, maxNo;
  PropRefP selRefP;
  PropRefP allRefP;
  int bondNo;
} AtomData;

static BOOL RadTabRead = FALSE;
static float RadTab[LETTER_NO], RadMax;
static float BondMinTab[LETTER_NO], BondMaxTab[LETTER_NO];

static int MinStrucNo = 1;
static BOOL DoReport = TRUE;
static BOOL DoInsert = TRUE;

static PuTextWindow TextW;

static void
readRadTab(void)
{
  GFile gf;
  GFileRes res;
  char buf[20], ch;
  FLOAT32 f32;
  INT16 min, max;
  int i;

  if (RadTabRead)
    return;

  gf = SetupOpen(PN_ATOM_RADIUS, "AtomRadius", FALSE);
  if (gf == NULL)
    return;

  RadMax = - MAXFLOAT;

  for (i = 0; i < LETTER_NO; i++)
    RadTab[i] = - MAXFLOAT;

  while (! GFileEOF(gf)) {
    res = GFileReadStr(gf, buf, sizeof(buf));
    if (res != GF_RES_OK)
      break;
    ch = buf[0];

    res = GFileReadFLOAT32(gf, &f32);
    if (res != GF_RES_OK)
      break;

    res = GFileReadINT16(gf, &min);
    if (res != GF_RES_OK)
      break;

    res = GFileReadINT16(gf, &max);
    if (res != GF_RES_OK)
      break;

    if (ch < FIRST_LETTER || ch > LAST_LETTER)
      continue;

    RadTab[ch - FIRST_LETTER] = f32;
    BondMinTab[ch - FIRST_LETTER] = min;
    BondMaxTab[ch - FIRST_LETTER] = max;

    if (f32 > RadMax)
      RadMax = f32;
  }

  GFileClose(gf);

  RadTabRead = TRUE;
}

static BOOL
checkAtom(DhAtomP atomP)
{
  DSTR name;
  char ch;

  name = DhAtomGetName(atomP);
  ch = DStrToStr(name)[0];
  if (ch < FIRST_LETTER || ch > LAST_LETTER)
    return FALSE;

  return RadTab[ch - FIRST_LETTER] > 0.0f;
}

static BOOL
checkPair(DhAtomP atom1P, DhAtomP atom2P, float distS, float *valP)
{
  DSTR name1, name2;
  char ch1, ch2;
  float radSum;
  DhBondP bondP;

  name2 = DhAtomGetName(atom2P);
  ch2 = DStrToStr(name2)[0];

  name1 = DhAtomGetName(atom1P);
  ch1 = DStrToStr(name1)[0];

  radSum = RadTab[ch1 - FIRST_LETTER] + RadTab[ch2 - FIRST_LETTER];
  if (distS > radSum * radSum)
    return FALSE;

  if (DoInsert) {
    bondP = DhBondNew(atom1P, atom2P);
    if (bondP == NULL)
      return FALSE;
    DhBondInit(bondP);
  }

  return TRUE;
}

static void
writeStr(char *str)
{
  PuWriteStr(TextW, str);
}

#define ARG_NUM 2
#define OPT_NUM 2

ErrCode
ExCalcBond(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[OPT_NUM];
  ErrCode errCode;
  char buf[20];

  readRadTab();
  if (! RadTabRead)
    return EC_ERROR;

  arg[0].type = AT_INT;
  arg[1].type = AT_MULT_ENUM;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Min. Struc. #";
  arg[0].v.intVal = MinStrucNo;

  enumEntry[0].str = "report";
  enumEntry[0].onOff = DoReport;
  enumEntry[1].str = "display";
  enumEntry[1].onOff = DoInsert;

  arg[1].prompt = "Options";
  arg[1].u.enumD.entryP = enumEntry;
  arg[1].u.enumD.n = OPT_NUM;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  MinStrucNo = arg[0].v.intVal;
  DoReport = enumEntry[0].onOff;
  DoInsert = enumEntry[1].onOff;

  ArgCleanup(arg, ARG_NUM);

  BreakActivate(TRUE);

  if (DoReport) {
    TextW = PuCreateTextWindow(cmd);

    PuWriteStr(TextW, "New bonds in at least ");
    (void) sprintf(buf, "%d", MinStrucNo);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, " structures:\n");
  }

  CalcPair(2.0f * RadMax, TRUE, FALSE, PE_BOND, MinStrucNo, NULL,
      PROP_SELECTED, PROP_SELECTED, checkAtom, checkAtom, checkPair, 0,
      NULL, 0,
      NULL, DoReport ? writeStr : NULL);

  BreakActivate(FALSE);

  if (DoInsert) {
    GraphMolChanged(PROP_SELECTED);
    GraphRedrawNeeded();
  }

  return EC_OK;
}

static void
countBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  AtomData *dataP = clientData;

  dataP->bondNo++;
}

static void
checkAtomBond(DhAtomP atomP, void *clientData)
{
  AtomData *dataP = clientData;
  char ch;

  if (BreakCheck(1000))
    return;

  ch = DStrToStr(DhAtomGetName(atomP))[0];
  if (ch < FIRST_LETTER || ch > LAST_LETTER)
    return;

  dataP->bondNo = 0;
  DhAtomApplyBond(dataP->allRefP, atomP, countBond, dataP);

  if (dataP->bondNo < BondMinTab[ch - FIRST_LETTER])
    dataP->minNo++;
  else if (dataP->bondNo > BondMaxTab[ch - FIRST_LETTER])
    dataP->maxNo++;
  else
    DhAtomSetProp(dataP->selRefP, atomP, FALSE);
}

ErrCode
ExCheckBond(char *cmd)
{
  AtomData data;
  DSTR statStr;
  char numBuf[20];

  readRadTab();
  if (! RadTabRead)
    return EC_ERROR;

  BreakActivate(TRUE);

  data.minNo = 0;
  data.maxNo = 0;
  data.selRefP = PropGetRef(PROP_SELECTED, FALSE);
  data.allRefP = PropGetRef(PROP_ALL, FALSE);
  DhApplyAtom(data.selRefP, checkAtomBond, &data);

  BreakActivate(FALSE);

  statStr = DStrNew();
  (void) sprintf(numBuf, "%d", data.minNo);
  DStrAppStr(statStr, numBuf);
  DStrAppStr(statStr, " atoms with too few and ");
  (void) sprintf(numBuf, "%d", data.maxNo);
  DStrAppStr(statStr, numBuf);
  DStrAppStr(statStr, " atoms with too many bonds selected");
  PuSetTextField(PU_TF_STATUS, DStrToStr(statStr));
  DStrFree(statStr);

  return EC_OK;
}
