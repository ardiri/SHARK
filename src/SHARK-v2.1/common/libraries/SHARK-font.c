/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-font.c
 */

#include "../../SHARK.h"

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define FNT_LIBRARY_ID  0x5F666E74 // '_fnt'

// font HTML escape sequences, numbered (&#xxx;) and named (&name;)
//
//   see: http://www.html-reference.com/Escape.htm
//
// font_raw:   simple uint8 character encoding (aka: latin1)
// font_utf8:  unicode utf8 character encoding
//
//   0x00000000-0x0000007f  0x0#######
//   0x00000080-0x000007ff  0x110##### 0x10######
//   0x00000800-0x0000ffff  0x1110#### 0x10###### 0x10######
//
//   see: http://www.cl.cam.ac.uk/~mgk25/unicode.html#utf-8
//        http://people.w3.org/rishida/scripts/uniview/conversion
//        http://www.ssec.wisc.edu/~tomw/java/unicode.html
//        http://www.alanwood.net/unicode/
//
// unicode spacing rules:
//   see: http://www.cs.tut.fi/~jkorpela/chars/spaces.html

#define FNT_GETCHAR(p,chr,chr_w)                              \
  chr   = 0;                                                  \
  chr_w = 1;                                                  \
  if ((p)[0] == '&')                                          \
  {                                                           \
    chr = '&';                                                \
    if ((p)[1] == '#')                                        \
    {                                                         \
      char *ps = _StrNChr(&(p)[2], ';', 7);                   \
      if (ps != NULL)                                         \
      {                                                       \
        if ((p)[2] == 'x')                                    \
          chr   = _StrAToH(&(p)[3]);                          \
        else                                                  \
          chr   = _StrAToI(&(p)[2]);                          \
        chr_w = (ps-(p)) + 1;                                 \
      }                                                       \
    }                                                         \
    else                                                      \
    if (_StrNCompare((p), "&apos;",   6) == 0) { chr =  39; chr_w = 6; } else \
    if (_StrNCompare((p), "&quot;",   6) == 0) { chr = '"'; chr_w = 6; } else \
    if (_StrNCompare((p), "&amp;",    5) == 0) { chr = '&'; chr_w = 5; } else \
    if (_StrNCompare((p), "&lt;",     4) == 0) { chr = '<'; chr_w = 4; } else \
    if (_StrNCompare((p), "&gt;",     4) == 0) { chr = '>'; chr_w = 4; } else \
    if (_StrNCompare((p), "&nbsp;",   6) == 0) { chr = 160; chr_w = 6; } else \
    if (_StrNCompare((p), "&iexcl;",  7) == 0) { chr = 161; chr_w = 7; } else \
    if (_StrNCompare((p), "&cent;",   6) == 0) { chr = 162; chr_w = 6; } else \
    if (_StrNCompare((p), "&pound;",  7) == 0) { chr = 163; chr_w = 7; } else \
    if (_StrNCompare((p), "&curren;", 8) == 0) { chr = 164; chr_w = 8; } else \
    if (_StrNCompare((p), "&yen;",    5) == 0) { chr = 165; chr_w = 5; } else \
    if (_StrNCompare((p), "&brvbar;", 8) == 0) { chr = 166; chr_w = 8; } else \
    if (_StrNCompare((p), "&sect;",   6) == 0) { chr = 167; chr_w = 6; } else \
    if (_StrNCompare((p), "&uml;",    5) == 0) { chr = 168; chr_w = 5; } else \
    if (_StrNCompare((p), "&copy;",   6) == 0) { chr = 169; chr_w = 6; } else \
    if (_StrNCompare((p), "&ordf;",   6) == 0) { chr = 170; chr_w = 6; } else \
    if (_StrNCompare((p), "&laquo;",  7) == 0) { chr = 171; chr_w = 7; } else \
    if (_StrNCompare((p), "&not;",    5) == 0) { chr = 172; chr_w = 5; } else \
    if (_StrNCompare((p), "&shy;",    5) == 0) { chr = 173; chr_w = 5; } else \
    if (_StrNCompare((p), "&reg;",    5) == 0) { chr = 174; chr_w = 5; } else \
    if (_StrNCompare((p), "&macr;",   6) == 0) { chr = 175; chr_w = 6; } else \
    if (_StrNCompare((p), "&deg;",    5) == 0) { chr = 176; chr_w = 5; } else \
    if (_StrNCompare((p), "&plusmn;", 8) == 0) { chr = 177; chr_w = 8; } else \
    if (_StrNCompare((p), "&sup2;",   6) == 0) { chr = 178; chr_w = 6; } else \
    if (_StrNCompare((p), "&sup3;",   6) == 0) { chr = 179; chr_w = 6; } else \
    if (_StrNCompare((p), "&acute;",  7) == 0) { chr = 180; chr_w = 7; } else \
    if (_StrNCompare((p), "&micro;",  7) == 0) { chr = 181; chr_w = 7; } else \
    if (_StrNCompare((p), "&para;",   6) == 0) { chr = 182; chr_w = 6; } else \
    if (_StrNCompare((p), "&middot;", 8) == 0) { chr = 183; chr_w = 8; } else \
    if (_StrNCompare((p), "&cedil;",  7) == 0) { chr = 184; chr_w = 7; } else \
    if (_StrNCompare((p), "&sup1;",   6) == 0) { chr = 185; chr_w = 6; } else \
    if (_StrNCompare((p), "&ordm;",   6) == 0) { chr = 186; chr_w = 6; } else \
    if (_StrNCompare((p), "&raquo;",  7) == 0) { chr = 187; chr_w = 7; } else \
    if (_StrNCompare((p), "&frac14;", 8) == 0) { chr = 188; chr_w = 8; } else \
    if (_StrNCompare((p), "&frac12;", 8) == 0) { chr = 189; chr_w = 8; } else \
    if (_StrNCompare((p), "&frac34;", 8) == 0) { chr = 190; chr_w = 8; } else \
    if (_StrNCompare((p), "&iquest;", 8) == 0) { chr = 191; chr_w = 8; } else \
    if (_StrNCompare((p), "&Agrave;", 8) == 0) { chr = 192; chr_w = 8; } else \
    if (_StrNCompare((p), "&Acute;",  7) == 0) { chr = 193; chr_w = 7; } else \
    if (_StrNCompare((p), "&Acirc;",  7) == 0) { chr = 194; chr_w = 7; } else \
    if (_StrNCompare((p), "&Atilde;", 8) == 0) { chr = 195; chr_w = 8; } else \
    if (_StrNCompare((p), "&Auml;",   6) == 0) { chr = 196; chr_w = 6; } else \
    if (_StrNCompare((p), "&Aring;",  7) == 0) { chr = 197; chr_w = 7; } else \
    if (_StrNCompare((p), "&AElig;",  7) == 0) { chr = 198; chr_w = 7; } else \
    if (_StrNCompare((p), "&Ccedil;", 8) == 0) { chr = 199; chr_w = 8; } else \
    if (_StrNCompare((p), "&Egrave;", 8) == 0) { chr = 200; chr_w = 8; } else \
    if (_StrNCompare((p), "&Eacute;", 8) == 0) { chr = 201; chr_w = 8; } else \
    if (_StrNCompare((p), "&Ecirc;",  7) == 0) { chr = 202; chr_w = 7; } else \
    if (_StrNCompare((p), "&Euml;",   6) == 0) { chr = 203; chr_w = 6; } else \
    if (_StrNCompare((p), "&Igrave;", 8) == 0) { chr = 204; chr_w = 8; } else \
    if (_StrNCompare((p), "&Iacute;", 8) == 0) { chr = 205; chr_w = 8; } else \
    if (_StrNCompare((p), "&Icirc;",  7) == 0) { chr = 206; chr_w = 7; } else \
    if (_StrNCompare((p), "&Iuml;",   6) == 0) { chr = 207; chr_w = 6; } else \
    if (_StrNCompare((p), "&ETH;",    5) == 0) { chr = 208; chr_w = 5; } else \
    if (_StrNCompare((p), "&Ntilde;", 8) == 0) { chr = 209; chr_w = 8; } else \
    if (_StrNCompare((p), "&Ograve;", 8) == 0) { chr = 210; chr_w = 8; } else \
    if (_StrNCompare((p), "&Oacute;", 8) == 0) { chr = 211; chr_w = 8; } else \
    if (_StrNCompare((p), "&Ocirc;",  7) == 0) { chr = 212; chr_w = 7; } else \
    if (_StrNCompare((p), "&Otilde;", 8) == 0) { chr = 213; chr_w = 8; } else \
    if (_StrNCompare((p), "&Ouml;",   6) == 0) { chr = 214; chr_w = 6; } else \
    if (_StrNCompare((p), "&times;",  7) == 0) { chr = 215; chr_w = 7; } else \
    if (_StrNCompare((p), "&Oslash;", 8) == 0) { chr = 216; chr_w = 8; } else \
    if (_StrNCompare((p), "&Ugrave;", 8) == 0) { chr = 217; chr_w = 8; } else \
    if (_StrNCompare((p), "&Uacute;", 8) == 0) { chr = 218; chr_w = 8; } else \
    if (_StrNCompare((p), "&Ucirc;",  7) == 0) { chr = 219; chr_w = 7; } else \
    if (_StrNCompare((p), "&Uuml;",   6) == 0) { chr = 220; chr_w = 6; } else \
    if (_StrNCompare((p), "&Yacute;", 8) == 0) { chr = 221; chr_w = 8; } else \
    if (_StrNCompare((p), "&THORN;",  7) == 0) { chr = 222; chr_w = 7; } else \
    if (_StrNCompare((p), "&szlig;",  7) == 0) { chr = 223; chr_w = 7; } else \
    if (_StrNCompare((p), "&agrave;", 8) == 0) { chr = 224; chr_w = 8; } else \
    if (_StrNCompare((p), "&aacute;", 8) == 0) { chr = 225; chr_w = 8; } else \
    if (_StrNCompare((p), "&acirc;",  7) == 0) { chr = 226; chr_w = 7; } else \
    if (_StrNCompare((p), "&atilde;", 8) == 0) { chr = 227; chr_w = 8; } else \
    if (_StrNCompare((p), "&auml;",   6) == 0) { chr = 228; chr_w = 6; } else \
    if (_StrNCompare((p), "&aring;",  7) == 0) { chr = 229; chr_w = 7; } else \
    if (_StrNCompare((p), "&aelig;",  7) == 0) { chr = 230; chr_w = 7; } else \
    if (_StrNCompare((p), "&ccedil;", 8) == 0) { chr = 231; chr_w = 8; } else \
    if (_StrNCompare((p), "&egrave;", 8) == 0) { chr = 232; chr_w = 8; } else \
    if (_StrNCompare((p), "&eacute;", 8) == 0) { chr = 233; chr_w = 8; } else \
    if (_StrNCompare((p), "&ecirc;",  7) == 0) { chr = 234; chr_w = 7; } else \
    if (_StrNCompare((p), "&euml;",   6) == 0) { chr = 235; chr_w = 6; } else \
    if (_StrNCompare((p), "&igrave;", 8) == 0) { chr = 236; chr_w = 8; } else \
    if (_StrNCompare((p), "&iacute;", 8) == 0) { chr = 237; chr_w = 8; } else \
    if (_StrNCompare((p), "&icirc;",  7) == 0) { chr = 238; chr_w = 7; } else \
    if (_StrNCompare((p), "&iuml;",   6) == 0) { chr = 239; chr_w = 6; } else \
    if (_StrNCompare((p), "&eth;",    5) == 0) { chr = 240; chr_w = 5; } else \
    if (_StrNCompare((p), "&ntidle;", 8) == 0) { chr = 241; chr_w = 8; } else \
    if (_StrNCompare((p), "&ograve;", 8) == 0) { chr = 242; chr_w = 8; } else \
    if (_StrNCompare((p), "&oacute;", 8) == 0) { chr = 243; chr_w = 8; } else \
    if (_StrNCompare((p), "&ocirc;",  7) == 0) { chr = 244; chr_w = 7; } else \
    if (_StrNCompare((p), "&otilde;", 8) == 0) { chr = 245; chr_w = 8; } else \
    if (_StrNCompare((p), "&ouml;",   6) == 0) { chr = 246; chr_w = 6; } else \
    if (_StrNCompare((p), "&divide;", 8) == 0) { chr = 247; chr_w = 8; } else \
    if (_StrNCompare((p), "&oslash;", 8) == 0) { chr = 248; chr_w = 8; } else \
    if (_StrNCompare((p), "&ugrave;", 8) == 0) { chr = 249; chr_w = 8; } else \
    if (_StrNCompare((p), "&uacute;", 8) == 0) { chr = 250; chr_w = 8; } else \
    if (_StrNCompare((p), "&ucirc;",  7) == 0) { chr = 251; chr_w = 7; } else \
    if (_StrNCompare((p), "&uuml;",   6) == 0) { chr = 252; chr_w = 6; } else \
    if (_StrNCompare((p), "&yacute;", 8) == 0) { chr = 253; chr_w = 8; } else \
    if (_StrNCompare((p), "&thorn;",  7) == 0) { chr = 254; chr_w = 7; } else \
    if (_StrNCompare((p), "&euro;",   6) == 0) { chr = 0x20AC; chr_w = 6; }   \
  }                                                           \
  else FNT_GETCHAR_ENC(p,chr,chr_w);

