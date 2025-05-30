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
/*
=======================================================================

USER INTERFACE MAIN

=======================================================================
*/


// use this to get a demo build without an explicit demo build, i.e. to get the demo ui files to build
//#define PRE_RELEASE_TADEMO

#include "ui_local.h"

uiInfo_t uiInfo;

static const char *MonthAbbrev[] =
{
  "Jan", "Feb", "Mar",
  "Apr", "May", "Jun",
  "Jul", "Aug", "Sep",
  "Oct", "Nov", "Dec"
};

static const char *skillLevels[] =
{
  "I Can Win",
  "Bring It On",
  "Hurt Me Plenty",
  "Hardcore",
  "Nightmare"
};

static const int numSkillLevels = ARRAY_LEN( skillLevels );


static const char *netSources[] =
{
  "Favorites",
  "Local",
  "Internet (all masters)"
  /*"Internet (master0)",
  "Internet (master1)",
  "Internet (master2)",
  "Internet (master3)",
  "Internet (master4)",
  "Internet (master5)",*/

};
static const int numNetSources = ARRAY_LEN( netSources );
/*
static const serverFilter_t serverFilters[] = {
    {"All", "" },
    {"Quake 3 Arena", "" },
    {"Team Arena", "missionpack" },
    {"Rocket Arena", "arena" },
    {"Alliance", "alliance20" },
    {"Weapons Factory Arena", "wfa" },
    {"OSP", "osp" },
};
*/
static const serverFilter_t serverFilters[] = {
  {"All", ""},
  {"KingpinQ3", ""},
};

// FIXME: 0xA5EA, muss nicht gefilert werden, nur nach gametype
#if 0
static const char *servertype_items[] = {
  "All",
  "Gangbang", //"Free For All",	/* 0xA5EA */
  "Team Deathmatch",
  "1vs1",     //"Tournament",		/* 0xA5EA */
  "Capture the Flag",
  "Bagman",
  NULL
};
#endif
static const char *teamArenaGameTypes[] = {
  "Gangbang",
  "1 vs 1",
#ifdef USE_GT_SINGLEPLAYER
  "SP",
#endif
  "Team DM",
  "CTF",
  "OneFlag",
  "Bagman",
#ifdef GT_USE_TA_TYPES
  "HARVESTER",
  "TEAMTOURNAMENT"
#endif
};

static int const numTeamArenaGameTypes = sizeof(teamArenaGameTypes) / sizeof(const char *);

static const char *teamArenaGameNames[] =
{
  "gangbang",
  "1vs1",
#ifdef USE_GT_SINGLEPLAYER
  "single",
#endif
  "Team Deathmatch",
  "Capture the Flag",
  "OneFlag",
  "Bagman",
#ifdef GT_USE_TA_TYPES
  "Overload",
  "Harvester",
  "Team Tournament",
#endif
};

static int const numTeamArenaGameNames = sizeof(teamArenaGameNames) / sizeof(const char *);
static const int numServerFilters = sizeof(serverFilters) / sizeof(serverFilter_t);

static const char *sortKeys[] =
{
  "Server Name",
  "Map Name",
  "Open Player Spots",
  "Game Type",
  "Ping Time"
};
static const int numSortKeys = sizeof(sortKeys) / sizeof(const char *);

static char *netnames[] =
{
  "???",
  "UDP",
  "UDP6"
};

static char *ui_vidGLModeNames[4] = {
  "GL_LINEAR",
  "GL_LINEAR_MIPMAP_NEAREST",
  "GL_LINEAR_MIPMAP_LINEAR",
  NULL
};

//hypo new menu. item changes are stored tempory
#define UI_CVAR_STRCOUNT 27
//{ cvar, uix_cvar }
static char *ui_custStrings[UI_CVAR_STRCOUNT][2]{
  //{"", "uix_videoEdited", },
  {"",                                  "uix_r_texturemode_menu"}, //#0 //int (menu only)
  {"r_texturemode",                     "uix_r_texturemode"},      //#1 //string
  {"r_mode",                            "uix_r_mode"},             //#2 //uix_r_mode
  {"r_forceAmbient",                    "uix_r_forceAmbient"},     //#3 //float
  {"r_fullscreen",                      "uix_r_fullscreen"},       //#4 //int
  {"r_hdrRendering",                    "uix_r_hdrRendering"},     //...
  {"r_dynamicLight",                    "uix_r_dynamicLight"},
  {"r_dynamicLightCastShadows",         "uix_r_dynamicLightCastShadows"},
  {"cg_shadows",                        "uix_cg_shadows"},
  {"r_softShadows",                     "uix_r_softShadows"},
  {"r_shadowBlur",                      "uix_r_shadowBlur"},
  {"r_shadowMapQuality",                "uix_r_shadowMapQuality"},
  {"r_bloom",                           "uix_r_bloom"},
  {"r_dynamicBspOcclusionCulling",      "uix_r_dynamicBspOcclusionCulling"},
  {"r_picmip",                          "uix_r_picmip"},
  {"r_ext_texture_filter_anisotropic",  "uix_r_ext_texture_filter_anisotropic"},
  {"r_lodbias",                         "uix_r_lodbias"},
  {"r_reflectionmapping",               "uix_r_reflectionmapping"},
  {"r_ext_compressed_textures",         "uix_r_ext_compressed_textures"},
  {"r_halfLambertLighting",             "uix_r_halfLambertLighting"}, //player light style
  {"r_normalMapping",                   "uix_r_normalMapping"},
  {"r_parallaxMapping",                 "uix_r_parallaxMapping"},
  {"r_colorbits",                       "uix_r_colorbits"},
  {"r_subdivisions",                    "uix_r_subdivisions"},
  {"r_depthbits",                       "uix_r_depthbits"},
  {"r_stencilbits",                     "uix_r_stencilbits"},
  {"r_finish",                          "uix_r_finish"},
};





//#ifndef MISSIONPACK
//static char quake3worldMessage[] = "Visit www.kingpinq3.com - News, Community, Files";
//#endif

static int gamecodetoui[] = {4, 2, 3, 0, 5, 1, 6};        // hat was mit farben zu tun 0xA5EA
static int uitogamecode[] = {4, 6, 2, 3, 1, 5, 7};


static void UI_StartServerRefresh(qboolean full);
static void UI_StopServerRefresh(void);
static void UI_DoServerRefresh(void);
static void UI_FeederSelection(float feederID, int index);
static void UI_BuildServerDisplayList(qboolean force);
static void UI_BuildServerStatus(qboolean force);
static void UI_BuildFindPlayerList(qboolean force);
static int QDECL UI_ServersQsortCompare(const void *arg1, const void *arg2);
static int UI_MapCountByGameType(qboolean singlePlayer);
static int UI_HeadCountByTeam(void);
static void UI_ParseGameInfo(const char *teamFile);
static void UI_ParseTeamInfo(const char *teamFile);
static const char *UI_SelectedMap(int index, int *actual);
static const char *UI_SelectedHead(int index, int *actual);
#ifdef USE_GT_SINGLEPLAYER
static int UI_GetIndexFromSelection(int actual);
#endif
static void UI_UpdateLightCvar(void);
static void UI_UpdateCvarList(void);
static void UI_ResetTempCvarList(void);

int ProcessNewUI(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);

/*
================
vmMain
This is the only way control passes into the module.
This must be the very first function compiled into the .qvm file
================
*/
vmCvar_t ui_new;
vmCvar_t ui_debug;
vmCvar_t ui_initialized;
vmCvar_t ui_KingpinQ3FirstRun;

void _UI_Init(qboolean, int);
void _UI_Shutdown(void);
void _UI_KeyEvent(int key, qboolean down);
void _UI_MouseEvent(int dx, int dy);
void _UI_Refresh(int realtime);
qboolean _UI_IsFullscreen(void);
#ifdef __cplusplus
extern "C" Q_EXPORT intptr_t vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11)
#else
Q_EXPORT intptr_t vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11)
#endif
{
  switch (command)
  {
  case UI_GETAPIVERSION:
    return UI_API_VERSION;

  case UI_INIT:
    _UI_Init(arg0, arg1);
    return 0;

  case UI_SHUTDOWN:
    _UI_Shutdown();
    return 0;

  case UI_KEY_EVENT:
    _UI_KeyEvent(arg0, arg1);
    return 0;

  case UI_MOUSE_EVENT:
    _UI_MouseEvent(arg0, arg1);
    return 0;

  case UI_REFRESH:
    _UI_Refresh(arg0);
    return 0;

  case UI_IS_FULLSCREEN:
    return _UI_IsFullscreen();

  case UI_SET_ACTIVE_MENU:
    _UI_SetActiveMenu((uiMenuCommand_t)arg0);
    return 0;

  case UI_CONSOLE_COMMAND:
    return UI_ConsoleCommand(arg0);

  case UI_DRAW_CONNECT_SCREEN:
    UI_DrawConnectScreen(arg0);
    return 0;

#ifndef STANDALONE              // 0xA5EA
  case UI_HASUNIQUECDKEY: // mod authors need to observe this
    return qtrue;       // change this to qfalse for mods!

#endif

  }
  return -1;
}

void AssetCache(void)
{
  int n;
  //if (Assets.textFont == NULL) {
  //}
  //Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
  //Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
  uiInfo.uiDC.Assets.gradientBar         = trap_R_RegisterShaderNoMip(ASSET_GRADIENTBAR);
  uiInfo.uiDC.Assets.fxBasePic           = trap_R_RegisterShaderNoMip(ART_FX_BASE);
  uiInfo.uiDC.Assets.fxPic[0]            = trap_R_RegisterShaderNoMip(ART_FX_RED);
  uiInfo.uiDC.Assets.fxPic[1]            = trap_R_RegisterShaderNoMip(ART_FX_YELLOW);
  uiInfo.uiDC.Assets.fxPic[2]            = trap_R_RegisterShaderNoMip(ART_FX_GREEN);
  uiInfo.uiDC.Assets.fxPic[3]            = trap_R_RegisterShaderNoMip(ART_FX_TEAL);
  uiInfo.uiDC.Assets.fxPic[4]            = trap_R_RegisterShaderNoMip(ART_FX_BLUE);
  uiInfo.uiDC.Assets.fxPic[5]            = trap_R_RegisterShaderNoMip(ART_FX_CYAN);
  uiInfo.uiDC.Assets.fxPic[6]            = trap_R_RegisterShaderNoMip(ART_FX_WHITE);
  uiInfo.uiDC.Assets.scrollBar           = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR);
  uiInfo.uiDC.Assets.scrollBarArrowDown  = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWDOWN);
  uiInfo.uiDC.Assets.scrollBarArrowUp    = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWUP);
  uiInfo.uiDC.Assets.scrollBarArrowLeft  = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWLEFT);
  uiInfo.uiDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWRIGHT);
  uiInfo.uiDC.Assets.scrollBarThumb      = trap_R_RegisterShaderNoMip(ASSET_SCROLL_THUMB);
  uiInfo.uiDC.Assets.sliderBar           = trap_R_RegisterShaderNoMip(ASSET_SLIDER_BAR);
  uiInfo.uiDC.Assets.sliderThumb         = trap_R_RegisterShaderNoMip(ASSET_SLIDER_THUMB);

  for (n = 0; n < NUM_CROSSHAIRS; n++)
  {
    uiInfo.uiDC.Assets.crosshairShader[n] = trap_R_RegisterShaderNoMip(va("gfx/2d/crosshair%c", 'a' + n));
  }

  uiInfo.newHighScoreSound = trap_S_RegisterSound("sound/feedback/voc_newhighscore.ogg", qfalse); //hypov8 todo: end music..
}

void _UI_DrawSides(float x, float y, float w, float h, float size)
{
  UI_AdjustFrom640(&x, &y, &w, &h);
  size *= uiInfo.uiDC.xscale;
  trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader);
  trap_R_DrawStretchPic(x + w - size, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader);
}

void _UI_DrawTopBottom(float x, float y, float w, float h, float size)
{
  UI_AdjustFrom640(&x, &y, &w, &h);
  size *= uiInfo.uiDC.yscale;
  trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader);
  trap_R_DrawStretchPic(x, y + h - size, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader);
}
/*
================
UI_DrawRect
Coordinates are 640*480 virtual values
=================
*/
void _UI_DrawRect(float x, float y, float width, float height, float size, const float *color)
{
  trap_R_SetColor(color);

  _UI_DrawTopBottom(x, y, width, height, size);
  _UI_DrawSides(x, y, width, height, size);

  trap_R_SetColor(NULL);
}

int Text_Width(const char *text, float scale, int limit, fontInfo_t *Font)
{
  size_t count;
  size_t len;
  float out;
  glyphInfo_t *glyph;
  float useScale;
  const char *s    = text;
  fontInfo_t *font = Font;

  if (font == NULL_FONT)
  {
    font = &uiInfo.uiDC.Assets.textFont;
    if (scale <= ui_fontTiny.value)
      font = &uiInfo.uiDC.Assets.tinyFont;
    else if (scale <= ui_fontSmall.value)
      font = &uiInfo.uiDC.Assets.smallFont;
    else if (scale > ui_fontHuge.value)
      font = &uiInfo.uiDC.Assets.hugeFont;
    else if (scale > ui_fontBig.value)
      font = &uiInfo.uiDC.Assets.bigFont;
  }
  useScale = scale * font->glyphScale;
  out      = 0;
  if (text)
  {
    len = qstrlen(text);
    if (limit > 0 && len > (size_t)limit)
    {
      len = limit;
    }
    count = 0;
    while (s && *s && count < len)
    {
      if (Q_IsColorString(s))
      {
        s += 2;
        continue;
      }
      else
      {
        glyph = &font->glyphs[(int)*s];
        out  += glyph->xSkip;
        s++;
        count++;
      }
    }
  }
  return out * useScale;
}

int Text_Height(const char *text, float scale, int limit, fontInfo_t *Font)
{
  size_t len, count;
  float max;
  glyphInfo_t *glyph;
  float useScale;
  const char *s    = text;
  fontInfo_t *font = Font;

  if (font == NULL_FONT)
  {
    font = &uiInfo.uiDC.Assets.textFont;
    if (scale <= ui_fontTiny.value)
      font = &uiInfo.uiDC.Assets.tinyFont;
    else if (scale <= ui_fontSmall.value)
      font = &uiInfo.uiDC.Assets.smallFont;
    else if (scale > ui_fontHuge.value)
      font = &uiInfo.uiDC.Assets.hugeFont;
    else if (scale > ui_fontBig.value)
      font = &uiInfo.uiDC.Assets.bigFont;
  }
  useScale = scale * font->glyphScale;
  max      = 0;
  if (text)
  {
    len = qstrlen(text);
    if (limit > 0 && len > (size_t)limit)
      len = limit;

    count = 0;
    while (s && *s && count < len)
    {
      if (Q_IsColorString(s))
      {
        s += 2;
        continue;
      }
      else
      {
        glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
        if (max < glyph->height)
        {
          max = glyph->height;
        }
        s++;
        count++;
      }
    }
  }
  return max * useScale;
}

void Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader)
{
  float w, h;

  w = width * scale;
  h = height * scale;
  UI_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

void Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, fontInfo_t *Font)
{
  size_t len, count;
  vec4_t newColor;
  glyphInfo_t *glyph;
  float useScale;
  fontInfo_t *font = Font;

  if (font == NULL_FONT)
  {
    font = &uiInfo.uiDC.Assets.textFont;
    if (scale <= ui_fontTiny.value)
      font = &uiInfo.uiDC.Assets.tinyFont;
    else if (scale <= ui_fontSmall.value)
      font = &uiInfo.uiDC.Assets.smallFont;
    else if (scale > ui_fontHuge.value)
      font = &uiInfo.uiDC.Assets.hugeFont;
    else if (scale > ui_fontBig.value)
      font = &uiInfo.uiDC.Assets.bigFont;
  }
  useScale = scale * font->glyphScale;
  if (text)
  {
    const char *s = text;
    trap_R_SetColor(color);
    Com_Memcpy(&newColor[0], &color[0], sizeof(vec4_t));
    len = qstrlen(text);
    if (limit > 0 && len > (size_t)limit)
    {
      len = limit;
    }
    count = 0;
    while (s && *s && count < len)
    {
      glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
      //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
      if (Q_IsColorString(s))
      {
        Com_Memcpy(newColor, g_color_table[ColorIndex(*(s + 1))], sizeof(newColor));
        newColor[3] = color[3];
        trap_R_SetColor(newColor);
        s += 2;
        continue;
      }
      else
      {
        float yadj = useScale * glyph->top;
        if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE)
        {
          int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
          colorBlack[3] = newColor[3];
          trap_R_SetColor(colorBlack);
          Text_PaintChar(x + ofs, y - yadj + ofs,
                         glyph->imageWidth,
                         glyph->imageHeight,
                         useScale,
                         glyph->s,
                         glyph->t,
                         glyph->s2,
                         glyph->t2,
                         glyph->glyph);
          trap_R_SetColor(newColor);
          colorBlack[3] = 1.0;
        }
        Text_PaintChar(x, y - yadj,
                       glyph->imageWidth,
                       glyph->imageHeight,
                       useScale,
                       glyph->s,
                       glyph->t,
                       glyph->s2,
                       glyph->t2,
                       glyph->glyph);

        x += (glyph->xSkip * useScale) + adjust;
        s++;
        count++;
      }
    }
    trap_R_SetColor(NULL);
  }
}

void Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style)
{
  size_t len, count;
  vec4_t newColor;
  glyphInfo_t *glyph, *glyph2;
  float yadj;
  float useScale;
  fontInfo_t *font = &uiInfo.uiDC.Assets.textFont;
  //FIXME (0xA5EA): cleanup
#if 0
  if (scale <= ui_fontSmall.value)
  {
    font = &uiInfo.uiDC.Assets.smallFont;
  }
  else if (scale >= ui_fontBig.value)
  {
    font = &uiInfo.uiDC.Assets.bigFont;
  }
#endif
  if (scale <= ui_fontTiny.value)
    font = &uiInfo.uiDC.Assets.tinyFont;
  else if (scale <= ui_fontSmall.value)
    font = &uiInfo.uiDC.Assets.smallFont;
  else if (scale > ui_fontHuge.value)
    font = &uiInfo.uiDC.Assets.hugeFont;
  else if (scale > ui_fontBig.value)
    font = &uiInfo.uiDC.Assets.bigFont;

  useScale = scale * font->glyphScale;
  if (text)
  {
    const char *s = text;
    trap_R_SetColor(color);
    Com_Memcpy(&newColor[0], &color[0], sizeof(vec4_t));
    len = qstrlen(text);
    if (limit > 0 && len > (size_t)limit)
    {
      len = limit;
    }
    count  = 0;
    glyph2 = &font->glyphs[(int)cursor];
    while (s && *s && count < len)
    {
      glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
      //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
      if (Q_IsColorString(s))
      {
        Com_Memcpy(newColor, g_color_table[ColorIndex(*(s + 1))], sizeof(newColor));
        newColor[3] = color[3];
        trap_R_SetColor(newColor);
        s += 2;
        continue;
      }
      else
      {
        yadj = useScale * glyph->top;
        if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE)
        {
          int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
          colorBlack[3] = newColor[3];
          trap_R_SetColor(colorBlack);
          Text_PaintChar(x + ofs, y - yadj + ofs,
                         glyph->imageWidth,
                         glyph->imageHeight,
                         useScale,
                         glyph->s,
                         glyph->t,
                         glyph->s2,
                         glyph->t2,
                         glyph->glyph);
          colorBlack[3] = 1.0;
          trap_R_SetColor(newColor);
        }
        Text_PaintChar(x, y - yadj,
                       glyph->imageWidth,
                       glyph->imageHeight,
                       useScale,
                       glyph->s,
                       glyph->t,
                       glyph->s2,
                       glyph->t2,
                       glyph->glyph);

        yadj = useScale * glyph2->top;
        if (count == (size_t)cursorPos && !((uiInfo.uiDC.realTime / BLINK_DIVISOR) & 1))
        {
          Text_PaintChar(x, y - yadj,
                         glyph2->imageWidth,
                         glyph2->imageHeight,
                         useScale,
                         glyph2->s,
                         glyph2->t,
                         glyph2->s2,
                         glyph2->t2,
                         glyph2->glyph);
        }

        x += (glyph->xSkip * useScale);
        s++;
        count++;
      }
    }
    // need to paint cursor at end of text
    if ((size_t)cursorPos == len && !((uiInfo.uiDC.realTime / BLINK_DIVISOR) & 1))
    {
      yadj = useScale * glyph2->top;
      Text_PaintChar(x, y - yadj,
                     glyph2->imageWidth,
                     glyph2->imageHeight,
                     useScale,
                     glyph2->s,
                     glyph2->t,
                     glyph2->s2,
                     glyph2->t2,
                     glyph2->glyph);

    }

    trap_R_SetColor(NULL);
  }
}

static void Text_Paint_Limit(float *maxX, float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit)
{
  size_t count;
  size_t len;
  vec4_t newColor;
  glyphInfo_t *glyph;

  if (text)
  {
    const char *s = text;
    float max     = *maxX;
    float useScale;
    fontInfo_t *font = &uiInfo.uiDC.Assets.textFont;
    if (scale <= ui_fontTiny.value)
      font = &uiInfo.uiDC.Assets.tinyFont;
    else if (scale <= ui_fontSmall.value)
      font = &uiInfo.uiDC.Assets.smallFont;
    else if (scale > ui_fontHuge.value)
      font = &uiInfo.uiDC.Assets.hugeFont;
    else if (scale > ui_fontBig.value)
      font = &uiInfo.uiDC.Assets.bigFont;

    useScale = scale * font->glyphScale;
    trap_R_SetColor(color);
    len = qstrlen(text);
    if (limit > 0 && len > (size_t)limit)
    {
      len = limit;
    }
    count = 0;
    while (s && *s && count < len)
    {
      glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
      if (Q_IsColorString(s))
      {
        Com_Memcpy(newColor, g_color_table[ColorIndex(*(s + 1))], sizeof(newColor));
        newColor[3] = color[3];
        trap_R_SetColor(newColor);
        s += 2;
        continue;
      }
      else
      {
        float yadj = useScale * glyph->top;
        if (Text_Width(s, useScale, 1, NULL_FONT) + x > max)
        {
          *maxX = 0;
          break;
        }
        Text_PaintChar(x, y - yadj,
                       glyph->imageWidth,
                       glyph->imageHeight,
                       useScale,
                       glyph->s,
                       glyph->t,
                       glyph->s2,
                       glyph->t2,
                       glyph->glyph);
        x    += (glyph->xSkip * useScale) + adjust;
        *maxX = x;
        count++;
        s++;
      }
    }
    trap_R_SetColor(NULL);
  }
}

void UI_ShowPostGame(qboolean newHigh)
{
  trap_Cvar_Set("cg_cameraOrbit", "0");
  trap_Cvar_Set("cg_thirdPerson", "0");
  uiInfo.soundHighScore = newHigh;
  _UI_SetActiveMenu(UIMENU_POSTGAME);
}
/*
=================
_UI_Refresh
=================
*/
void UI_DrawCenteredPic(qhandle_t image, int w, int h)
{
  int x, y;

  x = (SCREEN_WIDTH - w) / 2;
  y = (SCREEN_HEIGHT - h) / 2;
  UI_DrawHandlePic(x, y, w, h, image);
}

int frameCount = 0;
int startTime;

#define UI_FPS_FRAMES 4
void _UI_Refresh(int realtime)
{
  static int index;
  static int previousTimes[UI_FPS_FRAMES];

  //if ( !( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
  //	return;
  //}

  uiInfo.uiDC.frameTime = realtime - uiInfo.uiDC.realTime;
  uiInfo.uiDC.realTime  = realtime;

  previousTimes[index % UI_FPS_FRAMES] = uiInfo.uiDC.frameTime;
  index++;
  if (index > UI_FPS_FRAMES)
  {
    int i, total;
    // average multiple frames together to smooth changes out a bit
    total = 0;
    for (i = 0; i < UI_FPS_FRAMES; i++)
    {
      total += previousTimes[i];
    }
    if (!total)
    {
      total = 1;
    }
    uiInfo.uiDC.FPS = 1000 * UI_FPS_FRAMES / total;
  }

  UI_UpdateCvars();

  if (Menu_Count() > 0)
  {
    // paint all the menus
    Menu_PaintAll();
    // refresh server browser list
    UI_DoServerRefresh();
    // refresh server status
    UI_BuildServerStatus(qfalse);
    // refresh find player list
    UI_BuildFindPlayerList(qfalse);
  }

  // draw cursor
  UI_SetColor(NULL);
  if (Menu_Count() > 0) //s
  {										//hypov8 was 16 & 32 // ui/cursor
    UI_DrawHandlePic(uiInfo.uiDC.cursorx - (UI_CURSOR_SIZE/2), uiInfo.uiDC.cursory - (UI_CURSOR_SIZE/2), UI_CURSOR_SIZE, UI_CURSOR_SIZE, uiInfo.uiDC.Assets.cursor);
  }

#ifndef NDEBUG
  if (uiInfo.uiDC.debug)
  {
    // cursor coordinates
    //FIXME
    //UI_DrawString( 0, 0, va("(%d,%d)",uis.cursorx,uis.cursory), UI_LEFT|UI_SMALLFONT, colorRed );
  }
#endif

}

/*
=================
_UI_Shutdown
=================
*/
void _UI_Shutdown(void)
{
  trap_LAN_SaveCachedServers();
}

char *defaultMenu = NULL;

char *GetMenuBuffer(const char *filename)
{
  int len;
  fileHandle_t f;
  static char buf[MAX_MENUFILE];

  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if (!f)
  {
    trap_Print(va(S_COLOR_RED "menu file not found: %s, using default\n", filename));
    return defaultMenu;
  }
  if (len >= MAX_MENUFILE)
  {
    trap_Print(va(S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE));
    trap_FS_FCloseFile(f);
    return defaultMenu;
  }

  trap_FS_Read(buf, len, f);
  buf[len] = 0;
  trap_FS_FCloseFile(f);
  //COM_Compress(buf);
  return buf;
}

qboolean Asset_Parse(int handle)
{
  pc_token_t token;
  const char *tempStr;

  if (!trap_PC_ReadToken(handle, &token))
    return qfalse;

  if (Q_stricmp(token.string, "{") != 0)
    return qfalse;

  while (1)
  {
    Com_Memset(&token, 0, sizeof(pc_token_t));

    if (!trap_PC_ReadToken(handle, &token))
      return qfalse;

    if (Q_stricmp(token.string, "}") == 0)
      return qtrue;

    // font
    if (Q_stricmp(token.string, "font") == 0)
    {
      int pointSize;

      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.textFont);
      uiInfo.uiDC.Assets.fontRegistered = qtrue;
      continue;
    }

    if (Q_stricmp(token.string, "tinyFont") == 0)
    {
      int pointSize;

      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.tinyFont);
      continue;
    }

    if (Q_stricmp(token.string, "smallFont") == 0)
    {
      int pointSize;

      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.smallFont);
      continue;
    }

    if (Q_stricmp(token.string, "bigFont") == 0)
    {
      int pointSize;

      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.bigFont);
      continue;
    }

    if (Q_stricmp(token.string, "hugeFont") == 0)
    {
      int pointSize;
      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.hugeFont);
      continue;
    }

    // gradientbar
    if (Q_stricmp(token.string, "gradientbar") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
      continue;
    }

    // enterMenuSound
    if (Q_stricmp(token.string, "menuEnterSound") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      uiInfo.uiDC.Assets.menuEnterSound = trap_S_RegisterSound(tempStr, qfalse);
      continue;
    }

    // exitMenuSound
    if (Q_stricmp(token.string, "menuExitSound") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      uiInfo.uiDC.Assets.menuExitSound = trap_S_RegisterSound(tempStr, qfalse);
      continue;
    }

    // itemFocusSound
    if (Q_stricmp(token.string, "itemFocusSound") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      uiInfo.uiDC.Assets.itemFocusSound = trap_S_RegisterSound(tempStr, qfalse);
      continue;
    }

    // menuBuzzSound
    if (Q_stricmp(token.string, "menuBuzzSound") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      uiInfo.uiDC.Assets.menuBuzzSound = trap_S_RegisterSound(tempStr, qfalse);
      continue;
    }

    if (Q_stricmp(token.string, "cursor") == 0)
    {
      if (!PC_String_Parse(handle, &uiInfo.uiDC.Assets.cursorStr))
        return qfalse;

      uiInfo.uiDC.Assets.cursor = trap_R_RegisterShaderNoMip(uiInfo.uiDC.Assets.cursorStr);
      continue;
    }

    if (Q_stricmp(token.string, "fadeClamp") == 0)
    {
      if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeClamp))
        return qfalse;

      continue;
    }

    if (Q_stricmp(token.string, "fadeCycle") == 0)
    {
      if (!PC_Int_Parse(handle, &uiInfo.uiDC.Assets.fadeCycle))
        return qfalse;

      continue;
    }

    if (Q_stricmp(token.string, "fadeAmount") == 0)
    {
      if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeAmount))
        return qfalse;

      continue;
    }

    if (Q_stricmp(token.string, "shadowX") == 0)
    {
      if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowX))
      {
        return qfalse;
      }
      continue;
    }

    if (Q_stricmp(token.string, "shadowY") == 0)
    {
      if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowY))
      {
        return qfalse;
      }
      continue;
    }

    if (Q_stricmp(token.string, "shadowColor") == 0)
    {
      if (!PC_Color_Parse(handle, &uiInfo.uiDC.Assets.shadowColor))
      {
        return qfalse;
      }
      uiInfo.uiDC.Assets.shadowFadeClamp = uiInfo.uiDC.Assets.shadowColor[3];
      continue;
    }

  }
  return qfalse;
}

void Font_Report(void)
{
  int i;

  Com_Printf("Font Info\n");
  Com_Printf("=========\n");
  for (i = 32; i < 96; i++)
  {
    Com_Printf("Glyph handle %i: %i\n", i, uiInfo.uiDC.Assets.textFont.glyphs[i].glyph);
  }
}

void UI_Report(void)
{
  String_Report();
  //Font_Report();
}

void UI_ParseMenu(const char *menuFile)
{
  int handle;
  pc_token_t token;

  Com_Printf("Parsing menu file:%s\n", menuFile);

  handle = trap_PC_LoadSource(menuFile);
  if (!handle)
  {
    return;
  }

  while (1)
  {
    Com_Memset(&token, 0, sizeof(pc_token_t));
    if (!trap_PC_ReadToken(handle, &token))
    {
      break;
    }

    //if ( Q_stricmp( token, "{" ) ) {
    //	Com_Printf( "Missing { in menu file\n" );
    //	break;
    //}

    //if ( menuCount == MAX_MENUS ) {
    //	Com_Printf( "Too many menus!\n" );
    //	break;
    //}

    if (token.string[0] == '}')
    {
      break;
    }

    if (Q_stricmp(token.string, "assetGlobalDef") == 0)
    {
      if (Asset_Parse(handle))
        continue;
      else
        break;
    }

    if (Q_stricmp(token.string, "menudef") == 0)
    {
      // start a new menu
      Menu_New(handle);
    }
  }
  trap_PC_FreeSource(handle);
}

qboolean Load_Menu(int handle)
{
  pc_token_t token;

  if (!trap_PC_ReadToken(handle, &token))
    return qfalse;

  if (token.string[0] != '{')
    return qfalse;

  while (1)
  {
    if (!trap_PC_ReadToken(handle, &token))
      return qfalse;

    if (token.string[0] == 0)
      return qfalse;

    if (token.string[0] == '}')
      return qtrue;

    UI_ParseMenu(token.string);
  }
  return qfalse;
}

void UI_LoadMenus(const char *menuFile, qboolean reset)
{
  pc_token_t token;
  int handle;
  int start;

  start = trap_Milliseconds();

  handle = trap_PC_LoadSource(menuFile);
  if (!handle)
  {
    Com_Printf( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile );
    handle = trap_PC_LoadSource("ui/menus.txt");
    if (!handle)
    {
      trap_Error(va(S_COLOR_RED "default menu file not found: ui/menus.txt, unable to continue!\n"));
    }
  }

  ui_new.integer = 1;

  if (reset)
  {
    Menu_Reset();
  }

  while (1)
  {
    if (!trap_PC_ReadToken(handle, &token))
      break;
    if (token.string[0] == 0 || token.string[0] == '}')
      break;

    if (token.string[0] == '}')
      break;

    if (Q_stricmp(token.string, "loadmenu") == 0)
    {
      if (Load_Menu(handle))
        continue;
      else
        break;
    }
  }

  Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

  trap_PC_FreeSource(handle);
}

