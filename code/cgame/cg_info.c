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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_info.c -- display information while data is being loading

#include "cg_local.h"

vec4_t colorLines = {0.6f, 0.6f, 0.8f, 0.15f};                       // lines color
vec4_t colorText = {0.9f, 0.9f, 1.0f, 0.75f};                        // text color
vec4_t colorProgress = {0.7f, 0.7f, 1.0f, 0.2f};                     // progress color

vec4_t text_color_disabled  = {0.50f, 0.50f, 0.50f, 1. /*0.75f */};   // light gray
vec4_t text_color_normal = {0.9f, 0.9f, 1.0f,  1. /*0.5f */};        // light blue/gray
vec4_t text_color_highlight = {0.90f, 0.90f, 1.00f, 1. /*0.95f */};  // bright white
vec4_t text_color_warning = {0.90f, 0.10f, 0.10f, 0.75f};            // bright white

/*
======================
CG_LoadingString
======================
*/
void CG_LoadingString(const char *s, qboolean strong)
{

  Q_strncpyz(cg.progressInfo[cg.progress].info, s, sizeof(cg.progressInfo[cg.progress].info));
  cg.progressInfo[cg.progress].strong = strong;
  cg.progress++;

  if (cg.progress >= NUM_PROGRESS)
    cg.progress = NUM_PROGRESS-1;

#if 0
  //find out how many cg.progress we made...
  Com_Printf("Progress: %i\n", cg.progress);
#endif

  trap_UpdateScreen();
  //FIXME (0xA5EA): wirklich update screen ?
}

/*
======================
CG_DrawProgress
======================
*/
static void CG_DrawProgress(void)
{
  int x, y;
//	float rectColor[4];
//	const char *s = NULL;
  int i;
  vec4_t color;
  int style = 0;

  if (cg.progress == 0)
  {
    CG_Text_PaintAligned(230, 228, "Precaching ... ", 0.3f, UI_RIGHT | UI_DROPSHADOW, colortmGreen, &cgs.media.freeSansBoldFont);
  }
  else
  {
    CG_Text_PaintAligned(230, 228, "Loading ", 0.3f, UI_RIGHT | UI_DROPSHADOW, colortmGreen, &cgs.media.freeSansBoldFont);
    CG_Text_PaintAligned(230, 228, va(" %i %% ...", (int)(100 / NUM_PROGRESS * cg.progress)), 0.3f, UI_LEFT | UI_DROPSHADOW,
        colortmGreen, &cgs.media.freeSansBoldFont);
  }

  x = 0;
  y = 450;


  for (i = 0; i < NUM_PROGRESS; i++)
  {
    CG_DrawPic(x + i * 16, y, 16, 16, cgs.media.load0);
  }

  CG_DrawRect(0, 440 - cg.progress * 12, 640, 1, 1, colorLines);

  x = 0;
  for (i = 0; i < cg.progress; i++)
  {
    Vector4Copy(text_color_disabled, color);
   // color[3] -= .35f;

    if (i == cg.progress - 1)
    {
      style = UI_DROPSHADOW;
     // Vector4Copy(text_color_highlight, color);
    }
    else if (cg.progressInfo[i].strong)
    {
      style     = 0;
      Vector4Copy(colortmBlue, color);
    }

   CG_Text_PaintAligned(20, 440 - i * 12, cg.progressInfo[i].info, 0.2f, style, color, &cgs.media.freeSansBoldFont);
   //  CG_Text_PaintAligned(20, 40 + i * 12, cg.progressInfo[i].info, 0.2f, style, color, &cgs.media.freeSansBoldFont);

    CG_DrawPic(x + i * 16, y, 16, 16, cgs.media.load1);

    if (i == cg.progress - 1)
    {
      CG_DrawRect(x + i * 16 + 8, 0, 1, 480, 1, colorLines);
      CG_DrawRect(0, y - 4, 640, 1, 1, colorLines);

      CG_Text_PaintAligned(x + i * 16 + 8, y - 8, cg.progressInfo[i].info, 0.2f, UI_RIGHT | UI_DROPSHADOW,
                           text_color_highlight, &cgs.media.freeSansBoldFont);
    }
  }
}