#define FNT_GETCHAR_ENC(p,chr,chr_w)                          \
       if (g_fnt->encoding == font_raw)  chr = (p)[0];        \
  else if (g_fnt->encoding == font_utf8)                      \
  {                                                           \
    if (((p)[0] & 0x80) == 0) chr = (p)[0];                   \
    else                                                      \
    {                                                         \
      uint8 i, len, bit, mask;                                \
      len = 0; bit = 0x40; mask = 0x3f;                       \
      while ((p)[0] & bit) { len++; bit >>= 1; mask >>= 1; }  \
      chr_w += len;                                           \
      i = 0; chr = ((p)[i++] & mask);                         \
      while (len--) chr = chr << 6 | ((p)[i++] & 0x3f);       \
    }                                                         \
  }                                                           \
  else _SysDebugMessage("_Fnt:: unsupported character encoding", true);

#define WORDWRAP_TERMINATE(c)                                 \
(((c) == 0)    || ((c) == '\n')  ||                           \
 ((c) == 13)   || ((c) == 0xB6))

#define WORDWRAP_WHITESPACE(c)                                \
(((c) == ' ')  || ((c) == '\t'))

#define WORDWRAP_NUMERIC(c)                                   \
(((c) == '.')  || ((c) == ','))

#define WORDWRAP_PUNCTUATION_BREAK(c)                         \
(((c) == '?')  || ((c) == '!')  ||                            \
 ((c) == 0xBF))