void UI_Load(void)
{
  char lastName[1024];
  menuDef_t *menu = Menu_GetFocused();
  char *menuSet   = UI_Cvar_VariableString("ui_menuFiles");

  if (menu && menu->window.name)
  {
    qstrcpy(lastName, menu->window.name);
  }
  if (menuSet == NULL || menuSet[0] == '\0')
  {
    menuSet = "ui/menus.txt";
  }

  String_Init();

  UI_ParseGameInfo("gameinfo.txt");
  UI_LoadArenas();

  UI_LoadMenus(menuSet, qtrue);
  Menus_CloseAll();
  Menus_ActivateByName(lastName);

}

static const char *handicapValues[] = {"None", "95", "90", "85", "80", "75", "70", "65", "60", "55", "50", "45", "40", "35", "30", "25", "20", "15", "10", "5", NULL};
//#ifndef MISSIONPACK
//static int numHandicaps = sizeof(handicapValues) / sizeof(const char *);
//#endif

static void UI_DrawHandicap(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int i, h;

  h = Com_Clamp(5, 100, trap_Cvar_VariableValue("handicap"));
  i = 20 - h / 5;

  Text_Paint(rect->x, rect->y, scale, color, handicapValues[i], 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawClanName(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  Text_Paint(rect->x, rect->y, scale, color, UI_Cvar_VariableString("ui_teamName"), 0, 0, textStyle, NULL_FONT);
}


static void UI_SetCapFragLimits(qboolean uiVars)
{
  int cap  = 5;
  int frag = 10;
#ifdef GT_USE_TA_TYPES
  if (uiInfo.gameTypes[ui_gameType.integer].gtEnum == GT_OBELISK)
  {
    cap = 4;
  }
  else if (uiInfo.gameTypes[ui_gameType.integer].gtEnum == GT_HARVESTER)
  {
    cap = 15;
  }
#endif
  if (uiVars)
  {
    trap_Cvar_Set("ui_captureLimit", va("%d", cap));
    trap_Cvar_Set("ui_fragLimit", va("%d", frag));
  }
  else
  {
    trap_Cvar_Set("g_capturelimit", va("%d", cap)); //hypov8 "capturelimit"
    trap_Cvar_Set("g_fraglimit", va("%d", frag)); //hypov8 "fraglimit"
  }
}
// ui_gameType assumes gametype 0 is -1 ALL and will not show
static void UI_DrawGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  Text_Paint(rect->x, rect->y, scale, color, uiInfo.gameTypes[ui_gameType.integer].gameType, 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawNetGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  if (ui_netGameType.integer < 0 || ui_netGameType.integer > uiInfo.numGameTypes)
  {
    trap_Cvar_Set("ui_netGameType", "0");
    trap_Cvar_Set("ui_actualNetGameType", "0");
  }
  Text_Paint(rect->x, rect->y, scale, color, uiInfo.gameTypes[ui_netGameType.integer].gameType, 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawJoinGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  if (ui_joinGameType.integer < 0 || ui_joinGameType.integer > uiInfo.numJoinGameTypes)
  {
    trap_Cvar_Set("ui_joinGameType", "0");
  }
  Text_Paint(rect->x, rect->y, scale, color, uiInfo.joinGameTypes[ui_joinGameType.integer].gameType, 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawWeaponMode(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  if (ui_joinWeaponMode.integer < 0 || ui_joinWeaponMode.integer > uiInfo.numWeaponModes)
  {
    trap_Cvar_Set("ui_joinWeaponMode", "0");
  }
  Text_Paint(rect->x, rect->y, scale, color, uiInfo.weaponModes[ui_joinWeaponMode.integer].weaponMode, 0, 0, textStyle, NULL_FONT);
}

static int UI_TeamIndexFromName(const char *name)
{
  int i;

  if (name && *name)
  {
    for (i = 0; i < uiInfo.teamCount; i++)
    {
      if (Q_stricmp(name, uiInfo.teamList[i].teamName) == 0)
        return i;
    }
  }
  return 0;
}

static void UI_DrawClanLogo(rectDef_t *rect, float scale, vec4_t color)
{
  int i;

  i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
  if (i >= 0 && i < uiInfo.teamCount)
  {
    trap_R_SetColor(color);

    if (uiInfo.teamList[i].teamIcon == -1)
    {
      //FIXME(0xA5EA): _metal and _name
      uiInfo.teamList[i].teamIcon       = trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
      uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal", uiInfo.teamList[i].imageName));
      uiInfo.teamList[i].teamIcon_Name  = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
    }

    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon);
    trap_R_SetColor(NULL);
  }
}

static void UI_DrawClanCinematic(rectDef_t *rect, float scale, vec4_t color)
{
  int i;
  Com_Printf(S_COLOR_MAGENTA"UI_DrawClanCinematic\n");
  i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
  if (i >= 0 && i < uiInfo.teamCount)
  {

    if (uiInfo.teamList[i].cinematic >= -2)
    {
      if (uiInfo.teamList[i].cinematic == -1)
      {
        uiInfo.teamList[i].cinematic = trap_CIN_PlayCinematic(va("%s.ogv", uiInfo.teamList[i].imageName), 0, 0, 0, 0, (CIN_loop | CIN_silent));
      }
      if (uiInfo.teamList[i].cinematic >= 0)
      {
        trap_CIN_RunCinematic(uiInfo.teamList[i].cinematic);
        trap_CIN_SetExtents(uiInfo.teamList[i].cinematic, rect->x, rect->y, rect->w, rect->h);
        trap_CIN_DrawCinematic(uiInfo.teamList[i].cinematic);
      }
      else
      {
        trap_R_SetColor(color);
        UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Metal);
        trap_R_SetColor(NULL);
        uiInfo.teamList[i].cinematic = -2;
      }
    }
    else
    {
      trap_R_SetColor(color);
      UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon);
      trap_R_SetColor(NULL);
    }
  }
}

static void UI_DrawPreviewCinematic(rectDef_t *rect, float scale, vec4_t color)
{
  Com_Printf(S_COLOR_MAGENTA"UI_DrawPreviewCinematic\n");
  if (uiInfo.previewMovie > -2)
  {
    uiInfo.previewMovie = trap_CIN_PlayCinematic(va("%s.ogv", uiInfo.movieList[uiInfo.movieIndex]), 0, 0, 0, 0, (CIN_loop | CIN_silent));
    if (uiInfo.previewMovie >= 0)
    {
      trap_CIN_RunCinematic(uiInfo.previewMovie);
      trap_CIN_SetExtents(uiInfo.previewMovie, rect->x, rect->y, rect->w, rect->h);
      trap_CIN_DrawCinematic(uiInfo.previewMovie);
    }
    else
    {
      uiInfo.previewMovie = -2;
    }
  }
}

static void UI_DrawSkill(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int i;

  i = (int)trap_Cvar_VariableValue("g_spSkill");
  if (i < 1 || i > numSkillLevels)
  {
    i = 1;
  }
  Text_Paint(rect->x, rect->y, scale, color, skillLevels[i - 1], 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawTeamName(rectDef_t *rect, float scale, vec4_t color, qboolean blue, int textStyle)
{
  int i;

  i = UI_TeamIndexFromName(UI_Cvar_VariableString((blue) ? "ui_nikkiTeam" : "ui_dragonTeam"));
  if (i >= 0 && i < uiInfo.teamCount)
  {
    Text_Paint(rect->x, rect->y, scale, color, va("%s: %s", (blue) ? TEAM_NAME_NIKKIS : TEAM_NAME_DRAGONS, uiInfo.teamList[i].teamName), 0, 0, textStyle, NULL_FONT);
  }
}

static void UI_DrawTeamMember(rectDef_t *rect, float scale, vec4_t color, qboolean blue, int num, int textStyle)
{
  // 0 - None
  // 1 - Human
  // 2..NumCharacters - Bot
  char * one = "ui_nikkiTeam%i";
  char * two = "ui_dragonTeam%i"; //hypov8 todo: check this
  const char* xx = (const char*)va(blue ? one : two, num);
  int value = (int)trap_Cvar_VariableValue(xx);
  const char *text;

  if (value <= 0)
    text = "Closed";
  else if (value == 1)
    text = "Human";
  else
  {
    value -= 2;

    if (ui_actualNetGameType.integer >= GT_TEAM)
    {
      if (value >= uiInfo.characterCount)
        value = 0;
      text = uiInfo.characterList[value].name;
    }
    else
    {
      if (value >= UI_GetNumBots())
        value = 0;
      text = UI_GetBotNameByNumber(value);
    }
  }
  Text_Paint(rect->x, rect->y, scale, color, text, 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawEffects(rectDef_t *rect, float scale, vec4_t color)
{
  UI_DrawHandlePic(rect->x, rect->y - 14, 128, 8, uiInfo.uiDC.Assets.fxBasePic);
  UI_DrawHandlePic(rect->x + uiInfo.effectsColor * 16 + 8, rect->y - 16, 16, 12, uiInfo.uiDC.Assets.fxPic[uiInfo.effectsColor]);
}

static void UI_DrawMapPreview(rectDef_t *rect, float scale, vec4_t color, qboolean net)
{
  int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;

  if (map < 0 || map > uiInfo.mapCount)
  {
    if (net)
    {
      ui_currentNetMap.integer = 0;
      trap_Cvar_Set("ui_currentNetMap", "0");
    }
    else
    {
      ui_currentMap.integer = 0;
      trap_Cvar_Set("ui_currentMap", "0");
    }
    map = 0;
  }

  if (uiInfo.mapList[map].levelShot == -1)
  {
    uiInfo.mapList[map].levelShot = trap_R_RegisterShaderNoMip(uiInfo.mapList[map].imageName);
  }

  if (uiInfo.mapList[map].levelShot > 0)
  {
    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.mapList[map].levelShot);
  }
  else
  {
    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("unknownmap"));
  }
}

static void UI_DrawMapTimeToBeat(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int minutes, seconds, time;

  if (ui_currentMap.integer < 0 || ui_currentMap.integer > uiInfo.mapCount)
  {
    ui_currentMap.integer = 0;
    trap_Cvar_Set("ui_currentMap", "0");
  }

  time = uiInfo.mapList[ui_currentMap.integer].timeToBeat[uiInfo.gameTypes[ui_gameType.integer].gtEnum];

  minutes = time / 60;
  seconds = time % 60;

  Text_Paint(rect->x, rect->y, scale, color, va("%02i:%02i", minutes, seconds), 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawMapCinematic(rectDef_t *rect, float scale, vec4_t color, qboolean net)
{

  int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;
  if (map < 0 || map > uiInfo.mapCount)
  {
    if (net)
    {
      ui_currentNetMap.integer = 0;
      trap_Cvar_Set("ui_currentNetMap", "0");
    }
    else
    {
      ui_currentMap.integer = 0;
      trap_Cvar_Set("ui_currentMap", "0");
    }
    map = 0;
  }

  if (uiInfo.mapList[map].cinematic >= -1)
  {
    if (uiInfo.mapList[map].cinematic == -1)
    {
      uiInfo.mapList[map].cinematic = trap_CIN_PlayCinematic(va("%s.ogv", uiInfo.mapList[map].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent));
     // Com_Printf(S_COLOR_MAGENTA"trap_CIN_PlayCinematic uiInfo.mapList[map].cinematic  %d, %s\n",  uiInfo.mapList[map].cinematic, uiInfo.mapList[map].mapLoadName ); //hypov8 disable console string
    }
    if (uiInfo.mapList[map].cinematic >= 0)
    {
      //Com_Printf(S_COLOR_MAGENTA"trap_CIN_RunCinematic uiInfo.mapList[map].cinematic  %d, %s\n",  uiInfo.mapList[map].cinematic, uiInfo.mapList[map].mapLoadName ); //hypov8 disable console string
      trap_CIN_RunCinematic(uiInfo.mapList[map].cinematic);
      trap_CIN_SetExtents(uiInfo.mapList[map].cinematic, rect->x, rect->y, rect->w, rect->h);
      trap_CIN_DrawCinematic(uiInfo.mapList[map].cinematic);
    }
    else
    {
      uiInfo.mapList[map].cinematic = -2;
    }
  }
  else
  {
    UI_DrawMapPreview(rect, scale, color, net);
  }
}

static qboolean updateModel = qtrue;
static qboolean q3Model     = qfalse;

static void UI_DrawPlayerModel(rectDef_t *rect)
{
  static playerInfo_t info;
  char model[32];
  char team[32];
  char clan[32];
  char head[32];


  if ((int)trap_Cvar_VariableValue("ui_Q3Model")) //DM
  {
    Q_strncpyz(model, UI_Cvar_VariableString("model"), sizeof(model));
    Q_strncpyz(head, UI_Cvar_VariableString("headmodel"), sizeof(head)); //not used. extras?
    if (!q3Model)
    {
      q3Model     = qtrue;
      updateModel = qtrue;
    }
    clan[0] = '\0';
    team[0] = '\0';
  }
  else //TEAM
  {
    Q_strncpyz(team, UI_Cvar_VariableString("ui_team"), sizeof(team));
    if (!Q_stricmp(team, "1"))
      Q_strncpyz(team, TEAM_SKIN_DRAGONS, sizeof(team));
    else
      Q_strncpyz(team, TEAM_SKIN_NIKKIS, sizeof(team));
    Q_strncpyz(clan, UI_Cvar_VariableString("ui_teamName"), sizeof(clan));
    Q_strncpyz(model, UI_Cvar_VariableString("team_model"), sizeof(model));
    Q_strncpyz(head, UI_Cvar_VariableString("team_headmodel"), sizeof(head)); //not used. extras?
    if (q3Model)
    {
      q3Model     = qfalse;
      updateModel = qtrue;
    }
  }
  if (updateModel)
  {
    Com_Memset(&info, 0, sizeof(playerInfo_t));
    UI_PlayerInfo_SetModel( &info, model, head, clan, team);
    //UI_PlayerInfo_SetInfo(,);
    updateModel = qfalse;
  }
  UI_DrawPlayer(rect->x, rect->y, rect->w, rect->h, &info, uiInfo.uiDC.realTime / 2, 
    uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory); //reactive player rotation
}

static void UI_DrawNetSource(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  if (ui_netSource.integer < 0 || ui_netSource.integer > numNetSources)
  {
    ui_netSource.integer = 0;
  }
  Text_Paint(rect->x, rect->y, scale, color, va("Source: %s", netSources[ui_netSource.integer]), 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawNetMapPreview(rectDef_t *rect, float scale, vec4_t color)
{

  if (uiInfo.serverStatus.currentServerPreview > 0)
  {
    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.serverStatus.currentServerPreview);
  }
  else
  {
    UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("unknownmap"));
  }
}

static void UI_DrawNetMapCinematic(rectDef_t *rect, float scale, vec4_t color)
{
  if (ui_currentNetMap.integer < 0 || ui_currentNetMap.integer > uiInfo.mapCount)
  {
    ui_currentNetMap.integer = 0;
    trap_Cvar_Set("ui_currentNetMap", "0");
  }

  if (uiInfo.serverStatus.currentServerCinematic >= 0)
  { // hypov8 comment out display video stream Com_Printf
   // Com_Printf(S_COLOR_MAGENTA"trap_CIN_RunCinematic uiInfo.serverStatus.currentServerCinematic  %d,\n", uiInfo.serverStatus.currentServerCinematic);  //hypov8 disable console string
    trap_CIN_RunCinematic(uiInfo.serverStatus.currentServerCinematic);
    trap_CIN_SetExtents(uiInfo.serverStatus.currentServerCinematic, rect->x, rect->y, rect->w, rect->h);
    trap_CIN_DrawCinematic(uiInfo.serverStatus.currentServerCinematic);
  }
  else
  {
    UI_DrawNetMapPreview(rect, scale, color);
  }
}

static void UI_DrawNetFilter(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  if (ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters)
  {
    ui_serverFilterType.integer = 0;
  }
  Text_Paint(rect->x, rect->y, scale, color, va("Filter: %s", serverFilters[ui_serverFilterType.integer].description), 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawTier(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int i;

  i = (int)trap_Cvar_VariableValue("ui_currentTier");
  if (i < 0 || i >= uiInfo.tierCount)
  {
    i = 0;
  }
  Text_Paint(rect->x, rect->y, scale, color, va("Tier: %s", uiInfo.tierList[i].tierName), 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawTierMap(rectDef_t *rect, int index)
{
  int i;

  i = (int)trap_Cvar_VariableValue("ui_currentTier");
  if (i < 0 || i >= uiInfo.tierCount)
  {
    i = 0;
  }

  if (uiInfo.tierList[i].mapHandles[index] == -1)
  {
    uiInfo.tierList[i].mapHandles[index] = trap_R_RegisterShaderNoMip(va("levelshots/%s", uiInfo.tierList[i].maps[index]));
  }

  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.tierList[i].mapHandles[index]);
}

static const char *UI_EnglishMapName(const char *map)
{
  int i;

  for (i = 0; i < uiInfo.mapCount; i++)
  {
    if (Q_stricmp(map, uiInfo.mapList[i].mapLoadName) == 0)
    {
      return uiInfo.mapList[i].mapName;
    }
  }
  return "";
}

static void UI_DrawTierMapName(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int i, j;

  i = (int)trap_Cvar_VariableValue("ui_currentTier");
  if (i < 0 || i >= uiInfo.tierCount)
  {
    i = 0;
  }
  j = (int)trap_Cvar_VariableValue("ui_currentMap");
  if (j < 0 || j > MAPS_PER_TIER)
  {
    j = 0;
  }

  Text_Paint(rect->x, rect->y, scale, color, UI_EnglishMapName(uiInfo.tierList[i].maps[j]), 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawTierGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int i, j;

  i = (int)trap_Cvar_VariableValue("ui_currentTier");
  if (i < 0 || i >= uiInfo.tierCount)
  {
    i = 0;
  }
  j = (int)trap_Cvar_VariableValue("ui_currentMap");
  if (j < 0 || j > MAPS_PER_TIER)
  {
    j = 0;
  }

  Text_Paint(rect->x, rect->y, scale, color, uiInfo.gameTypes[uiInfo.tierList[i].gameTypes[j]].gameType, 0, 0, textStyle, NULL_FONT);
}
#ifdef USE_GT_SINGLEPLAYER
static const char *UI_AIFromName(const char *name)
{
  int j;

  for (j = 0; j < uiInfo.aliasCount; j++)
  {
    if (Q_stricmp(uiInfo.aliasList[j].name, name) == 0)
    {
      return uiInfo.aliasList[j].ai;
    }
  }
  return "Thug";       // 0xA5EA
}
#endif
static qboolean updateOpponentModel = qtrue;
static void UI_DrawOpponent(rectDef_t *rect)
{
  static playerInfo_t info2;
  char model[MAX_QPATH];
  char headmodel[MAX_QPATH];
  char team[32];
  char head[32];
  char clan[32];
  //vec3_t viewangles;
  vec3_t moveangles;
  //refEntity_t     body;
  //playerInfo_t Skin;
  if (updateOpponentModel)
  {

    qstrcpy(model, UI_Cvar_VariableString("ui_opponentModel"));
    qstrcpy(headmodel, UI_Cvar_VariableString("ui_opponentModel"));
    team[0] = '\0';
    clan[0] = '\0';
    //head[0] = '\0'; //add hypov8

    //qstrcpy(team, UI_Cvar_VariableString("ui_teamName"));
    //qstrcpy(model, UI_Cvar_VariableString("team_model"));
    qstrcpy(head, UI_Cvar_VariableString("team_headmodel")); //note hypov8 not changed with team_model

    Com_Memset(&info2, 0, sizeof(playerInfo_t));
    //viewangles[YAW]   = 180 - 10;
    //viewangles[PITCH] = 0;
    // viewangles[ROLL]  = 0;
    VectorClear(moveangles);
    UI_PlayerInfo_SetModel(&info2, model, head, clan, team);
    updateOpponentModel = qfalse;
  }

  UI_DrawPlayer(rect->x, rect->y, rect->w, rect->h, &info2, uiInfo.uiDC.realTime / 2,
    uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
}

static void UI_NextOpponent(void)
{
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
  int j = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));

  i++;
  if (i >= uiInfo.teamCount)
    i = 0;

  if (i == j)
  {
    i++;
    if (i >= uiInfo.teamCount)
      i = 0;
  }
  trap_Cvar_Set("ui_opponentName", uiInfo.teamList[i].teamName);
}

static void UI_PriorOpponent(void)
{
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
  int j = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));

  i--;
  if (i < 0)
    i = uiInfo.teamCount - 1;

  if (i == j)
  {
    i--;
    if (i < 0)
      i = uiInfo.teamCount - 1;
  }
  trap_Cvar_Set("ui_opponentName", uiInfo.teamList[i].teamName);
}

static void UI_DrawPlayerLogo(rectDef_t *rect, vec3_t color)
{
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));

  if (uiInfo.teamList[i].teamIcon == -1)
  {
    uiInfo.teamList[i].teamIcon       = trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name  = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon);
  trap_R_SetColor(NULL);
}

static void UI_DrawPlayerLogoMetal(rectDef_t *rect, vec3_t color)
{
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));

  if (uiInfo.teamList[i].teamIcon == -1)
  {
    uiInfo.teamList[i].teamIcon       = trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name  = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Metal);
  trap_R_SetColor(NULL);
}

static void UI_DrawPlayerLogoName(rectDef_t *rect, vec3_t color)
{
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));

  if (uiInfo.teamList[i].teamIcon == -1)
  {
    uiInfo.teamList[i].teamIcon       = trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name  = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Name);
  trap_R_SetColor(NULL);
}

static void UI_DrawOpponentLogo(rectDef_t *rect, vec3_t color)
{
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));

  if (uiInfo.teamList[i].teamIcon == -1)
  {
    uiInfo.teamList[i].teamIcon       = trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name  = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon);
  trap_R_SetColor(NULL);
}

static void UI_DrawOpponentLogoMetal(rectDef_t *rect, vec3_t color)
{
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));

  if (uiInfo.teamList[i].teamIcon == -1)
  {
    uiInfo.teamList[i].teamIcon       = trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name  = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Metal);
  trap_R_SetColor(NULL);
}

static void UI_DrawOpponentLogoName(rectDef_t *rect, vec3_t color)
{
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));

  if (uiInfo.teamList[i].teamIcon == -1)
  {
    uiInfo.teamList[i].teamIcon       = trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal", uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name  = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
  }

  trap_R_SetColor(color);
  UI_DrawHandlePic(rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Name);
  trap_R_SetColor(NULL);
}

static void UI_DrawAllMapsSelection(rectDef_t *rect, float scale, vec4_t color, int textStyle, qboolean net)
{
  int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;

  if (map >= 0 && map < uiInfo.mapCount)
  {
    Text_Paint(rect->x, rect->y, scale, color, uiInfo.mapList[map].mapName, 0, 0, textStyle, NULL_FONT);
  }
}

static void UI_DrawOpponentName(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  Text_Paint(rect->x, rect->y, scale, color, UI_Cvar_VariableString("ui_opponentName"), 0, 0, textStyle, NULL_FONT);
}

static int UI_OwnerDrawWidth(int ownerDraw, float scale)
{
  int i, h, value;
  const char *text;
  const char *s = NULL;

  switch (ownerDraw)
  {
  case UI_HANDICAP:
    h = Com_Clamp(5, 100, trap_Cvar_VariableValue("handicap"));
    i = 20 - h / 5;
    s = handicapValues[i];
    break;
  case UI_CLANNAME:
    s = UI_Cvar_VariableString("ui_teamName");
    break;
  case UI_GAMETYPE:
    s = uiInfo.gameTypes[ui_gameType.integer].gameType;
    break;
  case UI_SKILL:
    i = (int)trap_Cvar_VariableValue("g_spSkill");
    if (i < 1 || i > numSkillLevels)
    {
      i = 1;
    }
    s = skillLevels[i - 1];
    break;
  case UI_NIKKITEAMNAME:
    i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_nikkiTeam"));
    if (i >= 0 && i < uiInfo.teamCount)
    {
      s = va("%s: %s", "Blue", uiInfo.teamList[i].teamName);
    }
    break;
  case UI_DRAGONTEAMNAME:
    i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_dragonTeam"));
    if (i >= 0 && i < uiInfo.teamCount)
    {
      s = va("%s: %s", "Red", uiInfo.teamList[i].teamName);
    }
    break;
  case UI_NIKKITEAM1:
  case UI_NIKKITEAM2:
  case UI_NIKKITEAM3:
  case UI_NIKKITEAM4:
  case UI_NIKKITEAM5:
    value = (int)trap_Cvar_VariableValue(va("ui_nikkiTeam%i", ownerDraw - UI_NIKKITEAM1 + 1));
    if (value <= 0)
    {
      text = "Closed";
    }
    else if (value == 1)
    {
      text = "Human";
    }
    else
    {
      value -= 2;
      if (value >= uiInfo.aliasCount)
      {
        value = 0;
      }
      text = uiInfo.aliasList[value].name;
    }
    s = va("%i. %s", ownerDraw - UI_NIKKITEAM1 + 1, text);
    break;
  case UI_DRAGONTEAM1:
  case UI_DRAGONTEAM2:
  case UI_DRAGONTEAM3:
  case UI_DRAGONTEAM4:
  case UI_DRAGONTEAM5:
    value = (int)trap_Cvar_VariableValue(va("ui_dragonTeam%i", ownerDraw - UI_DRAGONTEAM1 + 1));
    if (value <= 0)
      text = "Closed";
    else if (value == 1)
      text = "Human";
    else
    {
      value -= 2;
      if (value >= uiInfo.aliasCount)
      {
        value = 0;
      }
      text = uiInfo.aliasList[value].name;
    }
    s = va("%i. %s", ownerDraw - UI_DRAGONTEAM1 + 1, text);
    break;
  case UI_NETSOURCE:
    if (ui_netSource.integer < 0 || ui_netSource.integer > uiInfo.numJoinGameTypes)
    {
      ui_netSource.integer = 0;
    }
    s = va("Source: %s", netSources[ui_netSource.integer]);
    break;
  case UI_NETFILTER:
    Com_Printf("numServerFilters: %i", numServerFilters);
    if (ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters)
    {
      ui_serverFilterType.integer = 0;
    }
    s = va("Filter: %s", serverFilters[ui_serverFilterType.integer].description);
    break;
  case UI_TIER:
    break;
  case UI_TIER_MAPNAME:
    break;
  case UI_TIER_GAMETYPE:
    break;
  case UI_ALLMAPS_SELECTION:
    break;
  case UI_OPPONENT_NAME:
    break;
  case UI_KEYBINDSTATUS:
    if (Display_KeyBindPending())
      s = "Waiting for new key... Press ESCAPE to cancel";
    else
      s = "Press ENTER or CLICK to change, Press BACKSPACE to clear";
    break;
  case UI_SERVERREFRESHDATE:
    s = UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer));
    break;
  default:
    break;
  }

  if (s)
  {
    return Text_Width(s, scale, 0, NULL_FONT);
  }
  return 0;
}

static void UI_DrawBotName(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int value        = uiInfo.botIndex;
  int game         = (int)trap_Cvar_VariableValue("g_gametype");
  const char *text = "";

  if (game >= GT_TEAM)
  {
    if (value >= uiInfo.characterCount)
      value = 0;
    text = uiInfo.characterList[value].name;
  }
  else
  {
    if (value >= UI_GetNumBots())
      value = 0;
    text = UI_GetBotNameByNumber(value);
  }
  Text_Paint(rect->x, rect->y, scale, color, text, 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawBotSkill(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  if (uiInfo.skillIndex >= 0 && uiInfo.skillIndex < numSkillLevels)
  {
    Text_Paint(rect->x, rect->y, scale, color, skillLevels[uiInfo.skillIndex], 0, 0, textStyle, NULL_FONT);
  }
}

static void UI_DrawRedBlue(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  Text_Paint(rect->x, rect->y, scale, color, (uiInfo.redBlue == 0) ?TEAM_NAME_AUTO: (uiInfo.redBlue == 1)? TEAM_NAME_DRAGONS : TEAM_NAME_NIKKIS, 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawCrosshair(rectDef_t *rect, float scale, vec4_t color)
{
  trap_R_SetColor(color);
  if (uiInfo.currentCrosshair < 0 || uiInfo.currentCrosshair >= NUM_CROSSHAIRS)
  {
    uiInfo.currentCrosshair = 0;
  }
 //UI_DrawHandlePic(rect->x, rect->y - rect->h, rect->w, rect->h, uiInfo.uiDC.Assets.crosshairShader[uiInfo.currentCrosshair]);
  UI_DrawHandlePic(rect->x, rect->y - rect->h +(rect->h*scale), rect->h, rect->h, uiInfo.uiDC.Assets.crosshairShader[uiInfo.currentCrosshair]); //hypov8 add: stop stretch
  trap_R_SetColor(NULL);
}

/*
===============
UI_BuildPlayerList
===============
*/
static void UI_BuildPlayerList(void)
{
  uiClientState_t cs;
  int n, count, team, team2, playerTeamNumber;
  char info[MAX_INFO_STRING];

  trap_GetClientState(&cs);
  trap_GetConfigString(CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING);
  uiInfo.playerNumber = cs.clientNum;
  uiInfo.teamLeader   = atoi(Info_ValueForKey(info, "tl"));
  team                = atoi(Info_ValueForKey(info, "t"));
  trap_GetConfigString(CS_SERVERINFO, info, sizeof(info));
  count              = atoi(Info_ValueForKey(info, "sv_maxclients"));
  uiInfo.playerCount = 0;
  uiInfo.myTeamCount = 0;
  playerTeamNumber   = 0;
  for (n = 0; n < count; n++)
  {
    trap_GetConfigString(CS_PLAYERS + n, info, MAX_INFO_STRING);

    if (info[0])
    {
      Q_strncpyz(uiInfo.playerNames[uiInfo.playerCount], Info_ValueForKey(info, "n"), MAX_NAME_LENGTH);
      Q_CleanStr(uiInfo.playerNames[uiInfo.playerCount]);
      uiInfo.playerCount++;
      team2 = atoi(Info_ValueForKey(info, "t"));
      if (team2 == team)
      {
        Q_strncpyz(uiInfo.teamNames[uiInfo.myTeamCount], Info_ValueForKey(info, "n"), MAX_NAME_LENGTH);
        Q_CleanStr(uiInfo.teamNames[uiInfo.myTeamCount]);
        uiInfo.teamClientNums[uiInfo.myTeamCount] = n;
        if (uiInfo.playerNumber == n)
        {
          playerTeamNumber = uiInfo.myTeamCount;
        }
        uiInfo.myTeamCount++;
      }
    }
  }

  if (!uiInfo.teamLeader)
  {
    trap_Cvar_Set("cg_selectedPlayer", va("%d", playerTeamNumber));
  }

  n = (int)trap_Cvar_VariableValue("cg_selectedPlayer");
  if (n < 0 || n > uiInfo.myTeamCount)
  {
    n = 0;
  }
  if (n < uiInfo.myTeamCount)
  {
    trap_Cvar_Set("cg_selectedPlayerName", uiInfo.teamNames[n]);
  }
}

static void UI_DrawSelectedPlayer(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  if (uiInfo.uiDC.realTime > uiInfo.playerRefresh)
  {
    uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
    UI_BuildPlayerList();
  }
  Text_Paint(rect->x, rect->y, scale, color, (uiInfo.teamLeader) ? UI_Cvar_VariableString("cg_selectedPlayerName") : UI_Cvar_VariableString("name"), 0, 0, textStyle, NULL_FONT);
}

static void UI_DrawServerRefreshDate(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  if (uiInfo.serverStatus.refreshActive)
  {
    vec4_t lowLight, newColor;
    lowLight[0] = 0.8 * color[0];
    lowLight[1] = 0.8 * color[1];
    lowLight[2] = 0.8 * color[2];
    lowLight[3] = 0.8 * color[3];
    LerpColor(color, lowLight, newColor, 0.5 + 0.5 * sin(uiInfo.uiDC.realTime / PULSE_DIVISOR));
    Text_Paint(rect->x, rect->y, scale, newColor, va("Getting info for %d servers (ESC to cancel)", trap_LAN_GetServerCount(ui_netSource.integer)), 0, 0, textStyle, NULL_FONT);
  }
  else
  {
    char buff[64];
    Q_strncpyz(buff, UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer)), 64);
    Text_Paint(rect->x, rect->y, scale, color, va("Refresh Time: %s", buff), 0, 0, textStyle, NULL_FONT);
  }
}

static void UI_DrawServerMOTD(rectDef_t *rect, float scale, vec4_t color)
{
  if (uiInfo.serverStatus.motdLen)
  {
    float maxX;

    if (uiInfo.serverStatus.motdWidth == -1)
    {
      uiInfo.serverStatus.motdWidth   = 0;
      uiInfo.serverStatus.motdPaintX  = rect->x + 1;
      uiInfo.serverStatus.motdPaintX2 = -1;
    }

    if (uiInfo.serverStatus.motdOffset > uiInfo.serverStatus.motdLen)
    {
      uiInfo.serverStatus.motdOffset  = 0;
      uiInfo.serverStatus.motdPaintX  = rect->x + 1;
      uiInfo.serverStatus.motdPaintX2 = -1;
    }

    if (uiInfo.uiDC.realTime > uiInfo.serverStatus.motdTime)
    {
      uiInfo.serverStatus.motdTime = uiInfo.uiDC.realTime + 10;
      if (uiInfo.serverStatus.motdPaintX <= rect->x + 2)
      {
        if (uiInfo.serverStatus.motdOffset < uiInfo.serverStatus.motdLen)
        {
          uiInfo.serverStatus.motdPaintX += Text_Width(&uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], scale, 1, NULL_FONT) - 1;
          uiInfo.serverStatus.motdOffset++;
        }
        else
        {
          uiInfo.serverStatus.motdOffset = 0;
          if (uiInfo.serverStatus.motdPaintX2 >= 0)
          {
            uiInfo.serverStatus.motdPaintX = uiInfo.serverStatus.motdPaintX2;
          }
          else
          {
            uiInfo.serverStatus.motdPaintX = rect->x + rect->w - 2;
          }
          uiInfo.serverStatus.motdPaintX2 = -1;
        }
      }
      else
      {
        //serverStatus.motdPaintX--;
        uiInfo.serverStatus.motdPaintX -= 2;
        if (uiInfo.serverStatus.motdPaintX2 >= 0)
        {
          //serverStatus.motdPaintX2--;
          uiInfo.serverStatus.motdPaintX2 -= 2;
        }
      }
    }

    maxX = rect->x + rect->w - 2;
    Text_Paint_Limit(&maxX, uiInfo.serverStatus.motdPaintX, rect->y /*+ rect->h - 3*/, scale, color, &uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], 0, 0); //hypov8 edit. stop shifting y text
    if (uiInfo.serverStatus.motdPaintX2 >= 0)
    {
      float maxX2 = rect->x + rect->w - 2;
      Text_Paint_Limit(&maxX2, uiInfo.serverStatus.motdPaintX2, rect->y /*+ rect->h - 3*/, scale, color, uiInfo.serverStatus.motd, 0, uiInfo.serverStatus.motdOffset); //hypov8 edit. stop shifting y text
    }
    if (uiInfo.serverStatus.motdOffset && maxX > 0)
    {
      // if we have an offset ( we are skipping the first part of the string ) and we fit the string
      if (uiInfo.serverStatus.motdPaintX2 == -1)
      {
        uiInfo.serverStatus.motdPaintX2 = rect->x + rect->w - 2;
      }
    }
    else
    {
      uiInfo.serverStatus.motdPaintX2 = -1;
    }
  }
}

