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
// cg_drawtools.c -- helper functions called by cg_draw, cg_scoreboard, cg_info, etc
#include "cg_local.h"

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640(float *x, float *y, float *w, float *h)
{
#if 0       // ist nicht von mir !
  // adjust for wide screens
  if (cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640)
  {
    *x += 0.5 * (cgs.glconfig.vidWidth - (cgs.glconfig.vidHeight * 640 / 480));
  }
#endif
  // scale for screen sizes
  *x *= cgs.screenXScale;
  *y *= cgs.screenYScale;
  *w *= cgs.screenXScale;
  *h *= cgs.screenYScale;
}

/*
================
CG_FillRect
Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect(float x, float y, float width, float height, const float *color)
{
  trap_R_SetColor(color);

  CG_AdjustFrom640(&x, &y, &width, &height);
  trap_R_DrawStretchPic(x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader);

  trap_R_SetColor(NULL);
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides(float x, float y, float w, float h, float size)
{
  CG_AdjustFrom640(&x, &y, &w, &h);
  size *= cgs.screenXScale;
  trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
  trap_R_DrawStretchPic(x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size)
{
  CG_AdjustFrom640(&x, &y, &w, &h);
  size *= cgs.screenYScale;
  trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
  trap_R_DrawStretchPic(x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
}
/*
================
UI_DrawRect
Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect(float x, float y, float width, float height, float size, const float *color)
{
  trap_R_SetColor(color);

  CG_DrawTopBottom(x, y, width, height, size);
  CG_DrawSides(x, y, width, height, size);

  trap_R_SetColor(NULL);
}

/*
================
CG_DrawPic
Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPic(float x, float y, float width, float height, qhandle_t hShader)
{
  CG_AdjustFrom640(&x, &y, &width, &height);
  trap_R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}

/*
===============
CG_DrawChar
Coordinates and size in 640*480 virtual screen size
===============
*/
void CG_DrawChar(int x, int y, int width, int height, int ch)
{
  int row, col;
  float frow, fcol;
  float size;
  float ax, ay, aw, ah;

  ch &= 255;

  if (ch == ' ')
    return;

  ax = x;
  ay = y;
  aw = width;
  ah = height;
  CG_AdjustFrom640(&ax, &ay, &aw, &ah);

  row = ch >> 4;
  col = ch & 15;
#define FUCK
#ifdef FUCK
  frow = row * 0.0625;
  fcol = col * 0.0625;
  size = 0.0625;
#else
  frow = row * 0.0625;
  fcol = col * 0.0625;
  size = 0.0625;
#endif

  trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + size, frow + size, cgs.media.charsetShader);
}

/*
==================
CG_DrawStringExt
Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.
Coordinates are at 640 by 480 virtual resolution
==================
*/
void CG_DrawStringExt(int x, int y, const char *string, const float *setColor, qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars)
{
  vec4_t color;
  const char *s;
  int xx;
  int cnt;

  if (maxChars <= 0)
    maxChars = 32767;                                                                               // do them all!

  // draw the drop shadow
  if (shadow)
  {
    color[0] = color[1] = color[2] = 0;
    color[3] = setColor[3];
    trap_R_SetColor(color);
    s   = string;
    xx  = x;
    cnt = 0;
	if (!s){ return; } //add hypov8 'Dereferncing null pointer'

    while (*s && cnt < maxChars)
    {
      if (Q_IsColorString(s))
      {
        s += 2;
        continue;
      }
      CG_DrawChar(xx + 2, y + 2, charWidth, charHeight, *s);
      cnt++;
      //xx += charWidth;
      xx += charWidth - 2;                /* 0xA5EA, enger schreiben */
      s++;
    }
  }
  else //add hypov8 nonshadow text
  {
  // draw the colored text
  s   = string;
  xx  = x;
  cnt = 0;
  trap_R_SetColor(setColor);
 // if (s) 
   if (!s){  return;  } //add hypov8 'Dereferncing null pointer'

  while (*s && cnt < maxChars)
  {
    if (Q_IsColorString(s))
    {
      if (!forceColor)
      {
        Com_Memcpy(color, g_color_table[ColorIndex(*(s + 1))], sizeof(color));
        color[3] = setColor[3];
        trap_R_SetColor(color);
      }
      s += 2;
      continue;
    }
    CG_DrawChar(xx, y, charWidth, charHeight, *s);
    //xx += charWidth;
    xx += charWidth - 2;                /* 0xA5EA, enger schreiben */
    cnt++;
    s++;
  }
	 
  }
  trap_R_SetColor(NULL);
}

void CG_DrawBigString(int x, int y, const char *s, float alpha)
{
  float color[4];

  color[0] = color[1] = color[2] = 1.0;
  color[3] = alpha;
  CG_DrawStringExt(x, y, s, color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0);
}

void CG_DrawBigStringColor(int x, int y, const char *s, vec4_t color)
{
  CG_DrawStringExt(x, y, s, color, qtrue, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0);
}

