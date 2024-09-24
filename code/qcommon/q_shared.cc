/*
 ===========================================================================
 Copyright (C) 1999-2005 Id Software, Inc.
 Copyright (C) 2006-2010 Lars '0xA5EA' Kandler

 This file is part of KingpinQ3 source code.

 KingpinQ3 source code is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of the License,
 or (at your option) any later version.

 KingpinQ3 source code is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with KingpinQ3 source code; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ===========================================================================
 */
//
// q_shared.c -- stateless support routines that are included in each code dll
#include "q_shared.h"

/*
 ============================================================================
 GROWLISTS
 ============================================================================
 */

// malloc / free all in one place for debugging
//extern          "C" void *Com_Allocate(int bytes);
//extern          "C" void Com_Dealloc(void *ptr);
#ifndef BSPC
#ifndef Q3_VM
void Com_InitGrowList(growList_t *list, int maxElements)
{
  list->maxElements = maxElements;
  list->currentElements = 0;
  list->elements = (void **)Com_Allocate(list->maxElements * sizeof(void *));
}

void Com_DestroyGrowList(growList_t *list)
{
  Com_Dealloc(list->elements);
  Com_Memset(list, 0, sizeof(*list));
}

int Com_AddToGrowList(growList_t *list, void *data)
{
  void **old;

  if (list->currentElements != list->maxElements)
  {
    list->elements[list->currentElements] = data;
    return list->currentElements++;
  }

  // grow, reallocate and move
  old = list->elements;

  if (list->maxElements < 0)
  {
    Com_Error(ERR_FATAL, "Com_AddToGrowList: maxElements = %i", list->maxElements);
  }

  if (list->maxElements == 0)
  {
    // initialize the list to hold 100 elements
    Com_InitGrowList(list, 100);
    return Com_AddToGrowList(list, data);
  }

  list->maxElements *= 2;

  //  Com_DPrintf("Resizing growlist to %i maxElements\n", list->maxElements);

  list->elements = (void **)Com_Allocate(list->maxElements * sizeof(void *));

  if (!list->elements)
  {
    Com_Error(ERR_DROP, "Growlist alloc failed");
  }

  Com_Memcpy(list->elements, old, list->currentElements * sizeof(void *));

  Com_Dealloc(old);

  return Com_AddToGrowList(list, data);
}

void *Com_GrowListElement(const growList_t *list, int index)
{
  if (index < 0 || index >= list->currentElements)
  {
    Com_Error(ERR_DROP, "Com_GrowListElement: %i out of range of %i", index, list->currentElements);
  }
  return list->elements[index];
}

int Com_IndexForGrowListElement(const growList_t *list, const void *element)
{
  int i;
  for (i = 0; i < list->currentElements; i++)
  {
    if (list->elements[i] == element)
      return i;
  }
  return -1;
}


memStream_t *AllocMemStream(byte *buffer, int bufSize)
{
  memStream_t *s;

  if (buffer == NULL || bufSize <= 0)
  {
    return NULL;
  }

  s = (memStream_t*)Com_Allocate(sizeof(memStream_t));

  if (s == NULL)
  {
    return NULL;
  }

  Com_Memset(s, 0, sizeof(memStream_t));

  s->buffer = buffer;
  s->curPos = buffer;
  s->bufSize = bufSize;
  s->flags = 0;

  return s;
}

void FreeMemStream(memStream_t *s)
{
  Com_Dealloc(s);
}

int MemStreamRead(memStream_t *s, void *buffer, int len)
{
  int ret = 1;

  if (s == NULL || buffer == NULL)
  {
    return 0;
  }

  if (s->curPos + len > s->buffer + s->bufSize)
  {
    s->flags |= MEMSTREAM_FLAGS_EOF;
    len = s->buffer + s->bufSize - s->curPos;
    ret = 0;

    Com_Error(ERR_FATAL, "MemStreamRead: EOF reached");
  }

  Com_Memcpy(buffer, s->curPos, len);
  s->curPos += len;

  return ret;
}

int MemStreamGetC(memStream_t *s)
{
  int c = 0;

  if (s == NULL)
  {
    return -1;
  }

  if (MemStreamRead(s, &c, 1) == 0)
  {
    return -1;
  }

  return c;
}

int MemStreamGetLong(memStream_t *s)
{
  int c = 0;

  if (s == NULL)
  {
    return -1;
  }

  if (MemStreamRead(s, &c, 4) == 0)
  {
    return -1;
  }

  return LittleLong(c);
}

int MemStreamGetShort(memStream_t *s)
{
  int c = 0;

  if (s == NULL)
  {
    return -1;
  }

  if (MemStreamRead(s, &c, 2) == 0)
  {
    return -1;
  }

  return LittleShort(c);
}

float MemStreamGetFloat(memStream_t *s)
{
  floatint_t c;

  if (s == NULL)
  {
    return -1;
  }

  if (MemStreamRead(s, &c.i, 4) == 0)
  {
    return -1;
  }

  return LittleFloat(c.f);
}

#endif //Q3_VM
#endif //BSPC

void Q_SnapVectorStd(vec3_t vec)
{
  vec[0] = round(vec[0]);
  vec[1] = round(vec[1]);
  vec[2] = round(vec[2]);
}

#ifdef Q3_VM
int PlaneTypeForNormal(vec3_t normal)
{
  if(normal[0] == 1.0)
    return PLANE_X;
  if(normal[1] == 1.0)
    return PLANE_Y;
  if(normal[2] == 1.0)
    return PLANE_Z;
  if(normal[0] == 0.0 && normal[1] == 0.0 && normal[2] == 0.0)
    return PLANE_NON_PLANAR;
  return PLANE_NON_AXIAL;
}
#endif