static void UI_DrawKeyBindStatus(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
//	int ofs = 0; TTimo: unused
  if (Display_KeyBindPending())
  {
    Text_Paint(rect->x, rect->y, scale, color, "Waiting for new key... Press ESCAPE to cancel", 0, 0, textStyle, NULL_FONT);
  }
  else
  {
    Text_Paint(rect->x, rect->y, scale, color, "Press ENTER or CLICK to change, Press BACKSPACE to clear", 0, 0, textStyle, NULL_FONT);
  }
}

static void UI_DrawGLInfo(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  char *eptr;
  char buff[1024];
  const char *lines[64];
  int y, numLines, i;

  Text_Paint(rect->x + 2, rect->y, scale, color, va("Render Hardware: %s", uiInfo.uiDC.glconfig.renderer_string), 0, 100, textStyle, NULL_FONT);
  Text_Paint(rect->x + 2, rect->y + 10, scale, color, va("Graphic Card Driver: %s", uiInfo.uiDC.glconfig.vendor_string), 0, 100, textStyle, NULL_FONT);
  Text_Paint(rect->x + 2, rect->y + 20, scale, color, va("GL-Version: %s", uiInfo.uiDC.glconfig.version_string), 0, 100, textStyle, NULL_FONT);
  Text_Paint(rect->x + 2, rect->y + 30, scale, color, va("Pixelformat: color(%d-bits) Z(%d-bits) stencil(%d-bits)", uiInfo.uiDC.glconfig.colorBits, uiInfo.uiDC.glconfig.depthBits, uiInfo.uiDC.glconfig.stencilBits), 0, 100, textStyle, NULL_FONT);

  // build null terminated extension strings
  // TTimo: https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=399
  // in TA this was not directly crashing, but displaying a nasty broken shader right in the middle
  // brought down the string size to 1024, there's not much that can be shown on the screen anyway
  Q_strncpyz(buff, uiInfo.uiDC.glconfig.extensions_string, 1024);
  eptr     = buff;
  y        = rect->y + 45;
  numLines = 0;
  while (y < rect->y + rect->h && *eptr)
  {
    while (*eptr && *eptr == ' ')
      *eptr++ = '\0';

    // track start of valid string
    if (*eptr && *eptr != ' ')
    {
      lines[numLines++] = eptr;
    }

    while (*eptr && *eptr != ' ')
      eptr++;
  }

  i = 0;
  while (i < numLines)
  {
    Text_Paint(rect->x + 2, y, scale, color, lines[i++], 0, 60, textStyle, NULL_FONT);
    if (i < numLines)
    {
      Text_Paint(rect->x + rect->w / 2, y, scale, color, lines[i++], 0, 60, textStyle, NULL_FONT);
    }
    y += 10;
    if (y > rect->y + rect->h - 11)
      break;
  }
}

static void UI_DrawCpuInfo(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  char ext[256] = "Cpu extensions: ";
  Text_Paint(rect->x + 2, rect->y, scale, color, va("Cpu name: %s", uiInfo.uiDC.cpuConfig.cpuName), 0, 100, textStyle, NULL_FONT);

  if ((uiInfo.uiDC.cpuConfig.features & CF_MMX))
    Q_strcat(ext, 255, "MMX; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_MMX_EXT))
    Q_strcat(ext, 255, "MMX_EXT; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_3DNOW))
    Q_strcat(ext, 255, "3DNOW; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_3DNOW_EXT))
    Q_strcat(ext, 255, "3DNOW_EXT; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_SSE))
    Q_strcat(ext, 255, "SSE; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_SSE2))
    Q_strcat(ext, 255, "SSE2; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_SSE3))
    Q_strcat(ext, 255, "SSE3; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_SSE3_SUP))
    Q_strcat(ext, 255, "SSE3 SUPPL; ");

  Text_Paint(rect->x + 2, rect->y+10, scale, color, ext, 0, 100, textStyle, NULL_FONT);
  Q_strncpyz(ext, "    ", sizeof(ext)); //hypov8 add: indent new line

  if ((uiInfo.uiDC.cpuConfig.features & CF_SSE41))
    Q_strcat(ext, 255, "SSE41; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_SSE41_POPCENT))
    Q_strcat(ext, 255, "SSE41 POPCENT; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_SSE42))
    Q_strcat(ext, 255, "SSE42; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_AVXYMM))
    Q_strcat(ext, 255, "AVX (YMM); ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_PCLMUL_AES))
    Q_strcat(ext, 255, "PCLMUL AES; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_AVX2))
    Q_strcat(ext, 255, "AVX2; ");
  if ((uiInfo.uiDC.cpuConfig.features & CF_ALTIVEC))
    Q_strcat(ext, 255, "ALTIVEC; ");

  Text_Paint(rect->x + 2, rect->y+20, scale, color, ext, 0, 100, textStyle, NULL_FONT);
  Text_Paint(rect->x + 2, rect->y+30, scale, color, va("Number of Cpu's: %d", uiInfo.uiDC.cpuConfig.numCpus), 0, 0, textStyle, NULL_FONT);
  Text_Paint(rect->x + 2, rect->y+40, scale, color, va("Cache size: Level1 %d kb, Level2 %d kb", (int)(uiInfo.uiDC.cpuConfig.cacheSizeL1 /1024),  (int)uiInfo.uiDC.cpuConfig.cacheSizeL2/1024), 0, 0, textStyle, NULL_FONT);
  if (uiInfo.uiDC.cpuConfig.cacheSizeL3)
    Text_Paint(rect->x + 2, rect->y+50, scale, color, va("Level3 Cache: %d kb", (int)uiInfo.uiDC.cpuConfig.cacheSizeL3/1024), 0, 0, textStyle, NULL_FONT);
  //Text_Paint(rect->x + 2, rect->y+50, scale, color, va("Endianess: %s", uiInfo.uiDC.cpuConfig.bigEndian  ? "Bigendian" : "LittleEndian"), 0, 0, textStyle, NULL_FONT);
}


static qboolean UI_DrawGLAnistopic_HandleKey(int flags, float *special, int key)
{
  int aniso = (int)trap_Cvar_VariableValue("uix_r_ext_texture_filter_anisotropic");

  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
    {
      aniso--;
      if (aniso < 0)
        aniso = (int)uiInfo.uiDC.glconfig2.maxTextureAnisotropy;
    }
    else
    {
      aniso++;
      if (aniso > (int)uiInfo.uiDC.glconfig2.maxTextureAnisotropy)
        aniso = 0;
    }
    trap_Cvar_Set("uix_r_ext_texture_filter_anisotropic", va("%d", aniso));
  //trap_Cvar_SetValue("uix_r_custQuality", 0); // add hypov8
    return qtrue;
  }
  return qfalse;
}

static void UI_DrawGLAnistopic(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int aniso = (int)trap_Cvar_VariableValue("uix_r_ext_texture_filter_anisotropic");


  if (aniso < 0)
    aniso = 0;
  if (aniso  > (int)uiInfo.uiDC.glconfig2.maxTextureAnisotropy)
    aniso = (int)uiInfo.uiDC.glconfig2.maxTextureAnisotropy;

  Text_Paint(rect->x, rect->y, scale, color, va(" %d", aniso), 0, 0, textStyle, NULL_FONT);
}
//add hypov8 menu

static qboolean UI_DrawGLModes_HandleKey(int flags, float *special, int key)
{
  int aniso = (int)trap_Cvar_VariableValue("uix_r_mode");

  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
    {
      aniso--;
      if (aniso < -1)
        aniso = 20;
    }
    else
    {
      aniso++;
      if (aniso > 20)
        aniso = -1;
    }
    trap_Cvar_Set("uix_r_mode", va("%d", aniso));
    //trap_Cvar_SetValue("uix_r_custQuality", 0);
    return qtrue;
  }
  return qfalse;
}
//111
static void UI_DrawGLModes(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int aniso = (int)trap_Cvar_VariableValue("uix_r_mode");
  const char *video = "";

  if (aniso < -1)
    aniso = -1;

  if (aniso  >20)
    aniso = 20;

  switch (aniso)
  {
    case -1: video = "Custom"; break;
    case 0: video = " 320x240"; break;
    case 1: video =  " 400x300"; break;
    case 2: video =  " 512x384"; break;
    case 3: video =  " 640x480"; break;
    case 4: video =  " 800x600"; break;
    case 5: video =  " 960x720"; break;
    case 6: video =  "1024x768"; break;
    case 7: video =  "1024x576"; break;
    case 8: video =  "1152x864"; break;
    case 9: video =  "1280x720  (16:9)"; break;
    case 10: video =  "1280x768  (16:10)"; break;
    case 11: video =  "1280x800  (5:4)"; break;
    case 12: video =  "1280x1024 (4:3)"; break;
    case 13: video =  "1360x768  (16:9)"; break;
    case 14: video =  "1440x900  (16:10)"; break;
    case 15: video =  "1680x1050 (16:10)"; break;
    case 16: video =  "1600x1200 (4:3)"; break;
    case 17: video =  "1920x1080 (16:9)"; break;
    case 18: video =  "1920x1200 (16:10)"; break;
    case 19: video =  "2048x1536 (4:3)"; break;
    case 20: video =  "2560x1600 (16:10)"; break;
  }
  Text_Paint(rect->x, rect->y, scale, color, video, 0, 0, textStyle, NULL_FONT);

}


#if HYPODEBUG
#define numMenu 6
#else
#define numMenu 5
#endif

////2
static qboolean UI_DrawGLQuickQuality_HandleKey(int flags, float *special, int key)
{
  int aniso = (int)trap_Cvar_VariableValue("uix_r_custQuality");

  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
    {
      aniso--;
      if (aniso < 0)
        aniso = numMenu;
    }
    else
    {
      aniso++;
      if (aniso >numMenu)
        aniso = 0;
    }
    trap_Cvar_Set("uix_r_custQuality", va("%d", aniso));
    return qtrue;
  }
  return qfalse;
}
///222
static void UI_DrawGLQuickQuality(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int aniso = (int)trap_Cvar_VariableValue("uix_r_custQuality");
  const char *video="\0";

  if (aniso < 0)
    aniso = 0;
  if (aniso  >numMenu)
    aniso = numMenu;

  switch (aniso)
  {
    case 0: video = "Custom"; break; //"--"
    case 1: video = "Extreme"; break;
    case 2: video = "Very High"; break;
    case 3: video = "High"; break;
    case 4: video = "Medium"; break;
    case 5: video = "Low"; break;
    case 6: video = "hypo mode"; break;
  }

  //Text_Paint(rect->x, rect->y, scale, color, va(" %d", aniso), 0, 0, textStyle, NULL_FONT);
  Text_Paint(rect->x, rect->y, scale, color, video, 0, 0, textStyle, NULL_FONT);
}

static qboolean UI_GLMode_HandleKey(int flags, float *special, int key)
{
  int aniso = (int)trap_Cvar_VariableValue(ui_custStrings[0][1]); //uix_r_textureMode

  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
    {
      aniso--;
      if (aniso < 0)
        aniso = 3;
    }
    else
    {
      aniso++;
      if (aniso >3)
        aniso = 0;
    }
    trap_Cvar_Set(ui_custStrings[0][1], va("%i", aniso)); //uix_r_textureMode
    return qtrue;
  }
  return qfalse;
}

static void UI_GLModeQuality(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
  int aniso = (int)trap_Cvar_VariableValue(ui_custStrings[0][1]);//uix_r_textureMode
  const char *video="\0";

  if (aniso < 0)
    aniso = 0;
  if (aniso  >3)
    aniso = 3;

  switch (aniso)
  {
    case 0: video = "Linear"; break;
    case 1: video = "Bilinear"; break;
    case 2: video = "Trilinear"; break;
    case 3: video = "Custom"; break;
  }

  //Text_Paint(rect->x, rect->y, scale, color, va(" %d", aniso), 0, 0, textStyle, NULL_FONT);
  Text_Paint(rect->x, rect->y, scale, color, video, 0, 0, textStyle, NULL_FONT);
}

// end add hypov8

// FIXME: table drive
//
static void UI_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle)
{
  rectDef_t rect;

  rect.x = x + text_x;
  rect.y = y + text_y;
  rect.w = w;
  rect.h = h;

  switch (ownerDraw)
  {
  case UI_HANDICAP:
    UI_DrawHandicap(&rect, scale, color, textStyle);
    break;
  case UI_EFFECTS:
    UI_DrawEffects(&rect, scale, color);
    break;
  case UI_PLAYERMODEL:
    UI_DrawPlayerModel(&rect);
    break;
  case UI_CLANNAME:
    UI_DrawClanName(&rect, scale, color, textStyle);
    break;
  case UI_CLANLOGO:
    UI_DrawClanLogo(&rect, scale, color);
    break;
  case UI_CLANCINEMATIC:
    UI_DrawClanCinematic(&rect, scale, color);
    break;
  case UI_PREVIEWCINEMATIC:
    UI_DrawPreviewCinematic(&rect, scale, color);
    break;
  case UI_GAMETYPE:
    UI_DrawGameType(&rect, scale, color, textStyle);
    break;
  case UI_NETGAMETYPE:
    UI_DrawNetGameType(&rect, scale, color, textStyle);
    break;
  case UI_JOINGAMETYPE:
    UI_DrawJoinGameType(&rect, scale, color, textStyle);
    break;
  case UI_JOINWEAPONMODE:
    UI_DrawWeaponMode(&rect, scale, color, textStyle);
    break;
  case UI_MAPPREVIEW:
    UI_DrawMapPreview(&rect, scale, color, qtrue);
    break;
  case UI_MAP_TIMETOBEAT:
    UI_DrawMapTimeToBeat(&rect, scale, color, textStyle);
    break;
  case UI_MAPCINEMATIC:
    UI_DrawMapCinematic(&rect, scale, color, qfalse);
    break;
  case UI_STARTMAPCINEMATIC:
    UI_DrawMapCinematic(&rect, scale, color, qtrue);
    break;
  case UI_SKILL:
    UI_DrawSkill(&rect, scale, color, textStyle);
    break;
  case UI_NIKKITEAMNAME:
    UI_DrawTeamName(&rect, scale, color, qtrue, textStyle);
    break;
  case UI_DRAGONTEAMNAME:

    UI_DrawTeamName(&rect, scale, color, qfalse, textStyle);
    break;
  case UI_NIKKITEAM1:
  case UI_NIKKITEAM2:
  case UI_NIKKITEAM3:
  case UI_NIKKITEAM4:
  case UI_NIKKITEAM5:
    UI_DrawTeamMember(&rect, scale, color, qtrue, ownerDraw - UI_NIKKITEAM1 + 1, textStyle);
    break;
  case UI_DRAGONTEAM1:
  case UI_DRAGONTEAM2:
  case UI_DRAGONTEAM3:
  case UI_DRAGONTEAM4:
  case UI_DRAGONTEAM5:
    UI_DrawTeamMember(&rect, scale, color, qfalse, ownerDraw - UI_DRAGONTEAM1 + 1, textStyle);
    break;
  case UI_NETSOURCE:
    UI_DrawNetSource(&rect, scale, color, textStyle);
    break;
  case UI_NETMAPPREVIEW:
    UI_DrawNetMapPreview(&rect, scale, color);
    break;
  case UI_NETMAPCINEMATIC:
    UI_DrawNetMapCinematic(&rect, scale, color);
    break;
  case UI_NETFILTER:
    UI_DrawNetFilter(&rect, scale, color, textStyle);
    break;
  case UI_TIER:
    UI_DrawTier(&rect, scale, color, textStyle);
    break;
  case UI_OPPONENTMODEL:
    UI_DrawOpponent(&rect);
    break;
  case UI_TIERMAP1:
    UI_DrawTierMap(&rect, 0);
    break;
  case UI_TIERMAP2:
    UI_DrawTierMap(&rect, 1);
    break;
  case UI_TIERMAP3:
    UI_DrawTierMap(&rect, 2);
    break;
  case UI_PLAYERLOGO:
    UI_DrawPlayerLogo(&rect, color);
    break;
  case UI_PLAYERLOGO_METAL:
    UI_DrawPlayerLogoMetal(&rect, color);
    break;
  case UI_PLAYERLOGO_NAME:
    UI_DrawPlayerLogoName(&rect, color);
    break;
  case UI_OPPONENTLOGO:
    UI_DrawOpponentLogo(&rect, color);
    break;
  case UI_OPPONENTLOGO_METAL:
    UI_DrawOpponentLogoMetal(&rect, color);
    break;
  case UI_OPPONENTLOGO_NAME:
    UI_DrawOpponentLogoName(&rect, color);
    break;
  case UI_TIER_MAPNAME:
    UI_DrawTierMapName(&rect, scale, color, textStyle);
    break;
  case UI_TIER_GAMETYPE:
    UI_DrawTierGameType(&rect, scale, color, textStyle);
    break;
  case UI_ALLMAPS_SELECTION:
    UI_DrawAllMapsSelection(&rect, scale, color, textStyle, qtrue);
    break;
  case UI_MAPS_SELECTION:
    UI_DrawAllMapsSelection(&rect, scale, color, textStyle, qfalse);
    break;
  case UI_OPPONENT_NAME:
    UI_DrawOpponentName(&rect, scale, color, textStyle);
    break;
  case UI_BOTNAME:
    UI_DrawBotName(&rect, scale, color, textStyle);
    break;
  case UI_BOTSKILL:
    UI_DrawBotSkill(&rect, scale, color, textStyle);
    break;
  case UI_DRAGONNIKKI:
    UI_DrawRedBlue(&rect, scale, color, textStyle);
    break;
  case UI_CROSSHAIR:
    UI_DrawCrosshair(&rect, scale, color);
    break;
  case UI_SELECTEDPLAYER:
    UI_DrawSelectedPlayer(&rect, scale, color, textStyle);
    break;
  case UI_SERVERREFRESHDATE:
    UI_DrawServerRefreshDate(&rect, scale, color, textStyle);
    break;
  case UI_SERVERMOTD:
    UI_DrawServerMOTD(&rect, scale, color);
    break;
  case UI_GLINFO:
    UI_DrawGLInfo(&rect, scale, color, textStyle);
    break;
  case UI_KEYBINDSTATUS:
    UI_DrawKeyBindStatus(&rect, scale, color, textStyle);
    break;

  case UI_ANISOTROPIC_VALUE:
    UI_DrawGLAnistopic(&rect, scale, color, textStyle);
    break;

  case UI_VIDEO_VALUE:	//add hypov8
    UI_DrawGLModes(&rect, scale, color, textStyle);
    break;

  case UI_QUICKQUALITY_VALUE:	//add hypov8
    UI_DrawGLQuickQuality(&rect, scale, color, textStyle);
    break;

  case UI_TEXTUREMODE:	//add hypov8
    UI_GLModeQuality(&rect, scale, color, textStyle);
    break;


  case UI_CPU_INFO:
    UI_DrawCpuInfo(&rect, scale, color, textStyle);
    break;
  default:
    break;
  }
}

static qboolean UI_OwnerDrawVisible(int flags)
{
  qboolean vis = qtrue;

  while (flags)
  {

    if (flags & UI_SHOW_FFA)
    {
      if ((int)trap_Cvar_VariableValue("g_gametype") != GT_FFA)
        vis = qfalse;

      flags &= ~UI_SHOW_FFA;
    }

    if (flags & UI_SHOW_NOTFFA)
    {
      if ((int)trap_Cvar_VariableValue("g_gametype") == GT_FFA)
        vis = qfalse;

      flags &= ~UI_SHOW_NOTFFA;
    }

    if (flags & UI_SHOW_LEADER)
    {
      // these need to show when this client can give orders to a player or a group
      if (!uiInfo.teamLeader)
        vis = qfalse;
      else
      {
        // if showing yourself
        if (ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber)
          vis = qfalse;
      }
      flags &= ~UI_SHOW_LEADER;
    }
    if (flags & UI_SHOW_NOTLEADER)
    {
      // these need to show when this client is assigning their own status or they are NOT the leader
      if (uiInfo.teamLeader)
      {
        // if not showing yourself
        if (!(ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber))
          vis = qfalse;
        // these need to show when this client can give orders to a player or a group
      }
      flags &= ~UI_SHOW_NOTLEADER;
    }
    if (flags & UI_SHOW_FAVORITESERVERS)
    {
      // this assumes you only put this type of display flag on something showing in the proper context
      if (ui_netSource.integer != AS_FAVORITES)
        vis = qfalse;

      flags &= ~UI_SHOW_FAVORITESERVERS;
    }
    if (flags & UI_SHOW_NOTFAVORITESERVERS)
    {
      // this assumes you only put this type of display flag on something showing in the proper context
      if (ui_netSource.integer == AS_FAVORITES)
        vis = qfalse;

      flags &= ~UI_SHOW_NOTFAVORITESERVERS;
    }
    if (flags & UI_SHOW_ANYTEAMGAME)
    {
      if (uiInfo.gameTypes[ui_gameType.integer].gtEnum <= GT_TEAM)
      {
        vis = qfalse;
      }
      flags &= ~UI_SHOW_ANYTEAMGAME;
    }
    if (flags & UI_SHOW_ANYNONTEAMGAME)
    {
      if (uiInfo.gameTypes[ui_gameType.integer].gtEnum > GT_TEAM)
        vis = qfalse;

      flags &= ~UI_SHOW_ANYNONTEAMGAME;
    }
    if (flags & UI_SHOW_NETANYTEAMGAME)
    {
    if (uiInfo.gameTypes[ui_netGameType.integer].gtEnum <= GT_TEAM || uiInfo.gameTypes[ui_netGameType.integer].gtEnum == GT_BAGMAN)
        vis = qfalse;

      flags &= ~UI_SHOW_NETANYTEAMGAME;
    }
    if (flags & UI_SHOW_NETANYNONTEAMGAME)
    {
      if (uiInfo.gameTypes[ui_netGameType.integer].gtEnum > GT_TEAM)
        vis = qfalse;

      flags &= ~UI_SHOW_NETANYNONTEAMGAME;
    }
  //UI_SHOW_NETBMTEAMGAME
  if (flags & UI_SHOW_NETBMTEAMGAME)
  {
    if (uiInfo.gameTypes[ui_netGameType.integer].gtEnum != GT_BAGMAN)
      vis = qfalse;

    flags &= ~UI_SHOW_NETBMTEAMGAME;
  }
    if (flags & UI_SHOW_NEWHIGHSCORE)
    {
      if (uiInfo.newHighScoreTime < uiInfo.uiDC.realTime)
        vis = qfalse;
      else
      {
        if (uiInfo.soundHighScore)
        {
          if ((int)trap_Cvar_VariableValue("sv_killserver") == 0)
          {
            // wait on server to go down before playing sound
            trap_S_StartLocalSound(uiInfo.newHighScoreSound, CHAN_ANNOUNCER);
            uiInfo.soundHighScore = qfalse;
          }
        }
      }
      flags &= ~UI_SHOW_NEWHIGHSCORE;
    }
    if (flags & UI_SHOW_NEWBESTTIME)
    {
      if (uiInfo.newBestTime < uiInfo.uiDC.realTime)
        vis = qfalse;

      flags &= ~UI_SHOW_NEWBESTTIME;
    }
    if (flags & UI_SHOW_DEMOAVAILABLE)
    {
      if (!uiInfo.demoAvailable)
        vis = qfalse;

      flags &= ~UI_SHOW_DEMOAVAILABLE;
    }
    else
    {
      flags = 0;
    }
  }
  return vis;
}

static qboolean UI_Handicap_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    int h;
    h = Com_Clamp(5, 100, trap_Cvar_VariableValue("handicap"));
    if (key == K_MOUSE2)
      h -= 5;
    else
      h += 5;
    if (h > 100)
      h = 5;
    else if (h < 5)
      h = 100;
    trap_Cvar_Set("handicap", va("%i", h));
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_Effects_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {

    if (key == K_MOUSE2)
      uiInfo.effectsColor--;
    else
      uiInfo.effectsColor++;

    if (uiInfo.effectsColor > 6)
      uiInfo.effectsColor = 0;
    else if (uiInfo.effectsColor < 0)
      uiInfo.effectsColor = 6;

    trap_Cvar_SetValue("color1", uitogamecode[uiInfo.effectsColor]);
    return qtrue;
  }
  return qfalse;
}

static int UI_GetSelectedHeadIndex()
{
  char model[MAX_QPATH];
  //char team[MAX_QPATH];
  //char head[MAX_QPATH];
  int i, headIdx=0;

  if ((int)trap_Cvar_VariableValue("ui_Q3Model")) //DM
  {
    Q_strncpyz(model, UI_Cvar_VariableString("model"), sizeof(model));
    //Q_strncpyz(head, UI_Cvar_VariableString("headmodel"), sizeof(head));
  }
  else //TEAM
  {
    Q_strncpyz(model, UI_Cvar_VariableString("team_model"), sizeof(model));

    for (i = 0; i < uiInfo.characterCount; i++)
    {
      if (uiInfo.characterList[i].active)
      {
        if (!Q_stricmp(uiInfo.characterList[i].base, model))
        {
          return headIdx;
        }
        headIdx += 1;
      }
    }
  }

  return 0;
}

//teamplay player selector. team toggle skin
static qboolean UI_ClanName_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    int i;
    i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
    if (uiInfo.teamList[i].cinematic >= 0)
    {
      trap_CIN_StopCinematic(uiInfo.teamList[i].cinematic);
      uiInfo.teamList[i].cinematic = -1;
    }
    if (key == K_MOUSE2)
      i--;
    else
      i++;
    if (i >= uiInfo.teamCount)
      i = 0;
    else if (i < 0)
      i = uiInfo.teamCount - 1;

    trap_Cvar_Set("ui_teamName", uiInfo.teamList[i].teamName);
    UI_HeadCountByTeam();
    UI_FeederSelection(FEEDER_HEADS, UI_GetSelectedHeadIndex());
    updateModel = qtrue;
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_GameType_HandleKey(int flags, float *special, int key, qboolean resetMap)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    int oldCount = UI_MapCountByGameType(qtrue);

    // hard coded mess here
    if (key == K_MOUSE2)
    {
      ui_gameType.integer--;

      //FIXME (0xA5EA): removed GT_SINGLEPLAYER, does this mess up things ?
#if 1       // 0xA5EA,
      if (ui_gameType.integer == 2)
      {
        ui_gameType.integer = 1;
      }
      else if (ui_gameType.integer < 2)
      {
        ui_gameType.integer = uiInfo.numGameTypes - 1;
      }
#endif
    }
    else
    {
      ui_gameType.integer++;
#if 1       // 0xA5EA,
      if (ui_gameType.integer >= uiInfo.numGameTypes)
      {
        ui_gameType.integer = 1;
      }
      else if (ui_gameType.integer == 2)
      {
        ui_gameType.integer = 3;
      }
#endif
    }
#if 0       // 0xA5EA, not necessary
    if (uiInfo.gameTypes[ui_gameType.integer].gtEnum == GT_TOURNAMENT)
    {
      trap_Cvar_Set("ui_Q3Model", "1");
    }
    else
    {
      trap_Cvar_Set("ui_Q3Model", "0");
    }
#endif
    trap_Cvar_Set("ui_gameType", va("%d", ui_gameType.integer));
    UI_SetCapFragLimits(qtrue);
    UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
    if (resetMap && oldCount != UI_MapCountByGameType(qtrue))
    {
      trap_Cvar_Set("ui_currentMap", "0");
      Menu_SetFeederSelection(NULL, FEEDER_MAPS, 0, NULL);
    }
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_NetGameType_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
      ui_netGameType.integer--;
    else
      ui_netGameType.integer++;

    if (ui_netGameType.integer < 0)
      ui_netGameType.integer = uiInfo.numGameTypes - 1;
    else if (ui_netGameType.integer >= uiInfo.numGameTypes)
      ui_netGameType.integer = 0;

    trap_Cvar_Set("ui_netGameType", va("%d", ui_netGameType.integer));
    trap_Cvar_Set("ui_actualnetGameType", va("%d", uiInfo.gameTypes[ui_netGameType.integer].gtEnum));
    trap_Cvar_Set("ui_currentNetMap", "0");
    UI_MapCountByGameType(qfalse);
    Menu_SetFeederSelection(NULL, FEEDER_ALLMAPS, 0, NULL);
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_JoinGameType_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
      ui_joinGameType.integer--;
    else
      ui_joinGameType.integer++;

    if (ui_joinGameType.integer < 0)
      ui_joinGameType.integer = uiInfo.numJoinGameTypes - 1;
    else if (ui_joinGameType.integer >= uiInfo.numJoinGameTypes)
      ui_joinGameType.integer = 0;

    trap_Cvar_Set("ui_joinGameType", va("%d", ui_joinGameType.integer));
    UI_BuildServerDisplayList(qtrue);
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_JoinWeaponMode_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
      ui_joinWeaponMode.integer--;
    else
      ui_joinWeaponMode.integer++;

    if (ui_joinWeaponMode.integer < 0)
      ui_joinWeaponMode.integer = uiInfo.numWeaponModes - 1;
    else if (ui_joinWeaponMode.integer >= uiInfo.numWeaponModes)
      ui_joinWeaponMode.integer = 0;

    trap_Cvar_Set("ui_joinWeaponMode", va("%d", ui_joinWeaponMode.integer));
    UI_BuildServerDisplayList(qtrue);
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_Skill_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    int i = (int)trap_Cvar_VariableValue("g_spSkill");

    if (key == K_MOUSE2)
      i--;
    else
      i++;

    if (i < 1)
      i = numSkillLevels;
    else if (i > numSkillLevels)
      i = 1;

    trap_Cvar_Set("g_spSkill", va("%i", i));
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_TeamName_HandleKey(int flags, float *special, int key, qboolean blue)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    int i;
    i = UI_TeamIndexFromName(UI_Cvar_VariableString((blue) ? "ui_nikkiTeam" : "ui_dragonTeam"));

    if (key == K_MOUSE2)
      i--;
    else
      i++;

    if (i >= uiInfo.teamCount)
      i = 0;
    else if (i < 0)
      i = uiInfo.teamCount - 1;

    trap_Cvar_Set((blue) ? "ui_nikkiTeam" : "ui_dragonTeam", uiInfo.teamList[i].teamName);

    return qtrue;
  }
  return qfalse;
}

static qboolean UI_TeamMember_HandleKey(int flags, float *special, int key, qboolean blue, int num)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    // 0 - None
    // 1 - Human
    // 2..NumCharacters - Bot
    char * one = "ui_nikkiTeam%i";
    char * two = "ui_dragonTeam%i";
    const char* cvar = (const char*)va(blue ? one : two, num);
    int value  = (int)trap_Cvar_VariableValue(cvar);

    if (key == K_MOUSE2)
      value--;
    else
      value++;

    if (ui_actualNetGameType.integer >= GT_TEAM)
    {
      if (value >= uiInfo.characterCount + 2)
        value = 0;
      else if (value < 0)
        value = uiInfo.characterCount + 2 - 1;
    }
    else
    {
      if (value >= UI_GetNumBots() + 2)
        value = 0;
      else if (value < 0)
        value = UI_GetNumBots() + 2 - 1;
    }

    trap_Cvar_Set(cvar, va("%i", value));
    return qtrue;
  }
  return qfalse;
}
//int UI_TeamIndexFromName(const char *name)
static qboolean UI_GetMaster(int net_ui)
{
  char *mastserv = NULL;

  switch (net_ui)
  {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      mastserv = UI_Cvar_VariableString(va("sv_master%i", net_ui));
      break;
  }

  if (mastserv && *mastserv)
    return qtrue;

  return qfalse;

}

static qboolean UI_NetSource_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    //int i;

    if (key == K_MOUSE2)
    {
      ui_netSource.integer -= 1;
      if (ui_netSource.integer < 0)
        ui_netSource.integer = numNetSources - 1;
    }
    else
    {
      ui_netSource.integer += 1;
      if (ui_netSource.integer >= numNetSources)
        ui_netSource.integer = 0;
    }

    //set varable now. //hypov8
    trap_Cvar_Set("ui_netSource", va("%d", ui_netSource.integer));
      UI_BuildServerDisplayList(qtrue);

    if (ui_netSource.integer != AS_FAVORITES) //hypov8 dont refresh fav
        UI_StartServerRefresh(qtrue);

    return qtrue;
  }
  return qfalse;
}