#define WORDWRAP_PUNCTUATION(c)                               \
(((c) == '.')  || ((c) == ',')  ||                            \
 ((c) == ':')  || ((c) == ';')  ||                            \
 ((c) == '`')  || ((c) == '"')  ||                            \
 ((c) == '\'') || ((c) == 0xB4))

#define WORDWRAP_MATH(c)                                      \
(((c) == '+')  || ((c) == '-')  ||                            \
 ((c) == '*')  || ((c) == '/')  ||                            \
 ((c) == '<')  || ((c) == '>')  ||                            \
 ((c) == '^')  || ((c) == '=')  ||                            \
 ((c) == 0xAB) || ((c) == 0xBB) ||                            \
 ((c) == 0xD7) || ((c) == 0xF7) || ((c) == 0xB1))

#define WORDWRAP_BRACKET(c)                                   \
(((c) == '(')  || ((c) == ')')  ||                            \
 ((c) == '[')  || ((c) == ']')  ||                            \
 ((c) == '{')  || ((c) == '}'))

#define WORDWRAP_INTERNET(c)                                  \
(((c) == '@')  || ((c) == '_'))

#define WORDWRAP_CURRENCY(c)                                  \
(((c) == '$')  || ((c) == 0xA2) ||                            \
 ((c) == 0xA3) || ((c) == 0xA5))