float Com_Clamp(float min, float max, float value)
{
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

void COM_FixPath(char *pathname)
{
  while (*pathname)
  {
    if (*pathname == '\\')
    {
      *pathname = '/';
    }

    pathname++;
  }
}

/*
 ============
 COM_SkipPath
 ============
 */
char *COM_SkipPath(char *pathname)
{
  char *last;

  last = pathname;
  while (*pathname)
  {
    if (*pathname == '/')
      last = pathname + 1;
    pathname++;
  }
  return last;
}

/*
 ============
 COM_GetExtension
 ============
 */
const char *COM_GetExtension(const char *name)
{
  const char *dot = strrchr(name, '.');
  const char *slash = strrchr(name, '/');

  if (dot && (!slash || slash < dot))
    return dot + 1;
  else
    return "";
}



/*
 ============
 COM_StripExtension
 ============
 */
void COM_StripExtension(const char *in, char *out, size_t destsize)
{
  const char *dot = strrchr(in, '.');
  const char *slash = strrchr(in, '/');

  if (dot && (!slash || slash < dot))
    Q_strncpyz(out, in, ((int)destsize < dot-in+1 ? (int)destsize : dot-in+1));
  else
    Q_strncpyz(out, in, destsize);
}

/*
============
COM_StripExtension2
a safer version
============
*/
void COM_StripExtension2(const char *in, char *out, int destsize)
{
  int len = 0;

  while (len < destsize - 1 && *in && *in != '.')
  {
    *out++ = *in++;
    len++;
  }

  *out = 0;
}

void COM_StripExtension3(const char *src, char *dest, size_t destsize)
{
  int length;

  Q_strncpyz(dest, src, destsize);

  length = (int)strlen(dest) - 1;

  while (length > 0 && dest[length] != '.')
  {
    length--;

    if (dest[length] == '/')
    {
      return; // no extension
    }
  }

  if (length)
  {
    dest[length] = 0;
  }
}
/*
============
COM_CompareExtension
string compare the end of the strings and return qtrue if strings match
============
*/
qboolean COM_CompareExtension(const char *in, const char *ext)
{
  size_t inlen, extlen;
  inlen = strlen(in);
  extlen = strlen(ext);
  if(extlen <= inlen)
  {
    in += inlen - extlen;
    if(!Q_stricmp(in, ext))
      return qtrue;
  }
  return qfalse;
}

/*
 ==================
 COM_DefaultExtension
if path doesn't have an extension, then append
 the specified one (which should include the .)
 ==================
 */
void COM_DefaultExtension(char *path, int maxSize, const char *extension)
{
  const char *dot = strrchr(path, '.');
  const char *slash = strrchr(path, '/');

  if (dot && (!slash || slash < dot))
    return;
  else
    Q_strcat(path, maxSize, extension);
}
/*
============
Com_HashKey
============
*/
int Com_HashKey(char *string, int maxlength)
{
  int             i;
  long            hash;
  char            letter;
  hash = 0;
  i = 0;
  while(string[i] != '\0' && i < maxlength)
  {
    letter = tolower(string[i]);
    if(letter == '.')
      break;				// don't include extension
    if(letter == '\\')
      letter = '/';		// damn path names
    if(letter == PATH_SEP)
      letter = '/';		// damn path names
    hash += (long)(letter) * (i + 119);
    i++;
  }
  hash = (hash ^ (hash >> 10) ^ (hash >> 20));
  return hash;
}

/*
 ============================================================================

 BYTE ORDER FUNCTIONS

 ============================================================================
 */
/*
 // can't just use function pointers, or dll linkage can
 // mess up when qcommon is included in multiple places
 static short	(*_BigShort) (short l);
 static short	(*_LittleShort) (short l);
 static int		(*_BigLong) (int l);
 static int		(*_LittleLong) (int l);
 static qint64	(*_BigLong64) (qint64 l);
 static qint64	(*_LittleLong64) (qint64 l);
 static float	(*_BigFloat) (const float *l);
 static float	(*_LittleFloat) (const float *l);

 short	BigShort(short l){return _BigShort(l);}
 short	LittleShort(short l) {return _LittleShort(l);}
 int		BigLong (int l) {return _BigLong(l);}
 int		LittleLong (int l) {return _LittleLong(l);}
 qint64  BigLong64 (qint64 l) {return _BigLong64(l);}
 qint64  LittleLong64 (qint64 l) {return _LittleLong64(l);}
 float	BigFloat (const float *l) {return _BigFloat(l);}
 float	LittleFloat (const float *l) {return _LittleFloat(l);}
 */

short ShortSwap(short l)
{
  byte b1, b2;

  b1 = l & 255;
  b2 = (l >> 8) & 255;

  return (b1 << 8) + b2;
}

short ShortNoSwap(short l)
{
  return l;
}

int LongSwap(int l)
{
  byte b1, b2, b3, b4;

  b1 = l & 255;
  b2 = (l >> 8) & 255;
  b3 = (l >> 16) & 255;
  b4 = (l >> 24) & 255;

  return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

int LongNoSwap(int l)
{
  return l;
}

qint64 Long64Swap(qint64 ll)
{
  qint64 result;

  result.b0 = ll.b7;
  result.b1 = ll.b6;
  result.b2 = ll.b5;
  result.b3 = ll.b4;
  result.b4 = ll.b3;
  result.b5 = ll.b2;
  result.b6 = ll.b1;
  result.b7 = ll.b0;

  return result;
}

qint64 Long64NoSwap(qint64 ll)
{
  return ll;
}

float FloatSwap(const float *f)
{
  floatint_t out;

  out.f = *f;
  out.ui = LongSwap(out.ui);

  return out.f;
}

float FloatNoSwap(const float *f)
{
  return *f;
}

/*
 ================
 Swap_Init
 ================
 */
/*
 void Swap_Init (void)
 {
 byte	swaptest[2] = {1,0};

 // set the byte swapping variables in a portable manner
 if ( *(short *)swaptest == 1)
 {
 _BigShort = ShortSwap;
 _LittleShort = ShortNoSwap;
 _BigLong = LongSwap;
 _LittleLong = LongNoSwap;
 _BigLong64 = Long64Swap;
 _LittleLong64 = Long64NoSwap;
 _BigFloat = FloatSwap;
 _LittleFloat = FloatNoSwap;
 }
 else
 {
 _BigShort = ShortNoSwap;
 _LittleShort = ShortSwap;
 _BigLong = LongNoSwap;
 _LittleLong = LongSwap;
 _BigLong64 = Long64NoSwap;
 _LittleLong64 = Long64Swap;
 _BigFloat = FloatNoSwap;
 _LittleFloat = FloatSwap;
 }

 }
 */

/*
 ============================================================================

 PARSING

 ============================================================================
 */
static char ALIGN16(com_token[MAX_TOKEN_CHARS]);
static char ALIGN16(com_parsename[MAX_TOKEN_CHARS]);
//FIXME (0xA5EA):  use allignment
static int com_lines;

// multiple character punctuation tokens
static const char *punctuation[] =
{ "+=", "-=", "*=", "/=", "&=", "|=", "++", "--", "&&", "||", "<=", ">=", "==", "!=", NULL };

/*
 =================
 Com_SkipBracedSection

 The next token should be an open brace.
 Skips until a matching close brace is found.
 Internal brace depths are properly skipped.
 =================
 */
void Com_SkipBracedSection(char **program)
{
  char *token;
  int depth;

  depth = 0;

  do
  {
    token = Com_ParseExt(program, qtrue);
    if (token[1] == 0)
    {
      if (token[0] == '{')
        depth++;
      else if (token[0] == '}')
        depth--;
    }
  } while (depth && *program);
}
/*
 =================
 Com_SkipRestOfLine
 =================
 */
void Com_SkipRestOfLine(char **data)
{
  char *p;
  int c;

  p = *data;
  while ((c = *p++) != 0)
  {
    if (c == '\n')
    {
      com_lines++;
      break;
    }
  }

  *data = p;
}

void COM_BeginParseSession(const char *name)
{
  com_lines = 0;
  Com_sprintf(com_parsename, sizeof(com_parsename), "%s", name);
}

int COM_GetCurrentParseLine(void)
{
  return com_lines;
}

char *COM_Parse(char **data_p)
{
#if 1 // mit 0 geht console nicht mehr auf
  return COM_ParseExt(data_p, qtrue);

#else
  return Com_ParseExt( data_p, qtrue );

#endif
}

void COM_ParseError(char *format, ...)
{
  va_list argptr;
  static char string[4096];

  va_start(argptr, format);
  Q_vsnprintf(string, sizeof(string), format, argptr);
  va_end(argptr);

  Com_Printf("ERROR: %s, line %d: %s\n", com_parsename, com_lines, string);
}

void COM_ParseWarning(char *format, ...)
{
  va_list argptr;
  static char string[4096];

  va_start(argptr, format);
  Q_vsnprintf(string, sizeof(string), format, argptr);
  va_end(argptr);

  Com_Printf("WARNING: %s, line %d: %s\n", com_parsename, com_lines, string);
}

/*
 ==============
 COM_Parse

 Parse a token out of a string
 Will never return NULL, just empty strings

 If "allowLineBreaks" is qtrue then an empty
 string will be returned if the next token is
 a newline.
 ==============
 */
static ID_INLINE char *SkipWhitespace(char *data, qboolean *hasNewLines)
{
  int c;

  while ((c = *data) <= ' ')
  {
    if (!c)
      return NULL;

    if (c == '\n')
    {
      com_lines++;
      *hasNewLines = qtrue;
    }
    data++;
  }

  return data;
}

int COM_Compress(char *data_p)
{
  char *in, *out;
  int c;
  qboolean newline = qfalse, whitespace = qfalse;

  if (!data_p) //add hypov8
    return 0; //add hypov8

  in = out = data_p;
  if (in)
  {
    while ((c = *in) != 0)
    {
      // skip double slash comments
      if (c == '/' && in[1] == '/')
      {
        while (*in && *in != '\n')
          in++;
        // skip /* */ comments
      }
      else if (c == '/' && in[1] == '*')
      {
        while (*in && (*in != '*' || in[1] != '/'))
          in++;
        if (*in)
          in += 2;
        // record when we hit a newline
      }
      else if (c == '\n' || c == '\r')
      {
        newline = qtrue;
        in++;
        // record when we hit whitespace
      }
      else if (c == ' ' || c == '\t')
      {
        whitespace = qtrue;
        in++;
        // an actual token
      }
      else
      {
        // if we have a pending newline, emit it (and it counts as whitespace)
        if (newline)
        {
          *out++ = '\n';
          newline = qfalse;
          whitespace = qfalse;
        }
        if (whitespace)
        {
          *out++ = ' ';
          whitespace = qfalse;
        }

        // copy quoted strings unmolested
        if (c == '"')
        {
          *out++ = c;
          in++;
          while (1)
          {
            c = *in;
            if (c && c != '"')
            {
              *out++ = c;
              in++;
            }
            else
              break;
          }
          if (c == '"')
          {
            *out++ = c;
            in++;
          }
        }
        else
        {
          *out = c;
          out++;
          in++;
        }
      }
    }
  }
  *out = 0;
  return out - data_p;
}

char *COM_ParseExt(char **data_p, qboolean allowLineBreaks)
{
#if 1   // mit null geht console nicht mehr auf
  int c = 0, len;
  qboolean hasNewLines = qfalse;
  char *data;

  data = *data_p;
  len = 0;
  com_token[0] = 0;

  // make sure incoming data is valid
  if (!data)
  {
    *data_p = NULL;
    return com_token;
  }

  while (1)
  {
    // skip whitespace
    data = SkipWhitespace(data, &hasNewLines);
    if (!data)
    {
      *data_p = NULL;
      return com_token;
    }
    if (hasNewLines && !allowLineBreaks)
    {
      *data_p = data;
      return com_token;
    }

    c = *data;

    // skip double slash comments
    if (c == '/' && data[1] == '/')
    {
      data += 2;
      while (*data && *data != '\n')
        data++;
    }
    // skip /* */ comments
    else if (c == '/' && data[1] == '*')
    {
      data += 2;
      while (*data && (*data != '*' || data[1] != '/'))
        data++;

      if (*data)
        data += 2;
    }
    else
      break;
  }

  // handle quoted strings
  if (c == '\"')
  {
    data++;
    while (1)
    {
      c = *data++;
      if (c == '\"' || !c)
      {
        com_token[len] = 0;
        *data_p = (char *)data;
        return com_token;
      }
      if (len < MAX_TOKEN_CHARS - 1)
      {
        com_token[len] = c;
        len++;
      }
    }
  }

  // parse a regular word
  do
  {
    if (len < MAX_TOKEN_CHARS - 1)
    {
      com_token[len] = c;
      len++;
    }
    data++;
    c = *data;
    if (c == '\n')
      com_lines++;
  }
  while (c > 32);

  com_token[len] = 0;

  *data_p = (char *)data;
  return com_token;

#else
  return Com_ParseExt( data_p, allowLineBreaks );

#endif
}

char *Com_Parse(char **data_p)
{
  return Com_ParseExt(data_p, qtrue);
}
char *Com_ParseExt(char **data_p, qboolean allowLineBreaks)
{
  int c = 0, len;
  qboolean hasNewLines = qfalse;
  char *data;
  const char **punc;

  if (!data_p)
    Com_Error(ERR_FATAL, "Com_ParseExt: NULL data_p");

  data = *data_p;
  len = 0;
  com_token[0] = 0;

  // make sure incoming data is valid
  if (!data)
  {
    *data_p = NULL;
    return com_token;
  }

  // skip whitespace
  while (1)
  {
    data = SkipWhitespace(data, &hasNewLines);
    if (!data)
    {
      *data_p = NULL;
      return com_token;
    }
    if (hasNewLines && !allowLineBreaks)
    {
      *data_p = data;
      return com_token;
    }

    c = *data;

    // skip double slash comments
    if (c == '/' && data[1] == '/')
    {
      data += 2;
      while (*data && *data != '\n')
        data++;
    }
    // skip /* */ comments
    else if (c == '/' && data[1] == '*')
    {
      data += 2;
      while (*data && (*data != '*' || data[1] != '/'))
        data++;
      if (*data)
        data += 2;
    }
    else
      break;   // a real token to parse
  }

  // handle quoted strings
  if (c == '\"')
  {
    data++;
    while (1)
    {
      c = *data++;

      if ((c == '\\') && (*data == '\"'))
        data++; // allow quoted strings to use \" to indicate the " character
      else if (c == '\"' || !c)
      {
        com_token[len] = 0;
        *data_p = (char *)data;
        return com_token;
      }
      else if (*data == '\n')
        com_lines++;

      if (len < MAX_TOKEN_CHARS - 1)
      {
        com_token[len] = c;
        len++;
      }
    }
  }

  // check for a number
  // is this parsing of negative numbers going to cause expression problems
  if (   (c >= '0' && c <= '9')
      || (c == '-' && data[1] >= '0' && data[1] <= '9')
      || (c == '.' && data[1] >= '0' && data[1] <= '9')
      || (c == '-' && data[1] == '.' && data[2] >= '0'
      && data[2] <= '9'))
  {
    do
    {
      if (len < MAX_TOKEN_CHARS - 1)
      {
        com_token[len] = c;
        len++;
      }
      data++;

      c = *data;
    } while ((c >= '0' && c <= '9') || c == '.');

    // parse the exponent
    if (c == 'e' || c == 'E')
    {
      if (len < MAX_TOKEN_CHARS - 1)
      {
        com_token[len] = c;
        len++;
      }
      data++;
      c = *data;

      if (c == '-' || c == '+')
      {
        if (len < MAX_TOKEN_CHARS - 1)
        {
          com_token[len] = c;
          len++;
        }
        data++;
        c = *data;
      }

      do
      {
        if (len < MAX_TOKEN_CHARS - 1)
        {
          com_token[len] = c;
          len++;
        }
        data++;

        c = *data;
      } while (c >= '0' && c <= '9');
    }

    if (len == MAX_TOKEN_CHARS)
    {
      len = 0;
    }
    com_token[len] = 0;

    *data_p = (char *)data;
    return com_token;
  }

  // check for a regular word
  // we still allow forward and back slashes in name tokens for pathnames
  // and also colons for drive letters
  if ((c >= 'a'   &&  c <= 'z') || (c >= 'A'   &&  c <= 'Z')  || (c == '_')  || (c == '/')  || (c == '\\') || (c == '$') || (c == '*')) //  for bad shader strings
  {
    do
    {
      if (len < MAX_TOKEN_CHARS - 1)
      {
        com_token[len] = c;
        len++;
      }
      data++;

      c = *data;
    } while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c == '-') || (c >= '0' && c <= '9') || (c == '/') || (c == '\\') || (c == ':') || (c == '.') || (c == '$') || (c == '*') || (c == '@'));

    if (len == MAX_TOKEN_CHARS)
    {
      len = 0;
    }
    com_token[len] = 0;

    *data_p = (char *)data;
    return com_token;
  }

  // check for multi-character punctuation token
  for (punc = punctuation; *punc; punc++)
  {
    size_t l;
    size_t j;

    l = qstrlen(*punc);
    for (j = 0; j < l; j++)
    {
      if (data[j] != (*punc)[j])
        break;
    }
    if (j == l)
    {
      // a valid multi-character punctuation
      Com_Memcpy(com_token, *punc, l);
      com_token[l] = 0;
      data += l;
      *data_p = (char *)data;
      return com_token;
    }
  }

  // single character punctuation
  com_token[0] = *data;
  com_token[1] = 0;
  data++;
  *data_p = (char *)data;

  return com_token;
}