static qboolean UI_NetFilter_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
      ui_serverFilterType.integer--;
    else
      ui_serverFilterType.integer++;

    if (ui_serverFilterType.integer >= numServerFilters)
      ui_serverFilterType.integer = 0;
    else if (ui_serverFilterType.integer < 0)
      ui_serverFilterType.integer = numServerFilters - 1;

    UI_BuildServerDisplayList(qtrue);
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_OpponentName_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
      UI_PriorOpponent();
    else
      UI_NextOpponent();
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_BotName_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    int game  = (int)trap_Cvar_VariableValue("g_gametype");
    int value = uiInfo.botIndex;

    if (key == K_MOUSE2)
      value--;
    else
      value++;

    if (game >= GT_TEAM)
    {
      if (value >= uiInfo.characterCount + 2)
        value = 0;
      else if (value < 0)
        value = uiInfo.characterCount + 2 - 1;
    }
    else
    {
      if (value >= UI_GetNumBots() + 2)
        value = 0;
      else if (value < 0)
        value = UI_GetNumBots() + 2 - 1;
    }
    uiInfo.botIndex = value;
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_BotSkill_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
      uiInfo.skillIndex--;
    else
      uiInfo.skillIndex++;

    if (uiInfo.skillIndex >= numSkillLevels)
      uiInfo.skillIndex = 0;
    else if (uiInfo.skillIndex < 0)
      uiInfo.skillIndex = numSkillLevels - 1;

    return qtrue;
  }
  return qfalse;
}

static qboolean UI_RedBlue_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_ENTER || key == K_KP_ENTER)
  {
    uiInfo.redBlue += 1;
    if (uiInfo.redBlue > 2)
    uiInfo.redBlue = 0;
    return qtrue;
  }
  else if ( key == K_MOUSE2 )
  {
    uiInfo.redBlue -= 1;
    if (uiInfo.redBlue < 0)
    uiInfo.redBlue = 2;
    return qtrue;
  }

  return qfalse;
}

static qboolean UI_Crosshair_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    if (key == K_MOUSE2)
      uiInfo.currentCrosshair--;
    else
      uiInfo.currentCrosshair++;

    if (uiInfo.currentCrosshair >= NUM_CROSSHAIRS)
      uiInfo.currentCrosshair = 0;
    else if (uiInfo.currentCrosshair < 0)
      uiInfo.currentCrosshair = NUM_CROSSHAIRS - 1;

    trap_Cvar_Set("cg_drawCrosshair", va("%d", uiInfo.currentCrosshair));
    return qtrue;
  }
  return qfalse;
}



static qboolean UI_SelectedPlayer_HandleKey(int flags, float *special, int key)
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
  {
    int selected;

    UI_BuildPlayerList();
    if (!uiInfo.teamLeader)
    {
      return qfalse;
    }
    selected = (int)trap_Cvar_VariableValue("cg_selectedPlayer");

    if (key == K_MOUSE2)
    {
      selected--;
    }
    else
    {
      selected++;
    }

    if (selected > uiInfo.myTeamCount)
    {
      selected = 0;
    }
    else if (selected < 0)
    {
      selected = uiInfo.myTeamCount;
    }

    if (selected == uiInfo.myTeamCount)
    {
      trap_Cvar_Set("cg_selectedPlayerName", "Everyone");
    }
    else
    {
      trap_Cvar_Set("cg_selectedPlayerName", uiInfo.teamNames[selected]);
    }
    trap_Cvar_Set("cg_selectedPlayer", va("%d", selected));
  }
  return qfalse;
}


static qboolean UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key)
{
  switch (ownerDraw)
  {
  case UI_HANDICAP:
    return UI_Handicap_HandleKey(flags, special, key);

    break;
  case UI_EFFECTS:
    return UI_Effects_HandleKey(flags, special, key);

    break;
  case UI_CLANNAME:
    return UI_ClanName_HandleKey(flags, special, key);

    break;
  case UI_GAMETYPE:
    return UI_GameType_HandleKey(flags, special, key, qtrue);

    break;
  case UI_NETGAMETYPE:
    return UI_NetGameType_HandleKey(flags, special, key);

    break;
  case UI_JOINGAMETYPE:
    return UI_JoinGameType_HandleKey(flags, special, key);

    break;
  case UI_JOINWEAPONMODE:
    return UI_JoinWeaponMode_HandleKey(flags, special, key);

    break;
  case UI_SKILL:
    return UI_Skill_HandleKey(flags, special, key);

    break;
  case UI_NIKKITEAMNAME:
    return UI_TeamName_HandleKey(flags, special, key, qtrue);

    break;
  case UI_DRAGONTEAMNAME:
    return UI_TeamName_HandleKey(flags, special, key, qfalse);

    break;
  case UI_NIKKITEAM1:
  case UI_NIKKITEAM2:
  case UI_NIKKITEAM3:
  case UI_NIKKITEAM4:
  case UI_NIKKITEAM5:
    UI_TeamMember_HandleKey(flags, special, key, qtrue, ownerDraw - UI_NIKKITEAM1 + 1);
    break;
  case UI_DRAGONTEAM1:
  case UI_DRAGONTEAM2:
  case UI_DRAGONTEAM3:
  case UI_DRAGONTEAM4:
  case UI_DRAGONTEAM5:
    UI_TeamMember_HandleKey(flags, special, key, qfalse, ownerDraw - UI_DRAGONTEAM1 + 1);
    break;
  case UI_NETSOURCE:
    UI_NetSource_HandleKey(flags, special, key);
    break;
  case UI_NETFILTER:
    UI_NetFilter_HandleKey(flags, special, key);
    break;
  case UI_OPPONENT_NAME:
    UI_OpponentName_HandleKey(flags, special, key);
    break;
  case UI_BOTNAME:
    return UI_BotName_HandleKey(flags, special, key);

    break;
  case UI_BOTSKILL:
    return UI_BotSkill_HandleKey(flags, special, key);

    break;
  case UI_DRAGONNIKKI:
    UI_RedBlue_HandleKey(flags, special, key);
    break;
  case UI_CROSSHAIR:
    UI_Crosshair_HandleKey(flags, special, key);
    break;
  case UI_SELECTEDPLAYER:
    UI_SelectedPlayer_HandleKey(flags, special, key);
    break;
  case UI_ANISOTROPIC_VALUE:
    UI_DrawGLAnistopic_HandleKey(flags, special, key);
    break;

  case UI_VIDEO_VALUE:	//add hypov8
    UI_DrawGLModes_HandleKey(flags, special, key);
    break;

  case UI_QUICKQUALITY_VALUE:	//add hypov8
    UI_DrawGLQuickQuality_HandleKey(flags, special, key);
    break;

  case UI_TEXTUREMODE:	//add hypov8
    UI_GLMode_HandleKey(flags, special, key);
    break;


  default:
    break;
  }
  return qfalse;
}


static float UI_GetValue(int ownerDraw)
{
  return 0;
}

/*
=================
UI_ServersQsortCompare
=================
*/
static int QDECL UI_ServersQsortCompare(const void *arg1, const void *arg2)
{
  return trap_LAN_CompareServers(ui_netSource.integer, uiInfo.serverStatus.sortKey, uiInfo.serverStatus.sortDir, *(int *)arg1, *(int *)arg2);
}


/*
=================
UI_ServersSort
=================
*/
void UI_ServersSort(int column, qboolean force)
{

  if (!force)
  {
    if (uiInfo.serverStatus.sortKey == column)
      return;
  }

  uiInfo.serverStatus.sortKey = column;
  qsort(&uiInfo.serverStatus.displayServers[0], uiInfo.serverStatus.numDisplayServers, sizeof(int), UI_ServersQsortCompare);
}

/*
static void UI_StartSinglePlayer(void) {
    int i,j, k, skill;
    char buff[1024];
    i = trap_Cvar_VariableValue( "ui_currentTier" );
  if (i < 0 || i >= tierCount) {
    i = 0;
  }
    j = trap_Cvar_VariableValue("ui_currentMap");
    if (j < 0 || j > MAPS_PER_TIER) {
        j = 0;
    }

    trap_Cvar_SetValue( "singleplayer", 1 );
    trap_Cvar_SetValue( "g_gametype", Com_Clamp( 0, 7, tierList[i].gameTypes[j] ) );
    trap_Cmd_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n", tierList[i].maps[j] ) );
    skill = trap_Cvar_VariableValue( "g_spSkill" );

    if (j == MAPS_PER_TIER-1) {
        k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
        Com_sprintf( buff, sizeof(buff), "wait ; addbot %s %i %s 250 %s\n", UI_AIFromName(teamList[k].teamMembers[0]), skill, "", teamList[k].teamMembers[0]);
    } else {
        k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
        for (i = 0; i < PLAYERS_PER_TEAM; i++) {
            Com_sprintf( buff, sizeof(buff), "wait ; addbot %s %i %s 250 %s\n", UI_AIFromName(teamList[k].teamMembers[i]), skill, "Blue", teamList[k].teamMembers[i]);
            trap_Cmd_ExecuteText( EXEC_APPEND, buff );
        }

        k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
        for (i = 1; i < PLAYERS_PER_TEAM; i++) {
            Com_sprintf( buff, sizeof(buff), "wait ; addbot %s %i %s 250 %s\n", UI_AIFromName(teamList[k].teamMembers[i]), skill, "Red", teamList[k].teamMembers[i]);
            trap_Cmd_ExecuteText( EXEC_APPEND, buff );
        }
        trap_Cmd_ExecuteText( EXEC_APPEND, "wait 5; team Red\n" );
    }


}
*/

/*
===============
UI_LoadMods
===============
*/
static void UI_LoadMods(void)
{
  int numdirs;
  char dirlist[2048];
  char *dirptr;
  char *descptr;
  int i;
  size_t dirlen;

  uiInfo.modCount = 0;
  numdirs         = trap_FS_GetFileList("$modlist", "", dirlist, sizeof(dirlist));
  dirptr          = dirlist;
  for (i = 0; i < numdirs; i++)
  {
    dirlen                                   = qstrlen(dirptr) + 1;
    descptr                                  = dirptr + dirlen;
    uiInfo.modList[uiInfo.modCount].modName  = String_Alloc(dirptr);
    uiInfo.modList[uiInfo.modCount].modDescr = String_Alloc(descptr);
    dirptr                                  += dirlen + qstrlen(descptr) + 1;
    uiInfo.modCount++;
    if (uiInfo.modCount >= MAX_MODS)
      break;
  }
}

/*
===============
UI_LoadTeams
===============
*/
static void UI_LoadTeams(void)
{
  char teamList[4096];
  char *teamName;
  int i, count;
  size_t len;
  char teamPath[MAX_QPATH];

  count = trap_FS_GetFileList("teams", "team", teamList, 4096);

  if (count)
  {
    teamName = teamList;
    for (i = 0; i < count; i++)
    {
      len = qstrlen(teamName);
      Com_sprintf(teamPath, sizeof(teamPath), "teams/%s", teamName); //hypov8 moved to teams/ folder
      UI_ParseTeamInfo(teamPath);
      teamName += len + 1;
    }
  }

}

/*
===============
UI_LoadMovies
===============
*/
static void UI_LoadMovies(void)
{
  char movielist[4096];
  char *moviename;
  int i;
  size_t len;

  uiInfo.movieCount = trap_FS_GetFileList("video", "ogv", movielist, 4096);

  if (uiInfo.movieCount)
  {
    if (uiInfo.movieCount > MAX_MOVIES)
    {
      uiInfo.movieCount = MAX_MOVIES;
    }
    moviename = movielist;
    for (i = 0; i < uiInfo.movieCount; i++)
    {
      len = qstrlen(moviename);
      if (!Q_stricmp(moviename +  len - 4, ".ogv"))
      {
        moviename[len - 4] = '\0';
      }
      Q_strupr(moviename);
      uiInfo.movieList[i] = String_Alloc(moviename);
      moviename          += len + 1;
    }
  }

}

/*
===============
UI_LoadDemos
===============
*/
static void UI_LoadDemos(void)
{
  char demolist[4096];
  char demoExt[32];
  char *demoname;
  int i;
  size_t len;
  int	protocol;

  protocol = (int)trap_Cvar_VariableValue("com_protocol");//add hypov8
  if(!protocol)
    protocol = (int)trap_Cvar_VariableValue("protocol");

  Com_sprintf(demoExt, sizeof(demoExt), ".%s%d", DEMOEXT, protocol); //hypov8 add .
  uiInfo.demoCount = trap_FS_GetFileList("demos", demoExt, demolist, 4096);

  if (uiInfo.demoCount)
  {
    if (uiInfo.demoCount > MAX_DEMOS)
      uiInfo.demoCount = MAX_DEMOS;

    demoname = demolist;
    for (i = 0; i < uiInfo.demoCount; i++)
    {
      len = qstrlen(demoname);
      if (!Q_stricmp(demoname + len - qstrlen(demoExt), demoExt))
      demoname[len - qstrlen(demoExt)] = '\0';

      //Q_strupr(demoname);

      uiInfo.demoList[i] = String_Alloc(demoname);
      demoname += len + 1;
    }
  }
}


static int UI_FeederCount(float feederID);
static void UI_Update(const char *name)
{
  int val = (int)trap_Cvar_VariableValue(name);

  Com_DPrintf("UI_Update called %s\n", name);

  if (Q_stricmp(name, "ui_SetName") == 0)
  {
    trap_Cvar_Set("name", UI_Cvar_VariableString("ui_Name"));
  }
  else if (Q_stricmp(name, "ui_setRate") == 0)
  {
    float rate = trap_Cvar_VariableValue("rate");
    if (rate >= 5000)
    {
      trap_Cvar_Set("cl_maxpackets", "30");
      trap_Cvar_Set("cl_packetdup", "1");
    }
    else if (rate >= 4000)
    {
      trap_Cvar_Set("cl_maxpackets", "15");
      trap_Cvar_Set("cl_packetdup", "2");       // favor less prediction errors when there's packet loss
    }
    else
    {
      trap_Cvar_Set("cl_maxpackets", "15");
      trap_Cvar_Set("cl_packetdup", "1");       // favor lower bandwidth
    }
  }
  else if (Q_stricmp(name, "ui_GetName") == 0)
  {
    trap_Cvar_Set("ui_Name", UI_Cvar_VariableString("name")); //todo: check. dont save on esc?
  }
  else if (Q_stricmp(name, "custGFX") == 0)
  { //show apply button when gfx changed
    trap_Cvar_SetValue("uix_videoEdited", 1);
    //trap_Cvar_SetValue("uix_r_custQuality", 0);
  }
  else if (Q_stricmp(name, "ui_team") == 0)
  { 
    UI_FeederSelection(FEEDER_HEADS, UI_GetSelectedHeadIndex());
  }

  //hypov8
  else if (Q_stricmp(name, "uix_r_hdrRendering") == 0)
  {
    trap_Cvar_SetValue("uix_r_recompileShaders", 1); //add hypov8. not needed
  }

#if 0
  else if (Q_stricmp(name, "uix_rebuildgls") == 0)
  {
   trap_Cvar_SetValue("uix_r_recompileShaders", 1);
  }
#endif
  else if (Q_stricmp(name, "uix_r_colorbits") == 0)
  {
    switch (val)
    {
    case 0:
      trap_Cvar_SetValue("uix_r_depthbits", 0);
      trap_Cvar_SetValue("uix_r_stencilbits", 0);
      break;
    case 16:
      trap_Cvar_SetValue("uix_r_depthbits", 16);
      trap_Cvar_SetValue("uix_r_stencilbits", 0);
      break;
    case 32:
      trap_Cvar_SetValue("uix_r_depthbits", 24);
      break;
    }
  trap_Cvar_SetValue("uix_r_recompileShaders", 1); //add hypov8
  }
  else if (Q_stricmp(name, "uix_r_lodBias") == 0)
  {
    switch (val)
    {
    case 0:
      trap_Cvar_SetValue("uix_r_subdivisions", 4);
      break;
    case 1:
      trap_Cvar_SetValue("uix_r_subdivisions", 12);
      break;
    case 2:
      trap_Cvar_SetValue("uix_r_subdivisions", 20);
      break;
    }
  }
  else if (Q_stricmp(name, "uix_r_custQuality") == 0)
  { //hypov8: updated fast GFX.
    trap_Cvar_Set("uix_videoEdited", "1"); //show apply button

    //FIXME (0xA5EA): this could mess up the gfx setting !!!!!
    //hypov8 fixed: no longer setting rez, bit depth etc..
    //settings are also stored temporary and can esc without settings being applied
    switch (val)
    {
    case 0:	//"Custom"
      UI_UpdateCvarList();	//hypo reset to old values
      break;

    case 1:	// Extreme
      trap_Cvar_SetValue("uix_r_dynamicLight", 1);
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 1);
      trap_Cvar_SetValue("uix_r_softShadows", 1);
      trap_Cvar_SetValue("uix_r_shadowBlur", 3);
      trap_Cvar_SetValue("uix_r_shadowMapQuality", 3); //3 = high
      trap_Cvar_SetValue("uix_r_bloom", 1);
      trap_Cvar_SetValue("uix_r_dynamicBspOcclusionCulling", 0); //if cg_shadows >1 == 0
      trap_Cvar_SetValue("uix_cg_shadows", 6); ///3 5 6
      trap_Cvar_SetValue("uix_r_hdrRendering", 1);
      trap_Cvar_SetValue("uix_r_picmip", 0);
      trap_Cvar_SetValue("uix_r_ext_texture_filter_anisotropic", 16);
      trap_Cvar_SetValue("uix_r_lodbias", 0);
      trap_Cvar_SetValue("uix_r_reflectionmapping", 1);
      trap_Cvar_SetValue("uix_r_ext_compressed_textures", 0);
      trap_Cvar_SetValue("uix_r_halfLambertLighting", 1);
      trap_Cvar_SetValue("uix_r_normalMapping", 1);
      trap_Cvar_SetValue("uix_r_parallaxMapping", 1);
      trap_Cvar_SetValue("uix_r_subdivisions", 4);
      trap_Cvar_SetValue("uix_r_texturemode_menu", 0); //GL_LINEAR
      //trap_Cvar_Set();
      break;

    case 2:	//"Very High"
      trap_Cvar_SetValue("uix_r_dynamicLight", 1);
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 1);
      trap_Cvar_SetValue("uix_r_softShadows", 1);
      trap_Cvar_SetValue("uix_r_shadowBlur", 3);
      trap_Cvar_SetValue("uix_r_shadowMapQuality", 3); //3 = high
      trap_Cvar_SetValue("uix_r_bloom", 1);
      trap_Cvar_SetValue("uix_r_dynamicBspOcclusionCulling", 0); //if cg_shadows >1 == 0
      trap_Cvar_SetValue("uix_cg_shadows", 6); ///3 5 6
      trap_Cvar_SetValue("uix_r_hdrRendering", 1);
      trap_Cvar_SetValue("uix_r_picmip", 0);
      trap_Cvar_SetValue("uix_r_ext_texture_filter_anisotropic", 16);
      trap_Cvar_SetValue("uix_r_lodbias", 0);
      trap_Cvar_SetValue("uix_r_reflectionmapping", 1);
      trap_Cvar_SetValue("uix_r_ext_compressed_textures", 0);
      trap_Cvar_SetValue("uix_r_halfLambertLighting", 1);
      trap_Cvar_SetValue("uix_r_normalMapping", 1);
      trap_Cvar_SetValue("uix_r_parallaxMapping", 1);
      trap_Cvar_SetValue("uix_r_subdivisions", 4);
      trap_Cvar_SetValue("uix_r_texturemode_menu", 2); //GL_LINEAR_MIPMAP_LINEAR
      break;

    case 3: 	//"High"
      trap_Cvar_SetValue("uix_r_dynamicLight", 1);
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 0);
      trap_Cvar_SetValue("uix_r_softShadows", 0);
      trap_Cvar_SetValue("uix_r_shadowBlur", 0);
      trap_Cvar_SetValue("uix_r_shadowMapQuality", 1); //3 = high
      trap_Cvar_SetValue("uix_r_bloom", 0);
      trap_Cvar_SetValue("uix_r_dynamicBspOcclusionCulling", 0); //if cg_shadows >1 == 0
      trap_Cvar_SetValue("uix_cg_shadows", 3); ///3 5 6
      trap_Cvar_SetValue("uix_r_hdrRendering", 0);
      trap_Cvar_SetValue("uix_r_picmip", 0);
      trap_Cvar_SetValue("uix_r_ext_texture_filter_anisotropic", 8);
      trap_Cvar_SetValue("uix_r_lodbias", 0);
      trap_Cvar_SetValue("uix_r_reflectionmapping", 0);
      trap_Cvar_SetValue("uix_r_ext_compressed_textures", 1);
      trap_Cvar_SetValue("uix_r_halfLambertLighting", 1);
      trap_Cvar_SetValue("uix_r_normalMapping", 1);
      trap_Cvar_SetValue("uix_r_parallaxMapping", 0);
      trap_Cvar_SetValue("uix_r_subdivisions", 4);
      trap_Cvar_SetValue("uix_r_texturemode_menu", 2); //GL_LINEAR_MIPMAP_LINEAR
      break;

    case 4:		//"Medium"
      trap_Cvar_SetValue("uix_r_dynamicLight", 1);
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 0);
      trap_Cvar_SetValue("uix_r_softShadows", 0);
      trap_Cvar_SetValue("uix_r_shadowBlur", 0);
      trap_Cvar_SetValue("uix_r_shadowMapQuality", 1); //3 = high
      trap_Cvar_SetValue("uix_r_bloom", 0);
      trap_Cvar_SetValue("uix_r_dynamicBspOcclusionCulling", 0); //if cg_shadows >1 == 0
      trap_Cvar_SetValue("uix_cg_shadows", 3); ///3 5 6
      trap_Cvar_SetValue("uix_r_hdrRendering", 0);
      trap_Cvar_SetValue("uix_r_picmip", 0);
      trap_Cvar_SetValue("uix_r_ext_texture_filter_anisotropic", 4);
      trap_Cvar_SetValue("uix_r_lodbias", 0);
      trap_Cvar_SetValue("uix_r_reflectionmapping", 0);
      trap_Cvar_SetValue("uix_r_ext_compressed_textures", 1);
      trap_Cvar_SetValue("uix_r_halfLambertLighting", 0);
      trap_Cvar_SetValue("uix_r_normalMapping", 1);
      trap_Cvar_SetValue("uix_r_parallaxMapping", 0);
      trap_Cvar_SetValue("uix_r_subdivisions", 4);
      trap_Cvar_SetValue("uix_r_texturemode_menu", 2); //GL_LINEAR_MIPMAP_LINEAR
      break;

    case 5:		//"Low"
      trap_Cvar_SetValue("uix_r_dynamicLight", 1);
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 0);
      trap_Cvar_SetValue("uix_r_softShadows", 0);
      trap_Cvar_SetValue("uix_r_shadowBlur", 0);
      trap_Cvar_SetValue("uix_r_shadowMapQuality", 1); //3 = high
      trap_Cvar_SetValue("uix_r_bloom", 0);
      trap_Cvar_SetValue("uix_r_dynamicBspOcclusionCulling", 1); //if cg_shadows >1 == 0
      trap_Cvar_SetValue("uix_cg_shadows", 0); ///3 5 6
      trap_Cvar_SetValue("uix_r_hdrRendering", 0);
      trap_Cvar_SetValue("uix_r_picmip", 1);
      trap_Cvar_SetValue("uix_r_ext_texture_filter_anisotropic", 0);
      trap_Cvar_SetValue("uix_r_lodbias", 1);
      trap_Cvar_SetValue("uix_r_reflectionmapping", 0);
      trap_Cvar_SetValue("uix_r_ext_compressed_textures", 1);
      trap_Cvar_SetValue("uix_r_halfLambertLighting", 0);
      trap_Cvar_SetValue("uix_r_normalMapping", 1);
      trap_Cvar_SetValue("uix_r_parallaxMapping", 0);
      trap_Cvar_SetValue("uix_r_subdivisions", 12);
      trap_Cvar_SetValue("uix_r_texturemode_menu", 1); //GL_LINEAR_MIPMAP_NEAREST
      break;

    case 6:	//hypov8 windowed
      trap_Cvar_SetValue("uix_r_fullScreen", 0);
      trap_Cvar_SetValue("uix_r_mode", 5); //hypov8 4=800x600 5=1024x768
      trap_Cvar_SetValue("uix_r_dynamicLight", 1);
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 0);
      trap_Cvar_SetValue("uix_r_softShadows", 0);
      trap_Cvar_SetValue("uix_r_shadowBlur", 0);
      trap_Cvar_SetValue("uix_r_shadowMapQuality", 1); //3 = high
      trap_Cvar_SetValue("uix_r_bloom", 0);
      trap_Cvar_SetValue("uix_r_dynamicBspOcclusionCulling", 0); //if cg_shadows >1 == 0
      trap_Cvar_SetValue("uix_cg_shadows", 3); ///3 5 6
      trap_Cvar_SetValue("uix_r_hdrRendering", 0);
      trap_Cvar_SetValue("uix_r_picmip", 1);
      trap_Cvar_SetValue("uix_r_ext_texture_filter_anisotropic", 4);
      trap_Cvar_SetValue("uix_r_lodbias", 0);
      trap_Cvar_SetValue("uix_r_reflectionmapping", 1);
      trap_Cvar_SetValue("uix_r_ext_compressed_textures", 1);
      trap_Cvar_SetValue("uix_r_halfLambertLighting", 1);
      trap_Cvar_SetValue("uix_r_normalMapping", 1);
      trap_Cvar_SetValue("uix_r_parallaxMapping", 1);
      trap_Cvar_SetValue("uix_r_subdivisions", 4);
      trap_Cvar_SetValue("uix_r_texturemode_menu", 1); //GL_LINEAR_MIPMAP_NEAREST
      break;
    }

    // trap_Cvar_SetValue("r_colorbits", 32);
    // trap_Cvar_SetValue("r_depthbits", 24);
    // trap_Cvar_SetValue("r_subdivisions", 4);
    //trap_Cvar_SetValue("uix_r_fastSky", 0);
    // trap_Cvar_SetValue("r_inGameVideo", 1);
    //trap_Cvar_SetValue("cg_brassTime", 2500);
    //"r_colorbits" text "Color Depth:" 0 16 32
    //trap_Cvar_SetValue("r_vertexlight", 0);

    trap_Cvar_SetValue("uix_r_recompileShaders", 1); //move
  }

  else if (Q_stricmp(name, "ui_mousePitch") == 0)
  {
    if (val == 0)
      trap_Cvar_SetValue("m_pitch", 0.022f);
    else
      trap_Cvar_SetValue("m_pitch", -0.022f);
  }
#if 0
  else if (!Q_stricmp(name, "uix_r_normalMapping"))
  {
    int tmpn, tmpp;
    tmpn = (int)trap_Cvar_VariableValue("uix_r_normalMapping");
    tmpp = (int)trap_Cvar_VariableValue("uix_r_parallaxMapping");

    if ( tmpn == 0 && tmpp != 0)
      trap_Cvar_SetValue("uix_r_parallaxMapping", 0);
  }
  else if (!Q_stricmp(name, "uix_r_parallaxMapping"))
  {
    int tmpn, tmpp;
    tmpn = (int)trap_Cvar_VariableValue("uix_r_normalMapping");
    tmpp = (int)trap_Cvar_VariableValue("uix_r_parallaxMapping");

    if ( tmpn == 0 && tmpp != 0)
      trap_Cvar_SetValue("uix_r_normalMapping", 1);
  }
#endif
  else if (Q_stricmp(name, "uix_r_mode") == 0)
  {
    trap_Cvar_SetValue("uix_r_recompileShaders", 1); //add hypov8
  }

 // ui_glLights
  else if (Q_stricmp(name, "uix_cg_shadows") == 0)
  {
    int cg_shd = (int)trap_Cvar_VariableValue("uix_cg_shadows");
    //int dyn_lt_shd = (int)trap_Cvar_VariableValue("uix_r_dynamicLightCastShadows");

    if (cg_shd <= 1) //not realtime shadows
    {
      trap_Cvar_SetValue("uix_r_softShadows", 0);
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 0);
      trap_Cvar_SetValue("uix_r_dynamicBspOcclusionCulling", 0); //hypov8 ON. causing parallal lights to crash with shadows disabled
    }
    else //if (cg_shd > 1)
    {
      //trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 1);
      trap_Cvar_SetValue("uix_r_dynamicBspOcclusionCulling", 0);
    }
    trap_Cvar_SetValue("uix_r_recompileShaders", 1); //add hypov8
    UI_UpdateLightCvar(); //hypov8
  }

  else if (Q_stricmp(name, "uix_r_dynamicLightCastShadows") == 0)
  {
    int cg_shadws = (int)trap_Cvar_VariableValue("uix_cg_shadows");
    int r_dynLightShad = (int)trap_Cvar_VariableValue("uix_r_dynamicLightCastShadows");

    if (cg_shadws > 1 && r_dynLightShad == 1) //"Shadows"
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 1);
    else // if (cg_shadws <= 1)
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 0);
  }
  else if (Q_stricmp(name, "uix_r_dynamicLight") == 0) //realtime lights. gun flash etc.. on world
  {
    int dyn_light = (int)trap_Cvar_VariableValue("uix_r_dynamicLight");
    if (dyn_light == 0)
      trap_Cvar_SetValue("uix_r_dynamicLightCastShadows", 0);

    UI_UpdateLightCvar();
  }

  else if (Q_stricmp(name, "uix_r_shadowMapQuality") == 0)
  {
    trap_Cvar_SetValue("uix_r_recompileShaders", 1); //add hypov8
  }
}