#define WORDWRAP_MISCELLANEOUS(c)                             \
(((c) == '~')  || ((c) == '#')  ||                            \
 ((c) == '%')  || ((c) == '|')  ||                            \
 ((c) == '&')  || ((c) == '\\') ||                            \
 ((c) == 0xA6) || ((c) == 0xA7))

#define WORDWRAP_PUNCTUATION_U(c)                             \
( ((c) == 0x2011)                     ||                      \
 (((c) >= 0x2018) && ((c) <= 0x2017)) ||                      \
 (((c) >= 0x2032) && ((c) <= 0x203a)) ||                      \
 (((c) >= 0x2043) && ((c) <= 0x2044)))

#define WORDWRAP_PUNCTUATION_U_BREAK(c)                       \
((((c) >= 0x2000) && ((c) <= 0x2010)) ||                      \
 (((c) >= 0x2012) && ((c) <= 0x2017)) ||                      \
 (((c) >= 0x2020) && ((c) <= 0x202e)) ||                      \
 (((c) >= 0x2030) && ((c) <= 0x2031)) ||                      \
 (((c) >= 0x203b) && ((c) <= 0x2042)) ||                      \
 (((c) >= 0x2045) && ((c) <= 0x2068)))

#define WORDWRAP_DEFINITIONS(c)                               \
if (WORDWRAP_TERMINATE(c)           ||                        \
    WORDWRAP_WHITESPACE(c)          ||                        \
    WORDWRAP_NUMERIC(c)             ||                        \
    WORDWRAP_PUNCTUATION(c)         ||                        \
    WORDWRAP_PUNCTUATION_BREAK(c)   ||                        \
    WORDWRAP_PUNCTUATION_U(c)       ||                        \
    WORDWRAP_PUNCTUATION_U_BREAK(c) ||                        \
    WORDWRAP_MATH(c)                ||                        \
    WORDWRAP_BRACKET(c)             ||                        \
    WORDWRAP_INTERNET(c)            ||                        \
    WORDWRAP_CURRENCY(c)            ||                        \
    WORDWRAP_MISCELLANEOUS(c))                                \
{                                                             \
  brkpos = result;                                            \
  brk_x  = x + _FntGetCharWidth((c));                         \
}

#define WORDWRAP_POSSIBLE_CANCEL(c)                           \
(WORDWRAP_PUNCTUATION(c)            ||                        \
 WORDWRAP_PUNCTUATION_U(c)          ||                        \
 WORDWRAP_NUMERIC(c)                ||                        \
 WORDWRAP_MATH(c)                   ||                        \
 WORDWRAP_BRACKET(c)                ||                        \
 WORDWRAP_INTERNET(c))

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct font_info
{
  boolean  in_use;
  char     name[MAX_FONTNAME_LENGTH];
  void    *extension;
} font_info;