// *INDENT-ON*

/*
 ==================
 COM_MatchToken
 ==================
 */
void COM_MatchToken(char **buf_p, char *match)
{
  char *token;

  token = COM_Parse(buf_p);
  if (qstrcmp(token, match))
    Com_Error(ERR_DROP, "MatchToken: %s != %s", token, match);
}

/*
=================
SkipBracedSection_Depth
=================
*/
qboolean SkipBracedSection_Depth(char **program, int depth)
{
  char *token;

  do
  {
    token = COM_ParseExt(program, qtrue);

    if (token[1] == 0)
    {
      if (token[0] == '{')
      {
        depth++;
      }
      else if (token[0] == '}')
      {
        depth--;
      }
    }
  } while (depth && *program);

  return depth == 0;
}

/*
 =================
 SkipBracedSection

 The next token should be an open brace.
 Skips until a matching close brace is found.
 Internal brace depths are properly skipped.
 =================
 */
void SkipBracedSection(char **program)
{
  char *token;
  int depth;

  depth = 0;

  do
  {
    token = COM_ParseExt(program, qtrue);
    if (token[1] == 0)
    {
      if (token[0] == '{')
        depth++;
      else if (token[0] == '}')
        depth--;
    }
  } while (depth && *program);
}

/*
 =================
 SkipRestOfLine
 =================
 */