static qboolean UI_CVARCompareStr(char*s1, char*s2)
{
  char tmp1[128], tmp2[128];

  trap_Cvar_VariableStringBuffer(s1, tmp1, 128);
  trap_Cvar_VariableStringBuffer(s2, tmp2, 128);

  if (Q_stricmp(tmp1, tmp2))
    return qtrue;
  return qfalse;
}
static qboolean UI_CVARCompareInt(char*s1, char*s2)
{
  if ((int)trap_Cvar_VariableValue(s1) != (int)trap_Cvar_VariableValue(s2))
    return qtrue;
  return qfalse;
}
static qboolean UI_CVARCompareFloat(char*s1, char*s2)
{
  char tmp1[128], tmp2[128];

  trap_Cvar_VariableStringBuffer(s1, tmp1, 128);
  trap_Cvar_VariableStringBuffer(s2, tmp2, 128);
  if (Q_stricmp(va("%.6s", tmp1), va("%.6s", tmp2))) //truncate to 6 for compare
    return qtrue;
  return qfalse;
}

static void UI_CVARGetVidModeString(char * s)
{
  int uix_r_texturemode_menu = (int)trap_Cvar_VariableValue(ui_custStrings[0][1]); //uix_r_texturemode_menu

  switch (uix_r_texturemode_menu)
  {
    case 0:
    case 1:
    case 2:  qstrcpy(s, ui_vidGLModeNames[uix_r_texturemode_menu]); 
      break;
    default: trap_Cvar_VariableStringBuffer(ui_custStrings[1][1], s, 128); //3.. restore orig value
  }
}

static qboolean UI_GFXSettingsChanged()
{
  int i;
  char uix_r_texturemode_st[128];

  //update from menu index to cvar
  UI_CVARGetVidModeString(uix_r_texturemode_st);
  trap_Cvar_Set(ui_custStrings[1][1], uix_r_texturemode_st);

  if (UI_CVARCompareStr(ui_custStrings[1][0], ui_custStrings[1][1])) //r_texturemode
    return qtrue;
  if (UI_CVARCompareInt(ui_custStrings[2][0], ui_custStrings[2][1])) //r_mode
    return qtrue;
  if (UI_CVARCompareFloat(ui_custStrings[3][0], ui_custStrings[3][1])) //r_forceAmbient
    return qtrue;
  for (i = 4; i < UI_CVAR_STRCOUNT; i++) //>3
  {
    if (UI_CVARCompareInt(ui_custStrings[i][0], ui_custStrings[i][1]))
      return qtrue;
  }

  return qfalse;
}

static void UI_RunMenuScript(char **args)
{
  const char *name, *name2;
  char buff[1024];

  if (String_Parse(args, &name))
  {
    Com_DPrintf("in UI_RunMenuScript( %s ) \n", name);
    if (Q_stricmp(name, "StartServer") == 0)
    {
      int i, clients, oldclients;
      float skill;
      trap_Cvar_Set("cg_thirdPerson", "0");
      trap_Cvar_Set("cg_cameraOrbit", "0");
#ifdef USE_GT_SINGLEPLAYER
      trap_Cvar_Set("ui_singlePlayerActive", "0");
#endif
      trap_Cvar_SetValue("dedicated", Com_Clamp(0, 2, ui_dedicated.integer));
      trap_Cvar_SetValue("g_gametype", Com_Clamp(0, 8, uiInfo.gameTypes[ui_netGameType.integer].gtEnum));
      trap_Cvar_SetValue("g_weaponmod", Com_Clamp(0, 8, uiInfo.weaponModes[ui_joinWeaponMode.integer].gtEnum));
      trap_Cvar_Set("g_dragonTeam", UI_Cvar_VariableString("ui_teamName"));
      trap_Cvar_Set("g_nikkiTeam", UI_Cvar_VariableString("ui_opponentName"));
      trap_Cmd_ExecuteText(EXEC_APPEND, va("wait ; wait ; map %s\n", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName));
      skill = trap_Cvar_VariableValue("g_spSkill");
      // set max clients based on spots
      oldclients = (int)trap_Cvar_VariableValue("sv_maxClients");
      clients    = 0;
      for (i = 0; i < PLAYERS_PER_TEAM; i++)
      {
        int bot = (int)trap_Cvar_VariableValue(va("ui_nikkiTeam%i", i + 1));
        if (bot >= 0)
          clients++;

        bot = (int)trap_Cvar_VariableValue(va("ui_dragonTeam%i", i + 1));
        if (bot >= 0)
          clients++;
      }
      if (clients == 0)
        clients = 8;

      if (oldclients > clients)
        clients = oldclients;

      trap_Cvar_Set("sv_maxClients", va("%d", clients));

      for (i = 0; i < PLAYERS_PER_TEAM; i++)
      {
        int bot = (int)trap_Cvar_VariableValue(va("ui_nikkiTeam%i", i + 1));
        if (bot > 1)
        {
          if (ui_actualNetGameType.integer >= GT_TEAM)
            Com_sprintf(buff, sizeof(buff), "addbot %s %f %s\n", uiInfo.characterList[bot - 2].name, skill, TEAM_SKIN_NIKKIS);
          else
            Com_sprintf(buff, sizeof(buff), "addbot %s %f \n", UI_GetBotNameByNumber(bot - 2), skill);

          trap_Cmd_ExecuteText(EXEC_APPEND, buff);
        }
        bot = (int)trap_Cvar_VariableValue(va("ui_dragonTeam%i", i + 1));
        if (bot > 1)
        {
          if (ui_actualNetGameType.integer >= GT_TEAM)
            Com_sprintf(buff, sizeof(buff), "addbot %s %f %s\n", uiInfo.characterList[bot - 2].name, skill, TEAM_SKIN_DRAGONS);
          else
            Com_sprintf(buff, sizeof(buff), "addbot %s %f \n", UI_GetBotNameByNumber(bot - 2), skill);

          trap_Cmd_ExecuteText(EXEC_APPEND, buff);
        }
      }
    }
#ifdef USE_GT_SINGLEPLAYER
    else if (Q_stricmp(name, "updateSPMenu") == 0)
    {
      UI_SetCapFragLimits(qtrue);
      UI_MapCountByGameType(qtrue);
      ui_mapIndex.integer = UI_GetIndexFromSelection(ui_currentMap.integer);
      trap_Cvar_Set("ui_mapIndex", va("%d", ui_mapIndex.integer));
      Menu_SetFeederSelection(NULL, FEEDER_MAPS, ui_mapIndex.integer, "skirmish");
      UI_GameType_HandleKey(0, NULL, K_MOUSE1, qfalse);
      UI_GameType_HandleKey(0, NULL, K_MOUSE2, qfalse);
    }
#endif
    else if (Q_stricmp(name, "resetDefaults") == 0)
    {
      trap_Cmd_ExecuteText(EXEC_APPEND, "exec default.cfg\n");
      trap_Cmd_ExecuteText(EXEC_APPEND, "cvar_restart\n");
      Controls_SetDefaults();
      trap_Cvar_Set("com_introPlayed", "1");
      trap_Cvar_Set("r_recompileShaders", "1"); //hypov8
      trap_Cmd_ExecuteText(EXEC_APPEND, "vid_restart\n");
#ifndef STANDALONE  // 0xA5EA
    }
    else if (Q_stricmp(name, "getCDKey") == 0)
    {
      char out[17];
      trap_GetCDKey(buff, 17);
      trap_Cvar_Set("cdkey1", "");
      trap_Cvar_Set("cdkey2", "");
      trap_Cvar_Set("cdkey3", "");
      trap_Cvar_Set("cdkey4", "");
      if (qstrlen(buff) == CDKEY_LEN)
      {
        Q_strncpyz(out, buff, 5);
        trap_Cvar_Set("cdkey1", out);
        Q_strncpyz(out, buff + 4, 5);
        trap_Cvar_Set("cdkey2", out);
        Q_strncpyz(out, buff + 8, 5);
        trap_Cvar_Set("cdkey3", out);
        Q_strncpyz(out, buff + 12, 5);
        trap_Cvar_Set("cdkey4", out);
      }
    }
    else if (Q_stricmp(name, "verifyCDKey") == 0)
    {
      buff[0] = '\0';
      Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey1"));
      Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey2"));
      Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey3"));
      Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey4"));
      trap_Cvar_Set("cdkey", buff);
      if (trap_VerifyCDKey(buff, UI_Cvar_VariableString("cdkeychecksum")))
      {
        trap_Cvar_Set("ui_cdkeyvalid", "CD Key Appears to be valid.");
        trap_SetCDKey(buff);
      }
      else
      {
        trap_Cvar_Set("ui_cdkeyvalid", "CD Key does not appear to be valid.");
      }
#endif      // STANDALONE
    }
    else if (Q_stricmp(name, "loadArenas") == 0)
    {
      UI_LoadArenas();
      UI_MapCountByGameType(qfalse);
      Menu_SetFeederSelection(NULL, FEEDER_ALLMAPS, 0, "createserver");
    }
    else if (Q_stricmp(name, "saveControls") == 0)
    {
      Controls_SetConfig(qtrue);
    }
    else if (Q_stricmp(name, "loadControls") == 0)
    {
      Controls_GetConfig();
    }
    else if (Q_stricmp(name, "clearError") == 0)
    {
      trap_Cvar_Set("com_errorMessage", "");
    }
    else if (Q_stricmp(name, "loadGameInfo") == 0)
    {
      UI_ParseGameInfo("gameinfo.txt");
      UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
    }
    else if (Q_stricmp(name, "resetScores") == 0)
    {
      UI_ClearScores();
    }
    else if (Q_stricmp(name, "RefreshServers") == 0)
    {
      UI_StartServerRefresh(qtrue);
      UI_BuildServerDisplayList(qtrue);
    }
    else if (Q_stricmp(name, "RefreshFilter") == 0)
    {
      UI_StartServerRefresh(qfalse); //hypov8 note: pings servers?
      UI_BuildServerDisplayList(qtrue);
    }
#ifdef USE_GT_SINGLEPLAYER
    else if (Q_stricmp(name, "RunSPDemo") == 0)
    {
      if (uiInfo.demoAvailable)
      {
        trap_Cmd_ExecuteText(EXEC_APPEND, va("demo %s_%i\n", uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum));
      }
    }
#endif
    else if (Q_stricmp(name, "LoadDemos") == 0)
    {
      UI_LoadDemos();
    }
    else if (Q_stricmp(name, "LoadMovies") == 0)
    {
      UI_LoadMovies();
    }
    else if (Q_stricmp(name, "LoadMods") == 0)
    {
      UI_LoadMods();
    }
    else if (Q_stricmp(name, "playMovie") == 0)
    {
      if (uiInfo.previewMovie >= 0)
      {
        trap_CIN_StopCinematic(uiInfo.previewMovie);
      }
      trap_Cmd_ExecuteText(EXEC_APPEND, va("cinematic %s.ogv 2\n", uiInfo.movieList[uiInfo.movieIndex]));
    }
    else if (Q_stricmp(name, "RunMod") == 0)
    {
      trap_Cvar_Set("fs_game", uiInfo.modList[uiInfo.modIndex].modName);
      trap_Cmd_ExecuteText(EXEC_APPEND, "vid_restart;");
    }
    else if (Q_stricmp(name, "RunDemo") == 0)
    {
      if (uiInfo.demoIndex >= 0 && uiInfo.demoIndex < MAX_DEMOS && uiInfo.demoList[uiInfo.demoIndex] != NULL)
        trap_Cmd_ExecuteText(EXEC_APPEND, va("demo %s\n", uiInfo.demoList[uiInfo.demoIndex]));
      else
        Com_Printf(S_COLOR_RED"ERROR: failed to load a demo.\n");
    }
    else if (Q_stricmp(name, "KingpinQ3") == 0)
    {
      trap_Cvar_Set("fs_game", "");
      trap_Cmd_ExecuteText(EXEC_APPEND, "vid_restart;");
    }
    else if (Q_stricmp(name, "closeJoin") == 0)
    {
      if (uiInfo.serverStatus.refreshActive)
      {
        UI_StopServerRefresh();
        uiInfo.serverStatus.nextDisplayRefresh = 0;
        uiInfo.nextServerStatusRefresh         = 0;
        uiInfo.nextFindPlayerRefresh           = 0;
        UI_BuildServerDisplayList(qtrue);
      }
      else
      {
        Menus_CloseByName("joinserver");
        Menus_OpenByName("main");
      }
    }
    else if (Q_stricmp(name, "StopRefresh") == 0)
    {
      UI_StopServerRefresh();
      uiInfo.serverStatus.nextDisplayRefresh = 0;
      uiInfo.nextServerStatusRefresh         = 0;
      uiInfo.nextFindPlayerRefresh           = 0;
    }
    else if (Q_stricmp(name, "UpdateFilter") == 0)
    {
      if (ui_netSource.integer == AS_LOCAL)
      {
        UI_StartServerRefresh(qtrue);
      }
      UI_BuildServerDisplayList(qtrue);
      UI_FeederSelection(FEEDER_SERVERS, 0);
    }
    else if (Q_stricmp(name, "ServerStatus") == 0)
    {
      trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], uiInfo.serverStatusAddress, sizeof(uiInfo.serverStatusAddress));
      UI_BuildServerStatus(qtrue);
    }
    else if (Q_stricmp(name, "FoundPlayerServerStatus") == 0)
    {
      Q_strncpyz(uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof(uiInfo.serverStatusAddress));
      UI_BuildServerStatus(qtrue);
      Menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
    }
    else if (Q_stricmp(name, "FindPlayer") == 0)
    {
      UI_BuildFindPlayerList(qtrue);
      // clear the displayed server status info
      uiInfo.serverStatusInfo.numLines = 0;
      Menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
    }
    else if (Q_stricmp(name, "JoinServer") == 0)
    {
      trap_Cvar_Set("cg_thirdPerson", "0");
      trap_Cvar_Set("cg_cameraOrbit", "0");
#ifdef USE_GT_SINGLEPLAYER
      trap_Cvar_Set("ui_singlePlayerActive", "0");
#endif
      if (uiInfo.serverStatus.currentServer >= 0 && uiInfo.serverStatus.currentServer < uiInfo.serverStatus.numDisplayServers)
      {
        trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, 1024);
        trap_Cmd_ExecuteText(EXEC_APPEND, va("connect %s\n", buff));
      }
    }
    else if (Q_stricmp(name, "FoundPlayerJoinServer") == 0)
    {
#ifdef USE_GT_SINGLEPLAYER
      trap_Cvar_Set("ui_singlePlayerActive", "0");
#endif
      if (uiInfo.currentFoundPlayerServer >= 0 && uiInfo.currentFoundPlayerServer < uiInfo.numFoundPlayerServers)
      {
        trap_Cmd_ExecuteText(EXEC_APPEND, va("connect %s\n", uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer]));
      }
    }
    else if (Q_stricmp(name, "Quit") == 0)
    {
#ifdef USE_GT_SINGLEPLAYER
      trap_Cvar_Set("ui_singlePlayerActive", "0");
#endif
      trap_Cmd_ExecuteText(EXEC_NOW, "quit");
    }
    else if (Q_stricmp(name, "Controls") == 0)
    {
      trap_Cvar_Set("cl_paused", "1");
      trap_Key_SetCatcher(KEYCATCH_UI);
      Menus_CloseAll();
      Menus_ActivateByName("setup_menu2");
    }
    else if (Q_stricmp(name, "Leave") == 0)
    {
      trap_Cmd_ExecuteText(EXEC_APPEND, "disconnect\n");
      trap_Key_SetCatcher(KEYCATCH_UI);
      Menus_CloseAll();
      Menus_ActivateByName("main");
    }
    else if (Q_stricmp(name, "ServerSort") == 0)
    {
      int sortColumn;
      if (Int_Parse(args, &sortColumn))
      {
        // if same column we're already sorting on then flip the direction
        if (sortColumn == uiInfo.serverStatus.sortKey)
        {
          uiInfo.serverStatus.sortDir = !uiInfo.serverStatus.sortDir;
        }
        // make sure we sort again
        UI_ServersSort(sortColumn, qtrue);
      }
    }
#ifdef USE_GT_SINGLEPLAYER
    else if (Q_stricmp(name, "nextSkirmish") == 0)
    {
      UI_StartSkirmish(qtrue);
    }
    else if (Q_stricmp(name, "SkirmishStart") == 0)
    {
      UI_StartSkirmish(qfalse);
    }
#endif
    else if (Q_stricmp(name, "closeingame") == 0)
    {
      trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
      trap_Key_ClearStates();
      trap_Cvar_Set("cl_paused", "0");
      Menus_CloseAll();
    }
    else if (Q_stricmp(name, "voteMap") == 0)
    {
      if (ui_currentNetMap.integer >= 0 && ui_currentNetMap.integer < uiInfo.mapCount)
      {
        trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote map %s\n", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName));
      }
    }
    else if (Q_stricmp(name, "voteKick") == 0)
    {
      if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount)
      {
        trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote kick %s\n", uiInfo.playerNames[uiInfo.playerIndex]));
      }
    }
    else if (Q_stricmp(name, "voteGame") == 0)
    {
      if (ui_netGameType.integer >= 0 && ui_netGameType.integer < uiInfo.numGameTypes)
      {
        trap_Cmd_ExecuteText(EXEC_APPEND, va("callvote g_gametype %i\n", uiInfo.gameTypes[ui_netGameType.integer].gtEnum));
      }
      //FIXME(0xA5EA): weaponmode also needed
    }
    else if (Q_stricmp(name, "voteLeader") == 0)
    {
      if (uiInfo.teamIndex >= 0 && uiInfo.teamIndex < uiInfo.myTeamCount)
      {
        trap_Cmd_ExecuteText(EXEC_APPEND, va("callteamvote leader %s\n", uiInfo.teamNames[uiInfo.teamIndex]));
      }
    }
    else if (Q_stricmp(name, "addBot") == 0)
    {
      if ((int)trap_Cvar_VariableValue("g_gametype") >= GT_TEAM)
      {
        trap_Cmd_ExecuteText(EXEC_APPEND, va("addbot %s %i %s\n", uiInfo.characterList[uiInfo.botIndex].name, uiInfo.skillIndex + 1, (uiInfo.redBlue == 0) ?"": (uiInfo.redBlue == 1) ?  TEAM_SKIN_DRAGONS : TEAM_SKIN_NIKKIS));
      }
      else
      {
        trap_Cmd_ExecuteText(EXEC_APPEND, va("addbot %s %i %s\n", UI_GetBotNameByNumber(uiInfo.botIndex), uiInfo.skillIndex + 1, (uiInfo.redBlue == 0) ? "": (uiInfo.redBlue == 1) ? TEAM_SKIN_DRAGONS : TEAM_SKIN_NIKKIS));
      }
    }
    else if (Q_stricmp(name, "addFavorite") == 0)
    {
      if (ui_netSource.integer != AS_FAVORITES)
      {
        char name[MAX_NAME_LENGTH];
        char addr[MAX_NAME_LENGTH];
        int res;

        trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
        name[0] = addr[0] = '\0';
        Q_strncpyz(name,    Info_ValueForKey(buff, "hostname"), MAX_NAME_LENGTH);
        Q_strncpyz(addr,    Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
        if (qstrlen(name) > 0 && qstrlen(addr) > 0)
        {
          res = trap_LAN_AddServer(AS_FAVORITES, name, addr);
          if (res == 0)
          {
            // server already in the list
            Com_Printf("Favorite already in list\n");
          }
          else if (res == -1)
          {
            // list full
            Com_Printf("Favorite list full\n");
          }
          else
          {
            // successfully added
            Com_Printf("Added favorite server %s\n", addr);
          }
        }
      }
    }
    else if (Q_stricmp(name, "deleteFavorite") == 0)
    {
      if (ui_netSource.integer == AS_FAVORITES)
      {
        char addr[MAX_NAME_LENGTH];
        trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
        addr[0] = '\0';
        Q_strncpyz(addr,    Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
        if (qstrlen(addr) > 0)
        {
          trap_LAN_RemoveServer(AS_FAVORITES, addr);
        }
      }
    }
    else if (Q_stricmp(name, "createFavorite") == 0)
    {
      if (ui_netSource.integer == AS_FAVORITES)
      {
        char name[MAX_NAME_LENGTH];
        char addr[MAX_NAME_LENGTH];
        int res;

        name[0] = addr[0] = '\0';
        Q_strncpyz(name,    UI_Cvar_VariableString("ui_favoriteName"), MAX_NAME_LENGTH);
        Q_strncpyz(addr,    UI_Cvar_VariableString("ui_favoriteAddress"), MAX_NAME_LENGTH);
        if (qstrlen(name) > 0 && qstrlen(addr) > 0)
        {
          res = trap_LAN_AddServer(AS_FAVORITES, name, addr);
          if (res == 0)
          {
            // server already in the list
            Com_Printf("Favorite already in list\n");
          }
          else if (res == -1)
          {
            // list full
            Com_Printf("Favorite list full\n");
          }
          else
          {
            // successfully added
            Com_Printf("Added favorite server %s\n", addr);
          }
        }
      }
    }
    else if (Q_stricmp(name, "orders") == 0)
    {
      const char *orders;
      if (String_Parse(args, &orders))
      {
        int selectedPlayer = (int)trap_Cvar_VariableValue("cg_selectedPlayer");
        if (selectedPlayer < uiInfo.myTeamCount)
        {
          qstrcpy(buff, orders);
          trap_Cmd_ExecuteText(EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]));
          trap_Cmd_ExecuteText(EXEC_APPEND, "\n");
        }
        else
        {
          int i;
          for (i = 0; i < uiInfo.myTeamCount; i++)
          {
            if (Q_stricmp(UI_Cvar_VariableString("name"), uiInfo.teamNames[i]) == 0)
            {
              continue;
            }
            qstrcpy(buff, orders);
            trap_Cmd_ExecuteText(EXEC_APPEND, va(buff, uiInfo.teamNames[i]));
            trap_Cmd_ExecuteText(EXEC_APPEND, "\n");
          }
        }
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
        trap_Key_ClearStates();
        trap_Cvar_Set("cl_paused", "0");
        Menus_CloseAll();
      }
    }
    else if (Q_stricmp(name, "voiceOrdersTeam") == 0)
    {
      const char *orders;
      if (String_Parse(args, &orders))
      {
        int selectedPlayer = (int)trap_Cvar_VariableValue("cg_selectedPlayer");
        if (selectedPlayer == uiInfo.myTeamCount)
        {
          trap_Cmd_ExecuteText(EXEC_APPEND, orders);
          trap_Cmd_ExecuteText(EXEC_APPEND, "\n");
        }
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
        trap_Key_ClearStates();
        trap_Cvar_Set("cl_paused", "0");
        Menus_CloseAll();
      }
    }
    else if (Q_stricmp(name, "voiceOrders") == 0)
    {
      const char *orders;
      if (String_Parse(args, &orders))
      {
        int selectedPlayer = (int)trap_Cvar_VariableValue("cg_selectedPlayer");
        if (selectedPlayer < uiInfo.myTeamCount)
        {
          qstrcpy(buff, orders);
          trap_Cmd_ExecuteText(EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]));
          trap_Cmd_ExecuteText(EXEC_APPEND, "\n");
        }
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
        trap_Key_ClearStates();
        trap_Cvar_Set("cl_paused", "0");
        Menus_CloseAll();
      }
    }
#ifdef COMPAT_KPQ3
    else if (Q_stricmp(name, "resetMenu") == 0)
    {
      if (UI_GFXSettingsChanged())
      {
        trap_Cvar_SetValue("uix_r_custQuality", 0); // add hypov8
        trap_Cvar_SetValue("uix_videoEdited", 1); //show apply button when gfx changed
      }
      else
      {
        trap_Cvar_SetValue("uix_videoEdited", 0); //show apply button when gfx changed
      }
    }
    else if (Q_stricmp(name, "custGFX") == 0)
    { //show apply button when gfx changed
      trap_Cvar_SetValue("uix_videoEdited", 1); // add hypov8
      //not used in menu
    }

  else if (Q_stricmp(name, "systemCvarsGet") == 0) //hypov8
  {
    UI_UpdateCvarList();
    trap_Cvar_SetValue("uix_r_custQuality", 0);
    UI_UpdateLightCvar();
  }
  else if (Q_stricmp(name, "systemCvarsReset") == 0)
  {
    UI_ResetTempCvarList();
  }
  else if (Q_stricmp(name, "systemCvarsApply") == 0)
  {
    int i;
    int uix_r_shadowMapQuality = (int)trap_Cvar_VariableValue("uix_r_shadowMapQuality");
    int uix_r_texturemode_menu = (int)trap_Cvar_VariableValue("uix_r_texturemode_menu");
    char uix_r_texturemode_st[128];

    UI_CVARGetVidModeString(uix_r_texturemode_st);

    trap_Cvar_Set("uix_videoEdited", "0"); //show apply button

    // make tempory values permanent
    trap_Cvar_Set(ui_custStrings[1][0], va("%s",      uix_r_texturemode_st)); //string
    trap_Cvar_Set(ui_custStrings[2][0], va("%i", (int)trap_Cvar_VariableValue(ui_custStrings[2][1]))); //r_mode //check this.
    trap_Cvar_Set(ui_custStrings[3][0], va("%.4f",    trap_Cvar_VariableValue(ui_custStrings[3][1]))); //float r_forceAmbient
    for (i = 4; i < UI_CVAR_STRCOUNT; i++) //>3
    {
      trap_Cvar_Set(ui_custStrings[i][0], va("%i", (int)trap_Cvar_VariableValue(ui_custStrings[i][1])));
    }
    //todo should this only be set relating to specific cvar changes?
    trap_Cvar_Set("r_recompileShaders",   va("%i", (int)trap_Cvar_VariableValue("uix_r_recompileShaders")));

    if (uix_r_shadowMapQuality <= 0 || uix_r_shadowMapQuality > 4)
      Com_Printf(S_COLOR_YELLOW"invalid r_shadowMapQuality value %i", uix_r_shadowMapQuality);
    else
    {
        trap_Cvar_SetValue("r_shadowMapSizeLow",      (1 << (uix_r_shadowMapQuality+3))); //start from 16
        trap_Cvar_SetValue("r_shadowMapSizeMedium",   (1 << (uix_r_shadowMapQuality+4)));
        trap_Cvar_SetValue("r_shadowMapSizeHigh",     (1 << (uix_r_shadowMapQuality+5)));
        trap_Cvar_SetValue("r_shadowMapSizeVeryHigh", (1 << (uix_r_shadowMapQuality+6)));
        trap_Cvar_SetValue("r_shadowMapSizeUltra",    (1 << (uix_r_shadowMapQuality+7)));
    }

    //reset tempory values
    UI_ResetTempCvarList();

  }
  //*******************************************
#endif //hypo new menu

    else if (Q_stricmp(name, "update") == 0)
    {
      if (String_Parse(args, &name2))
      {
        UI_Update(name2);
      }
    }
    else
    {
      Com_Printf(S_COLOR_YELLOW"unknown UI script %s\n", name);
    }
  }
  else
  {
    Com_DPrintf("UI_RunMenuScript() called without arguments \n");
  }
}

//add hypov8 used multiple times. get old cvar values
static void UI_UpdateCvarList(void)
{
  int i;
  char uix_r_texturemode_str[128];
  int uix_r_texturemode_menu = 3; //3 custom. 
  
  trap_Cvar_VariableStringBuffer("r_texturemode", uix_r_texturemode_str , 128);

  for (i = 0; i < 3; i++)
  {
    if (!qstrcmp(uix_r_texturemode_str, ui_vidGLModeNames[i]))
    {
      uix_r_texturemode_menu = i;
      break;
    }
  }

  trap_Cvar_Set("uix_videoEdited", "0"); //show apply button. when vid settings have changed

  trap_Cvar_Set(ui_custStrings[0][1], va("%i",      uix_r_texturemode_menu));                        //uix_r_texturemode_menu
  trap_Cvar_Set(ui_custStrings[1][1], va("%s",      uix_r_texturemode_str));                         //uix_r_texturemode_str
  trap_Cvar_Set(ui_custStrings[2][1], va("%i", (int)trap_Cvar_VariableValue(ui_custStrings[2][0]))); //r_mode
  trap_Cvar_Set(ui_custStrings[3][1], va("%.4f",    trap_Cvar_VariableValue(ui_custStrings[3][0]))); //r_forceAmbient" //float
  for (i = 4; i < UI_CVAR_STRCOUNT; i++) //>2
  {
    trap_Cvar_Set(ui_custStrings[i][1],   va("%i", (int)trap_Cvar_VariableValue(ui_custStrings[i][0])));
  }
  trap_Cvar_Set("uix_r_recompileShaders", va("%i", (int)trap_Cvar_VariableValue("r_recompileShaders")));

  //trap_Cvar_Set("uix_r_custQuality", 0);
}

static void UI_ResetTempCvarList(void)
{
  int i;
  for (i = 0; i < UI_CVAR_STRCOUNT; i++)
  {
    trap_Cvar_Set(ui_custStrings[i][1], "");
  }
  trap_Cvar_Set("uix_videoEdited", ""); //show ui 'apply' button
  trap_Cvar_Set("uix_r_recompileShaders", "");
  trap_Cvar_SetValue("uix_r_custQuality", 0); //int
}


static void UI_GetTeamColor(vec4_t *color)
{
}

/*
==================
UI_MapCountByGameType
==================
*/
static int UI_MapCountByGameType(qboolean singlePlayer)
{
  int i, c, game;

  c    = 0;
  game = uiInfo.gameTypes[ui_netGameType.integer].gtEnum;
#ifdef USE_GT_SINGLEPLAYER
  game = singlePlayer ? uiInfo.gameTypes[ui_gameType.integer].gtEnum : uiInfo.gameTypes[ui_netGameType.integer].gtEnum;
  if (game == GT_SINGLE_PLAYER)
  {
    game++;
  }
#endif
  if (game == GT_TEAM)
  {
    game = GT_FFA;
  }

  for (i = 0; i < uiInfo.mapCount; i++)
  {
    uiInfo.mapList[i].active = qfalse;
    if (uiInfo.mapList[i].typeBits & (1 << game))
    {
#ifdef USE_GT_SINGLEPLAYER
      if (singlePlayer)
      {
        if (!(uiInfo.mapList[i].typeBits & (1 << GT_SINGLE_PLAYER)))
          continue;
      }
#endif
      c++;
      uiInfo.mapList[i].active = qtrue;
    }
  }
  return c;
}

qboolean UI_hasSkinForBase(const char *base, const char *clan)
{
  char test[MAX_QPATH];

  //                               models/players/thug/clans/area/dragons.skin
  Com_sprintf(test, sizeof(test), "models/players/%s/clans/%s/%s.skin", base, clan, TEAM_SKIN_NIKKIS);

  if (trap_FS_FOpenFile(test, NULL, FS_READ))
  { 
    //check for both skins
    Com_sprintf(test, sizeof(test), "models/players/%s/clans/%s/%s.skin", base, clan, TEAM_SKIN_DRAGONS);
    if (trap_FS_FOpenFile(test, NULL, FS_READ))
      return qtrue;
  }

  return qfalse;
}

/*
==================
UI_MapCountByTeam
==================
*/
static int UI_HeadCountByTeam(void)
{
  static int init = 0;
  int i, j, k, c, tIndex;

  c = 0;
  if (!init)
  {
    for (i = 0; i < uiInfo.characterCount; i++)
    {
      uiInfo.characterList[i].reference = 0;
      for (j = 0; j < uiInfo.teamCount; j++)
      {
        if (UI_hasSkinForBase(uiInfo.characterList[i].base, uiInfo.teamList[j].teamName))
        {
          uiInfo.characterList[i].reference |= (1 << j);
        }
      }
    }
    init = 1;
  }

  tIndex = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));

  // do names
  for (i = 0; i < uiInfo.characterCount; i++)
  {
    uiInfo.characterList[i].active = qfalse;
    for (j = 0; j < TEAM_MEMBERS; j++)
    {
      if (uiInfo.teamList[tIndex].teamMembers[j] != NULL)
      {
        if (uiInfo.characterList[i].reference & (1 << tIndex)) // && Q_stricmp(uiInfo.teamList[tIndex].teamMembers[j], uiInfo.characterList[i].name)==0) {
        {
          uiInfo.characterList[i].active = qtrue;
          c++;
          break;
        }
      }
    }
  }

  // and then aliases
  for (j = 0; j < TEAM_MEMBERS; j++)
  {
    for (k = 0; k < uiInfo.aliasCount; k++)
    {
      if (uiInfo.aliasList[k].name != NULL)
      {
        if (Q_stricmp(uiInfo.teamList[tIndex].teamMembers[j], uiInfo.aliasList[k].name) == 0)
        {
          for (i = 0; i < uiInfo.characterCount; i++)
          {
            if (uiInfo.characterList[i].headImage != -1 && uiInfo.characterList[i].reference & (1 << tIndex) && Q_stricmp(uiInfo.aliasList[k].ai, uiInfo.characterList[i].name) == 0)
            {
              if (uiInfo.characterList[i].active == qfalse)
              {
                uiInfo.characterList[i].active = qtrue;
                c++;
              }
              break;
            }
          }
        }
      }
    }
  }
  return c;
}

