/*
************************************************************************
*
*   GFile.c - general file handling
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
*   Date of last modification : 96/11/25
*   Pathname of SCCS file     : /local/home/kor/molmol/src/os/SCCS/s.GFile.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <g_file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#include <linlist.h>

#define MEM_BLOCK_SIZE 60000

const INT32 GFMagic = 0x3b7a12f9;

#ifndef __linux__
#endif
extern int errno;

typedef struct MemBlockStruc *MemBlockP;

typedef struct MemBlockStruc {
  char data[MEM_BLOCK_SIZE];
  unsigned len;
  MemBlockP nextP;
} MemBlock;

struct GFileStruc {
  char *name;
  GFileFormat format;
  MemBlockP firstP, currP;
  unsigned currPos;
  FILE *fp;
  BOOL byteSwap;
};

static char *NextMsg = NULL;
static LINLIST MemFileList = NULL;

static void
defaultErrorHandler(char *fileName, char *msg)
{
  (void) fprintf(stderr, "IO ERROR: %s - %s\n", fileName, msg);
}

static GFileErrorHandler ErrorHandler = defaultErrorHandler;

static void
setError(char *msg)
{
  NextMsg = msg;
}

static void
raiseError(char *fileName, char *msg)
{
  if (msg == NULL) {
    if (NextMsg == NULL)
      msg = sys_errlist[errno];
    else
      msg = NextMsg;
  }

  ErrorHandler(fileName, msg);
  NextMsg = NULL;
}

void
GFileSetErrorHandler(GFileErrorHandler errHandler)
{
  if (errHandler == NULL)
    ErrorHandler = defaultErrorHandler;
  else
    ErrorHandler = errHandler;
}

static void
swap2(void *p)
{
  char *cp = p;
  char c;

  c = cp[0]; cp[0] = cp[1]; cp[1] = c;
}

static void
swap4(void *p)
{
  char *cp = p;
  char c;

  c = cp[0]; cp[0] = cp[3]; cp[3] = c;
  c = cp[1]; cp[1] = cp[2]; cp[2] = c;
}

static void
swap8(void *p)
{
  char *cp = p;
  char c;

  c = cp[0]; cp[0] = cp[7]; cp[7] = c;
  c = cp[1]; cp[1] = cp[6]; cp[6] = c;
  c = cp[2]; cp[2] = cp[5]; cp[5] = c;
  c = cp[3]; cp[3] = cp[4]; cp[4] = c;
}

static GFile
findMemFile(char *name)
{
  GFile gf;

  gf = ListFirst(MemFileList);
  while (gf != NULL) {
    if (strcmp(name, gf->name) == 0)
      return gf;
    gf = ListNext(MemFileList, gf);
  }

  return NULL;
}

static GFile
openFile(char *name, GFileFormat format, char *mode)
{
  struct GFileStruc gfS;
  GFile gf;

  if (format == GF_FORMAT_MEMORY) {
    gf = findMemFile(name);
    if (gf != NULL)
      free(gf->name);
    if (mode[0] == 'w' && gf == NULL) {
      if (MemFileList == NULL)
	MemFileList = ListOpen(sizeof(struct GFileStruc));

      gf = ListInsertFirst(MemFileList, &gfS);
      if (gf == NULL) {
	raiseError(name, "out of memory");
	return NULL;
      }

      gf->firstP = NULL;
    }

    if (mode[0] == 'a') {
      gf->currP = gf->firstP;
      while (gf->currP->nextP != NULL)
	gf->currP = gf->currP->nextP;
      gf->currPos = gf->currP->len;
    } else {
      gf->currP = gf->firstP;
      gf->currPos = 0;
    }

    gf->name = malloc(strlen(name) + 1);
    if (gf->name == NULL) {
      ListRemove(MemFileList, gf);
      raiseError(name, "out of memory");
      return NULL;
    }
  } else {
    gf = malloc(sizeof(*gf));
    if (gf == NULL) {
      raiseError(name, "out of memory");
      return NULL;
    }

#ifdef WIN32
    if (format == GF_FORMAT_BINARY) {
      int len;
      char winMode[5];

      len = strlen(mode);
      (void) strcpy(winMode, mode);
      winMode[len] = 'b';
      winMode[len + 1] = '\0';
      gf->fp = fopen(name, winMode);
    } else {
      gf->fp = fopen(name, mode);
    }
#else
    gf->fp = fopen(name, mode);
#endif
    if (gf->fp == NULL) {
      free(gf);
      raiseError(name, NULL);
      return NULL;
    }

    gf->name = malloc(strlen(name) + 1);
    if (gf->name == NULL) {
      free(gf);
      raiseError(name, "out of memory");
      return NULL;
    }
  }

  gf->format = format;
  (void) strcpy(gf->name, name);

  return gf;
}

static void
getFileInfo(char *name, BOOL *errorP, GFileFormat *formatP, BOOL *swapP)
{
  FILE *fp;
  INT32 magic = 0;

  if (findMemFile(name) != NULL) {
    *errorP = FALSE;
    *formatP = GF_FORMAT_MEMORY;
    return;
  }

  fp = fopen(name, "r");
  if (fp == NULL) {
    *errorP = TRUE;
    raiseError(name, NULL);
    return;
  } else {
    *errorP = FALSE;
  }

  (void) fread(&magic, 4, 1, fp);
  (void) fclose(fp);
  if (magic == GFMagic) {
    *formatP = GF_FORMAT_BINARY;
    *swapP = FALSE;
    return;
  }

  swap4(&magic);
  if (magic == GFMagic) {
    *formatP = GF_FORMAT_BINARY;
    *swapP = TRUE;
    return;
  }

  *formatP = GF_FORMAT_ASCII;
}

GFile
GFileOpenNew(char *name, GFileFormat format)
{
  GFile gf;

  gf = openFile(name, format, "w");
  if (gf == NULL)
    return NULL;

  if (format == GF_FORMAT_BINARY) {
    gf->byteSwap = FALSE;
    (void) fwrite(&GFMagic, 4, 1, gf->fp);
  }

  return gf;
}

GFile
GFileOpenRewrite(char *name)
{
  BOOL error, swap;
  GFileFormat format;
  GFile gf;
  INT32 magic;

  getFileInfo(name, &error, &format, &swap);
  if (error)
    return NULL;

  gf = openFile(name, format, "w");
  if (gf == NULL)
    return NULL;

  if (format == GF_FORMAT_BINARY) {
    gf->byteSwap = swap;
    magic = GFMagic;
    if (swap)
      swap4(&magic);
    (void) fwrite(&magic, 4, 1, gf->fp);
  }

  return gf;
}

GFile
GFileOpenAppend(char *name)
{
  BOOL error, swap;
  GFileFormat format;
  GFile gf;

  getFileInfo(name, &error, &format, &swap);
  if (error)
    return NULL;

  gf = openFile(name, format, "a");
  if (gf == NULL)
    return NULL;

  if (format == GF_FORMAT_BINARY)
    gf->byteSwap = swap;

  return gf;
}

GFile
GFileOpenRead(char *name)
{
  BOOL error, swap;
  GFileFormat format;
  GFile gf;
  INT32 dummy;

  getFileInfo(name, &error, &format, &swap);
  if (error)
    return NULL;

  gf = openFile(name, format, "r");
  if (gf == NULL)
    return NULL;

  if (format == GF_FORMAT_BINARY) {
    gf->byteSwap = swap;
    (void) fread(&dummy, 4, 1, gf->fp);
  }

  return gf;
}

void
GFileClose(GFile gf)
{
  MemBlockP thisP, nextP;

  if (gf->format == GF_FORMAT_MEMORY) {
    if (gf->currP == NULL)
      return;

    thisP = gf->currP->nextP;
    while (thisP != NULL) {
      nextP = thisP->nextP;
      free(thisP);
      thisP = nextP;
    }
    gf->currP->len = gf->currPos;
    gf->currP->nextP = NULL;
  } else {
    (void) fclose(gf->fp);
    free(gf->name);
    free(gf);
  }
}

BOOL
GFileEOF(GFile gf)
{
  if (gf->format == GF_FORMAT_MEMORY)
    return gf->currP == NULL ||
	(gf->currP->nextP == NULL && gf->currPos == gf->currP->len);
  else
    return feof(gf->fp);
}

void
GFileFlush(GFile gf)
{
  if (gf->format != GF_FORMAT_MEMORY)
    (void) fflush(gf->fp);
}

static BOOL
writeMem(GFile gf, void *p, int n)
{
  if (gf->currP == NULL) {
    gf->currP = malloc(sizeof(*gf->currP));
    if (gf->currP == NULL) {
      setError("out of memory");
      return TRUE;
    }
    gf->currP->len = 0;
    gf->currP->nextP = NULL;
    gf->currPos = 0;
    gf->firstP = gf->currP;
  } else if (gf->currPos + n > MEM_BLOCK_SIZE) {
    gf->currP->len = gf->currPos;
    if (gf->currP->nextP == NULL) {
      gf->currP->nextP = malloc(sizeof(*gf->currP->nextP));
      if (gf->currP->nextP == NULL) {
	setError("out of memory");
	return TRUE;
      }
      gf->currP->nextP->nextP = NULL;
    }
    gf->currP = gf->currP->nextP;
    gf->currP->len = 0;
    gf->currPos = 0;
  }

  (void) memcpy(gf->currP->data + gf->currPos, p, n);
  gf->currPos += n;

  return FALSE;
}

GFileRes
GFileWriteChar(GFile gf, char c)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = writeMem(gf, &c, 1);
  } else if (gf->format == GF_FORMAT_ASCII) {
    err = (fprintf(gf->fp, "%c", c) == EOF);
  } else {
    err = (fwrite(&c, 1, 1, gf->fp) != 1);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileWriteINT16(GFile gf, INT16 i)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = writeMem(gf, &i, 2);
  } else if (gf->format == GF_FORMAT_ASCII) {
    err = (fprintf(gf->fp, "%d ", i) == EOF);
  } else {
    if (gf->byteSwap)
      swap2(&i);
    err = (fwrite(&i, 2, 1, gf->fp) != 1);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileWriteINT32(GFile gf, INT32 i)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = writeMem(gf, &i, 4);
  } else if (gf->format == GF_FORMAT_ASCII) {
    err = (fprintf(gf->fp, "%d ", i) == EOF);
  } else {
    if (gf->byteSwap)
      swap4(&i);
    err = (fwrite(&i, 4, 1, gf->fp) != 1);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileWriteFLOAT32(GFile gf, FLOAT32 f)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = writeMem(gf, &f, 4);
  } else if (gf->format == GF_FORMAT_ASCII) {
    err = (fprintf(gf->fp, "%g ", f) == EOF);
  } else {
    if (gf->byteSwap)
      swap4(&f);
    err = (fwrite(&f, 4, 1, gf->fp) != 1);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileWriteFLOAT64(GFile gf, FLOAT64 f)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = writeMem(gf, &f, 8);
  } else if (gf->format == GF_FORMAT_ASCII) {
    err = (fprintf(gf->fp, "%g ", f) == EOF);
  } else {
    if (gf->byteSwap)
      swap8(&f);
    err = (fwrite(&f, 8, 1, gf->fp) != 1);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileWriteStr(GFile gf, char *s)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = writeMem(gf, s, strlen(s) + 1);
  } else if (gf->format == GF_FORMAT_ASCII) {
    err = (fprintf(gf->fp, "%s ", s) == EOF);
  } else {
    err = (fwrite(s, strlen(s) + 1, 1, gf->fp) != 1);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileWriteQuotedStr(GFile gf, char *s)
{
  int len, i;
  char quoteCh;
  BOOL err;

  len = strlen(s);

  if (gf->format == GF_FORMAT_MEMORY) {
    err = writeMem(gf, s, len + 1);
  } else if (gf->format == GF_FORMAT_ASCII) {
    quoteCh = '"';
    for (i = 0; i < len; i++)
      if (s[i] == quoteCh) {
	quoteCh = '\'';
	break;
      }
    err = (fprintf(gf->fp, "%c", quoteCh) == EOF);
    if (! err)
      err = (fprintf(gf->fp, "%s", s) == EOF);
    if (! err)
      err = (fprintf(gf->fp, "%c ", quoteCh) == EOF);
  } else {
    err = (fwrite(s, len + 1, 1, gf->fp) != 1);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileWriteLine(GFile gf, char *s)
{
  BOOL err;

  err = (fprintf(gf->fp, "%s\n", s) == EOF);

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileWriteNL(GFile gf)
{
  BOOL err;

  if (gf->format == GF_FORMAT_ASCII) {
    err = (fprintf(gf->fp, "\n") == EOF);
  } else {
    err = FALSE;
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

static BOOL
readMem(GFile gf, void *p, int n)
{
  if (gf->currP == NULL) {
    setError("EOF reached");
    return TRUE;
  }

  if (gf->currPos == gf->currP->len) {
    gf->currP = gf->currP->nextP;
    if (gf->currP == NULL) {
      setError("EOF reached");
      return TRUE;
    }
    gf->currPos = 0;
  }

  (void) memcpy(p, gf->currP->data + gf->currPos, n);
  gf->currPos += n;

  return FALSE;
}

static void
readWhiteSpace(FILE *fp)
{
  int c;

  while (! feof(fp)) {
    c = getc(fp);
    if (! isspace(c)) {
      ungetc(c, fp);
      return;
    }
  }
}

GFileRes
GFileReadChar(GFile gf, char *cP)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = readMem(gf, cP, 1);
  } else if (gf->format == GF_FORMAT_ASCII) {
    err = (fscanf(gf->fp, "%c", cP) != 1);
  } else {
    err = (fread(cP, 1, 1, gf->fp) != 1);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileReadINT16(GFile gf, INT16 *iP)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = readMem(gf, iP, 2);
  } else if (gf->format == GF_FORMAT_ASCII) {
    long i;
    err = (fscanf(gf->fp, "%ld", &i) != 1);
    *iP = (INT16) i;
    readWhiteSpace(gf->fp);
  } else {
    err = (fread(iP, 2, 1, gf->fp) != 1);
    if (gf->byteSwap)
      swap2(iP);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileReadINT32(GFile gf, INT32 *iP)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = readMem(gf, iP, 4);
  } else if (gf->format == GF_FORMAT_ASCII) {
    long i;
    err = (fscanf(gf->fp, "%ld", &i) != 1);
    *iP = i;
    readWhiteSpace(gf->fp);
  } else {
    err = (fread(iP, 4, 1, gf->fp) != 1);
    if (gf->byteSwap)
      swap4(iP);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileReadFLOAT32(GFile gf, FLOAT32 *fP)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = readMem(gf, fP, 4);
  } else if (gf->format == GF_FORMAT_ASCII) {
    double f;
    err = (fscanf(gf->fp, "%lg", &f) != 1);
    *fP = (FLOAT32) f;
    readWhiteSpace(gf->fp);
  } else {
    err = (fread(fP, 4, 1, gf->fp) != 1);
    if (gf->byteSwap)
      swap4(fP);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileReadFLOAT64(GFile gf, FLOAT64 *fP)
{
  BOOL err;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = readMem(gf, fP, 8);
  } else if (gf->format == GF_FORMAT_ASCII) {
    double f;
    err = (fscanf(gf->fp, "%lg", &f) != 1);
    *fP = f;
    readWhiteSpace(gf->fp);
  } else {
    err = (fread(fP, 8, 1, gf->fp) != 1);
    if (gf->byteSwap)
      swap8(fP);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileReadStr(GFile gf, char *s, int size)
{
  BOOL err;
  int i;

  if (gf->format == GF_FORMAT_MEMORY) {
    err = FALSE;
    i = 0;
    for (;;) {
      err = readMem(gf, s + i, 1);
      if (s[i] == '\0' || err)
	break;
      if (GFileEOF(gf)) {
	err = TRUE;
	break;
      }
      i++;
    }
  } else if (gf->format == GF_FORMAT_ASCII) {
    err = (fscanf(gf->fp, "%s", s) != 1);
    readWhiteSpace(gf->fp);
  } else {
    err = FALSE;
    i = 0;
    for (;;) {
      err = (fread(s + i, 1, 1, gf->fp) != 1);
      if (s[i] == '\0' || err)
	break;
      if (feof(gf->fp)) {
	err = TRUE;
	break;
      }
      i++;
    }
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileReadQuotedStr(GFile gf, char *s, int size)
{
  BOOL err;
  int quoteCh;
  int i;

  if (gf->format == GF_FORMAT_MEMORY) {
    return GFileReadStr(gf, s, size);
  } else if (gf->format == GF_FORMAT_ASCII) {
    quoteCh = getc(gf->fp);
    if (quoteCh == '"' || quoteCh == '\'') {
      err = FALSE;
      i = 0;
      for (;;) {
	s[i] = getc(gf->fp);
	if (s[i] == quoteCh) {
	  s[i] = '\0';
	  readWhiteSpace(gf->fp);
	  break;
	}
	if (feof(gf->fp)) {
	  err = TRUE;
	  break;
	}
	i++;
      }
    } else {
      err = TRUE;
    }
  } else {
    return GFileReadStr(gf, s, size);
  }

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}

GFileRes
GFileReadLine(GFile gf, char *s, int size)
{
  BOOL err;
  int len;
  int c;

  err = (fgets(s, size, gf->fp) == NULL);

  if (! err) {
    len = strlen(s);
    if (s[len - 1] == '\n') {
      /* remove '\n' */
      s[len - 1] = '\0';
    } else {
      /* go to beginning of next line if line was too long */
      while (! feof(gf->fp)) {
	c = getc(gf->fp);
	if (c == '\n')
	  break;
      }
    }
  }

  /* hack, needed to be able to detect EOF on time */
  c = getc(gf->fp);
  ungetc(c, gf->fp);

  if (err) {
    raiseError(gf->name, NULL);
    return GF_RES_ERR;
  } else {
    return GF_RES_OK;
  }
}