void SkipRestOfLine(char **data)
{
  char *p;
  int c;

  p = *data;
  while ((c = *p++) != 0)
  {
    if (c == '\n')
    {
      com_lines++;
      break;
    }
  }

  *data = p;
}

void Parse1DMatrix(char **buf_p, int x, float *m)
{
  char *token;
  int i;

  COM_MatchToken(buf_p, "(");

  for (i = 0; i < x; i++)
  {
    token = COM_Parse(buf_p);
    m[i] = atof(token);
  }

  COM_MatchToken(buf_p, ")");
}

void Com_Parse1DMatrix(char **buf_p, int x, float *m, qboolean checkBrackets)
{
  char *token;
  int i;

  if (checkBrackets)
    COM_MatchToken(buf_p, "(");

  for (i = 0; i < x; i++)
  {
    token = COM_Parse(buf_p);
    m[i] = atof(token);
  }

  if (checkBrackets)
    COM_MatchToken(buf_p, ")");
}

void Parse2DMatrix(char **buf_p, int y, int x, float *m)
{
  int i;

  COM_MatchToken(buf_p, "(");

  for (i = 0; i < y; i++)
  {
    Parse1DMatrix(buf_p, x, m + i * x);
  }

  COM_MatchToken(buf_p, ")");
}

void Parse3DMatrix(char **buf_p, int z, int y, int x, float *m)
{
  int i;

  COM_MatchToken(buf_p, "(");

  for (i = 0; i < z; i++)
  {
    Parse2DMatrix(buf_p, y, x, m + i * x * y);
  }

  COM_MatchToken(buf_p, ")");
}