/*
==================
UI_InsertServerIntoDisplayList
==================
*/
static void UI_InsertServerIntoDisplayList(int num, int position)
{
  int i;

  if (position < 0 || position > uiInfo.serverStatus.numDisplayServers)
    return;

  //
  uiInfo.serverStatus.numDisplayServers++;
  for (i = uiInfo.serverStatus.numDisplayServers; i > position; i--)
  {
    uiInfo.serverStatus.displayServers[i] = uiInfo.serverStatus.displayServers[i - 1];
  }
  uiInfo.serverStatus.displayServers[position] = num;
}

/*
==================
UI_RemoveServerFromDisplayList
==================
*/
static void UI_RemoveServerFromDisplayList(int num)
{
  int i, j;

  for (i = 0; i < uiInfo.serverStatus.numDisplayServers; i++)
  {
    if (uiInfo.serverStatus.displayServers[i] == num)
    {
      uiInfo.serverStatus.numDisplayServers--;
      for (j = i; j < uiInfo.serverStatus.numDisplayServers; j++)
      {
        uiInfo.serverStatus.displayServers[j] = uiInfo.serverStatus.displayServers[j + 1];
      }
      return;
    }
  }
}

/*
==================
UI_BinaryServerInsertion
==================
*/
static void UI_BinaryServerInsertion(int num)
{
  int mid, offset, res, len;

  // use binary search to insert server
  len    = uiInfo.serverStatus.numDisplayServers;
  mid    = len;
  offset = 0;
  res    = 0;
  while (mid > 0)
  {
    mid = len >> 1;
    //
    res = trap_LAN_CompareServers(ui_netSource.integer, uiInfo.serverStatus.sortKey,
                                  uiInfo.serverStatus.sortDir, num, uiInfo.serverStatus.displayServers[offset + mid]);
    // if equal
    if (res == 0)
    {
      UI_InsertServerIntoDisplayList(num, offset + mid);
      return;
    }
    // if larger
    else if (res == 1)
    {
      offset += mid;
      len    -= mid;
    }
    // if smaller
    else
    {
      len -= mid;
    }
  }
  if (res == 1)
  {
    offset++;
  }
  UI_InsertServerIntoDisplayList(num, offset);
}

/*
==================
UI_BuildServerDisplayList
==================
*/
static void UI_BuildServerDisplayList(qboolean force)
{
  int i, count, clients, maxClients, ping, game, visible, weapMode;
  size_t len;
  char info[MAX_STRING_CHARS];
//	qboolean startRefresh = qtrue; TTimo: unused
  static int numinvisible;

  if (!(force || uiInfo.uiDC.realTime > uiInfo.serverStatus.nextDisplayRefresh))
    return;
  // if we shouldn't reset
  if (force == 2)
    force = 0;

  // do motd updates here too
  trap_Cvar_VariableStringBuffer("cl_motdString", uiInfo.serverStatus.motd, sizeof(uiInfo.serverStatus.motd));
  len = qstrlen(uiInfo.serverStatus.motd);
  if (len == 0)
  {
    qstrcpy(uiInfo.serverStatus.motd, "Welcome to " PRODUCT_NAME " !");
    len = qstrlen(uiInfo.serverStatus.motd);
  }
  if (len != (size_t)uiInfo.serverStatus.motdLen)
  {
    uiInfo.serverStatus.motdLen   = (int)len;
    uiInfo.serverStatus.motdWidth = -1;
  }

  if (force)
  {
    numinvisible = 0;
    // clear number of displayed servers
    uiInfo.serverStatus.numDisplayServers   = 0;
    uiInfo.serverStatus.numPlayersOnServers = 0;
    // set list box index to zero
    Menu_SetFeederSelection(NULL, FEEDER_SERVERS, 0, NULL);
    // mark all servers as visible so we store ping updates for them
    trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
  }

  // get the server count (comes from the master)
  count = trap_LAN_GetServerCount(ui_netSource.integer);
  if (count == -1 || (ui_netSource.integer == AS_LOCAL && count == 0))
  {
    // still waiting on a response from the master
    uiInfo.serverStatus.numDisplayServers   = 0;
    uiInfo.serverStatus.numPlayersOnServers = 0;
    uiInfo.serverStatus.nextDisplayRefresh  = uiInfo.uiDC.realTime + 500;
    return;
  }

  visible = qfalse;
  for (i = 0; i < count; i++)
  {
    // if we already got info for this server
    if (!trap_LAN_ServerIsVisible(ui_netSource.integer, i))
      continue;

    visible = qtrue;
    // get the ping for this server
    ping = trap_LAN_GetServerPing(ui_netSource.integer, i);
    if (ping > 0 || ui_netSource.integer == AS_FAVORITES)
    {
      //cvar's not updated externaly.
      int ui_showEmpty = uiInfo.uiDC.getCVarValue("ui_browserShowEmpty");
      int ui_showFull = uiInfo.uiDC.getCVarValue("ui_browserShowEmpty");

      trap_LAN_GetServerInfo(ui_netSource.integer, i, info, MAX_STRING_CHARS);

      clients                                  = atoi(Info_ValueForKey(info, "clients"));
      uiInfo.serverStatus.numPlayersOnServers += clients;

      if (ui_showEmpty == 0 /*ui_browserShowEmpty.integer == 0*/)
      {
        if (clients == 0)
        {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      if (ui_showFull == 0 /*ui_browserShowFull.integer == 0*/)
      {
        maxClients = atoi(Info_ValueForKey(info, "sv_maxclients"));
        if (maxClients != 0 && clients == maxClients) // kpq3 can set this to zero
        {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      if (uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum != -1)
      {
        game = atoi(Info_ValueForKey(info, "gametype"));
        if (game != uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum)
        {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }

      if (uiInfo.weaponModes[ui_joinWeaponMode.integer].gtEnum != -1)
      {
        weapMode = atoi(Info_ValueForKey(info, "weaponmode"));
        if (weapMode != uiInfo.weaponModes[ui_joinWeaponMode.integer].gtEnum)
        {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }

      }

      if (ui_serverFilterType.integer > 0) // FIXME: 0xA5EA, ueberfluessig
      {
        if (Q_stricmp(Info_ValueForKey(info, "game"), serverFilters[ui_serverFilterType.integer].basedir) != 0)
        {
          trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
          continue;
        }
      }
      // make sure we never add a favorite server twice
      if (ui_netSource.integer == AS_FAVORITES)
      {
        UI_RemoveServerFromDisplayList(i);
      }
      // insert the server into the list
      UI_BinaryServerInsertion(i);
      // done with this server
      if (ping > 0)
      {
        trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
        numinvisible++;
      }
    }
  }

  uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime;

  // if there were no servers visible for ping updates
  if (!visible)
  {
//		UI_StopServerRefresh();
//		uiInfo.serverStatus.nextDisplayRefresh = 0;
  }
}

typedef struct
{
  char *name, *altName;
} serverStatusCvar_t;

serverStatusCvar_t serverStatusCvars[] = {
  {"sv_hostname", "Name"},
  {"Address", ""},
  {"gamename", "Game name"}, //hypov8 "com_gamename" ? //basekpq3 (mod)
  {"g_gametype", "Game type"},
  {"mapname", "Map"},
  {"version", ""},
  {"protocol", ""},
  {"g_timelimit", ""}, //hypov8 "timelimit"
  {"g_fraglimit", ""}, //hypov8 "fraglimit"
  {NULL, NULL}
};

/*
==================
UI_SortServerStatusInfo
==================
*/
static void UI_SortServerStatusInfo(serverStatusInfo_t *info)
{
  int i, j, index;
  char *tmp1, *tmp2;

  // FIXME: if "gamename" == "baseq3" or "missionpack" then
  // replace the gametype number by FFA, CTF etc.
  //
  index = 0;
  for (i = 0; serverStatusCvars[i].name; i++)
  {
    for (j = 0; j < info->numLines; j++)
    {
      if (!info->lines[j][1] || info->lines[j][1][0])
        continue;

      if (!Q_stricmp(serverStatusCvars[i].name, info->lines[j][0]))
      {
        // swap lines
        tmp1                  = info->lines[index][0];
        tmp2                  = info->lines[index][3];
        info->lines[index][0] = info->lines[j][0];
        info->lines[index][3] = info->lines[j][3];
        info->lines[j][0]     = tmp1;
        info->lines[j][3]     = tmp2;
        //
        if (qstrlen(serverStatusCvars[i].altName))
        {
          info->lines[index][0] = serverStatusCvars[i].altName;
        }
        index++;
      }
    }
  }
}

/*
==================
UI_GetServerStatusInfo
==================
*/
static int UI_GetServerStatusInfo(const char *serverAddress, serverStatusInfo_t *info)
{
  char *p, *score, *ping, *name;
  int i;
  size_t len;

  if (!info)
  {
    trap_LAN_ServerStatus(serverAddress, NULL, 0);
    return qfalse;
  }
  Com_Memset(info, 0, sizeof(*info));
  if (trap_LAN_ServerStatus(serverAddress, info->text, sizeof(info->text)))
  {
    Q_strncpyz(info->address, serverAddress, sizeof(info->address));
    p                              = info->text;
    info->numLines                 = 0;
    info->lines[info->numLines][0] = "Address";
    info->lines[info->numLines][1] = "";
    info->lines[info->numLines][2] = "";
    info->lines[info->numLines][3] = info->address;
    info->numLines++;
    // get the cvars
    while (p && *p)
    {
      p = strchr(p, '\\');
      if (!p) break;
      *p++ = '\0';
      if (*p == '\\')
        break;
      info->lines[info->numLines][0] = p;
      info->lines[info->numLines][1] = "";
      info->lines[info->numLines][2] = "";
      p                              = strchr(p, '\\');
      if (!p) break;
      *p++                           = '\0';
      info->lines[info->numLines][3] = p;

      info->numLines++;
      if (info->numLines >= MAX_SERVERSTATUS_LINES)
        break;
    }
    // get the player list
    if (info->numLines < MAX_SERVERSTATUS_LINES - 3)
    {
      // empty line
      info->lines[info->numLines][0] = "";
      info->lines[info->numLines][1] = "";
      info->lines[info->numLines][2] = "";
      info->lines[info->numLines][3] = "";
      info->numLines++;
      // header
      info->lines[info->numLines][0] = "num";
      info->lines[info->numLines][1] = "score";
      info->lines[info->numLines][2] = "ping";
      info->lines[info->numLines][3] = "name";
      info->numLines++;
      // parse players
      i   = 0;
      len = 0;
      while (p && *p)
      {
        if (*p == '\\')
          *p++ = '\0';
        if (!p)
          break;
        score = p;
        p     = strchr(p, ' ');
        if (!p)
          break;
        *p++ = '\0';
        ping = p;
        p    = strchr(p, ' ');
        if (!p)
          break;
        *p++ = '\0';
        name = p;
        Com_sprintf(&info->pings[len], sizeof(info->pings) - len, "%d", i);
        info->lines[info->numLines][0] = &info->pings[len];
        len                           += qstrlen(&info->pings[len]) + 1;
        info->lines[info->numLines][1] = score;
        info->lines[info->numLines][2] = ping;
        info->lines[info->numLines][3] = name;
        info->numLines++;
        if (info->numLines >= MAX_SERVERSTATUS_LINES)
          break;
        p = strchr(p, '\\');
        if (!p)
          break;
        *p++ = '\0';
        //
        i++;
      }
    }
    UI_SortServerStatusInfo(info);
    return qtrue;
  }
  return qfalse;
}

/*
==================
stristr
==================
*/
static char *stristr(char *str, char *charset)
{
  int i;

  while (*str)
  {
    for (i = 0; charset[i] && str[i]; i++)
    {
      if (toupper(charset[i]) != toupper(str[i]))
        break;
    }
    if (!charset[i]) return str;
    str++;
  }
  return NULL;
}

/*
==================
UI_BuildFindPlayerList
==================
*/
static void UI_BuildFindPlayerList(qboolean force)
{
  static int numFound, numTimeOuts;
  int i, j, resend;
  serverStatusInfo_t info;
  char name[MAX_NAME_LENGTH + 2];
  char infoString[MAX_STRING_CHARS];

  if (!force)
  {
    if (!uiInfo.nextFindPlayerRefresh || uiInfo.nextFindPlayerRefresh > uiInfo.uiDC.realTime)
      return;
  }
  else
  {
    Com_Memset(&uiInfo.pendingServerStatus, 0, sizeof(uiInfo.pendingServerStatus));
    uiInfo.numFoundPlayerServers    = 0;
    uiInfo.currentFoundPlayerServer = 0;
    trap_Cvar_VariableStringBuffer("ui_findPlayer", uiInfo.findPlayerName, sizeof(uiInfo.findPlayerName));
    Q_CleanStr(uiInfo.findPlayerName);
    // should have a string of some length
    if (!qstrlen(uiInfo.findPlayerName))
    {
      uiInfo.nextFindPlayerRefresh = 0;
      return;
    }
    // set resend time
    resend = ui_serverStatusTimeOut.integer / 2 - 10;
    if (resend < 50)
    {
      resend = 50;
    }
    trap_Cvar_Set("cl_serverStatusResendTime", va("%d", resend));
    // reset all server status requests
    trap_LAN_ServerStatus(NULL, NULL, 0);
    //
    uiInfo.numFoundPlayerServers = 1;
    Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1]),
                "searching %d...", uiInfo.pendingServerStatus.num);
    numFound = 0;
    numTimeOuts++;
  }
  for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++)
  {
    // if this pending server is valid
    if (uiInfo.pendingServerStatus.server[i].valid)
    {
      // try to get the server status for this server
      if (UI_GetServerStatusInfo(uiInfo.pendingServerStatus.server[i].adrstr, &info))
      {
        //
        numFound++;
        // parse through the server status lines
        for (j = 0; j < info.numLines; j++)
        {
          // should have ping info
          if (!info.lines[j][2] || !info.lines[j][2][0])
          {
            continue;
          }
          // clean string first
          Q_strncpyz(name, info.lines[j][3], sizeof(name));
          Q_CleanStr(name);
          // if the player name is a substring
          if (stristr(name, uiInfo.findPlayerName))
          {
            // add to found server list if we have space (always leave space for a line with the number found)
            if (uiInfo.numFoundPlayerServers < MAX_FOUNDPLAYER_SERVERS - 1)
            {
              //
              Q_strncpyz(uiInfo.foundPlayerServerAddresses[uiInfo.numFoundPlayerServers - 1],
                         uiInfo.pendingServerStatus.server[i].adrstr,
                         sizeof(uiInfo.foundPlayerServerAddresses[0]));
              Q_strncpyz(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                         uiInfo.pendingServerStatus.server[i].name,
                         sizeof(uiInfo.foundPlayerServerNames[0]));
              uiInfo.numFoundPlayerServers++;
            }
            else
            {
              // can't add any more so we're done
              uiInfo.pendingServerStatus.num = uiInfo.serverStatus.numDisplayServers;
            }
          }
        }
        Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                    sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1]),
                    "searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
        // retrieved the server status so reuse this spot
        uiInfo.pendingServerStatus.server[i].valid = qfalse;
      }
    }
    // if empty pending slot or timed out
    if (!uiInfo.pendingServerStatus.server[i].valid ||
        uiInfo.pendingServerStatus.server[i].startTime < uiInfo.uiDC.realTime - ui_serverStatusTimeOut.integer)
    {
      if (uiInfo.pendingServerStatus.server[i].valid)
      {
        numTimeOuts++;
      }
      // reset server status request for this address
      UI_GetServerStatusInfo(uiInfo.pendingServerStatus.server[i].adrstr, NULL);
      // reuse pending slot
      uiInfo.pendingServerStatus.server[i].valid = qfalse;
      // if we didn't try to get the status of all servers in the main browser yet
      if (uiInfo.pendingServerStatus.num < uiInfo.serverStatus.numDisplayServers)
      {
        uiInfo.pendingServerStatus.server[i].startTime = uiInfo.uiDC.realTime;
        trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
                                        uiInfo.pendingServerStatus.server[i].adrstr, sizeof(uiInfo.pendingServerStatus.server[i].adrstr));
        trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num], infoString, sizeof(infoString));
        Q_strncpyz(uiInfo.pendingServerStatus.server[i].name, Info_ValueForKey(infoString, "hostname"), sizeof(uiInfo.pendingServerStatus.server[0].name));
        uiInfo.pendingServerStatus.server[i].valid = qtrue;
        uiInfo.pendingServerStatus.num++;
        Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                    sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1]),
                    "searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
      }
    }
  }
  for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++)
  {
    if (uiInfo.pendingServerStatus.server[i].valid)
    {
      break;
    }
  }
  // if still trying to retrieve server status info
  if (i < MAX_SERVERSTATUSREQUESTS)
  {
    uiInfo.nextFindPlayerRefresh = uiInfo.uiDC.realTime + 25;
  }
  else
  {
    // add a line that shows the number of servers found
    if (!uiInfo.numFoundPlayerServers)
    {
      Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1], sizeof(uiInfo.foundPlayerServerAddresses[0]), "no servers found");
    }
    else
    {
      Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1], sizeof(uiInfo.foundPlayerServerAddresses[0]),
                  "%d server%s found with player %s", uiInfo.numFoundPlayerServers - 1,
                  uiInfo.numFoundPlayerServers == 2 ? "" : "s", uiInfo.findPlayerName);
    }
    uiInfo.nextFindPlayerRefresh = 0;
    // show the server status info for the selected server
    UI_FeederSelection(FEEDER_FINDPLAYER, uiInfo.currentFoundPlayerServer);
  }
}

/*
==================
UI_BuildServerStatus
==================
*/
static void UI_BuildServerStatus(qboolean force)
{
  if (uiInfo.nextFindPlayerRefresh)
    return;

  if (!force)
  {
    if (!uiInfo.nextServerStatusRefresh || uiInfo.nextServerStatusRefresh > uiInfo.uiDC.realTime)
      return;
  }
  else
  {
    Menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
    uiInfo.serverStatusInfo.numLines = 0;
    // reset all server status requests
    trap_LAN_ServerStatus(NULL, NULL, 0);
  }
  if (uiInfo.serverStatus.currentServer < 0 || uiInfo.serverStatus.currentServer > uiInfo.serverStatus.numDisplayServers || uiInfo.serverStatus.numDisplayServers == 0)
    return;
  if (UI_GetServerStatusInfo(uiInfo.serverStatusAddress, &uiInfo.serverStatusInfo))
  {
    uiInfo.nextServerStatusRefresh = 0;
    UI_GetServerStatusInfo(uiInfo.serverStatusAddress, NULL);
  }
  else
  {
    uiInfo.nextServerStatusRefresh = uiInfo.uiDC.realTime + 500;
  }
}

/*
==================
UI_FeederCount
==================
*/
static int UI_FeederCount(float feederID)
{
  if (feederID == FEEDER_HEADS)
    return UI_HeadCountByTeam();
  else if (feederID == FEEDER_Q3HEADS)
    return uiInfo.q3HeadCount;
  else if (feederID == FEEDER_CINEMATICS)
    return uiInfo.movieCount;
  else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS)
    return UI_MapCountByGameType(feederID == FEEDER_MAPS ? qtrue : qfalse);
  else if (feederID == FEEDER_SERVERS)
    return uiInfo.serverStatus.numDisplayServers;
  else if (feederID == FEEDER_SERVERSTATUS)
    return uiInfo.serverStatusInfo.numLines;
  else if (feederID == FEEDER_FINDPLAYER)
    return uiInfo.numFoundPlayerServers;
  else if (feederID == FEEDER_PLAYER_LIST)
  {
    if (uiInfo.uiDC.realTime > uiInfo.playerRefresh)
    {
      uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
      UI_BuildPlayerList();
    }
    return uiInfo.playerCount;
  }
  else if (feederID == FEEDER_TEAM_LIST)
  {
    if (uiInfo.uiDC.realTime > uiInfo.playerRefresh)
    {
      uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
      UI_BuildPlayerList();
    }
    return uiInfo.myTeamCount;
  }
  else if (feederID == FEEDER_MODS)
    return uiInfo.modCount;
  else if (feederID == FEEDER_DEMOS)
    return uiInfo.demoCount;

  return 0;
}

static const char *UI_SelectedMap(int index, int *actual)
{
  int i, c;
  c = 0;
  *actual = 0;
  for (i = 0; i < uiInfo.mapCount; i++)
  {
    if (uiInfo.mapList[i].active)
    {
      if (c == index)
      {
        *actual = i;
        return uiInfo.mapList[i].mapName;
      }
      else
        c++;
    }
  }
  return "";
}

static const char *UI_SelectedHead(int index, int *actual)
{
  int i, c;
  c = 0;
  *actual = 0;
  for (i = 0; i < uiInfo.characterCount; i++)
  {
    if (uiInfo.characterList[i].active)
    {
      if (c == index)
      {
        *actual = i;
        return uiInfo.characterList[i].name;
      }
      else
        c++;
    }
  }
  return "";
}
#ifdef USE_GT_SINGLEPLAYER
static int UI_GetIndexFromSelection(int actual)
{
  int i, c;

  c = 0;
  for (i = 0; i < uiInfo.mapCount; i++)
  {
    if (uiInfo.mapList[i].active)
    {
      if (i == actual)
        return c;
      c++;
    }
  }
  return 0;
}
#endif

static void UI_UpdatePendingPings(void)
{
  trap_LAN_ResetPings(ui_netSource.integer);
  uiInfo.serverStatus.refreshActive = qtrue;
  uiInfo.serverStatus.refreshtime   = uiInfo.uiDC.realTime + 1000;

}

static const char *UI_FeederItemText(float feederID, int index, int column, qhandle_t *handle)
{
  static char info[MAX_STRING_CHARS];
  static char hostname[1024];
  static char clientBuff[32];
  static int lastColumn = -1;
  static int lastTime   = 0;

  *handle = -1;
  if (feederID == FEEDER_HEADS)
  {
    int actual;
    return UI_SelectedHead(index, &actual);
  }
  else if (feederID == FEEDER_Q3HEADS)
  {
    if (index >= 0 && index < uiInfo.q3HeadCount)
      return uiInfo.q3HeadNames[index];
  }
  else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS)
  {
    int actual;
    return UI_SelectedMap(index, &actual);
  }
  else if (feederID == FEEDER_SERVERS)
  {
    if (index >= 0 && index < uiInfo.serverStatus.numDisplayServers)
    {
      int ping, game, weaponmode; //punkbuster;
      if (lastColumn != column || lastTime > uiInfo.uiDC.realTime + 5000)
      {
        trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
        lastColumn = column;
        lastTime   = uiInfo.uiDC.realTime;
      }
      ping = atoi(Info_ValueForKey(info, "ping"));
      if (ping == -1)
      {
        // if we ever see a ping that is out of date, do a server refresh
        // UI_UpdatePendingPings();
      }
      switch (column)
      {
      case SORT_HOST:
        if (ping <= 0)
          return Info_ValueForKey(info, "addr");
        else
        {
          if (ui_netSource.integer == AS_LOCAL)
          {
            int nettype = atoi(Info_ValueForKey(info, "nettype"));

            if (nettype < 0 || nettype >= (int)ARRAY_LEN(netnames))
              nettype = 0;

            Com_sprintf(hostname, sizeof(hostname), "%s [%s]",
                        Info_ValueForKey(info, "hostname"),
                        netnames[nettype]);
            return hostname;
          }
          else
          {
            Com_sprintf(hostname, sizeof(hostname), "%s", Info_ValueForKey(info, "hostname"));
            return hostname;
          }
        }
      case SORT_MAP:
        return Info_ValueForKey(info, "mapname");

      case SORT_CLIENTS:
        Com_sprintf(clientBuff, sizeof(clientBuff), "%s (%s)", Info_ValueForKey(info, "clients"), Info_ValueForKey(info, "sv_maxclients"));
        return clientBuff;

      case SORT_GAME:
        game = atoi(Info_ValueForKey(info, "gametype"));
        if (game >= 0 && game < numTeamArenaGameTypes)
          return teamArenaGameTypes[game];
        else
          return "Unknown";
      case SORT_PING:
        if (ping <= 0)
          return "...";
        else
          return Info_ValueForKey(info, "ping");

      case SORT_PUNKBUSTER:
        //punkbuster = atoi(Info_ValueForKey(info, "punkbuster"));
        weaponmode = atoi(Info_ValueForKey(info, "weaponmode"));
        if (weaponmode) //FIXME: 0xA5EA !!
          return "Yes";
        else
          return "No";
      }
    }
  }
  else if (feederID == FEEDER_SERVERSTATUS)
  {
    if (index >= 0 && index < uiInfo.serverStatusInfo.numLines)
    {
      if (column >= 0 && column < 4)
        return uiInfo.serverStatusInfo.lines[index][column];
    }
  }
  else if (feederID == FEEDER_FINDPLAYER)
  {
    if (index >= 0 && index < uiInfo.numFoundPlayerServers)
    {
      //return uiInfo.foundPlayerServerAddresses[index];
      return uiInfo.foundPlayerServerNames[index];
    }
  }
  else if (feederID == FEEDER_PLAYER_LIST)
  {
    if (index >= 0 && index < uiInfo.playerCount)
      return uiInfo.playerNames[index];
  }
  else if (feederID == FEEDER_TEAM_LIST)
  {
    if (index >= 0 && index < uiInfo.myTeamCount)
      return uiInfo.teamNames[index];
  }
  else if (feederID == FEEDER_MODS)
  {
    if (index >= 0 && index < uiInfo.modCount)
    {
      if (uiInfo.modList[index].modDescr && *uiInfo.modList[index].modDescr)
        return uiInfo.modList[index].modDescr;
      else
        return uiInfo.modList[index].modName;
    }
  }
  else if (feederID == FEEDER_CINEMATICS)
  {
    if (index >= 0 && index < uiInfo.movieCount)
      return uiInfo.movieList[index];
  }
  else if (feederID == FEEDER_DEMOS)
  {
    if (index >= 0 && index < uiInfo.demoCount)
      return uiInfo.demoList[index];
  }
  return "";
}


static qhandle_t UI_FeederItemImage(float feederID, int index)
{
  if (feederID == FEEDER_HEADS)
  {
    int actual;
    UI_SelectedHead(index, &actual);
    index = actual;
    if (index >= 0 && index < uiInfo.characterCount)
    {
      if (uiInfo.characterList[index].headImage == -1)
      {
        uiInfo.characterList[index].headImage = trap_R_RegisterShaderNoMip(uiInfo.characterList[index].imageName);
      }
      return uiInfo.characterList[index].headImage;
    }
  }
  else if (feederID == FEEDER_Q3HEADS)
  {
    if (index >= 0 && index < uiInfo.q3HeadCount)
    {
      return uiInfo.q3HeadIcons[index];
    }
  }
  else if (feederID == FEEDER_ALLMAPS || feederID == FEEDER_MAPS)
  {
    int actual;
    UI_SelectedMap(index, &actual);
    index = actual;
    if (index >= 0 && index < uiInfo.mapCount)
    {
      if (uiInfo.mapList[index].levelShot == -1)
      {
        uiInfo.mapList[index].levelShot = trap_R_RegisterShaderNoMip(uiInfo.mapList[index].imageName);
      }
      return uiInfo.mapList[index].levelShot;
    }
  }
  return 0;
}

static void UI_FeederSelection(float feederID, int index)
{
  static char info[MAX_STRING_CHARS];

  if (feederID == FEEDER_HEADS) //TEAM models
  {
    int actual;

    UI_SelectedHead(index, &actual);
    index = actual;
    if (index >= 0 && index < uiInfo.characterCount)
    {
      //update client info when changing cln
      if (!Q_stricmp("1", UI_Cvar_VariableString("ui_team")))
        trap_Cvar_Set("g_dragonTeam", va("%s", UI_Cvar_VariableString("ui_teamName")));
      else
        trap_Cvar_Set("g_nikkiTeam", va("%s", UI_Cvar_VariableString("ui_teamName")));

      trap_Cvar_Set("team_model", va("%s", uiInfo.characterList[index].base));
      trap_Cvar_Set("team_headmodel", va("%s", uiInfo.characterList[index].name)); //hypov8 was "*%s"
      updateModel = qtrue;
    }
  }
  else if (feederID == FEEDER_Q3HEADS) //DM models
  {
    if (index >= 0 && index < uiInfo.q3HeadCount)
    {
      trap_Cvar_Set("model", uiInfo.q3HeadNames[index]);
      trap_Cvar_Set("headmodel", uiInfo.q3HeadNames[index]);
      updateModel = qtrue;
    }
  }
  else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS)
  {
    int actual, map;
    map = (feederID == FEEDER_ALLMAPS) ? ui_currentNetMap.integer : ui_currentMap.integer;
    if (uiInfo.mapList[map].cinematic >= 0)
    {
      trap_CIN_StopCinematic(uiInfo.mapList[map].cinematic);
      uiInfo.mapList[map].cinematic = -1;
    }
    UI_SelectedMap(index, &actual);
    trap_Cvar_Set("ui_mapIndex", va("%d", index));
    ui_mapIndex.integer = index;

    if (feederID == FEEDER_MAPS)
    {
      ui_currentMap.integer = actual;
      trap_Cvar_Set("ui_currentMap", va("%d", actual));
      uiInfo.mapList[ui_currentMap.integer].cinematic = trap_CIN_PlayCinematic(va("%s.ogv", uiInfo.mapList[ui_currentMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent));
      UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
      trap_Cvar_Set("ui_opponentModel", uiInfo.mapList[ui_currentMap.integer].opponentName);
      updateOpponentModel = qtrue;
    }
    else
    {
      ui_currentNetMap.integer = actual;
      trap_Cvar_Set("ui_currentNetMap", va("%d", actual));
      uiInfo.mapList[ui_currentNetMap.integer].cinematic = trap_CIN_PlayCinematic(va("%s.ogv", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent));
    }

  }
  else if (feederID == FEEDER_SERVERS)
  {
    const char *mapName = NULL;
    uiInfo.serverStatus.currentServer = index;
    trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
    uiInfo.serverStatus.currentServerPreview = trap_R_RegisterShaderNoMip(va("levelshots/%s", Info_ValueForKey(info, "mapname")));
    if (uiInfo.serverStatus.currentServerCinematic >= 0)
    {
      trap_CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
      uiInfo.serverStatus.currentServerCinematic = -1;
    }
    mapName = Info_ValueForKey(info, "mapname"); //hypov8 server video PlayCinematic .ogv
    if (mapName && *mapName)
    {
    //if (file) //hypov8 todo: if trap_CIN_PlayCinematic missing..
    //not all maps will have a video
      uiInfo.serverStatus.currentServerCinematic = trap_CIN_PlayCinematic(va("%s.ogv", mapName), 0, 0, 0, 0, (CIN_loop | CIN_silent));
    }
  }
  else if (feederID == FEEDER_SERVERSTATUS)
  {
    //
  }
  else if (feederID == FEEDER_FINDPLAYER)
  {
    uiInfo.currentFoundPlayerServer = index;
    //
    if (index < uiInfo.numFoundPlayerServers - 1)
    {
      // build a new server status for this server
      Q_strncpyz(uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof(uiInfo.serverStatusAddress));
      Menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
      UI_BuildServerStatus(qtrue);
    }
  }
  else if (feederID == FEEDER_PLAYER_LIST)
  {
    uiInfo.playerIndex = index;
  }
  else if (feederID == FEEDER_TEAM_LIST)
  {
    uiInfo.teamIndex = index;
  }
  else if (feederID == FEEDER_MODS)
  {
    uiInfo.modIndex = index;
  }
  else if (feederID == FEEDER_CINEMATICS)
  {
    uiInfo.movieIndex = index;
    if (uiInfo.previewMovie >= 0)
    {
      trap_CIN_StopCinematic(uiInfo.previewMovie);
    }
    uiInfo.previewMovie = -1;
  }
  else if (feederID == FEEDER_DEMOS)
  {
    uiInfo.demoIndex = index;
  }
}