void CG_DrawSmallString(int x, int y, const char *s, float alpha)
{
  float color[4];

  color[0] = color[1] = color[2] = 1.0;
  color[3] = alpha;
  CG_DrawStringExt(x, y, s, color, qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
}

void CG_DrawSmallStringColor(int x, int y, const char *s, vec4_t color)
{
  CG_DrawStringExt(x, y, s, color, qtrue, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
}

/*
=================
CG_DrawStrlen

Returns character count, skiping color escape codes
=================
*/
int CG_DrawStrlen(const char *str)
{
  const char *s = str;
  int count     = 0;

  while (*s)
  {
    if (Q_IsColorString(s))
    {
      s += 2;
    }
    else
    {
      count++;
      s++;
    }
  }

  return count;
}

/*
=============
CG_TileClearBox

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void CG_TileClearBox(int x, int y, int w, int h, qhandle_t hShader)
{
  float s1, t1, s2, t2;

  s1 = x / 64.0;
  t1 = y / 64.0;
  s2 = (x + w) / 64.0;
  t2 = (y + h) / 64.0;
  trap_R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, hShader);
}

/*
==============
CG_TileClear
Clear around a sized down screen
==============
*/
void CG_TileClear(void)
{
  int top, bottom, left, right;
  int w, h;

  w = cgs.glconfig.vidWidth;
  h = cgs.glconfig.vidHeight;

  if (cg.refdef.x == 0 && cg.refdef.y == 0 && cg.refdef.width == w && cg.refdef.height == h)
    return;     // full screen rendering

  top    = cg.refdef.y;
  bottom = top + cg.refdef.height - 1;
  left   = cg.refdef.x;
  right  = left + cg.refdef.width - 1;

  // clear above view screen
  CG_TileClearBox(0, 0, w, top, cgs.media.backTileShader);

  // clear below view screen
  CG_TileClearBox(0, bottom, w, h - bottom, cgs.media.backTileShader);

  // clear left of view screen
  CG_TileClearBox(0, top, left, bottom - top + 1, cgs.media.backTileShader);

  // clear right of view screen
  CG_TileClearBox(right, top, w - right, bottom - top + 1, cgs.media.backTileShader);
}

/*
================
CG_FadeColor
================
*/
float *CG_FadeColor(int startMsec, int totalMsec)
{
  static vec4_t color;
  int t;

  if (startMsec == 0)
    return NULL;

  t = cg.time - startMsec;

  if (t >= totalMsec)
    return NULL;

  // fade out
  if (totalMsec - t < FADE_TIME)
    color[3] = (totalMsec - t) * 1.0 / FADE_TIME;
  else
    color[3] = 1.0;

  color[0] = color[1] = color[2] = 1;

  return color;
}

/*
================
CG_TeamColor
================
*/
float *CG_TeamColor(int team)
{
  static vec4_t red      = {1, 0.2f, 0.2f, 1};
  //static vec4_t blue     = {0.2f, 0.2f, 1, 1};
  static vec4_t other    = {1, 1, 1, 1};
  static vec4_t spectator = {0.7f, 0.7f, 0.7f, 1};

  switch (team)
  {
  case TEAM_DRAGONS:
    return red;
  case TEAM_NIKKIS:
    return colortmgreenyellow;
  case TEAM_SPECTATOR:
    return spectator;
  default:
    return other;
  }
}

/*
=================
CG_GetColorForHealth
=================
*/
void CG_GetColorForHealth(int health, int armor, vec4_t hcolor)
{
  int count;
  int max;

  // calculate the total points of damage that can
  // be sustained at the current health / armor level
  if (health <= 0)
  {
    VectorClear(hcolor); // black
    hcolor[3] = 1;
    return;
  }
  //count = armorHead + armorBody + armorLegs;
  count = armor;
  //FIXME (0xA5EA): passt das mit dem armor
  max   = health * ARMOR_PROTECTION / (1.0 - ARMOR_PROTECTION);
  if (max < count)
    count = max;

  health += count;

  // set the color based on health
  hcolor[0] = 1.0;
  hcolor[3] = 1.0;
  if (health >= 100)
    hcolor[2] = 1.0;
  else if (health < 66)
    hcolor[2] = 0;
  else
    hcolor[2] = (health - 66) / 33.0;

  if (health > 60)
    hcolor[1] = 1.0;
  else if (health < 30)
    hcolor[1] = 0;
  else
    hcolor[1] = (health - 30) / 30.0;
}

/*
=================
CG_ColorForHealth
=================
*/
void CG_ColorForHealth(vec4_t hcolor)
{
  int armor = cg.snap->ps.stats[STAT_ARMOR_HEAD];
  armor += cg.snap->ps.stats[STAT_ARMOR_BODY];
  armor += cg.snap->ps.stats[STAT_ARMOR_LEGS];
  armor /= 3;

  CG_GetColorForHealth(cg.snap->ps.stats[STAT_HEALTH], armor, hcolor);
}