/*
 ===================
 Com_HexStrToInt
 ===================
 */
int Com_HexStrToInt(const char *str)
{
  if (!str || !str[0])
    return -1;

  // check for hex code
  if (str[0] == '0' && str[1] == 'x')
  {
    int i, n = 0;

    for (i = 2; i < (int)qstrlen(str); i++)
    {
      char digit;

      n *= 16;

      digit = tolower(str[i]);

      if (digit >= '0' && digit <= '9')
        digit -= '0';
      else if (digit >= 'a' && digit <= 'f')
        digit = digit - 'a' + 10;
      else
        return -1;

      n += digit;
    }

    return n;
  }

  return -1;
}

/*
============================================================================
LIBRARY REPLACEMENT FUNCTIONS
============================================================================
*/
int Q_isprint(int c)
{
  if (c >= 0x20 && c <= 0x7E)
    return (1);
  return (0);
}

int Q_islower(int c)
{
  if (c >= 'a' && c <= 'z')
    return (1);
  return (0);
}

int Q_isupper(int c)
{
  if (c >= 'A' && c <= 'Z')
    return (1);
  return (0);
}

int Q_isalpha(int c)
{
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
    return (1);
  return (0);
}

#if !defined (USE_ASM_LIB) || defined (Q3_VM)
char *Q_strrchr(const char *string, int c)
{
  char cc = c;
  char *s;
  char *sp = (char *)0;

  s = (char *)string;

  while (*s)
  {
    if (*s == cc)
      sp = s;
    s++;
  }
  if (cc == 0)
    sp = s;

  return sp;
}
#endif

qboolean Q_isanumber(const char *s)
{
#ifdef Q3_VM
  //FIXME: implement
  return qfalse;

#else
  char *p;
  double d;

  if (*s == '\0')
    return qfalse;

  d = strtod(s, &p);
  (void)d; // shutup compiler

  return *p == '\0';

#endif
}

qboolean Q_isintegral(float f)
{
  return (int)f == f;
}
#ifdef _MSC_VER
#include <cstdio>
/*
=============
Q_vsnprintf
Special wrapper function for Microsoft's broken _vsnprintf() function.
MinGW comes with its own snprintf() which is not broken.
=============
*/
size_t Q_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
  size_t retval;
  retval = _vsnprintf(str, size, format, ap);
  if(retval < 0 || retval == size)
  {
    // Microsoft doesn't adhere to the C99 standard of vsnprintf,
    // which states that the return value must be the number of
    // bytes written if the output string had sufficient length.
    //
    // Obviously we cannot determine that value from Microsoft's
    // implementation, so we have no choice but to return size.
    str[size - 1] = '\0';
    return size;
  }
  return retval;
}
#endif


int Q_stricmpn(const char *s1, const char *s2, size_t n) {
  int c1, c2;

  if ( s1 == NULL ) {
    if (s2 == NULL)
      return 0;
    else
      return -1;
  }
  else if (s2 == NULL)
    return 1;

  do {
    c1 = *s1++;
    c2 = *s2++;

    if (!n--) {
      return 0; // strings are equal until end point
    }

    if (c1 != c2) {
      if (c1 >= 'a' && c1 <= 'z') {
        c1 -= ('a' - 'A');
      }
      if (c2 >= 'a' && c2 <= 'z') {
        c2 -= ('a' - 'A');
      }
      if (c1 != c2) {
        return c1 < c2 ? -1 : 1;
      }
    }
  } while (c1);

  return 0; // strings are equal
}