static qboolean Team_Parse(char **p)
{
  char *token;
  const char *tempStr;
  int i;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{')
    return qfalse;

  while (1)
  {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0)
      return qtrue;

    if (!token || token[0] == 0)
      return qfalse;

    if (token[0] == '{')
    {
      // seven tokens per line, team name and icon, and 5 team member names
      if (!String_Parse(p, &uiInfo.teamList[uiInfo.teamCount].teamName) || !String_Parse(p, &tempStr))
        return qfalse;

      uiInfo.teamList[uiInfo.teamCount].imageName      = tempStr;
      uiInfo.teamList[uiInfo.teamCount].teamIcon       = trap_R_RegisterShaderNoMip(uiInfo.teamList[uiInfo.teamCount].imageName);
      uiInfo.teamList[uiInfo.teamCount].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal", uiInfo.teamList[uiInfo.teamCount].imageName)); //hypov8 todo: images?
      uiInfo.teamList[uiInfo.teamCount].teamIcon_Name  = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[uiInfo.teamCount].imageName));

      uiInfo.teamList[uiInfo.teamCount].cinematic = -1;

      for (i = 0; i < TEAM_MEMBERS; i++)
      {
        uiInfo.teamList[uiInfo.teamCount].teamMembers[i] = NULL;
        if (!String_Parse(p, &uiInfo.teamList[uiInfo.teamCount].teamMembers[i]))
          return qfalse;
      }

      Com_Printf("Loaded team %s with team icon %s.\n", uiInfo.teamList[uiInfo.teamCount].teamName, tempStr);
      if (uiInfo.teamCount < MAX_TEAMS)
        uiInfo.teamCount++;
      else
        Com_Printf("Too many teams, last team replaced!\n");

      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}')
        return qfalse;
    }
  }
  return qfalse;
}

static qboolean Character_Parse(char **p)
{
  char *token;
  const char *tempStr;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{')
    return qfalse;

  while (1)
  {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0)
      return qtrue;

    if (!token || token[0] == 0)
      return qfalse;

    if (token[0] == '{')
    {
      // two tokens per line, character <name> and <sex>
      if (!String_Parse(p, &uiInfo.characterList[uiInfo.characterCount].name) || !String_Parse(p, &tempStr))
        return qfalse;

      if (tempStr && (!Q_stricmp(tempStr, "female")))
        uiInfo.characterList[uiInfo.characterCount].base = String_Alloc(va("shina"));        // 0xA5EA FIXME:
      else if (tempStr && (!Q_stricmp(tempStr, "male")))
        uiInfo.characterList[uiInfo.characterCount].base = String_Alloc(va("thug"));         // 0xA5EA
      else
        uiInfo.characterList[uiInfo.characterCount].base = String_Alloc(va("%s", tempStr));

      uiInfo.characterList[uiInfo.characterCount].headImage = -1;//

      uiInfo.characterList[uiInfo.characterCount].imageName =
        String_Alloc(va("models/players/%s/icon_default.tga", //"models/players/heads/%s/icon_default.tga"
        uiInfo.characterList[uiInfo.characterCount].base));

      Com_Printf("Loaded %s character %s.\n",
        uiInfo.characterList[uiInfo.characterCount].base,
        uiInfo.characterList[uiInfo.characterCount].name);
#ifdef HYPODEBUG
      Com_Printf("Loaded %s icon %s\n",
        uiInfo.characterList[uiInfo.characterCount].base,
        uiInfo.characterList[uiInfo.characterCount].imageName);
#endif
      if (uiInfo.characterCount < MAX_HEADS)
        uiInfo.characterCount++;
      else
        Com_Printf("Too many characters, last character replaced!\n");

      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}')
        return qfalse;
    }
  }
  return qfalse;
}


static qboolean Alias_Parse(char **p)
{
  char *token;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{')
    return qfalse;

  while (1)
  {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0)
      return qtrue;

    if (!token || token[0] == 0)
      return qfalse;

    if (token[0] == '{')
    {
      // three tokens per line, character name, bot alias, and preferred action a - all purpose, d - defense, o - offense
      if (!String_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].name) ||
      !String_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].ai) ||
      !String_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].action))
        return qfalse;

      Com_Printf("Loaded character alias %s using character ai %s.\n", uiInfo.aliasList[uiInfo.aliasCount].name, uiInfo.aliasList[uiInfo.aliasCount].ai);
      if (uiInfo.aliasCount < MAX_ALIASES)
        uiInfo.aliasCount++;
      else
        Com_Printf("Too many aliases, last alias replaced!\n");

      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}')
        return qfalse;
    }
  }

  return qfalse;
}



// mode
// 0 - high level parsing
// 1 - team parsing
// 2 - character parsing
static void UI_ParseTeamInfo(const char *teamFile)
{
  char *token;
  char *p;
  char *buff = NULL;

  //static int mode = 0; TTimo: unused

  buff = GetMenuBuffer(teamFile);
  if (!buff)
    return;

  p = buff;

  while (1)
  {
    token = COM_ParseExt(&p, qtrue);
    if (!token || token[0] == 0 || token[0] == '}')
      break;

    if (Q_stricmp(token, "}") == 0)
      break;

    if (Q_stricmp(token, "teams") == 0)
    {
      if (Team_Parse(&p))
        continue;
      else
        break;
    }

    if (Q_stricmp(token, "characters") == 0)
      Character_Parse(&p);

    if (Q_stricmp(token, "aliases") == 0)
      Alias_Parse(&p);
  }
}

static qboolean WeaponMode_Parse(char **p)
{
  char *token;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{')
    return qfalse;

  uiInfo.numWeaponModes = 0;

  while (1)
  {
    token = COM_ParseExt(p, qtrue);

    if (!Q_stricmp(token, "}"))
      return qtrue;

    if (!token || token[0] == 0)
      return qfalse;

    if (token[0] == '{')
    {
      if (!String_Parse(p, &uiInfo.weaponModes[uiInfo.numWeaponModes].weaponMode) || !Int_Parse(p, &uiInfo.weaponModes[uiInfo.numWeaponModes].gtEnum))
        return qfalse;

      if (uiInfo.numWeaponModes < MAX_WEAPONMODES)
        uiInfo.numWeaponModes++;
      else
        Com_Printf("Too many weaponmodes, last one replace!\n");

      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}')
        return qfalse;
    }
  }
  return qfalse;
}


static qboolean GameType_Parse(char **p, qboolean join)
{
  char *token;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{')
    return qfalse;

  if (join)
    uiInfo.numJoinGameTypes = 0;
  else
    uiInfo.numGameTypes = 0;

  while (1)
  {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0)
      return qtrue;

    if (!token || token[0] == 0)
      return qfalse;

    if (token[0] == '{')
    {
      // two tokens per line, character name and sex
      if (join)
      {
        if (!String_Parse(p, &uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gameType) || !Int_Parse(p, &uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gtEnum))
          return qfalse;
      }
      else
      {
        if (!String_Parse(p, &uiInfo.gameTypes[uiInfo.numGameTypes].gameType) || !Int_Parse(p, &uiInfo.gameTypes[uiInfo.numGameTypes].gtEnum))
          return qfalse;
      }

      if (join)
      {
        if (uiInfo.numJoinGameTypes < MAX_GAMETYPES)
          uiInfo.numJoinGameTypes++;
        else
          Com_Printf("Too many net game types, last one replace!\n");
      }
      else
      {
        if (uiInfo.numGameTypes < MAX_GAMETYPES)
          uiInfo.numGameTypes++;
        else
          Com_Printf("Too many game types, last one replace!\n");
      }

      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}')
        return qfalse;
    }
  }
  return qfalse;
}

static qboolean MapList_Parse(char **p)
{
  char *token;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{')
    return qfalse;

  uiInfo.mapCount = 0;

  while (1)
  {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0)
      return qtrue;

    if (!token || token[0] == 0)
      return qfalse;

    if (token[0] == '{')
    {
      if (!String_Parse(p, &uiInfo.mapList[uiInfo.mapCount].mapName) || !String_Parse(p, &uiInfo.mapList[uiInfo.mapCount].mapLoadName)
          || !Int_Parse(p, &uiInfo.mapList[uiInfo.mapCount].teamMembers))
        return qfalse;

      if (!String_Parse(p, &uiInfo.mapList[uiInfo.mapCount].opponentName))
        return qfalse;

      uiInfo.mapList[uiInfo.mapCount].typeBits = 0;

      while (1)
      {
        token = COM_ParseExt(p, qtrue);
        if (token[0] >= '0' && token[0] <= '9')
        {
          uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << (token[0] - 0x030));
          if (!Int_Parse(p, &uiInfo.mapList[uiInfo.mapCount].timeToBeat[token[0] - 0x30]))
            return qfalse;
        }
        else
          break;
      }

      //mapList[mapCount].imageName = String_Alloc(va("levelshots/%s", mapList[mapCount].mapLoadName));
      //if (uiInfo.mapCount == 0) {
      // only load the first cinematic, selection loads the others
      //  uiInfo.mapList[uiInfo.mapCount].cinematic = trap_CIN_PlayCinematic(va("%s.roq",uiInfo.mapList[uiInfo.mapCount].mapLoadName), qfalse, qfalse, qtrue, 0, 0, 0, 0);
      //}
      //FIXME(0xA5EA):wtf
      uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
      uiInfo.mapList[uiInfo.mapCount].levelShot = trap_R_RegisterShaderNoMip(va("levelshots/%s", uiInfo.mapList[uiInfo.mapCount].mapLoadName));

      if (uiInfo.mapCount < MAX_MAPS)
        uiInfo.mapCount++;
      else
        Com_Printf("Too many maps, last one replaced!\n");
    }
  }
  return qfalse;
}

static void UI_ParseGameInfo(const char *teamFile)
{
  char *token;
  char *p;
  char *buff = NULL;

  //int mode = 0; TTimo: unused

  buff = GetMenuBuffer(teamFile);
  if (!buff)
    return;

  p = buff;

  while (1)
  {
    token = COM_ParseExt(&p, qtrue);

    if (!token || token[0] == 0 || token[0] == '}')
      break;

    if (Q_stricmp(token, "}") == 0)
      break;

    if (Q_stricmp(token, "gametypes") == 0)
    {
      if (GameType_Parse(&p, qfalse))
        continue;
      else
        break;
    }

    if (Q_stricmp(token, "joingametypes") == 0)
    {

      if (GameType_Parse(&p, qtrue))
        continue;
      else
        break;
    }

    if (!Q_stricmp(token, "weaponmodes"))
    {
      if (WeaponMode_Parse(&p))
        continue;
      else
        break;
    }
    if (Q_stricmp(token, "maps") == 0)
    {
      // start a new menu
      MapList_Parse(&p);
    }
  }
}

static void UI_Pause(qboolean b)
{
  if (b)
  {
    // pause the game and set the ui keycatcher
    trap_Cvar_Set("cl_paused", "1");
    trap_Key_SetCatcher(KEYCATCH_UI);
  }
  else
  {
    // unpause the game and clear the ui keycatcher
    trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
    trap_Key_ClearStates();
    trap_Cvar_Set("cl_paused", "0");
  }
}

#if 0 //ndef MISSIONPACK
static int UI_OwnerDraw_Width(int ownerDraw)
{
  return 0;
}
#endif

static int UI_PlayCinematic(const char *name, float x, float y, float w, float h)
{
  return trap_CIN_PlayCinematic(name, x, y, w, h, (CIN_loop | CIN_silent));
}

static void UI_StopCinematic(int handle)
{
  if (handle >= 0)
  {
    trap_CIN_StopCinematic(handle);
  }
  else
  {
    handle = abs(handle);
    if (handle == UI_MAPCINEMATIC)
    {
      if (uiInfo.mapList[ui_currentMap.integer].cinematic >= 0)
      {
        trap_CIN_StopCinematic(uiInfo.mapList[ui_currentMap.integer].cinematic);
        uiInfo.mapList[ui_currentMap.integer].cinematic = -1;
      }
    }
    else if (handle == UI_NETMAPCINEMATIC)
    {
      if (uiInfo.serverStatus.currentServerCinematic >= 0)
      {
        trap_CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
        uiInfo.serverStatus.currentServerCinematic = -1;
      }
    }
    else if (handle == UI_CLANCINEMATIC)
    {
      int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
      if (i >= 0 && i < uiInfo.teamCount)
      {
        if (uiInfo.teamList[i].cinematic >= 0)
        {
          trap_CIN_StopCinematic(uiInfo.teamList[i].cinematic);
          uiInfo.teamList[i].cinematic = -1;
        }
      }
    }
  }
}

static void UI_DrawCinematic(int handle, float x, float y, float w, float h)
{
  trap_CIN_SetExtents(handle, x, y, w, h);
  trap_CIN_DrawCinematic(handle);
}

static void UI_RunCinematicFrame(int handle)
{
 // Com_Printf(S_COLOR_MAGENTA"UI_RunCinematic  %d,\n", handle); //hypov8 remove
  trap_CIN_RunCinematic(handle);
}



/*
=================
PlayerModel_BuildList
=================
*/
static void UI_BuildQ3Model_List(void)
{
  int numdirs;
  int numfiles;
  char dirlist[2048];
  char filelist[2048];
  char skinname[MAX_QPATH];
  char scratch[256];
  char *dirptr;
  char *fileptr;
  int i, ext;
  int j, k, dirty;
  size_t dirlen;
  size_t filelen;

  uiInfo.q3HeadCount = 0;

  // iterate directory of all player models
  numdirs = trap_FS_GetFileList("models/players", "/", dirlist, 2048);
  dirptr  = dirlist;
  for (i = 0; i < numdirs && uiInfo.q3HeadCount < MAX_PLAYERMODELS; i++, dirptr += dirlen + 1)
  {
    dirlen = qstrlen(dirptr);

    if (dirlen && dirptr[dirlen - 1] == '/') dirptr[dirlen - 1] = '\0';

    if (!qstrcmp(dirptr, ".") || !qstrcmp(dirptr, ".."))
      continue;

    //tga or png
    for (ext = 0; ext < 2; ext++)
    {
      // iterate all skin files in directory
      numfiles = trap_FS_GetFileList(va("models/players/%s", dirptr), (ext==0)?"tga":"png", filelist, 2048);
      fileptr  = filelist;
      for (j = 0; j < numfiles && uiInfo.q3HeadCount < MAX_PLAYERMODELS; j++, fileptr += filelen + 1)
      {
        filelen = qstrlen(fileptr);
        COM_StripExtension(fileptr, skinname, sizeof(skinname));

        // look for icon_*.tga
        if (Q_stricmpn(skinname, "icon_", 5) == 0)
        {
          if (Q_stricmp(skinname, "icon_default") == 0)
            Com_sprintf(scratch, sizeof(scratch), "%s", dirptr);
          else
            Com_sprintf(scratch, sizeof(scratch), "%s/%s", dirptr, skinname + 5);

          dirty = 0;
          for (k = 0; k < uiInfo.q3HeadCount; k++)
          {
            if (!Q_stricmp(scratch, uiInfo.q3HeadNames[uiInfo.q3HeadCount]))
            {
              dirty = 1;
              break;
            }
          }
          if (!dirty) //"model" "headmodel"
          {
            Com_sprintf(uiInfo.q3HeadNames[uiInfo.q3HeadCount], sizeof(uiInfo.q3HeadNames[uiInfo.q3HeadCount]), "%s", scratch);
            uiInfo.q3HeadIcons[uiInfo.q3HeadCount++] = trap_R_RegisterShaderNoMip(va("models/players/%s/%s", dirptr, skinname));
          }
        }
      }
    }
  }
}


/*
=================
UI_Init
=================
*/
void _UI_Init(qboolean inGameLoad, int randomSeed)
{
  const char *menuSet;
  //int start;

  srand(randomSeed);
  //uiInfo.inGameLoad = inGameLoad;

  UI_RegisterCvars();
  UI_InitMemory();

  // cache redundant calulations
  trap_GetGlconfig(&uiInfo.uiDC.glconfig);
  trap_GetGlconfig2(&uiInfo.uiDC.glconfig2);
  trap_GetCpuConfig(&uiInfo.uiDC.cpuConfig);

  // for 640x480 virtualized screen
  uiInfo.uiDC.yscale = uiInfo.uiDC.glconfig.vidHeight * (1.0 / 480.0);
  uiInfo.uiDC.xscale = uiInfo.uiDC.glconfig.vidWidth * (1.0 / 640.0);
  if (uiInfo.uiDC.glconfig.vidWidth * 480 > uiInfo.uiDC.glconfig.vidHeight * 640)
  {
    // wide screen
    uiInfo.uiDC.bias = 0.5 * (uiInfo.uiDC.glconfig.vidWidth - (uiInfo.uiDC.glconfig.vidHeight * (640.0 / 480.0)));
  }
  else
  {
    // no wide screen
    uiInfo.uiDC.bias = 0;
  }


  //UI_Load();
  uiInfo.uiDC.registerShaderNoMip  = &trap_R_RegisterShaderNoMip;
  uiInfo.uiDC.setColor             = &UI_SetColor;
  uiInfo.uiDC.drawHandlePic        = &UI_DrawHandlePic;
  uiInfo.uiDC.drawStretchPic       = &trap_R_DrawStretchPic;
  uiInfo.uiDC.drawRotatedPic       = &trap_R_DrawRotatedPic; //add hypov8
  uiInfo.uiDC.drawText             = &Text_Paint;
  uiInfo.uiDC.textWidth            = &Text_Width;
  uiInfo.uiDC.textHeight           = &Text_Height;
  uiInfo.uiDC.registerModel        = &trap_R_RegisterModel;
  uiInfo.uiDC.modelBounds          = &trap_R_ModelBounds;
   uiInfo.uiDC.lerpTag             = &trap_R_LerpTag;
  uiInfo.uiDC.fillRect             = &UI_FillRect;
  uiInfo.uiDC.drawRect             = &_UI_DrawRect;
  uiInfo.uiDC.drawSides            = &_UI_DrawSides;
  uiInfo.uiDC.drawTopBottom        = &_UI_DrawTopBottom;
  uiInfo.uiDC.clearScene           = &trap_R_ClearScene;
  uiInfo.uiDC.drawSides            = &_UI_DrawSides;
  uiInfo.uiDC.addRefEntityToScene  = &trap_R_AddRefEntityToScene;
  uiInfo.uiDC.renderScene          = &trap_R_RenderScene;
  uiInfo.uiDC.registerFont         = &trap_R_RegisterFont;
  uiInfo.uiDC.ownerDrawItem        = &UI_OwnerDraw;
  uiInfo.uiDC.getValue             = &UI_GetValue;
  uiInfo.uiDC.ownerDrawVisible     = &UI_OwnerDrawVisible;
  uiInfo.uiDC.runScript            = &UI_RunMenuScript;
  uiInfo.uiDC.getTeamColor         = &UI_GetTeamColor;
  uiInfo.uiDC.setCVar              = trap_Cvar_Set;
  uiInfo.uiDC.getCVarString        = trap_Cvar_VariableStringBuffer;
  uiInfo.uiDC.getCVarValue         = trap_Cvar_VariableValue;
  uiInfo.uiDC.drawTextWithCursor   = &Text_PaintWithCursor;
  uiInfo.uiDC.setOverstrikeMode    = &trap_Key_SetOverstrikeMode;
  uiInfo.uiDC.getOverstrikeMode    = &trap_Key_GetOverstrikeMode;
  uiInfo.uiDC.startLocalSound      = &trap_S_StartLocalSound;
  uiInfo.uiDC.ownerDrawHandleKey   = &UI_OwnerDrawHandleKey;
  uiInfo.uiDC.feederCount          = &UI_FeederCount;
  uiInfo.uiDC.feederItemImage      = &UI_FeederItemImage;
  uiInfo.uiDC.feederItemText       = &UI_FeederItemText;
  uiInfo.uiDC.feederSelection      = &UI_FeederSelection;
  uiInfo.uiDC.setBinding           = &trap_Key_SetBinding;
  uiInfo.uiDC.getBindingBuf        = &trap_Key_GetBindingBuf;
  uiInfo.uiDC.keynumToStringBuf    = &trap_Key_KeynumToStringBuf;
  uiInfo.uiDC.executeText          = &trap_Cmd_ExecuteText;
  uiInfo.uiDC.Error                = &Com_Error;
  uiInfo.uiDC.Print                = &Com_Printf;
  uiInfo.uiDC.Pause                = &UI_Pause;
  uiInfo.uiDC.ownerDrawWidth       = &UI_OwnerDrawWidth;
  uiInfo.uiDC.registerSound        = &trap_S_RegisterSound;
  uiInfo.uiDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
  uiInfo.uiDC.stopBackgroundTrack  = &trap_S_StopBackgroundTrack;
  uiInfo.uiDC.playCinematic        = &UI_PlayCinematic;
  uiInfo.uiDC.stopCinematic        = &UI_StopCinematic;
  uiInfo.uiDC.drawCinematic        = &UI_DrawCinematic;
  uiInfo.uiDC.runCinematicFrame    = &UI_RunCinematicFrame;

  Init_Display(&uiInfo.uiDC);

  String_Init();

  uiInfo.uiDC.cursor      = trap_R_RegisterShaderNoMip("ui/cursor");
  uiInfo.uiDC.whiteShader = trap_R_RegisterShaderNoMip("white");

  AssetCache();

  //start = trap_Milliseconds();

  uiInfo.teamCount      = 0;
  uiInfo.characterCount = 0;
  uiInfo.aliasCount     = 0;

#ifdef PRE_RELEASE_TADEMO
  UI_ParseTeamInfo("demoteaminfo.txt");
  UI_ParseGameInfo("demogameinfo.txt");
#else
  UI_ParseTeamInfo("teaminfo.txt");
  UI_LoadTeams();
  UI_ParseGameInfo("gameinfo.txt");
#endif

  menuSet = UI_Cvar_VariableString("ui_menuFiles");
  if (menuSet == NULL || menuSet[0] == '\0')
  {
    menuSet = "ui/menus.txt";
  }

#if 0
  if (uiInfo.inGameLoad)
  {
    UI_LoadMenus("ui/ingame.txt", qtrue);
  }
  else
  {
  }
#else
  UI_LoadMenus(menuSet, qtrue);
  UI_LoadMenus("ui/ingame.txt", qfalse);
#endif

  Menus_CloseAll();

  trap_LAN_LoadCachedServers();
  UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);

  UI_BuildQ3Model_List();
  UI_LoadBots();

  // sets defaults for ui temp cvars
  uiInfo.effectsColor     = gamecodetoui[(int)trap_Cvar_VariableValue("color1") - 1];
  uiInfo.currentCrosshair = (int)trap_Cvar_VariableValue("cg_drawCrosshair");
  trap_Cvar_Set("ui_mousePitch", (trap_Cvar_VariableValue("m_pitch") >= 0) ? "0" : "1");

  trap_Cvar_Set("ui_glTexturecompression",  (uiInfo.uiDC.glconfig2.ARBTextureCompressionAvailable == 0) ? "0" : "1");
  //ui_glTexturecompression.integer = uiInfo.uiDC.glconfig.textureCompressionAvailabe;
  UI_UpdateLightCvar();
  uiInfo.serverStatus.currentServerCinematic = -1;
  uiInfo.previewMovie                        = -1;

  if ((int)trap_Cvar_VariableValue("ui_KingpinQ3FirstRun") == 0)
  {
    trap_Cvar_Set("s_volume", "0.8");
    trap_Cvar_Set("s_musicvolume", "0.5");
    trap_Cvar_Set("ui_KingpinQ3FirstRun", "1");
  }

  trap_Cvar_Register(NULL, "debug_protocol", "", 0);

  trap_Cvar_Set("ui_actualNetGameType", va("%d", ui_netGameType.integer));
}

//hypov8 used to disable dynamic light shadows with 2 varables
static void UI_UpdateLightCvar(void)
{
  int dyn_light = (int)trap_Cvar_VariableValue("uix_r_dynamicLight"); //hypov8 static light??
  int shadows = (int)trap_Cvar_VariableValue("uix_cg_shadows");
  int cvar_val = 0;

  if (dyn_light && shadows >1)
    cvar_val = 1;

  trap_Cvar_SetValue("ui_glLights", cvar_val);

}

/*
=================
UI_KeyEvent
=================
*/
void _UI_KeyEvent(int key, qboolean down)
{

  if (Menu_Count() > 0)
  {
    menuDef_t *menu = Menu_GetFocused();
    if (menu)
    {
      if (key == K_ESCAPE && down && !Menus_AnyFullScreenVisible())
        Menus_CloseAll();
      else
        Menu_HandleKey(menu, key, down);
    }
    else
    {
      trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
      trap_Key_ClearStates();
      trap_Cvar_Set("cl_paused", "0");
    }
  }

  //if ((s > 0) && (s != menu_null_sound)) {
  //  trap_S_StartLocalSound( s, CHAN_LOCAL_SOUND );
  //}
}