typedef struct _FntGlobals
{
  font_style    style;            // the global style (_FntInitialize)
  font_encoding encoding;

  font          fnt;              // the active font
  color         fnt_color;
  font_info     fnt_info[MAX_FONTS];

  void         *extension;

  boolean     (*fnFntInitialize)();
  void        (*fnFntTerminate)(void);
  boolean     (*fnFntDefineFont)(font, void *);
  void        (*fnFntReleaseFont)(font);
  void        (*fnFntGetGlyphProperties)(uint32, uint32 *, uint32 *);
  void        (*fnFntReleaseGlyph)(gfx_window *);
  void        (*fnFntSetColor)(color);
  gfx_window *(*fnFntLoadGlyph)(uint32, rectangle *);

} _FntGlobals;

#define FNT_GLOBALS_ACCESS \
  _FntGlobals *g_fnt = (_FntGlobals *)_LibGetGlobals(FNT_LIBRARY_ID);

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void    _FntInitialize_RESIZE();
static void    _FntTerminate_RESIZE();

static uint32  _FntGetCharWidth(uint32 chr);
static void    _FntDrawChar(uint32 chr, coord x, coord y, draw_operation mode);
static boolean _FntDriverConfiguration(lcd_format *format, font_encoding encoding, font_style sytle);
static boolean _FntHandleEvent(event *e);

boolean
_FntInitialize(lcd_format *format, font_encoding encoding, font_style style)
{
  boolean      init_ok;
  _FntGlobals *g_fnt;

  // assume everything is ok
  init_ok = true;

  // lets see if we have already been initialized
  g_fnt = (_FntGlobals *)_LibGetGlobals(FNT_LIBRARY_ID);

  // entry condition
  if ((format == NULL) || (g_fnt != NULL)) return false;

  // register the library
  if (_LibRegister(FNT_LIBRARY_ID, "_FntLibrary", (void *)_FntHandleEvent))
  {
    // allocate globals memory for the library
    g_fnt = (_FntGlobals *)_MemPtrNew(sizeof(_FntGlobals), false);
    init_ok &= (g_fnt != NULL); if (!init_ok) goto FNT_INIT_ABORT;
    _LibSetGlobals(FNT_LIBRARY_ID, (void *)g_fnt);

    // initialize the _Fnt driver
    init_ok = _FntDriverConfiguration(format, encoding, style);
    if (!init_ok) goto FNT_INIT_ABORT;

    // initialize _Fnt internally
    _FntInitialize_RESIZE();
  }
  else
    init_ok = false;

FNT_INIT_ABORT:

  return init_ok;
}

void
_FntTerminate()
{
  FNT_GLOBALS_ACCESS;

  // entry requirement (must call _FntInitialize())
  if (g_fnt == NULL) return;

  // shutdown the sub-font layer
  if (g_fnt->fnFntTerminate != NULL)
    g_fnt->fnFntTerminate();

  // terminate _Fnt internally
  _FntTerminate_RESIZE();

  // free the globals used
  _MemPtrFree(g_fnt); g_fnt = NULL;
  _LibSetGlobals(FNT_LIBRARY_ID, (void *)g_fnt);

  // shutdown the library
  _LibShutdown(FNT_LIBRARY_ID);
}

font
_FntDefineFont(font_style style, char *name, void *fontData)
{
  font result, index;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = INVALID_FONT;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || (g_fnt->style != style)) return result;

  // maybe they are just "re-defining" an existing font (can happen)
  index = _FntLocateFont(name);
  if (index != INVALID_FONT) goto DEFINE_FOUND;
  
  // lets try and find an entry point
  index = 0;
  while ((g_fnt->fnt_info[index].in_use) && (index < MAX_FONTS))
    index++;

DEFINE_FOUND:

  // did we find a free spot?
  if ((index < MAX_FONTS) && (g_fnt->fnFntDefineFont != NULL))
  {
    if (g_fnt->fnFntDefineFont(index, fontData))
    {
      result = index;
      g_fnt->fnt_info[index].in_use = true;
      _StrCopy(g_fnt->fnt_info[index].name, name);
    }
  }

  return result;
}

font
_FntLocateFont(char *name)
{
  font result, index;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = INVALID_FONT;

  // entry requirement (must call _FntInitialize())
  if (g_fnt == NULL) return result;

  // lets try and find the font in question an entry point
  index = 0;
  while ((index < MAX_FONTS) && (result == INVALID_FONT))
  {
    if (_StrCompare(g_fnt->fnt_info[index].name, name) == 0) result = index;
    index++;
  }

  return result;
}