int Q_strncmp(const char *s1, const char *s2, size_t n) {
  int c1, c2;

  do {
    c1 = *s1++;
    c2 = *s2++;

    if (!n--) {
      return 0;		// strings are equal until end point
    }
    if (c1 != c2) {
      return c1 < c2 ? -1 : 1;
    }
  } while (c1);

  return 0; // strings are equal
}
#if !defined(USE_ASM_LIB) && !defined(_MSC_VER) //hypov8
int Q_stricmp(const char *s1, const char *s2)
{
  return (s1 && s2) ? Q_stricmpn(s1, s2, 99999) : -1;
}
#endif

#if !defined (USE_ASM_LIB) || defined (Q3_VM)
char *Q_strlwr(char *s1)
{
  char *s;

  s = s1;
  while (*s)
  {
    *s = tolower(*s);
    s++;
  }
  return s1;
}

char *Q_strupr(char *s1)
{
  char *s;

  s = s1;
  while (*s)
  {
    *s = toupper(*s);
    s++;
  }
  return s1;
}
#endif

/*
 =============
 Q_strncpyz
 Safe strncpy that ensures a trailing zero
 =============
 */
#if 1 //!defined (USE_ASM_LIB) || defined (Q3_VM)
void Q_strncpyz(char *dest, const char *src, size_t destsize)
{
  if ( !dest )
    Com_Error(ERR_FATAL, "Q_strncpyz: NULL dest");

  if ( !src )
    Com_Error(ERR_FATAL, "Q_strncpyz: NULL src");

  if ( destsize < 1 )
    Com_Error(ERR_FATAL, "Q_strncpyz: destsize < 1");

  strncpy(dest, src, destsize - 1);
  dest[destsize - 1] = 0;
}
#else
void Q_strncpyz( char *dest, const char *src, int destsize )
{
  if ( !dest )
    Com_Error(ERR_FATAL, "Q_strncpyz: NULL dest");

  if ( !src )
    Com_Error(ERR_FATAL, "Q_strncpyz: NULL src");

  if ( destsize < 1 )
    Com_Error(ERR_FATAL, "Q_strncpyz: destsize < 1");

  A_substring(dest, src, 0, destsize - 2);
  dest[destsize - 1] = 0;
}
#endif

//FIXME(0xA5EA): does not work yet
#if 1 // !defined (USE_ASM_LIB) || defined (Q3_VM)
// never goes past bounds or leaves without a terminating 0
void Q_strcat(char *dest, size_t size, const char *src)
{
  size_t l1;
  l1 = qstrlen(dest);
  if (l1 >= size)
    Com_Error(ERR_FATAL, "Q_strcat: already overflowed");

  Q_strncpyz(dest + l1, src, size - l1);
}
#else

void Q_strcat(char *dest, int size, const char *src)
{
  int dest_len;
  dest_len = qstrlen(dest);
  if (dest_len >= size)
    Com_Error(ERR_FATAL, "Q_strcat: already overflowed");

  //A_substring(dest + dest_len, src, 0, size - dest_len - 1);
  A_substring(dest, src, dest_len, size - dest_len - 1);
  //dest[size - dest_len - 1] = 0;
}
#endif

int Q_strnicmp(const char *string1, const char *string2, size_t n)
{
  int c1, c2;

  if (string1 == NULL)
  {
    return (string2 == NULL) ? 0 : -1;
  }
  else if (string2 == NULL)
  {
    return 1;
  }

  do
  {
    c1 = *string1++;
    c2 = *string2++;

    if (!n--)
    {
      return 0; // Strings are equal until end point
    }

    if (c1 != c2)
    {
      if (c1 >= 'a' && c1 <= 'z')
      {
        c1 -= ('a' - 'A');
      }

      if (c2 >= 'a' && c2 <= 'z')
      {
        c2 -= ('a' - 'A');
      }

      if (c1 != c2)
      {
        return c1 < c2 ? -1 : 1;
      }
    }
  } while (c1);

  return 0; // Strings are equal
}

/*
 * Find the first occurrence of find in s.
 */
const char *Q_stristr(const char *s, const char *find)
{
  char c, sc;
  size_t len;

  if ((c = *find++) != 0)
  {
    if (c >= 'a' && c <= 'z')
      c -= ('a' - 'A');

    len = qstrlen(find);

    do
    {
      do
      {
        if ((sc = *s++) == 0)
          return NULL;

        if (sc >= 'a' && sc <= 'z')
          sc -= ('a' - 'A');
      }
      while (sc != c);
    }
    while (Q_stricmpn(s, find, len) != 0);
    s--;
  }
  return s;
}
/*
 =============
 Q_strreplace

 replaces content of find by replace in dest
 =============
 */
qboolean Q_strreplace(char *dest, int destsize, const char *find, const char *replace)
{
  size_t lstart, lfind, lreplace, lend;
  char *s;
  char backup[32000]; // big, but small enough to fit in PPC stack

  lend = qstrlen(dest);
  if (lend >= (size_t)destsize)
    Com_Error(ERR_FATAL, "Q_strreplace: already overflowed");

  s = Q_strstr(dest, find);

  if (!s)
    return qfalse;

  Q_strncpyz(backup, dest, lend + 1);
  lstart = s - dest;
  lfind    = qstrlen(find);
  lreplace = qstrlen(replace);

  strncpy(s, replace, destsize - lstart - 1);
  strncpy(s + lreplace, backup + lstart + lfind, destsize - lstart - lreplace - 1);

  return qtrue;
}

int Q_PrintStrlen(const char *string)
{
  int len;
  const char *p;

  if (!string)
    return 0;

  len = 0;
  p = string;
  while (*p)
  {
    if (Q_IsColorString( p ))
    {
      p += 2;
      continue;
    }
    p++;
    len++;
  }

  return len;
}

char *Q_CleanStr(char *string)
{
  char *d;
  char *s;
  int c;
  if (!string) //add hypov8
    return 0;

  s = string;
  d = string;
  while ((c = *s) != 0)
  {
    if (Q_IsColorString( s ))
      s++;
    else if (c >= 0x20 && c <= 0x7E)
      *d++ = c;
    s++;
  }
  *d = '\0';

  return string;
}