char const* CG_GetGamemodeText(int gametype, int weaponmod)
{
  char const* s = 0;
  switch (gametype)
  {
  case GT_FFA:
    switch (weaponmod)
    {
    case WM_HITMEN:         s = "Gangbang Hitmen";           break;
    case WM_REALMODE:       s = "Gangbang Realmode";         break;
    case WM_HITMENREALMODE: s = "Gangbang Hitmen Realmode";  break;
    default:                s = "Gangbang";                  break;
    }
    break;
#ifdef USE_GT_SINGLEPLAYER
  case GT_SINGLE_PLAYER:
    s = "Single Player";
    break;
#endif
  case GT_TOURNAMENT:
    switch (weaponmod)
    {
    case WM_HITMEN:         s = "Hitmen 1 vs 1";             break;
    case WM_REALMODE:       s = "1on1 Realmode";             break;
    case WM_HITMENREALMODE: s = "Hitmen 1 vs 1 Realmode";    break;
    default:                s = "1 vs 1";                    break;
    }
    break;
  case GT_TEAM:
    switch (weaponmod)
    {
    case WM_HITMEN:         s = "Team Hitmen";               break;
    case WM_REALMODE:       s = "Team Deathmatch Realmode";  break;
    case WM_HITMENREALMODE: s = "Team Hitmen Realmode";      break;
    default:                s = "Team Deathmatch";           break;
    }
    break;
  case GT_CTF:
    switch (weaponmod)
    {
    case WM_HITMEN:         s = "Capture The Flag - Hitmen";          break;
    case WM_REALMODE:       s = "Capture The Flag - Realmode";        break;
    case WM_HITMENREALMODE: s = "Capture The Flag - Hitmen Realmode"; break;
    default:                s = "Capture The Flag";                   break;
    }
    break;
#ifdef WITH_BAGMAN_MOD
  case GT_BAGMAN:
    switch (weaponmod)
    {
    case WM_HITMEN:         s = "Bagman - Hitmen";           break;
    case WM_REALMODE:       s = "Bagman - Realmode";         break;
    case WM_HITMENREALMODE: s = "Bagman - Hitmen Realmode";  break;
    default:                s = "Bagman";                    break;
    }
    break;
#endif // WITH_BAGMAN_MOD
  case GT_1FCTF:
    switch (weaponmod)
    {
    case WM_HITMEN:         s = "One Flag CTF - Hitmen";           break;
    case WM_REALMODE:       s = "One Flag CTF - Realmode";         break;
    case WM_HITMENREALMODE: s = "One Flag CTF - Hitmen Realmode";  break;
    default:                s = "One Flag CTF";                    break;
    }
    break;
    break;
  default:
    s = "Unknown Gametype";
    break;
  }
  return s;
}
/*
====================
CG_DrawInformation
Draw all the status / pacifier stuff during level loading
====================
*/
void CG_DrawInformation(void)
{
  const char *s = NULL;
  const char *info;
  const char *sysInfo;
  int x, y; //w;
  int value;
  qboolean hasLevelshot = qtrue;
  char buf[1024];
    int y_offset;
//	char st[1024];
  qhandle_t levelshot;
  VectorCopy(colortmRed, colorLines);

  info    = CG_ConfigString(CS_SERVERINFO);
  sysInfo = CG_ConfigString(CS_SYSTEMINFO);

  s         = Info_ValueForKey(info, "mapname");
  levelshot = trap_R_RegisterShaderNoMip(va("levelshots/%s.png", s));
  if (!levelshot)
  {
    levelshot    = trap_R_RegisterShaderNoMip("unknownmap");
    hasLevelshot = qfalse;
  }
  (void)hasLevelshot; //shutup compiler
  CG_DrawPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, cgs.media.menuback);

  //lines
  CG_DrawRect(0, 218, 640, 20, 1, colorLines);
  CG_DrawRect(440, 0, 20, 480, 1, colorLines);

  CG_DrawRect(16, 0, 1, 480, 1, colorLines);

  //mapshot
  trap_R_SetColor(NULL);
  CG_DrawPic(320, 140, 240, 160, levelshot);

  // blend a detail texture over it
  CG_DrawPic(320, 140, 240, 160, cgs.media.detail);

  // draw the cg.progress
  CG_DrawProgress();

  // server-specific message of the day
  s = CG_ConfigString(CS_MOTD);
  if (s[0])
  {
    CG_Text_PaintAligned(320, 110, va("%s",s), 0.2f, UI_CENTER | UI_DROPSHADOW, text_color_normal, &(cgs.media.freeSansBoldFont));
  }

  // draw info string information
  y        = 320;
  y_offset = 14;
  x        = 340;

  // don't print server lines if playing a local game
  trap_Cvar_VariableStringBuffer("sv_running", buf, sizeof(buf));
  if (!atoi(buf))
  {
    // server hostname
    Q_strncpyz(buf, Info_ValueForKey(info, "sv_hostname"), 1024);
    Q_CleanStr(buf);
    //s = va("%s", buf);
    CG_Text_PaintAligned(x, y, va("%s", buf), 0.2f, UI_DROPSHADOW, text_color_normal, &(cgs.media.freeSansBoldFont));
    y += y_offset;

    // pure server
    s = Info_ValueForKey(sysInfo, "sv_pure");
    if (s[0] == '1')
    {
      CG_Text_PaintAligned(x, y, "Pure Server", 0.2f, UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansBoldFont);
      y += y_offset;
    }
  }

  // map-specific message (long map name)
  s = CG_ConfigString(CS_MESSAGE);
  if (s[0])
  {
    CG_Text_PaintAligned(x, y, va("%s", s), 0.2f, UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansBoldFont);
    y += y_offset;
  }

  // cheats warning
  s = Info_ValueForKey(sysInfo, "sv_cheats");
  if (s[0] == '1')
  {
    int w;
    size_t len;
    static char* cheats = "Cheats are enabled";
    CG_Text_PaintAligned(x, y, cheats, 0.2f, UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansBoldFont);
    w = CG_Text_Width2(cheats, .2f, 0, &cgs.media.freeSansBoldFont);
    len = qstrlen(cheats);
    if (len)
    {
      CG_Text_PaintAligned((int)(x - (w/len) - 1), y + 1, ">", 0.2f, 0, text_color_warning, &cgs.media.freeSansBoldFont);
      CG_Text_PaintAligned(x + w, y + 1, "<", 0.2f, 0, text_color_warning, &cgs.media.freeSansBoldFont);
    }
    //FIXME(0xA5EA): < > don't fit cause CG_Text_Width uses scale for selecting font
    y += y_offset;
  }

  // game type
  s = CG_GetGamemodeText(cgs.gametype, cgs.weaponmod);

  CG_Text_PaintAligned(x, y, va("%s", s), 0.2f, UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansBoldFont);
  y += y_offset;

  value = atoi(Info_ValueForKey(info, "g_timelimit")); //hypov8 server/mplayer setting
  if (value)
  {
    CG_Text_PaintAligned(x, y, va("Timelimit %i", value), 0.2f, UI_DROPSHADOW, text_color_normal,
                         &cgs.media.freeSansBoldFont);
    y += y_offset;
  }

  if (cgs.gametype < GT_CTF)
  {
    value = atoi(Info_ValueForKey(info, "g_fraglimit")); //hypov8 "fraglimit"
    if (value)
    {
      CG_Text_PaintAligned(x, y, va("Fraglimit %i", value), 0.2f, UI_DROPSHADOW, text_color_normal,
                           &cgs.media.freeSansBoldFont);
      y += y_offset;
    }
  }
  else if (cgs.gametype == GT_CTF)
  {
    value = atoi(Info_ValueForKey(info, "g_capturelimit")); //hypov8 "capturelimit
    if (value)
    {
      CG_Text_PaintAligned(x, y, va("Capturelimit %i", value), 0.2f, UI_DROPSHADOW, text_color_normal,
                           &cgs.media.freeSansBoldFont);
      y += y_offset;
    }
  }
#ifdef WITH_BAGMAN_MOD
  else if (cgs.gametype == GT_BAGMAN)
  {
    value = atoi(Info_ValueForKey(info, "g_cashlimit")); //hypov8 "cashlimit"
    if (value)
    {
      CG_Text_PaintAligned(x, y, va("Cashlimit %i", value), 0.2f, UI_DROPSHADOW, text_color_normal,
                           &cgs.media.freeSansBoldFont);
      y += y_offset;
    }
  }
#endif // WITH_BAGMAN_MOD
}