/*
=================
UI_MouseEvent
=================
*/
void _UI_MouseEvent(int dx, int dy)
{
  // update mouse screen position
  uiInfo.uiDC.cursorx += dx;
  if (uiInfo.uiDC.cursorx < 0)
    uiInfo.uiDC.cursorx = 0;
  else if (uiInfo.uiDC.cursorx > SCREEN_WIDTH)
    uiInfo.uiDC.cursorx = SCREEN_WIDTH;

  uiInfo.uiDC.cursory += dy;
  if (uiInfo.uiDC.cursory < 0)
    uiInfo.uiDC.cursory = 0;
  else if (uiInfo.uiDC.cursory > SCREEN_HEIGHT)
    uiInfo.uiDC.cursory = SCREEN_HEIGHT;

  if (Menu_Count() > 0)
  {
    //menuDef_t *menu = Menu_GetFocused();
    //Menu_HandleMouseMove(menu, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
    Display_MouseMove(NULL, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
  }
}

void UI_LoadNonIngame(void)
{
  const char *menuSet = UI_Cvar_VariableString("ui_menuFiles");

  if (menuSet == NULL || menuSet[0] == '\0')
  {
    menuSet = "ui/menus.txt";
  }
  UI_LoadMenus(menuSet, qfalse);
  uiInfo.inGameLoad = qfalse;
}

void _UI_SetActiveMenu(uiMenuCommand_t menu)
{
  char buf[256];

  // this should be the ONLY way the menu system is brought up
  // enusure minumum menu data is cached
  if (Menu_Count() > 0)
  {
    vec3_t v;
    v[0] = v[1] = v[2] = 0;
    switch (menu)
    {
    case UIMENU_NONE:
      trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
      trap_Key_ClearStates();
      trap_Cvar_Set("cl_paused", "0");
      Menus_CloseAll();

      return;

    case UIMENU_MAIN:
      trap_Cvar_Set("sv_killserver", "1");
      trap_Key_SetCatcher(KEYCATCH_UI);
      //trap_S_StartLocalSound( trap_S_RegisterSound("sound/misc/menu_background.ogg", qfalse) , CHAN_LOCAL_SOUND );
      //trap_S_StartBackgroundTrack("sound/misc/menu_background.ogg", NULL);
      if (uiInfo.inGameLoad)
      {
        UI_LoadNonIngame();
      }
      Menus_CloseAll();
      Menus_ActivateByName("main");
      trap_Cvar_VariableStringBuffer("com_errorMessage", buf, sizeof(buf));
      if (qstrlen(buf))
      {
#ifdef USE_GT_SINGLEPLAYER
        if (!ui_singlePlayerActive.integer)
        {
          Menus_ActivateByName("error_popmenu");
        }
        else
#endif
        {
          trap_Cvar_Set("com_errorMessage", "");
        }
      }
      return;

    case UIMENU_TEAM:
      trap_Key_SetCatcher(KEYCATCH_UI);
      Menus_ActivateByName("team");
      return;

    case UIMENU_NEED_CD:
      // no cd check in TA
      //trap_Key_SetCatcher( KEYCATCH_UI );
      //Menus_ActivateByName("needcd");
      //UI_ConfirmMenu( "Insert the CD", NULL, NeedCDAction );
      return;

    case UIMENU_BAD_CD_KEY:
      // no cd check in TA
      //trap_Key_SetCatcher( KEYCATCH_UI );
      //Menus_ActivateByName("badcd");
      //UI_ConfirmMenu( "Bad CD Key", NULL, NeedCDKeyAction );
      return;

    case UIMENU_POSTGAME:
      trap_Cvar_Set("sv_killserver", "1");
      trap_Key_SetCatcher(KEYCATCH_UI);
      if (uiInfo.inGameLoad)
      {
        UI_LoadNonIngame();
      }
      Menus_CloseAll();
      Menus_ActivateByName("endofgame");
      //UI_ConfirmMenu( "Bad CD Key", NULL, NeedCDKeyAction );
      return;

    case UIMENU_INGAME:
      trap_Cvar_Set("cl_paused", "1");
      trap_Key_SetCatcher(KEYCATCH_UI);
      UI_BuildPlayerList();
      Menus_CloseAll();
      Menus_ActivateByName("ingame");
      return;
    }
  }
}

qboolean _UI_IsFullscreen(void)
{
  return Menus_AnyFullScreenVisible();
}



static connstate_t lastConnState;
static char lastLoadingText[MAX_INFO_VALUE];

static void UI_ReadableSize(char *buf, int bufsize, int value)
{
  if (value > 1024 * 1024 * 1024)  // gigs
  {
    Com_sprintf(buf, bufsize, "%d", value / (1024 * 1024 * 1024));
    Com_sprintf(buf + qstrlen(buf), bufsize - qstrlen(buf), ".%02d GB",
                (value % (1024 * 1024 * 1024)) * 100 / (1024 * 1024 * 1024));
  }
  else if (value > 1024 * 1024)    // megs
  {
    Com_sprintf(buf, bufsize, "%d", value / (1024 * 1024));
    Com_sprintf(buf + qstrlen(buf), bufsize - qstrlen(buf), ".%02d MB",
                (value % (1024 * 1024)) * 100 / (1024 * 1024));
  }
  else if (value > 1024)           // kilos
  {
    Com_sprintf(buf, bufsize, "%d KB", value / 1024);
  }
  else                             // bytes
  {
    Com_sprintf(buf, bufsize, "%d bytes", value);
  }
}

// Assumes time is in msec
static void UI_PrintTime(char *buf, int bufsize, int time)
{
  time /= 1000;                    // change to seconds

  if (time > 3600)                 // in the hours range
    Com_sprintf(buf, bufsize, "%d hr %d min", time / 3600, (time % 3600) / 60);
  else if (time > 60)              // mins
    Com_sprintf(buf, bufsize, "%d min %d sec", time / 60, time % 60);
  else                             // secs
    Com_sprintf(buf, bufsize, "%d sec", time);
}

void Text_PaintCenter(float x, float y, float scale, vec4_t color, const char *text, float adjust)
{
  int len = Text_Width(text, scale, 0, NULL_FONT);

  Text_Paint(x - len / 2, y, scale, color, text, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);
}

void Text_PaintCenter_AutoWrapped(float x, float y, float xmax, float ystep, float scale, vec4_t color, const char *str, float adjust)
{
  int width;
  char *s1, *s2, *s3;
  char c_bcp;
  char buf[1024];

  if (!str || str[0] == '\0')
    return;

  Q_strncpyz(buf, str, sizeof(buf));
  s1 = s2 = s3 = buf;

  while (1)
  {
    do
    {
      s3++;
    }
    while (*s3 != ' ' && *s3 != '\0');
    c_bcp = *s3;
    *s3   = '\0';
    width = Text_Width(s1, scale, 0, NULL_FONT);
    *s3   = c_bcp;
    if (width > xmax)
    {
      if (s1 == s2)
      {
        // fuck, don't have a clean cut, we'll overflow
        s2 = s3;
      }
      *s2 = '\0';
      Text_PaintCenter(x, y, scale, color, s1, adjust);
      y += ystep;
      if (c_bcp == '\0')
      {
        // that was the last word
        // we could start a new loop, but that wouldn't be much use
        // even if the word is too long, we would overflow it (see above)
        // so just print it now if needed
        s2++;
        if (*s2 != '\0') // if we are printing an overflowing line we have s2 == s3
          Text_PaintCenter(x, y, scale, color, s2, adjust);
        break;
      }
      s2++;
      s1 = s2;
      s3 = s2;
    }
    else
    {
      s2 = s3;
      if (c_bcp == '\0')  // we reached the end
      {
        Text_PaintCenter(x, y, scale, color, s1, adjust);
        break;
      }
    }
  }
}

static void UI_DisplayDownloadInfo(const char *downloadName, float centerPoint, float yStart, float scale)
{
  static char dlText[]   = "Downloading:";
  static char etaText[]  = "Estimated time left:";
  static char xferText[] = "Transfer rate:";

  int downloadSize, downloadCount, downloadTime;
  char dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
  int xferRate;
  int leftWidth;
  const char *s;

  downloadSize  = (int)trap_Cvar_VariableValue("cl_downloadSize");
  downloadCount = (int)trap_Cvar_VariableValue("cl_downloadCount");
  downloadTime  = (int)trap_Cvar_VariableValue("cl_downloadTime");

  leftWidth = 320;

  UI_SetColor(colorWhite);
  Text_PaintCenter(centerPoint, yStart + 112, scale, colorWhite, dlText, 0);
  Text_PaintCenter(centerPoint, yStart + 192, scale, colorWhite, etaText, 0);
  Text_PaintCenter(centerPoint, yStart + 248, scale, colorWhite, xferText, 0);

  if (downloadSize > 0)
    s = va("%s (%d%%)", downloadName, (int)((float)downloadCount * 100.0f / downloadSize));
  else
    s = downloadName;

  Text_PaintCenter(centerPoint, yStart + 136, scale, colorWhite, s, 0);

  UI_ReadableSize(dlSizeBuf,     sizeof dlSizeBuf,       downloadCount);
  UI_ReadableSize(totalSizeBuf,  sizeof totalSizeBuf,    downloadSize);

  if (downloadCount < 4096 || !downloadTime)
  {
    Text_PaintCenter(leftWidth, yStart + 216, scale, colorWhite, "estimating", 0);
    Text_PaintCenter(leftWidth, yStart + 160, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
  }
  else
  {
    if ((uiInfo.uiDC.realTime - downloadTime) / 1000)
    {
      xferRate = downloadCount / ((uiInfo.uiDC.realTime - downloadTime) / 1000);
    }
    else
    {
      xferRate = 0;
    }
    UI_ReadableSize(xferRateBuf, sizeof xferRateBuf, xferRate);

    // Extrapolate estimated completion time
    if (downloadSize && xferRate)
    {
      int n = downloadSize / xferRate; // estimated time for entire d/l in secs

      // We do it in K (/1024) because we'd overflow around 4MB
      UI_PrintTime(dlTimeBuf, sizeof dlTimeBuf,
                   (n - (((downloadCount / 1024) * n) / (downloadSize / 1024))) * 1000);

      Text_PaintCenter(leftWidth, yStart + 216, scale, colorWhite, dlTimeBuf, 0);
      Text_PaintCenter(leftWidth, yStart + 160, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
    }
    else
    {
      Text_PaintCenter(leftWidth, yStart + 216, scale, colorWhite, "estimating", 0);
      if (downloadSize)
        Text_PaintCenter(leftWidth, yStart + 160, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
      else
        Text_PaintCenter(leftWidth, yStart + 160, scale, colorWhite, va("(%s copied)", dlSizeBuf), 0);
    }

    if (xferRate)
    {
      Text_PaintCenter(leftWidth, yStart + 272, scale, colorWhite, va("%s/Sec", xferRateBuf), 0);
    }
  }
}

/*
========================
UI_DrawConnectScreen

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
void UI_DrawConnectScreen(qboolean overlay)
{
  char *s;
  uiClientState_t cstate;
  char info[MAX_INFO_VALUE];
  char text[256];
  float centerPoint, yStart, scale;

  menuDef_t *menu = Menus_FindByName("Connect");


  if (!overlay && menu)
    Menu_Paint(menu, qtrue);

  if (!overlay)
  {
    centerPoint = 320;
    yStart      = 130;
    scale       = 0.6f;
  }
  else
  {
    centerPoint = 320;
    yStart      = 32;
    scale       = 0.6f;
    return;
  }

  // see what information we should display
  trap_GetClientState(&cstate);

  info[0] = '\0';
  if (trap_GetConfigString(CS_SERVERINFO, info, sizeof(info)))
    Text_PaintCenter(centerPoint, yStart, scale, colorWhite, va("Loading %s", Info_ValueForKey(info, "mapname")), 0);

  if (!Q_stricmp(cstate.servername, "localhost"))
  {
    Text_PaintCenter(centerPoint, yStart + 48, scale, colorWhite, va("Starting up..."), ITEM_TEXTSTYLE_SHADOWEDMORE);
  }
  else
  {
    qstrcpy(text, va("Connecting to %s", cstate.servername));
    Text_PaintCenter(centerPoint, yStart + 48, scale, colorWhite, text, ITEM_TEXTSTYLE_SHADOWEDMORE);
  }

  // display global MOTD at bottom
  Text_PaintCenter(centerPoint, 600, scale, colorWhite, Info_ValueForKey(cstate.updateInfoString, "motd"), 0);
  // print any server info (server full, bad version, etc)
  if (cstate.connState < CA_CONNECTED)
    Text_PaintCenter_AutoWrapped(centerPoint, yStart + 176, 630, 20, scale, colorWhite, cstate.messageString, 0);

  if (lastConnState > cstate.connState)
    lastLoadingText[0] = '\0';

  lastConnState = cstate.connState;

  switch (cstate.connState)
  {
  case CA_CONNECTING:
    s = va("Awaiting connection...%i", cstate.connectPacketCount);
    break;
  case CA_CHALLENGING:
    s = va("Awaiting challenge...%i", cstate.connectPacketCount);
    break;
  case CA_CONNECTED:
  {
    char downloadName[MAX_INFO_VALUE];

    trap_Cvar_VariableStringBuffer("cl_downloadName", downloadName, sizeof(downloadName));
    if (*downloadName)
    {
      UI_DisplayDownloadInfo(downloadName, centerPoint, yStart, scale/2);
      return;
    }
  }
  s = "Awaiting gamestate...";
  break;
  case CA_LOADING:
    return;
  case CA_PRIMED:
    return;
  default:
    return;
  }

  if (Q_stricmp(cstate.servername, "localhost"))
  {
    Text_PaintCenter(centerPoint, yStart + 80, scale, colorWhite, s, 0);
  }

  // password required / connection rejected information goes here
}

/*
================
cvars
================
*/

typedef struct
{
  vmCvar_t *vmCvar;
  char *cvarName;
  char *defaultString;
  int cvarFlags;
} cvarTable_t;

vmCvar_t ui_ffa_fraglimit;
vmCvar_t ui_ffa_timelimit;

vmCvar_t ui_tourney_fraglimit;
vmCvar_t ui_tourney_timelimit;

vmCvar_t ui_team_fraglimit;
vmCvar_t ui_team_timelimit;
vmCvar_t ui_team_friendly;

vmCvar_t ui_ctf_capturelimit;
vmCvar_t ui_ctf_timelimit;
vmCvar_t ui_ctf_friendly;

vmCvar_t ui_arenasFile;
vmCvar_t ui_botsFile;
vmCvar_t ui_spScores1;
vmCvar_t ui_spScores2;
vmCvar_t ui_spScores3;
vmCvar_t ui_spScores4;
vmCvar_t ui_spScores5;
vmCvar_t ui_spAwards;
vmCvar_t ui_spVideos;
vmCvar_t ui_spSkill;

vmCvar_t ui_spSelection;

vmCvar_t ui_browserMaster;
vmCvar_t ui_browserGameType;
vmCvar_t ui_browserSortKey;
vmCvar_t ui_browserShowFull;
vmCvar_t ui_browserShowEmpty;

vmCvar_t ui_brassTime;
vmCvar_t ui_drawCrosshair;
vmCvar_t ui_drawCrosshairNames;
vmCvar_t ui_marks;

vmCvar_t ui_server1;
vmCvar_t ui_server2;
vmCvar_t ui_server3;
vmCvar_t ui_server4;
vmCvar_t ui_server5;
vmCvar_t ui_server6;
vmCvar_t ui_server7;
vmCvar_t ui_server8;
vmCvar_t ui_server9;
vmCvar_t ui_server10;
vmCvar_t ui_server11;
vmCvar_t ui_server12;
vmCvar_t ui_server13;
vmCvar_t ui_server14;
vmCvar_t ui_server15;
vmCvar_t ui_server16;

#ifndef STANDALONE
vmCvar_t ui_cdkeychecked;
#endif

vmCvar_t ui_dragonTeam;
vmCvar_t ui_dragonTeam1;
vmCvar_t ui_dragonTeam2;
vmCvar_t ui_dragonTeam3;
vmCvar_t ui_dragonTeam4;
vmCvar_t ui_dragonTeam5;
vmCvar_t ui_nikkiTeam;
vmCvar_t ui_nikkiTeam1;
vmCvar_t ui_nikkiTeam2;
vmCvar_t ui_nikkiTeam3;
vmCvar_t ui_nikkiTeam4;
vmCvar_t ui_nikkiTeam5;
vmCvar_t ui_team;
vmCvar_t ui_teamName;
vmCvar_t ui_dedicated;
vmCvar_t ui_gameType;
vmCvar_t ui_netGameType;
vmCvar_t ui_actualNetGameType;
vmCvar_t ui_joinGameType;
vmCvar_t ui_joinWeaponMode;
vmCvar_t ui_netSource;
vmCvar_t ui_serverFilterType;
vmCvar_t ui_opponentName;
vmCvar_t ui_menuFiles;
vmCvar_t ui_currentTier;
vmCvar_t ui_currentMap;
vmCvar_t ui_currentNetMap;
vmCvar_t ui_mapIndex;
vmCvar_t ui_currentOpponent;
vmCvar_t ui_selectedPlayer;
vmCvar_t ui_selectedPlayerName;
vmCvar_t ui_lastServerRefresh_0;
vmCvar_t ui_lastServerRefresh_1;
vmCvar_t ui_lastServerRefresh_2;
vmCvar_t ui_lastServerRefresh_3;
#ifdef USE_GT_SINGLEPLAYER
vmCvar_t ui_singlePlayerActive;
#endif
vmCvar_t ui_scoreAccuracy;
vmCvar_t ui_scoreImpressives;
vmCvar_t ui_scoreExcellents;
vmCvar_t ui_scoreCaptures;
vmCvar_t ui_scoreDefends;
vmCvar_t ui_scoreAssists;
vmCvar_t ui_scoreGauntlets;
vmCvar_t ui_scoreScore;
vmCvar_t ui_scorePerfect;
vmCvar_t ui_scoreTeam;
vmCvar_t ui_scoreBase;
vmCvar_t ui_scoreTimeBonus;
vmCvar_t ui_scoreSkillBonus;
vmCvar_t ui_scoreShutoutBonus;
vmCvar_t ui_scoreTime;
vmCvar_t ui_captureLimit;
vmCvar_t ui_fragLimit;
vmCvar_t ui_fontSmall;
vmCvar_t ui_fontBig;
vmCvar_t ui_fontTiny;
vmCvar_t ui_fontHuge;
vmCvar_t ui_findPlayer;
vmCvar_t ui_Q3Model;

vmCvar_t ui_hudFiles;
vmCvar_t ui_recordSPDemo;
vmCvar_t ui_realCaptureLimit;
vmCvar_t ui_realWarmUp;
vmCvar_t ui_serverStatusTimeOut;

vmCvar_t ui_glTexturecompression;



static cvarTable_t cvarTable[] = {
  {&ui_ffa_fraglimit, "ui_ffa_fraglimit", "20", CVAR_ARCHIVE},
  {&ui_ffa_timelimit, "ui_ffa_timelimit", "0", CVAR_ARCHIVE},

  {&ui_tourney_fraglimit, "ui_tourney_fraglimit", "0", CVAR_ARCHIVE},
  {&ui_tourney_timelimit, "ui_tourney_timelimit", "15", CVAR_ARCHIVE},

  {&ui_team_fraglimit, "ui_team_fraglimit", "0", CVAR_ARCHIVE},
  {&ui_team_timelimit, "ui_team_timelimit", "20", CVAR_ARCHIVE},
  {&ui_team_friendly, "ui_team_friendly",  "1", CVAR_ARCHIVE},

  {&ui_ctf_capturelimit, "ui_ctf_capturelimit", "8", CVAR_ARCHIVE},
  {&ui_ctf_timelimit, "ui_ctf_timelimit", "30", CVAR_ARCHIVE},
  {&ui_ctf_friendly, "ui_ctf_friendly",  "0", CVAR_ARCHIVE},

  {&ui_arenasFile, "g_arenasFile", "", CVAR_INIT | CVAR_ROM},
  {&ui_botsFile, "g_botsFile", "", CVAR_INIT | CVAR_ROM},
  { &ui_spScores1, "g_spScores1", "", CVAR_ARCHIVE },
  { &ui_spScores2, "g_spScores2", "", CVAR_ARCHIVE },
  { &ui_spScores3, "g_spScores3", "", CVAR_ARCHIVE },
  { &ui_spScores4, "g_spScores4", "", CVAR_ARCHIVE },
  { &ui_spScores5, "g_spScores5", "", CVAR_ARCHIVE },
  { &ui_spAwards, "g_spAwards", "", CVAR_ARCHIVE },
  { &ui_spVideos, "g_spVideos", "", CVAR_ARCHIVE },
  {&ui_spSkill, "g_spSkill", "2", CVAR_ARCHIVE},

  {&ui_spSelection, "ui_spSelection", "", CVAR_ROM},

  {&ui_browserMaster, "ui_browserMaster", "0", CVAR_ARCHIVE},
  {&ui_browserGameType, "ui_browserGameType", "0", CVAR_ARCHIVE},
  {&ui_browserSortKey, "ui_browserSortKey", "4", CVAR_ARCHIVE},
  {&ui_browserShowFull, "ui_browserShowFull", "1", CVAR_ARCHIVE},
  {&ui_browserShowEmpty, "ui_browserShowEmpty", "1", CVAR_ARCHIVE},

  {&ui_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE},
  {&ui_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE},
  {&ui_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE},
  {&ui_marks, "cg_marks", "1", CVAR_ARCHIVE},

  {&ui_server1, "server1", "", CVAR_ARCHIVE},
  {&ui_server2, "server2", "", CVAR_ARCHIVE},
  {&ui_server3, "server3", "", CVAR_ARCHIVE},
  {&ui_server4, "server4", "", CVAR_ARCHIVE},
  {&ui_server5, "server5", "", CVAR_ARCHIVE},
  {&ui_server6, "server6", "", CVAR_ARCHIVE},
  {&ui_server7, "server7", "", CVAR_ARCHIVE},
  {&ui_server8, "server8", "", CVAR_ARCHIVE},
  {&ui_server9, "server9", "", CVAR_ARCHIVE},
  {&ui_server10, "server10", "", CVAR_ARCHIVE},
  {&ui_server11, "server11", "", CVAR_ARCHIVE},
  {&ui_server12, "server12", "", CVAR_ARCHIVE},
  {&ui_server13, "server13", "", CVAR_ARCHIVE},
  {&ui_server14, "server14", "", CVAR_ARCHIVE},
  {&ui_server15, "server15", "", CVAR_ARCHIVE},
  {&ui_server16, "server16", "", CVAR_ARCHIVE},
#ifndef STANDALONE
  {&ui_cdkeychecked, "ui_cdkeychecked", "0", CVAR_ROM},
#endif
  {&ui_new, "ui_new", "0", CVAR_TEMP},
  {&ui_debug, "ui_debug", "0", CVAR_TEMP},
  {&ui_initialized, "ui_initialized", "0", CVAR_TEMP},

  {&ui_team, "ui_team", "1", CVAR_ARCHIVE}, //ui team. for model selection
  {&ui_teamName, "ui_teamName", DEFAULT_CLAN_DRAGONS, CVAR_ARCHIVE}, //'clan' name. default dragons
  {&ui_opponentName, "ui_opponentName", DEFAULT_CLAN_NIKKIS, CVAR_ARCHIVE},

  {&ui_dragonTeam, "ui_dragonTeam", DEFAULT_CLAN_DRAGONS, CVAR_ARCHIVE},
  {&ui_nikkiTeam, "ui_nikkiTeam", DEFAULT_CLAN_NIKKIS, CVAR_ARCHIVE},

  {&ui_dedicated, "ui_dedicated", "0", CVAR_ARCHIVE},
  {&ui_gameType, "ui_gametype", "3", CVAR_ARCHIVE},
  {&ui_joinGameType, "ui_joinGametype", "0", CVAR_ARCHIVE},
  {&ui_joinWeaponMode, "ui_joinWeaponMode", "0", CVAR_ARCHIVE},
  {&ui_netGameType, "ui_netGametype", "3", CVAR_ARCHIVE},
  //FIXME (0xA5EA): ? removed GT_SINGLEPLAYER; change 3 ??
  {&ui_actualNetGameType, "ui_actualNetGametype", "3", CVAR_ARCHIVE},
  {&ui_dragonTeam1, "ui_dragonTeam1", "0", CVAR_ARCHIVE},
  {&ui_dragonTeam2, "ui_dragonTeam2", "0", CVAR_ARCHIVE},
  {&ui_dragonTeam3, "ui_dragonTeam3", "0", CVAR_ARCHIVE},
  {&ui_dragonTeam4, "ui_dragonTeam4", "0", CVAR_ARCHIVE},
  {&ui_dragonTeam5, "ui_dragonTeam5", "0", CVAR_ARCHIVE},
  {&ui_nikkiTeam1, "ui_nikkiTeam1", "0", CVAR_ARCHIVE},
  {&ui_nikkiTeam2, "ui_nikkiTeam2", "0", CVAR_ARCHIVE},
  {&ui_nikkiTeam3, "ui_nikkiTeam3", "0", CVAR_ARCHIVE},
  {&ui_nikkiTeam4, "ui_nikkiTeam4", "0", CVAR_ARCHIVE},
  {&ui_nikkiTeam5, "ui_nikkiTeam5", "0", CVAR_ARCHIVE},
  { &ui_netSource, "ui_netSource", "1", CVAR_ARCHIVE }, //hypov8 default: 1 = master0
  {&ui_menuFiles, "ui_menuFiles", "ui/menus.txt", CVAR_ARCHIVE},
  {&ui_currentTier, "ui_currentTier", "0", CVAR_ARCHIVE},
  {&ui_currentMap, "ui_currentMap", "0", CVAR_ARCHIVE},
  {&ui_currentNetMap, "ui_currentNetMap", "0", CVAR_ARCHIVE},
  {&ui_mapIndex, "ui_mapIndex", "0", CVAR_ARCHIVE},
  {&ui_currentOpponent, "ui_currentOpponent", "0", CVAR_ARCHIVE},
  {&ui_selectedPlayer, "cg_selectedPlayer", "0", CVAR_ARCHIVE},
  {&ui_selectedPlayerName, "cg_selectedPlayerName", "", CVAR_ARCHIVE},
  {&ui_lastServerRefresh_0, "ui_lastServerRefresh_0", "", CVAR_ARCHIVE},
  {&ui_lastServerRefresh_1, "ui_lastServerRefresh_1", "", CVAR_ARCHIVE},
  {&ui_lastServerRefresh_2, "ui_lastServerRefresh_2", "", CVAR_ARCHIVE},
  {&ui_lastServerRefresh_3, "ui_lastServerRefresh_3", "", CVAR_ARCHIVE},
#ifdef USE_GT_SINGLEPLAYER
  {&ui_singlePlayerActive, "ui_singlePlayerActive", "0", 0},
#endif
  {&ui_scoreAccuracy, "ui_scoreAccuracy", "0", CVAR_ARCHIVE},
  {&ui_scoreImpressives, "ui_scoreImpressives", "0", CVAR_ARCHIVE},
  {&ui_scoreExcellents, "ui_scoreExcellents", "0", CVAR_ARCHIVE},
  {&ui_scoreCaptures, "ui_scoreCaptures", "0", CVAR_ARCHIVE},
  {&ui_scoreDefends, "ui_scoreDefends", "0", CVAR_ARCHIVE},
  {&ui_scoreAssists, "ui_scoreAssists", "0", CVAR_ARCHIVE},
  {&ui_scoreGauntlets, "ui_scoreGauntlets", "0", CVAR_ARCHIVE},
  {&ui_scoreScore, "ui_scoreScore", "0", CVAR_ARCHIVE},
  {&ui_scorePerfect, "ui_scorePerfect", "0", CVAR_ARCHIVE},
  {&ui_scoreTeam, "ui_scoreTeam", "0 to 0", CVAR_ARCHIVE},
  {&ui_scoreBase, "ui_scoreBase", "0", CVAR_ARCHIVE},
  {&ui_scoreTime, "ui_scoreTime", "00:00", CVAR_ARCHIVE},
  {&ui_scoreTimeBonus, "ui_scoreTimeBonus", "0", CVAR_ARCHIVE},
  {&ui_scoreSkillBonus, "ui_scoreSkillBonus", "0", CVAR_ARCHIVE},
  {&ui_scoreShutoutBonus, "ui_scoreShutoutBonus", "0", CVAR_ARCHIVE},
  {&ui_fragLimit, "ui_fragLimit", "10", 0},
  {&ui_captureLimit, "ui_captureLimit", "5", 0},
#if 0
  {&cg_fontTiny, "ui_fontTiny", "0.19", CVAR_ARCHIVE},
  {&cg_fontSmall, "ui_fontSmall", "0.27", CVAR_ARCHIVE},
  {&cg_fontBig, "ui_fontBig", "0.38", CVAR_ARCHIVE},
  {&cg_fontHuge, "ui_fontHuge", "0.45", CVAR_ARCHIVE},
#endif
  {&ui_fontTiny, "ui_fontTiny", "0.19", CVAR_ARCHIVE}, //hypov8 note: renamed to search ingame faster
  {&ui_fontSmall, "ui_fontSmall", "0.27", CVAR_ARCHIVE},
  {&ui_fontBig, "ui_fontBig", "0.38", CVAR_ARCHIVE},
  {&ui_fontHuge, "ui_fontHuge",  "0.45", CVAR_ARCHIVE},
  {&ui_findPlayer, "ui_findPlayer", "thug", CVAR_ARCHIVE},
  {&ui_Q3Model, "ui_q3model", "0", CVAR_ARCHIVE}, //hypov8: toggle between team/dm models in player menu's //archive not needed?
  {&ui_hudFiles, "cg_hudFiles", DFLT_HUDFILE, CVAR_ARCHIVE},
  {&ui_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE},
  {&ui_KingpinQ3FirstRun, "ui_KingpinQ3FirstRun", "0", CVAR_ARCHIVE},
  {&ui_realWarmUp, "g_warmup", "20", CVAR_ARCHIVE},
  {&ui_realCaptureLimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART},
  {&ui_serverStatusTimeOut, "ui_serverStatusTimeOut", "7000", CVAR_ARCHIVE},

};

static int cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);


/*
=================
UI_RegisterCvars
=================
*/
void UI_RegisterCvars(void)
{
  int i;
  cvarTable_t *cv;

  for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++)
  {
    trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags);
  }
}

/*
=================
UI_UpdateCvars
=================
*/
void UI_UpdateCvars(void)
{
  int i;
  cvarTable_t *cv;

  for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++)
  {
    trap_Cvar_Update(cv->vmCvar);
  }
}

/*
=================
ArenaServers_StopRefresh
=================
*/
static void UI_StopServerRefresh(void)
{
  int count;

  if (!uiInfo.serverStatus.refreshActive)
  {
    // not currently refreshing
    return;
  }
  uiInfo.serverStatus.refreshActive = qfalse;
  Com_Printf("%d servers listed in browser with %d players.\n",
             uiInfo.serverStatus.numDisplayServers,
             uiInfo.serverStatus.numPlayersOnServers);
  count = trap_LAN_GetServerCount(ui_netSource.integer);
  if (count - uiInfo.serverStatus.numDisplayServers > 0)
  {
    Com_Printf("%d servers not listed due to packet loss or pings higher than %d\n",
               count - uiInfo.serverStatus.numDisplayServers,
               (int)trap_Cvar_VariableValue("cl_maxPing"));
  }
}

/*
=================
ArenaServers_MaxPing
=================
*/
//FIXME(0xA5EA): do i need this ?
#if 0 //ndef MISSIONPACK
static int ArenaServers_MaxPing(void)
{
  int maxPing;

  maxPing = (int)trap_Cvar_VariableValue("cl_maxPing");
  if (maxPing < 100)
  {
    maxPing = 100;
  }
  return maxPing;
}
#endif

/*
=================
UI_DoServerRefresh
=================
*/
static void UI_DoServerRefresh(void)
{
  qboolean wait = qfalse;

  if (!uiInfo.serverStatus.refreshActive)
  {
    return;
  }
  if (ui_netSource.integer != AS_FAVORITES)
  {
    if (ui_netSource.integer == AS_LOCAL)
    {
      if (!trap_LAN_GetServerCount(ui_netSource.integer))
        wait = qtrue;
    }
    else // global
    {
      if (trap_LAN_GetServerCount(ui_netSource.integer) < 0)
        wait = qtrue;
    }
  }

  if (uiInfo.uiDC.realTime < uiInfo.serverStatus.refreshtime)
  {
    if (wait)
      return;
  }

  // if still trying to retrieve pings
  if (trap_LAN_UpdateVisiblePings(ui_netSource.integer))
  {
    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
  }
  else if (!wait)
  {
    // get the last servers in the list
    UI_BuildServerDisplayList(2);
    // stop the refresh
    UI_StopServerRefresh();
  }
  //
  UI_BuildServerDisplayList(qfalse);
}

/*
=================
UI_StartServerRefresh
=================
*/
static void UI_StartServerRefresh(qboolean full)
{
  char *ptr = NULL;

  qtime_t q;

  trap_RealTime(&q);
  trap_Cvar_Set(va("ui_lastServerRefresh_%i", ui_netSource.integer), va("%s-%i, %i at %i:%i", MonthAbbrev[q.tm_mon], q.tm_mday, 1900 + q.tm_year, q.tm_hour, q.tm_min));

  if (!full)
  {
    UI_UpdatePendingPings();
    return;
  }

  uiInfo.serverStatus.refreshActive      = qtrue;
  uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 1000;
  // clear number of displayed servers
  uiInfo.serverStatus.numDisplayServers   = 0;
  uiInfo.serverStatus.numPlayersOnServers = 0;
  // mark all servers as visible so we store ping updates for them
  trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
  // reset all the pings
  trap_LAN_ResetPings(ui_netSource.integer);
  //
  if (ui_netSource.integer == AS_LOCAL)
  {
    trap_Cmd_ExecuteText(EXEC_NOW, "localservers\n");
    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
    return;
  }

  if (ui_netSource.integer == AS_FAVORITES)
  {
    //skip
    return;
  }

  uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 5000;
  if (ui_netSource.integer == AS_GLOBAL)
  {
    int i;
    int end = AS_GLOBAL5 -AS_GLOBAL;

    //hypov8 added ui_netsource master 0-4
    for (i = 0; i < end; i++)
    {
      if (UI_GetMaster(i))
      {
        ptr = UI_Cvar_VariableString("debug_protocol");
        if (qstrlen(ptr))
        {
          trap_Cmd_ExecuteText(EXEC_NOW, va("globalservers %i %s full empty\n", i, ptr)); //ui_netSource.integer - 1
        }
        else
        {
          trap_Cmd_ExecuteText(EXEC_NOW, va("globalservers %i %d full empty\n", i, (int)trap_Cvar_VariableValue("protocol"))); //ui_netSource.integer - 1
        }
      }
    }
  }
}

#ifdef USE_GT_SINGLEPLAYER
static qboolean UI_SetNextMap(int actual, int index)
{
  int i;

  for (i = actual + 1; i < uiInfo.mapCount; i++)
  {
    if (uiInfo.mapList[i].active)
    {
      Menu_SetFeederSelection(NULL, FEEDER_MAPS, index + 1, "skirmish");
      return qtrue;
    }
  }
  return qfalse;
}

static void UI_StartSkirmish(qboolean next)
{
  int i, k, g, delay, temp;
  float skill;
  char buff[MAX_STRING_CHARS];

  if (next)
  {
    int actual;
    int index = (int)trap_Cvar_VariableValue("ui_mapIndex");
    UI_MapCountByGameType(qtrue);
    UI_SelectedMap(index, &actual);
    if (UI_SetNextMap(actual, index))
    {
    }
    else
    {
      UI_GameType_HandleKey(0, NULL, K_MOUSE1, qfalse);
      UI_MapCountByGameType(qtrue);
      Menu_SetFeederSelection(NULL, FEEDER_MAPS, 0, "skirmish");
    }
  }

  g = uiInfo.gameTypes[ui_gameType.integer].gtEnum;
  trap_Cvar_SetValue("g_gametype", g);
  trap_Cmd_ExecuteText(EXEC_APPEND, va("wait ; wait ; map %s\n", uiInfo.mapList[ui_currentMap.integer].mapLoadName));
  skill = trap_Cvar_VariableValue("g_spSkill");
  trap_Cvar_Set("ui_scoreMap", uiInfo.mapList[ui_currentMap.integer].mapName);

  k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));

  trap_Cvar_Set("ui_singlePlayerActive", "1");

  // set up sp overrides, will be replaced on postgame
  temp = trap_Cvar_VariableValue("capturelimit");
  trap_Cvar_Set("ui_saveCaptureLimit", va("%i", temp));
  temp = trap_Cvar_VariableValue("fraglimit");
  trap_Cvar_Set("ui_saveFragLimit", va("%i", temp));

  UI_SetCapFragLimits(qfalse);

  temp = trap_Cvar_VariableValue("cg_drawTimer");
  trap_Cvar_Set("ui_drawTimer", va("%i", temp));
  temp = trap_Cvar_VariableValue("g_doWarmup");
  trap_Cvar_Set("ui_doWarmup", va("%i", temp));
  temp = trap_Cvar_VariableValue("g_friendlyFire");
  trap_Cvar_Set("ui_friendlyFire", va("%i", temp));
  temp = trap_Cvar_VariableValue("sv_maxClients");
  trap_Cvar_Set("ui_maxClients", va("%i", temp));
  temp = trap_Cvar_VariableValue("g_warmup");
  trap_Cvar_Set("ui_Warmup", va("%i", temp));
  temp = trap_Cvar_VariableValue("sv_pure");
  trap_Cvar_Set("ui_pure", va("%i", temp));

  trap_Cvar_Set("cg_cameraOrbit", "0");
  trap_Cvar_Set("cg_thirdPerson", "0");
  trap_Cvar_Set("cg_drawTimer", "1");
  trap_Cvar_Set("g_doWarmup", "1");
  trap_Cvar_Set("g_warmup", "15");
  trap_Cvar_Set("sv_pure", "0");
  trap_Cvar_Set("g_friendlyFire", "0");
  trap_Cvar_Set("g_dragonTeam", UI_Cvar_VariableString("ui_teamName"));
  trap_Cvar_Set("g_nikkiTeam", UI_Cvar_VariableString("ui_opponentName"));

  if (trap_Cvar_VariableValue("ui_recordSPDemo"))
  {
    Com_sprintf(buff, MAX_STRING_CHARS, "%s_%i", uiInfo.mapList[ui_currentMap.integer].mapLoadName, g);
    trap_Cvar_Set("ui_recordSPDemoName", buff);
  }

  delay = 500;

  if (g == GT_TOURNAMENT)
  {
    trap_Cvar_Set("sv_maxClients", "2");
    Com_sprintf(buff, sizeof(buff), "wait ; addbot %s %f " ", %i \n", uiInfo.mapList[ui_currentMap.integer].opponentName, skill, delay);
    trap_Cmd_ExecuteText(EXEC_APPEND, buff);
  }
  else
  {
    temp = uiInfo.mapList[ui_currentMap.integer].teamMembers * 2;
    trap_Cvar_Set("sv_maxClients", va("%d", temp));
    for (i = 0; i < uiInfo.mapList[ui_currentMap.integer].teamMembers; i++)
    {
    Com_sprintf(buff, sizeof(buff), "addbot %s %f %s %i %s\n", UI_AIFromName(uiInfo.teamList[k].teamMembers[i]), skill, (g == GT_FFA) ? "" : TEAM_NAME_NIKKIS, delay, uiInfo.teamList[k].teamMembers[i]);
      trap_Cmd_ExecuteText(EXEC_APPEND, buff);
      delay += 500;
    }
    k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
    for (i = 0; i < uiInfo.mapList[ui_currentMap.integer].teamMembers - 1; i++)
    {
    Com_sprintf(buff, sizeof(buff), "addbot %s %f %s %i %s\n", UI_AIFromName(uiInfo.teamList[k].teamMembers[i]), skill, (g == GT_FFA) ? "" : TEAM_NAME_DRAGONS, delay, uiInfo.teamList[k].teamMembers[i]);
      trap_Cmd_ExecuteText(EXEC_APPEND, buff);
      delay += 500;
    }
  }
  if (g >= GT_TEAM)
  {
    trap_Cmd_ExecuteText(EXEC_APPEND, "wait 5; team Red\n");
  }
}
#endif