font
_FntDefineFontFromFile(font_style style, char *name, 
                       uint32 disk, file *dir, char *fileName)
{
  font    result;
  file   *file;
  uint8  *fnt_buffer;
  uint32  cnt;

  // default return value
  result = INVALID_FONT;
  fnt_buffer = NULL;

  file = _FileGetHandle(disk, dir, fileName);
  if (file != NULL)
  {
    cnt = _FileGetSize(file);

    fnt_buffer = _MemPtrNew(cnt, false);
    if ((fnt_buffer != NULL) && (_FileLoadIntoBuffer(file, fnt_buffer, cnt)))
    {
      // lets define the font
      result = _FntDefineFont(style, name, fnt_buffer);
    }
    if (fnt_buffer != NULL) _MemPtrFree(fnt_buffer);

    _FileReleaseHandle(file);
  }

  return result;
}

void
_FntSetFont(font id)
{
  FNT_GLOBALS_ACCESS;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || (!g_fnt->fnt_info[id].in_use)) return;

  // set the active font
  g_fnt->fnt = id;
}

font
_FntGetFont()
{
  font result;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = INVALID_FONT;

  // entry requirement (must call _FntInitialize())
  if (g_fnt == NULL) return result;

  // obtain a reference to the active font
  result = g_fnt->fnt;

  return result;
}

void
_FntReleaseFont(font id)
{
  FNT_GLOBALS_ACCESS;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || (!g_fnt->fnt_info[id].in_use)) return;

  // call the appropriate library command
  if (g_fnt->fnFntReleaseFont != NULL)
  {
    g_fnt->fnFntReleaseFont(id);
    g_fnt->fnt_info[id].in_use = false;
  }
}

void
_FntSetColor(color c)
{
  FNT_GLOBALS_ACCESS;

  // entry requirement (must call _FntInitialize())
  if (g_fnt == NULL) return;

  // set the font color
  g_fnt->fnt_color = c;

  // call the appropriate library command
  if (g_fnt->fnFntSetColor != NULL)
    g_fnt->fnFntSetColor(c);
}

color
_FntGetColor()
{
  color result;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = _GfxGetPaletteIndex(0,0,0);

  // entry requirement (must call _FntInitialize())
  if (g_fnt == NULL) return result;

  // obtain the font color
  result = g_fnt->fnt_color;

  return result;
}

void
_FntDrawString(char *str, uint32 len, coord x, coord y, draw_operation mode)
{
  char *p;
  uint32 chr, chr_w;
  FNT_GLOBALS_ACCESS;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || 
      (!g_fnt->fnt_info[g_fnt->fnt].in_use) || 
      (str == NULL) || (len == 0)) return;

  // draw each character in the string, one after another
  p = str;
  do
  {
    // get the uint32 representation of the character
    chr_w = 1; FNT_GETCHAR(p, chr, chr_w); p += chr_w;

    // we dont draw \n or \t characters (they should be filtered)
    if ((chr != 0) && (chr != '\n') && (chr != '\t'))
    {
      _FntDrawChar(chr, x, y, mode);
      x += (coord)_FntGetCharWidth(chr);
    }

    len -= (chr_w - 1);
  }
  while (--len);
}

uint32
_FntGetCharsWidth(char *str, uint32 len)
{
  uint32 result;
  char  *p;
  uint32 chr, chr_w;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || 
      (!g_fnt->fnt_info[g_fnt->fnt].in_use) || 
      (str == NULL) || (len == 0)) return result;

  // calculate how wide the string can be
  p = str;
  do
  {
    // get the uint32 representation of the character
    chr_w = 1; FNT_GETCHAR(p, chr, chr_w); p += chr_w;

    // how wide was that character?
    result += _FntGetCharWidth(chr);
    len -= (chr_w - 1);
  }
  while (--len);

  return result;
}

uint32
_FntGetCharsInWidth(char *str, uint32 pixels)
{
  uint32 result;
  char  *p;
  uint32 chr, chr_w;
  uint32 x, len;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || 
      (!g_fnt->fnt_info[g_fnt->fnt].in_use) || (str == NULL)) return result;

  len = _StrLen(str);
  if (len == 0) return result;
  
  // calculate how wide the string can be
  x = 0;
  p = str;
  do
  {
    // get the uint32 representation of the character
    chr_w = 1; FNT_GETCHAR(p, chr, chr_w); p += chr_w;

    // how wide was the character; have we broken width? 
    x += _FntGetCharWidth(chr);
    if (x >= pixels) break;

    // process...
    result += chr_w;
    len    -= (chr_w - 1);
  }
  while (--len);

  return result;
}