int Q_CountChar(const char *string, char tocount)
{
  int count;

  for (count = 0; *string; string++)
  {
    if (*string == tocount)
      count++;
  }

  return count;
}

size_t QDECL Com_sprintf(char *dest, size_t size, const char *fmt, ...)
{
  size_t len;
  va_list argptr;

  va_start(argptr, fmt);
  len = Q_vsnprintf(dest, size, fmt, argptr);
  va_end(argptr);

  if (len >= size)
    Com_Printf("Com_sprintf: Output length %d too short, require %d bytes.\n", (int)size, (int)len + 1);

  return len;
}

/*
 ============
 va

 does a varargs printf into a temp buffer, so I don't need to have
 varargs versions of all text functions.
 ============
 */
char	* QDECL va( char *format, ... ) {
  va_list argptr;
  static char string[2][32000]; // in case va is called by nested functions
  static int index = 0;
  char *buf;

  buf = string[index & 1];
  index++;

  va_start(argptr, format);
  Q_vsnprintf(buf, sizeof(*string), format, argptr);
  va_end(argptr);

  return buf;
}

/*
 ============
 Com_TruncateLongString

 Assumes buffer is atleast TRUNCATE_LENGTH big
 ============
 */
void Com_TruncateLongString(char *buffer, const char *s)
{
  size_t length = qstrlen(s);

  if (length <= TRUNCATE_LENGTH)
    Q_strncpyz(buffer, s, TRUNCATE_LENGTH);
  else
  {
    Q_strncpyz(buffer, s, (TRUNCATE_LENGTH / 2) - 3);
    Q_strcat(buffer, TRUNCATE_LENGTH, " ... ");
    Q_strcat(buffer, TRUNCATE_LENGTH, s + length - (TRUNCATE_LENGTH / 2) + 3);
  }
}

/*
 =====================================================================

 INFO STRINGS

 =====================================================================
 */

/*
 ===============
 Info_ValueForKey

 Searches the string for the given
 key and returns the associated value, or an empty string.
 FIXME: overflow check?
 ===============
 */
char *Info_ValueForKey(const char *s, const char *key)
{
  char pkey[BIG_INFO_KEY];
  static char value[2][BIG_INFO_VALUE]; // use two buffers so compares
  // work without stomping on each other
  static int valueindex = 0;
  char *o;

  if (!s || !key)
    return "";

  if (qstrlen(s) >= BIG_INFO_STRING)
    Com_Error(ERR_DROP, "Info_ValueForKey: oversize infostring");

  valueindex ^= 1;
  if (*s == '\\')
    s++;

  while (1)
  {
    o = pkey;
    while (*s != '\\')
    {
      if (!*s)
        return "";

      *o++ = *s++;
    }
    *o = 0;
    s++;

    o = value[valueindex];

    while (*s != '\\' && *s)
    {
      *o++ = *s++;
    }
    *o = 0;

    if (!Q_stricmp(key, pkey))
      return value[valueindex];

    if (!*s)
      break;
    s++;
  }

  return "";
}

/*
 ===================
 Info_NextPair

 Used to itterate through all the key/value pairs in an info string
 ===================
 */
void Info_NextPair(const char **head, char *key, char *value)
{
  char *o;
  const char *s;

  s = *head;

  if (*s == '\\')
    s++;

  key[0] = 0;
  value[0] = 0;

  o = key;
  while (*s != '\\')
  {
    if (!*s)
    {
      *o = 0;
      *head = s;
      return;
    }
    *o++ = *s++;
  }
  *o = 0;
  s++;

  o = value;
  while (*s != '\\' && *s)
  {
    *o++ = *s++;
  }
  *o = 0;

  *head = s;
}

/*
 ===================
 Info_RemoveKey
 ===================
 */
void Info_RemoveKey(char *s, const char *key)
{
  char *start;
  char pkey[MAX_INFO_KEY];
  char value[MAX_INFO_VALUE];
  char *o;

  if (qstrlen(s) >= MAX_INFO_STRING)
    Com_Error(ERR_DROP, "Info_RemoveKey: oversize infostring");

  if (Q_strrchr(key, '\\'))
    return;

  while (1)
  {
    start = s;
    if (*s == '\\')
      s++;
    o = pkey;
    while (*s != '\\')
    {
      if (!*s)
        return;

      *o++ = *s++;
    }
    *o = 0;
    s++;

    o = value;
    while (*s != '\\' && *s)
    {
      if (!*s)
        return;

      *o++ = *s++;
    }
    *o = 0;

    if (!qstrcmp(key, pkey))
    {
      Com_Memmove(start, s, qstrlen(s) + 1); // remove this part
      return;
    }

    if (!*s)
      return;
  }
}

/*
 ===================
 Info_RemoveKey_Big
 ===================
 */
void Info_RemoveKey_Big(char *s, const char *key)
{
  char *start;
  char pkey[BIG_INFO_KEY];
  char value[BIG_INFO_VALUE];
  char *o;

  if (qstrlen(s) >= BIG_INFO_STRING)
    Com_Error(ERR_DROP, "Info_RemoveKey_Big: oversize infostring");

  if (Q_strrchr(key, '\\'))
    return;

  while (1)
  {
    start = s;
    if (*s == '\\')
      s++;
    o = pkey;
    while (*s != '\\')
    {
      if (!*s)
        return;

      *o++ = *s++;
    }
    *o = 0;
    s++;

    o = value;
    while (*s != '\\' && *s)
    {
      if (!*s)
        return;

      *o++ = *s++;
    }
    *o = 0;

    if (!qstrcmp(key, pkey))
    {
      qstrcpy(start, s); // remove this part
      return;
    }

    if (!*s)
      return;
  }

}

/*
 ==================
 Info_Validate

 Some characters are illegal in info strings because they
 can mess up the server's parsing
 ==================
 */
qboolean Info_Validate(const char *s)
{
  if (Q_strrchr(s, '\"'))
    return qfalse;
  if (Q_strrchr(s, ';'))
    return qfalse;
  return qtrue;
}