uint32
_FntGetFontHeight()
{
  uint32 result;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || (!g_fnt->fnt_info[g_fnt->fnt].in_use)) return result;

  // get the height of the font using FntGetGlyphProperties
  if (g_fnt->fnFntGetGlyphProperties != NULL)
    g_fnt->fnFntGetGlyphProperties('0', NULL, &result);

  return result;
}

uint32
_FntGetWordWrap(char *str, uint32 maxPixels, boolean *force, uint32 *cntPixels)
{
  uint32  result;
  char   *p;
  uint32  x, chr, chr_w;
  uint32   brk_x, old_brk_x;
  uint32  brkpos, old_brkpos;
  boolean done;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || 
      (!g_fnt->fnt_info[g_fnt->fnt].in_use) || 
      (str == NULL) || (maxPixels < 1)) return result;

  // initialize
  p = str;
  x = 0;  // progress values
  brk_x  = 0;
  result = brkpos = 0;

  // process
  done = false;
  while (!done) 
  {
    old_brkpos = brkpos; 
    old_brk_x  = brk_x; 

    // get the uint32 representation of the character
    chr_w = 1; FNT_GETCHAR(p, chr, chr_w); p += chr_w; result += chr_w;

    // have we hit the end of the string/line?
    if ((*p == 0) || WORDWRAP_TERMINATE(chr)) 
    {
      done = true; 
      if ((x + _FntGetCharWidth(chr)) < maxPixels)
      {
        brkpos = result;
        brk_x  = x + _FntGetCharWidth(chr);
      }

      goto WORD_WRAP_DONE; // we are done
    }
    else
    if ((x + _FntGetCharWidth(chr)) < maxPixels)
    {
      // did we hit a possible word break?
      WORDWRAP_DEFINITIONS(chr);
      if (brkpos == result)
      {
        // maybe the next character can void the cancellation?
        if (WORDWRAP_POSSIBLE_CANCEL(chr))
        {
          if ((x + _FntGetCharWidth(chr)) > maxPixels)
          {
            done = true;
            if (brkpos == 0) { brkpos = old_brkpos; brk_x = old_brk_x; }

            goto WORD_WRAP_DONE; // we are done
          }

          // lets add this character
          x += _FntGetCharWidth(chr);

          // get the uint32 representation of the character
          chr_w = 1; FNT_GETCHAR(p, chr, chr_w); p += chr_w; result += chr_w;

          // have we hit the end of the string/line?
          if ((*p == 0) || WORDWRAP_TERMINATE(chr)) 
          {
            done   = true; 
            if ((x + _FntGetCharWidth(chr)) < maxPixels)
            {
              brkpos = result;
              brk_x  = x + _FntGetCharWidth(chr);
            }

            goto WORD_WRAP_DONE; // we are done
          }
          else
          { 
            brkpos = 0;
            WORDWRAP_DEFINITIONS(chr);  // lets try this char
            if ((brkpos == 0) || WORDWRAP_POSSIBLE_CANCEL(chr))
            { brkpos = old_brkpos; brk_x = old_brk_x; }
          }
        }
      }

      // have we gone too far?
      if ((x + _FntGetCharWidth(chr)) > maxPixels)
        done = true;
      else
        x += _FntGetCharWidth(chr);
    } 
    else
      done = true;
  }

WORD_WRAP_DONE:

  // what offset should we tell the developer to cut the string at?
  result = (brkpos != 0) ? brkpos : result-chr_w;

  // these are reporting information
  if (force)     *force     = (brkpos == 0);
  if (cntPixels) *cntPixels = (brk_x  != 0) ? brk_x : x;

  return result;
}

static void
_FntInitialize_RESIZE()
{
}

static void
_FntTerminate_RESIZE()
{
}

static uint32
_FntGetCharWidth(uint32 chr)
{
  uint32 result;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = 0;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || (!g_fnt->fnt_info[g_fnt->fnt].in_use)) return result;

  // get the width of the character using FntGetGlyphProperties
  if (g_fnt->fnFntGetGlyphProperties != NULL)
  {
    // ascii chars: no problem, fully encoded in font
    if (chr < 0x100)
      g_fnt->fnFntGetGlyphProperties(chr, &result, NULL); 
    else

    // unicode chars: less frequent, but customizations to be done!
    {
      switch (chr)
      {
        // 'em' based space chars
        case 0x2001:
        case 0x2003: result =  (_FntGetFontHeight() / 2);                break;
        case 0x2000:
        case 0x2002: result =  (_FntGetCharWidth(0x2001) / 2);           break;
        case 0x2004: result =  (_FntGetCharWidth(0x2001) / 3);           break;
        case 0x2005: result =  (_FntGetCharWidth(0x2001) / 4);           break;
        case 0x2006: result =  (_FntGetCharWidth(0x2001) / 6);           break;
        case 0x2009: result =  (_FntGetCharWidth(0x2001) / 5);           break;
        case 0x200a: result =  (_FntGetCharWidth(0x2001) / 5) - 1;       break;
        case 0x205f: result = ((_FntGetCharWidth(0x2001) * 4) / 18);     break;

        // width based on other properties
        case 0x2007: g_fnt->fnFntGetGlyphProperties('0', &result, NULL); break;
        case 0x2008: g_fnt->fnFntGetGlyphProperties('.', &result, NULL); break;
        case 0x3000: result = _FntGetCharsWidth("CJK", 3);               break;

        // no width
        case 0x200b: 
        case 0x202f: 
        case 0xfeff: result = 0;                                         break;

        // every other character
        default:     g_fnt->fnFntGetGlyphProperties(chr, &result, NULL); break;
      }
    }
  }

  return result;
}