/*
 ==================
 Info_SetValueForKey

 Changes or adds a key/value pair
 ==================
 */
void Info_SetValueForKey(char *s, const char *key, const char *value)
{
  char newi[MAX_INFO_STRING];
  const char *blacklist = "\\;\"";

  if (qstrlen(s) >= MAX_INFO_STRING)
  {
    Com_Error(ERR_DROP, "Info_SetValueForKey: oversize infostring");
  }

  for (; *blacklist; ++blacklist)
  {
    if (Q_strrchr(key, *blacklist) || Q_strrchr(value, *blacklist))
    {
      Com_Printf(S_COLOR_YELLOW "Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value);
      return;
    }
  }

  Info_RemoveKey(s, key);
  if (!value || !qstrlen(value))
    return;

  Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

  if (qstrlen(newi) + qstrlen(s) >= MAX_INFO_STRING)
  {
    Com_Printf("Info string length exceeded\n");
    return;
  }

  qstrcat(newi, s);
  qstrcpy(s, newi);
}

/*
 ==================
 Info_SetValueForKey_Big

 Changes or adds a key/value pair
 ==================
 */
void Info_SetValueForKey_Big(char *s, const char *key, const char *value)
{
  char newi[BIG_INFO_STRING];
  const char *blacklist = "\\;\"";

  if (qstrlen(s) >= BIG_INFO_STRING)
  {
    Com_Error(ERR_DROP, "Info_SetValueForKey: oversize infostring");
  }

  for (; *blacklist; ++blacklist)
  {
    if (Q_strrchr(key, *blacklist) || Q_strrchr(value, *blacklist))
    {
      Com_Printf(S_COLOR_YELLOW "Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value);
      return;
    }
  }

  Info_RemoveKey_Big(s, key);
  if (!value)
    return;

  Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

  if (qstrlen(newi) + qstrlen(s) >= BIG_INFO_STRING)
  {
    Com_Printf("BIG Info string length exceeded\n");
    return;
  }

  qstrcat(s, newi);
}

//====================================================================

//unlagged - attack prediction #3
// moved from g_weapon.c
/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
  int		i;

  for ( i = 0 ; i < 3 ; i++ ) {
    if ( to[i] <= v[i] ) {
      v[i] = (int)v[i];
    } else {
      v[i] = (int)v[i] + 1;
    }
  }
}
//unlagged - attack prediction #3

/*
 ==================
 Com_CharIsOneOfCharset
 ==================
 */
static qboolean Com_CharIsOneOfCharset(char c, char *set)
{
  int i;

  for (i = 0; i < (int)qstrlen(set); i++)
  {
    if (set[i] == c)
      return qtrue;
  }

  return qfalse;
}

/*
 ==================
 Com_SkipCharset
 ==================
 */
char *Com_SkipCharset(char *s, char *sep)
{
  char *p = s;

  while (p)
  {
    if (Com_CharIsOneOfCharset(*p, sep))
      p++;
    else
      break;
  }
  return p;
}

/*
 ==================
 Com_SkipTokens
 ==================
 */
char *Com_SkipTokens(char *s, int numTokens, char *sep)
{
  int sepCount = 0;
  char *p = s;

  while (sepCount < numTokens)
  {
    if (Com_CharIsOneOfCharset(*p++, sep))
    {
      sepCount++;
      while (Com_CharIsOneOfCharset(*p, sep))
        p++;
    }
    else if (*p == '\0')
      break;
  }

  if (sepCount == numTokens)
    return p;
  else
    return s;
}

#if 0
/*
 =================
 SetPlaneSignbits
 =================
 */
void SetPlaneSignbits (cplane_t *out)
{
  int bits, j;

  // for fast box on planeside test
  bits = 0;
  for (j=0; j<3; j++)
  {
    if (out->normal[j] < 0)
    {
      bits |= 1<<j;
    }
  }
  out->signbits = bits;
}
#endif
#ifdef USE_PSK_MODEL_LOADER
memStream_t *AllocMemStream(byte *buffer, int bufSize)
{
  memStream_t *s;

  if (buffer == NULL || bufSize <= 0)
  return NULL;

  s = Com_Allocate(sizeof(memStream_t));
  if (s == NULL)
  return NULL;

  Com_Memset(s, 0, sizeof(memStream_t));

  s->buffer = buffer;
  s->curPos = buffer;
  s->bufSize = bufSize;
  s->flags = 0;

  return s;
}

void FreeMemStream(memStream_t * s)
{
  Com_Dealloc(s);
}

int MemStreamRead(memStream_t *s, void *buffer, int len)
{
  int ret = 1;

  if (s == NULL || buffer == NULL)
  return 0;

  if (s->curPos + len > s->buffer + s->bufSize)
  {
    s->flags |= MEMSTREAM_FLAGS_EOF;
    len = s->buffer + s->bufSize - s->curPos;
    ret = 0;

    Com_Error(ERR_FATAL, "MemStreamRead: EOF reached");
  }

  Com_Memcpy(buffer, s->curPos, len);
  s->curPos += len;

  return ret;
}

int MemStreamGetC(memStream_t *s)
{
  int c = 0;

  if (s == NULL)
  return -1;

  if (MemStreamRead(s, &c, 1) == 0)
  return -1;

  return c;
}

int MemStreamGetLong(memStream_t * s)
{
  int c = 0;

  if (s == NULL)
  return -1;

  if (MemStreamRead(s, &c, 4) == 0)
  return -1;

  return LittleLong(c);
}

int MemStreamGetShort(memStream_t * s)
{
  int c = 0;

  if (s == NULL)
  return -1;

  if (MemStreamRead(s, &c, 2) == 0)
  return -1;

  return LittleShort(c);
}

float MemStreamGetFloat(memStream_t * s)
{
  floatint_t c;

  if (s == NULL)
  return -1;

  if (MemStreamRead(s, &c.i, 4) == 0)
  return -1;

  return LittleFloat(c.f);
}
#endif