static void
_FntDrawChar(uint32 chr, coord x, coord y, draw_operation mode)
{
  gfx_window    *win;
  rectangle      rect;
  FNT_GLOBALS_ACCESS;

  // entry requirement (must call _FntInitialize())
  if ((g_fnt == NULL) || (!g_fnt->fnt_info[g_fnt->fnt].in_use)) return;

  // load the glyph and copy it to the current draw window
  if ((g_fnt->fnFntLoadGlyph != NULL) && (g_fnt->fnFntReleaseGlyph != NULL))
  {
    // we have to handle some unicode characters in a special way
    if (chr > 0xff)
    {
      switch (chr)
      {
        case 0x2000: case 0x2001: case 0x2002: case 0x2003: 
        case 0x2004: case 0x2005: case 0x2006: case 0x2007:
        case 0x2008: case 0x2009: case 0x200a: case 0x200b:
        case 0x202f: case 0x205f: case 0x3000: case 0xfeff:  

             // we are dealing with custom space characters 
             win        = g_fnt->fnFntLoadGlyph(' ', &rect);
             rect.width = (int16)_FntGetCharWidth(chr);

             goto DRAW_GLPYH;  // this is a serious optimization!
             break;

        default: break;
      }
    }

    win = g_fnt->fnFntLoadGlyph(chr, &rect);
    if (win != NULL) 
    {

DRAW_GLPYH:

      _GfxCopyRegion(win, _GfxGetDrawWindow(), &rect, x, y, mode);
      g_fnt->fnFntReleaseGlyph(win);
    }
  }
}

#include "SHARK-font-bitmap.inc"

static boolean
_FntDriverConfiguration(lcd_format *format, 
                        font_encoding encoding, font_style style)
{
  boolean     result;
  FNT_GLOBALS_ACCESS;

  // default return value
  result = false;

  // we need to store this information for later use
  g_fnt->style     = style;
  g_fnt->encoding  = encoding;

  // we need to confirm that we can actually handle this configuration
  switch (style)
  {
    case font_bitmap:

         // initialize the _Fnt routines
         g_fnt->fnFntInitialize         = (void *)_Fnt_B_Initialize;
         g_fnt->fnFntTerminate          = (void *)_Fnt_B_Terminate;
         g_fnt->fnFntDefineFont         = (void *)_Fnt_B_DefineFont;
         g_fnt->fnFntReleaseFont        = (void *)_Fnt_B_ReleaseFont;
         g_fnt->fnFntGetGlyphProperties = (void *)_Fnt_B_GetGlyphProperties;
         g_fnt->fnFntReleaseGlyph       = (void *)_Fnt_B_ReleaseGlyph;
         g_fnt->fnFntSetColor           = (void *)_Fnt_B_SetColor;

         switch (format->type)
         {
           case lcd_palette:

                // whats the requested depth/palette size
                switch (format->depth)
                {
                  case 8:
                       g_fnt->fnFntLoadGlyph = (void *)_Fnt_B_I8_LoadGlyph;
                       break;

                  default:
                       break;
                }
                break;

           case lcd_direct:
                if (format->depth == 16)
                  g_fnt->fnFntLoadGlyph = (void *)_Fnt_B_D16_LoadGlyph;
                break;

           default:
                break;
         }
         break;

    case font_ttf:
         break;

    default:
         break;
  }

  // this is confirmation that we support this mode
  result = (g_fnt->fnFntLoadGlyph != NULL);

  // initialize the sub-font layer
  if (result && (g_fnt->fnFntInitialize != NULL))
    result = g_fnt->fnFntInitialize();

  // black and white by default
  _FntSetColor(_GfxGetPaletteIndex(0,0,0));

  return result;
}

static boolean
_FntHandleEvent(event *e)
{
  switch (e->eType)
  {
    case _keyEvent:
         switch (e->data.key.chr)
         {
           case _key_gfx_change:
                _FntTerminate_RESIZE();
                _FntInitialize_RESIZE();
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return false;
}

/********************************* EOF ***********************************/
