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
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"
#include "../ui/ui_shared.h"

// used for scoreboard
extern displayContextDef_t cgDC;
menuDef_t *menuScoreboard = NULL;

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int numSortedTeamPlayers;

char systemChat[256];
char teamChat1[256];
char teamChat2[256];

int CG_Text_Width2(const char *text, float scale, int limit, fontInfo_t *Font)
{
  size_t count, len;
  float out;
  glyphInfo_t *glyph;
  float useScale;
// FIXME: see ui_main.c, same problem
//  const unsigned char *s = text;
  const char *s    = text;
  fontInfo_t *font = Font;

  if (font == NULL_FONT)
    return 0;

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

int CG_Text_Height2(const char *text, float scale, int limit, fontInfo_t *Font)
{
  size_t len, count;
  float max;
  glyphInfo_t *glyph;
  float useScale;
  fontInfo_t *font = Font;

  const char *s = text;

  if (font == NULL_FONT)
    return 0;

  useScale = scale * font->glyphScale;
  max      = 0;
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

int CG_Text_Width(const char *text, float scale, int limit, fontInfo_t *Font)
{
  size_t count, len;
  float out;
  glyphInfo_t *glyph;
  float useScale;
// FIXME: see ui_main.c, same problem
//	const unsigned char *s = text;
  const char *s    = text;
  fontInfo_t *font = Font;

  if (font == NULL_FONT)
    font = &cgDC.Assets.textFont;
  if (scale <= cg_fontTiny.value)
    font = &cgDC.Assets.tinyFont;
  else if (scale <= cg_fontSmall.value)
    font = &cgDC.Assets.smallFont;
  else if (scale > cg_fontHuge.value)
    font = &cgDC.Assets.hugeFont;
  else if (scale > cg_fontBig.value)
    font = &cgDC.Assets.bigFont;

  useScale = scale * font->glyphScale;
  out      = 0;
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
        glyph = &font->glyphs[(int)*s];
        out  += glyph->xSkip;
        s++;
        count++;
      }
    }
  }
  return out * useScale;
}

/*
================
CG_Text_Height
================
*/
int CG_Text_Height(const char *text, float scale, int limit, fontInfo_t *Font)
{
  size_t len, count;
  float max;
  glyphInfo_t *glyph;
  float useScale;
  fontInfo_t *font = Font;
// TTimo: FIXME
//	const unsigned char *s = text;
  const char *s = text;

  if (font == NULL_FONT)
  {
    font = &cgDC.Assets.textFont;
    if (scale <= cg_fontTiny.value)
      font = &cgDC.Assets.tinyFont;
    else if (scale <= cg_fontSmall.value)
      font = &cgDC.Assets.smallFont;
    else if (scale > cg_fontHuge.value)
      font = &cgDC.Assets.hugeFont;
    else if (scale > cg_fontBig.value)
      font = &cgDC.Assets.bigFont;
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
        glyph = &font->glyphs[(int)*s];

        if (max < glyph->height)
          max = glyph->height;

        s++;
        count++;
      }
    }
  }
  return max * useScale;
}

/*
================
CG_Text_PaintChar
================
*/
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader)
{
  float w, h;

  w = width * scale;
  h = height * scale;
  CG_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

/*
================
CG_Text_Paint
================
*/
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style
                   , fontInfo_t *Font)
{
  size_t len, count;
  vec4_t newColor;
  glyphInfo_t *glyph;
  float useScale;
  fontInfo_t *font = Font;

  if (font == NULL_FONT)
  {
    font = &cgDC.Assets.textFont;
    if (scale <= cg_fontTiny.value)
      font = &cgDC.Assets.tinyFont;
    else if (scale <= cg_fontSmall.value)
      font = &cgDC.Assets.smallFont;
    else if (scale > cg_fontHuge.value)
      font = &cgDC.Assets.hugeFont;
    else if (scale > cg_fontBig.value)
      font = &cgDC.Assets.bigFont;
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
      glyph = &font->glyphs[(int)*s];
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
          CG_Text_PaintChar(x + ofs, y - yadj + ofs,
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
        CG_Text_PaintChar(x, y - yadj,
                          glyph->imageWidth,
                          glyph->imageHeight,
                          useScale,
                          glyph->s,
                          glyph->t,
                          glyph->s2,
                          glyph->t2,
                          glyph->glyph);
        // CG_DrawPic(x, y - yadj, scale * cgDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * cgDC.Assets.textFont.glyphs[text[i]].imageHeight, cgDC.Assets.textFont.glyphs[text[i]].glyph);
        x += (glyph->xSkip * useScale) + adjust;
        s++;
        count++;
      }
    }
    trap_R_SetColor(NULL);
  }
}

/*
================
CG_Draw3DModel
hypov8 todo: remove this "feature"
================
*/
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles)
{
  refdef_t refdef;
  refEntity_t ent;
  refLight_t light;
  float fov_x;

  if (!cg_draw3dIcons.integer || !cg_drawIcons.integer)
    return;

  CG_AdjustFrom640(&x, &y, &w, &h);

  Com_Memset(&refdef, 0, sizeof(refdef));
  Com_Memset(&ent, 0, sizeof(ent));

  AnglesToAxis(angles, ent.axis);

  VectorCopy(origin, ent.origin);

  ent.hModel     = model;
  ent.customSkin = skin;
  ent.renderfx   = RF_NOSHADOW;     // no stencil shadows

  refdef.rdflags = RDF_NOWORLDMODEL | RDF_NOSHADOWS;

  AxisClear(refdef.viewaxis);

  refdef.fov_x = 30;
  refdef.fov_y = 30;

  refdef.x      = x;
  refdef.y      = y;
  refdef.width  = w;
  refdef.height = h;

  refdef.time = cg.time;

  trap_R_ClearScene();
  trap_R_AddRefEntityToScene(&ent);

// add light
  Com_Memset(&light, 0, sizeof(refLight_t));

  light.rlType = RL_PROJ;
#ifdef BUG_FIXED
  VectorMA(refdef.vieworg, -200, refdef.viewaxis[0], light.origin);
#else
  VectorCopy(cg.refdef.vieworg, light.origin);
#endif

  light.origin[1] += 10;
#ifdef BUG_FIXED
  QuatFromAngles(light.rotation, 0, 5, 0);
#else
  QuatClear(light.rotation);
#endif

  light.color[0] = 0.8f;
  light.color[1] = 0.8f;
  light.color[2] = 0.8f;
#if 0 // FIXME: 0xA5EA
  light.fovX     = 90;
  light.fovY     = 90;
  light.distNear = 1;
  light.distFar = 500;
#else
#ifdef BUG_FIXED
  VectorSet(light.projTarget, 500, 0, 0);
  VectorSet(light.projRight, 100, 0, 0);
  VectorSet(light.projUp, 100, 0, 0);
#else
#ifndef Q3_VM
  fov_x = tanf((cg.refdef.fov_x * 0.5f)*M_DEG2RAD);
#else
  fov_x = tan((cg.refdef.fov_x * 0.5f)*DEG2RAD);
#endif
  VectorCopy(cg.refdef.viewaxis[0], light.projTarget);
  VectorScale(cg.refdef.viewaxis[1], -fov_x, light.projRight);
  VectorScale(cg.refdef.viewaxis[2], fov_x, light.projUp);
  VectorScale(cg.refdef.viewaxis[0], -200, light.projStart);
  VectorScale(cg.refdef.viewaxis[0], 1000, light.projEnd);
#endif
#endif
 // trap_R_AddRefLightToScene(&light); //hypov8 removed light from models
  trap_R_RenderScene(&refdef);
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles)
{
  clipHandle_t cm;
  clientInfo_t *ci;
  float len;
  vec3_t origin;
  vec3_t mins, maxs;

  ci = &cgs.clientinfo[clientNum];

  if (cg_draw3dIcons.integer)
  {
    cm = ci->headModel;
    if (!cm)
      return;

    // offset the origin y and z to center the head
    trap_R_ModelBounds(cm, mins, maxs);

    origin[2] = -0.5 * (mins[2] + maxs[2]);
    origin[1] = 0.5 * (mins[1] + maxs[1]);

    // calculate distance so the head nearly fills the box
    // assume heads are taller than wide
    len       = 0.7 * (maxs[2] - mins[2]);
    origin[0] = len / 0.268;    // len / tan( fov/2 )

    // allow per-model tweaking
    VectorAdd(origin, ci->headOffset, origin);

    CG_Draw3DModel(x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles);
  }
  else if (cg_drawIcons.integer)
    CG_DrawPic(x, y, w, h, ci->modelIcon);

  // if they are deferred, draw a cross out
  if (ci->deferred)
    CG_DrawPic(x, y, w, h, cgs.media.deferShader);
}

/*
================
CG_DrawFlagModel

Used for both the status bar and the scoreboard
================
*/
void CG_DrawFlagModel(float x, float y, float w, float h, int team, qboolean force2D)
{
  qhandle_t cm;
  float len;
  vec3_t origin, angles;
  vec3_t mins, maxs;
  qhandle_t handle;

  if (!force2D && cg_draw3dIcons.integer)
  {
    VectorClear(angles);

    cm = cgs.media.dragonFlagModel;

    // offset the origin y and z to center the flag
    trap_R_ModelBounds(cm, mins, maxs);

    origin[2] = -0.5 * (mins[2] + maxs[2]);
    origin[1] = 0.5 * (mins[1] + maxs[1]);

    // calculate distance so the flag nearly fills the box
    // assume heads are taller than wide
    len       = 0.5 * (maxs[2] - mins[2]);
    origin[0] = len / 0.268;    // len / tan( fov/2 )

    angles[YAW] = 60 * sin(cg.time / 2000.0);

    if (team == TEAM_DRAGONS)
      handle = cgs.media.dragonFlagModel;
    else if (team == TEAM_NIKKIS)
      handle = cgs.media.nikkiFlagModel;
    else if (team == TEAM_FREE)
      handle = cgs.media.neutralFlagModel;
    else
      return;

    CG_Draw3DModel(x, y, w, h, handle, 0, origin, angles);
  }
  else if (cg_drawIcons.integer)
  {
    gitem_t *item;

    if (team == TEAM_DRAGONS)
      item = BG_FindItemForPowerup(PW_DRAGONFLAG);
    else if (team == TEAM_NIKKIS)
      item = BG_FindItemForPowerup(PW_NIKKIFLAG);
    else if (team == TEAM_FREE)
      item = BG_FindItemForPowerup(PW_NEUTRALFLAG);
    else
      return;

    if (item)
      CG_DrawPic(x, y, w, h, cg_items[ITEM_INDEX(item)].icon);
  }
}

/*
================
CG_DrawTeamBackground
================
*/
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team)
{
  vec4_t hcolor;

  hcolor[3] = alpha;
  //FIXME(0xA5EA): better background colors for team background
  if (team == TEAM_DRAGONS)
  {
    hcolor[0] = 1;
    hcolor[1] = 0;
    hcolor[2] = 0;
  }
  else if (team == TEAM_NIKKIS)
  {
    hcolor[0] = 0;
    hcolor[1] = 0;
    hcolor[2] = 1;
  }
  else
    return;

  trap_R_SetColor(hcolor);
  CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
  trap_R_SetColor(NULL);
}

/*
===========================================================================================
  UPPER RIGHT CORNER
===========================================================================================
*/
/*
================
CG_DrawAttacker
================
*/
static float CG_DrawAttacker(float y)
{
  int t, w, h;
  float size;
  float scale = .3f;
  vec3_t angles;
  const char *info;
  const char *name;
  int clientNum;
  shortbytes_t attacker;

  if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
    return y;

  if (!cg.attackerTime)
    return y;

  attacker.s = cg.predictedPlayerState.persistant[PERS_ATTACKER_INFO];
  clientNum  = attacker.b[PERS_ATTACKER_POS];

  if (clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum)
    return y;

  t = cg.time - cg.attackerTime;
  if (t > ATTACKER_HEAD_TIME)
  {
    cg.attackerTime = 0;
    return y;
  }

  size = ICON_SIZE* 1.25;
  angles[PITCH] = 0;
  angles[YAW]   = 180;
  angles[ROLL]  = 0;
  CG_DrawHead(640 - size, y, size, size, clientNum, angles);

  info = CG_ConfigString(CS_PLAYERS + clientNum);
  name = Info_ValueForKey(info, "n");
  y   += size;

  w = CG_Text_Width(name, scale, 0, NULL_FONT);
  h = CG_Text_Height(name, scale, 0, NULL_FONT);
  y += h;
  CG_Text_Paint(640 - size/2 - w/2, y, scale, colorWhite, name, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);

  return y + h + 2;
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot(float y)
{
  char *s;
  int w;

  s = va("time:%i snap:%i cmd:%i", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence);
  w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

  CG_DrawBigString(635 - w, y + 2, s, 1.0F);

  return y + BIGCHAR_HEIGHT + 4;
}

#define OLD_DRAW_FUNCTION
/*
=================
CG_DrawTeamOverlay
=================
*/
static float CG_DrawTeamOverlay(float y, qboolean right, qboolean upper)
{
  int x, w, h, xx;
  int i, j, len;
  const char *p;
  vec4_t hcolor;
  int pwidth, lwidth;
  int plyrs;
  char st[16];
  clientInfo_t *ci;
  gitem_t *item;
  int ret_y, count;
  //vec_t scale = .175f;
  //fontInfo_t* p_font = &cgs.media.consoleFont;
  // int text_style = 0; // 6 = shadowed more
  //int char_width  = CG_Text_Width2("N", scale, 0, p_font);
  //int char_heigth = CG_Text_Height2("N", scale, 0, p_font);

  if (!cg_drawTeamOverlay.integer)
    return y;

  if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_DRAGONS && cg.snap->ps.persistant[PERS_TEAM] != TEAM_NIKKIS)
    return y; // Not on any team

  plyrs = 0;

  // max player name width
  pwidth = 0;
  count  = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
  for (i = 0; i < count; i++)
  {
    ci = cgs.clientinfo + sortedTeamPlayers[i];
    if (ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM])
    {
      plyrs++;
#ifdef OLD_DRAW_FUNCTION
      len = CG_DrawStrlen(ci->name);
#else
      len = CG_Text_Width2(ci->name, scale, 0, p_font);
#endif
      if (len > pwidth)
        pwidth = len;
    }
  }

  if (!plyrs)
    return y;

  if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
    pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

  // max location name width
  lwidth = 0;
  for (i = 1; i < MAX_LOCATIONS; i++)
  {
    p = CG_ConfigString(CS_LOCATIONS + i);
    if (p && *p)
    {
#ifdef OLD_DRAW_FUNCTION
      len = CG_DrawStrlen(p);
#else
      len = CG_Text_Width2(p, scale, 0, p_font);
#endif
      if (len > lwidth)
        lwidth = len;
    }
  }

  if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
    lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;
#ifdef OLD_DRAW_FUNCTION
  w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;
#else
  //w = (pwidth + lwidth + 4 + 7) * char_width;
  w = (pwidth + lwidth) * char_width;
#endif

  if (right)
    x = 640 - w;
  else
    x = 0;
#ifdef OLD_DRAW_FUNCTION
  h = plyrs * TINYCHAR_HEIGHT;
#else
  h = plyrs *(char_heigth + 1);
#endif

  if (upper)
  {
    ret_y = y + h;
  }
  else
  {
    y    -= h;
    ret_y = y;
  }

  if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_DRAGONS)
  {
    hcolor[0] = 1.0f;
    hcolor[1] = 0.0f;
    hcolor[2] = 0.0f;
    hcolor[3] = 0.33f;
  }
  else   // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_NIKKIS )
  {
    hcolor[0] = 154.f/255.f;
    hcolor[1] = 154.f/255.f;
    hcolor[2] =  77.f/255.f;
    hcolor[3] = 0.55f;
  }
  trap_R_SetColor(hcolor);
  CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
  trap_R_SetColor(NULL);

  for (i = 0; i < count; i++)
  {
    ci = cgs.clientinfo + sortedTeamPlayers[i];
    if (ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM])
    {
      hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

#ifdef OLD_DRAW_FUNCTION
      xx = x + TINYCHAR_WIDTH;
      CG_DrawStringExt(xx, y,
                       ci->name, hcolor, qfalse, qfalse,
                       TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);
#else
      xx = x + char_width;
      CG_Text_Paint(xx, y, scale, hcolor, ci->name, 0, 0, text_style, p_font);
#endif

      if (lwidth)
      {
        p = CG_ConfigString(CS_LOCATIONS + ci->location);
        if (!p || !*p)
          p = "unknown";
#ifdef OLD_DRAW_FUNCTION
        len = CG_DrawStrlen(p);
#else
        len = CG_Text_Width2(p, scale, 0, p_font);
#endif
        if (len > lwidth)
          len = lwidth;

//				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth +
//					((lwidth/2 - len/2) * TINYCHAR_WIDTH);

#ifdef OLD_DRAW_FUNCTION
        xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
        CG_DrawStringExt(xx, y,
                         p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
                         TEAM_OVERLAY_MAXNAME_WIDTH);
#else
        xx  += CG_Text_Width2(p, scale, lwidth, p_font);
        CG_Text_Paint(xx, y, scale, hcolor, p, 0, 0, text_style, p_font);
#endif
      }
      CG_GetColorForHealth(ci->health, ci->armor, hcolor);

      Com_sprintf(st, sizeof(st), "%3i %3i", ci->health,  ci->armor);
#ifdef OLD_DRAW_FUNCTION
      xx = x + TINYCHAR_WIDTH * 3 +
           TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

      CG_DrawStringExt(xx, y,
                       st, hcolor, qfalse, qfalse,
                       TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
#else
      xx  += CG_Text_Width2(st, scale, 0, p_font);
      CG_Text_Paint(xx, y, scale, hcolor, p, 0, 0, text_style, p_font);
#endif
#ifdef OLD_DRAW_FUNCTION
      // draw weapon icon
      xx += TINYCHAR_WIDTH * 3;
#else
      xx += char_width * 3;
#endif


      if (cg_weapons[ci->curWeapon].weaponIcon)
      {
#ifdef OLD_DRAW_FUNCTION
        CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
                   cg_weapons[ci->curWeapon].weaponIcon);
#else
        CG_DrawPic(xx, y, char_width, char_heigth,
                   cg_weapons[ci->curWeapon].weaponIcon);
#endif
      }
      else
      {

#ifdef OLD_DRAW_FUNCTION
        CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, cgs.media.deferShader);
#else
        CG_DrawPic(xx, y, char_width, char_heigth, cgs.media.deferShader);;
#endif
      }

      // Draw powerup icons
      //FIXME(0xA5EA): this should work for cooling mod
      if (right)
        xx = x;
      else
      {
#ifdef OLD_DRAW_FUNCTION
        xx = x + w - TINYCHAR_WIDTH;
#else
        xx = x + w - char_width;
#endif
      }
      for (j = 0; j <= PW_NUM_POWERUPS; j++)
      {
        if (ci->powerups & (1 << j))
        {
          item = BG_FindItemForPowerup((powerup_t)j);

          if (item)
          {
#ifdef OLD_DRAW_FUNCTION
            CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
                       trap_R_RegisterShader(item->icon));
#else
            CG_DrawPic(xx, y, char_width, char_heigth,
                       trap_R_RegisterShader(item->icon));
#endif
#ifdef OLD_DRAW_FUNCTION
            if (right)
              xx -= TINYCHAR_WIDTH;
            else
              xx += TINYCHAR_WIDTH;
#else
            if (right)
              xx -= char_width;
            else
              xx += char_width;
#endif
          }
        }
      }
#ifdef OLD_DRAW_FUNCTION
      y += TINYCHAR_HEIGHT;
#else
      y += char_heigth;
#endif
    }
  }
  return ret_y;
}


/*
=====================
CG_DrawUpperRight
=====================
*/
static void CG_DrawUpperRight(void)
{
  float y = 30;

  if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1)
    y = CG_DrawTeamOverlay(y, qtrue, qtrue);

  if (cg_drawSnapshot.integer)
    y = CG_DrawSnapshot(y);

  //FIXME(0xA5EA): fps draw wtf?
#if 0
  if (cg_drawFPS.integer)
  {
    y = CG_DrawFPS(y);
  }

 //0xA5EA: warum war das null ?
  if (cg_drawTimer.integer)
  {
    y = CG_DrawTimer(y);
  }
#endif
  if (cg_drawAttacker.integer)
  {
    y = CG_DrawAttacker(y);
  }

}

/*
===========================================================================================
  LOWER RIGHT CORNER
===========================================================================================
*/
//===========================================================================================
/*
=================
CG_DrawTeamInfo
=================
*/
#define CHATLOC_Y 380
#define CHATLOC_X 0
static void CG_DrawTeamInfo(gametype_t GameType)
{
  int w, i, len;
  int chatHeight;
  float chatScale = .20f;

  if (cg_ChatHeight.integer < TEAMCHAT_HEIGHT)
    chatHeight = cg_ChatHeight.integer;
  else
    chatHeight = TEAMCHAT_HEIGHT;

  if (chatHeight <= 0)
    return;			 // chat is disabled

  if (cgs.teamLastChatPos != cgs.teamChatPos)
  {
    if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_ChatTime.integer)
      cgs.teamLastChatPos++;

    //h = (cgs.teamChatPos - cgs.teamLastChatPos) * TINYCHAR_HEIGHT;
    //lh = CG_Text_Height(cgs.teamChatMsgs[cgs.teamChatPos%chatHeight], chatScale, 0, NULL_FONT);
    //h = (cgs.teamChatPos - cgs.teamLastChatPos) * lh;

    w = 0;

    for (i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++)
    {
      //len = CG_DrawStrlen(cgs.teamChatMsgs[i % chatHeight]);
      len = CG_Text_Width( cgs.teamChatMsgs[i % chatHeight], chatScale, 0, 0 );
      if (len > w)
        w = len;
    }
    //w *= TINYCHAR_WIDTH;
    //w += TINYCHAR_WIDTH * 2;

    for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--)
    {
      CG_Text_Paint(CHATLOC_X + 4, CHATLOC_Y - (cgs.teamChatPos - i) * 11,
                    chatScale, colorWhite, cgs.teamChatMsgs[i % chatHeight],
                    0, 0, 0, &cgs.media.consoleFont  );
      // &cgDC.Assets.chatFont cgs.media.freeSansFont

    }
  }
}
/*
=====================
CG_DrawChat
draws
added by 0xA5EA
=====================
*/
#if 0
static void CG_DrawChat(void)
{
  int chatHeight;

  chatHeight = CHAT_HEIGHT;

}
#endif //FIXME 0xA5EA: currently unused
/*
===================
CG_DrawHoldableItem
===================
*/
#if 0 //ndef MISSIONPACK
static void CG_DrawHoldableItem(void)
{
  int value;

  value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
  if (value)
  {
    CG_RegisterItemVisuals(value);
    CG_DrawPic(640 - ICON_SIZE, (SCREEN_HEIGHT - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE, cg_items[value].icon);
  }

}
#endif // MISSIONPACK

//#ifdef MISSIONPACK
/*
===================
CG_DrawPersistantPowerup
===================
*/
#if 0 // sos001208 - DEAD
static void CG_DrawPersistantPowerup(void)
{
  int value;

  value = cg.snap->ps.stats[STAT_PERSISTANT_POWERUP];
  if (value)
  {
    CG_RegisterItemVisuals(value);
    CG_DrawPic(640 - ICON_SIZE, (SCREEN_HEIGHT - ICON_SIZE) / 2 - ICON_SIZE, ICON_SIZE, ICON_SIZE, cg_items[value].icon);
  }
}
#endif
//#endif // MISSIONPACK


/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward(void)
{
    return;
#if 0 //hypov8
  float *color;
  int i, count;
  float x, y;
  char buf[32];

  if (!cg_drawRewards.integer) //hypov8 draw rewards. in kp!!!
    return;

  color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
  if (!color)
  {
    if (cg.rewardStack > 0)
    {
      for (i = 0; i < cg.rewardStack; i++)
      {
        cg.rewardSound[i]  = cg.rewardSound[i + 1];
        cg.rewardShader[i] = cg.rewardShader[i + 1];
        cg.rewardCount[i]  = cg.rewardCount[i + 1];
      }
      cg.rewardTime = cg.time;
      cg.rewardStack--;
      color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
      //trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
      //FIXME (0xA5EA): removed reward sound
    }
    else
      return;
  }

  trap_R_SetColor(color);

  /*
  count = cg.rewardCount[0]/10;				// number of big rewards to draw

  if (count) {
      y = 4;
      x = 320 - count * ICON_SIZE;
      for ( i = 0 ; i < count ; i++ ) {
          CG_DrawPic( x, y, (ICON_SIZE*2)-4, (ICON_SIZE*2)-4, cg.rewardShader[0] );
          x += (ICON_SIZE*2);
      }
  }

  count = cg.rewardCount[0] - count*10;		// number of small rewards to draw
  */

  if (cg.rewardCount[0] >= 10)
  {
    y = 56;
    x = 320 - ICON_SIZE / 2;
    CG_DrawPic(x, y, ICON_SIZE - 4, ICON_SIZE - 4, cg.rewardShader[0]);
    Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
    x = (SCREEN_WIDTH - SMALLCHAR_WIDTH * CG_DrawStrlen(buf)) / 2;
    CG_DrawStringExt(x, y + ICON_SIZE, buf, color, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
  }
  else
  {

    count = cg.rewardCount[0];

    y = 56;
    x = 320 - count * ICON_SIZE / 2;
    for (i = 0; i < count; i++)
    {
      CG_DrawPic(x, y, ICON_SIZE - 4, ICON_SIZE - 4, cg.rewardShader[0]);
      x += ICON_SIZE;
    }
  }
  trap_R_SetColor(NULL);
#endif
}

/*
===============================================================================
LAGOMETER
===============================================================================
*/
#define LAG_SAMPLES 128
typedef struct
{
  int frameSamples[LAG_SAMPLES];
  int frameCount;
  int snapshotFlags[LAG_SAMPLES];
  int snapshotSamples[LAG_SAMPLES];
  int snapshotCount;
} lagometer_t;

lagometer_t lagometer;

/*
==============
CG_AddLagometerFrameInfo
Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo(void)
{
  int offset;

  offset                                                           = cg.time - cg.latestSnapshotTime;
  lagometer.frameSamples[lagometer.frameCount & (LAG_SAMPLES - 1)] = offset;
  lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo
Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.
Pass NULL for a dropped packet.
==============
*/
#define PING_FRAMES 40
void CG_AddLagometerSnapshotInfo(snapshot_t *snap)
{
	static int previousPings[ PING_FRAMES ];
	static int index;
	int        i;

  // dropped packet
  if (!snap)
  {
    lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = -1;
    lagometer.snapshotCount++;
    return;
  }

  // add this snapshot's info
  lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->ping;
  lagometer.snapshotFlags[lagometer.snapshotCount & (LAG_SAMPLES - 1)]   = snap->snapFlags;
  lagometer.snapshotCount++;

	cg.ping = 0;

	if ( cg.snap )
	{
		previousPings[ index++ ] = cg.snap->ping;
		index = index % PING_FRAMES;

		for ( i = 0; i < PING_FRAMES; i++ )
		{
			cg.ping += previousPings[ i ];
		}

		cg.ping /= PING_FRAMES;
	}
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect(void)
{
  float x, y;
  int cmdNum;
  usercmd_t cmd;
  const char *s;
  int w;                           // bk010215 - FIXME char message[1024];

  // draw the phone jack if we are completely past our buffers
  cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
  trap_GetUserCmd(cmdNum, &cmd);

  if (cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time)  // special check for map_restart // bk 0102165 - FIXME
    return;

  // also add text in center of screen
  s = "Connection Interrupted";    // bk 010215 - FIXME
  w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
  CG_DrawBigString(320 - w / 2, 100, s, 1.0F);

  // blink the icon
  if ((cg.time >> 9) & 1)
    return;

 // x = 640 - 48;
 // y = 480 - (144 - 50);
  x = 640 - 52;
  y = 480 - 570;

  //CG_DrawPic( x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.png" ) );
  CG_DrawPic(x, y, 48, 48, trap_R_RegisterShader("disconnected"));
}


#define MAX_LAGOMETER_PING 900
#define MAX_LAGOMETER_RANGE 300
/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer(void)
{
  int a, x, y, i;
  float v;
  float ax, ay, aw, ah, mid, range;
  int color;
  float vscale;

//unlagged - misc
  //if (!cg_lagometer.integer || cgs.localServer) //kpq3 b4 unlag
	if ( !cg_lagometer.integer /* || cgs.localServer */) 
//unlagged - misc
  {
    CG_DrawDisconnect();
    return;
  }

  // draw the graph
#if 0
  x = 640 - cg_larshelp.integer ; //68;
  y = 480 - cg_larshelp2.integer ; //120;
#else
  x = 640 - 570;
  y = 480 - 52;
#endif

  trap_R_SetColor(NULL);
  CG_DrawPic(x - 8, y - 8, 48 + 16, 48 + 16, cgs.media.lagometerShader);
  //CG_DrawPic(x, y, 48, 48, cgs.media.lagometerShader);

  ax = x;
  ay = y;

  aw = 48;
  ah = 48;
  CG_AdjustFrom640(&ax, &ay, &aw, &ah);

  color = -1;
  range = ah / 3;
  mid   = ay + range;

  vscale = range / MAX_LAGOMETER_RANGE;

  // draw the frame interpoalte / extrapolate graph
  for (a = 0; a < aw; a++)
  {
    i  = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
    v  = lagometer.frameSamples[i];
    v *= vscale;
    if (v > 0)
    {
      if (color != 1)
      {
        color = 1;
        trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
      }

      if (v > range)
        v = range;

      trap_R_DrawStretchPic(ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
    }
    else if (v < 0)
    {
      if (color != 2)
      {
        color = 2;
        trap_R_SetColor(g_color_table[ColorIndex(COLOR_BLUE)]);
      }
      v = -v;

      if (v > range)
        v = range;

      trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
    }
  }

  // draw the snapshot latency / drop graph
  range  = ah / 2;
  vscale = range / MAX_LAGOMETER_PING;

  for (a = 0; a < aw; a++)
  {
    i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
    v = lagometer.snapshotSamples[i];
    if (v > 0)
    {
      if (lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED)
      {
        if (color != 5)
        {
          color = 5;  // YELLOW for rate delay
          trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
        }
      }
      else
      {
        if (color != 3)
        {
          color = 3;
          trap_R_SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
        }
      }
      v = v * vscale;

      if (v > range)
        v = range;

      trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
    }
    else if (v < 0)
    {
      if (color != 4)
      {
        color = 4;      // Dragon for dropped snapshots
        trap_R_SetColor(g_color_table[ColorIndex(COLOR_RED)]);
      }
      trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader);
    }
  }

  trap_R_SetColor(NULL);

  if (cg_nopredict.integer || cg_synchronousClients.integer)
  {
    CG_DrawBigString(x, y, "snc", 1.0);
  }

  CG_DrawDisconnect();
}

/*
===============================================================================
CENTER PRINTING
===============================================================================
*/
/*
==============
CG_CenterPrint
Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint(const char *str, int y, int charWidth)
{
  char *s;

  Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));

  cg.centerPrintTime      = cg.time;
  cg.centerPrintY         = y;
  cg.centerPrintCharWidth = charWidth;

  // count the number of lines for centering
  cg.centerPrintLines = 1;
  s                   = cg.centerPrint;
  while (*s)
  {
    if (*s == '\n')
      cg.centerPrintLines++;
    s++;
  }
}

/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString(void)
{
  char *start;
  int l;
  int x, y, w;
  int h;
  float *color;

  if (!cg.centerPrintTime)
    return;

  color = CG_FadeColor(cg.centerPrintTime, 1000 * cg_centertime.value);
  if (!color)
    return;

  trap_R_SetColor(color);

  start = cg.centerPrint;

  y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

  while (1)
  {
    char linebuffer[1024];

    for (l = 0; l < 50; l++)
    {
      if (!start[l] || start[l] == '\n')
        break;

      linebuffer[l] = start[l];
    }
    linebuffer[l] = 0;

    w = CG_Text_Width(linebuffer, 0.5, 0, NULL_FONT);
    h = CG_Text_Height(linebuffer, 0.5, 0, NULL_FONT);
    x = (SCREEN_WIDTH - w) / 2;
    CG_Text_Paint(x, y + h, 0.5, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);
    y += h + 6;

    while (*start && (*start != '\n'))
    {
      start++;
    }
    if (!*start)
      break;

    start++;
  }

  trap_R_SetColor(NULL);
}

/*
================================================================================
CROSSHAIR
================================================================================
*/
/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair(void)
{
  float w, h;
  qhandle_t hShader;
  float f;
  float x, y;
  int ca;

  if (!cg_drawCrosshair.integer)
    return;

  if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
    return;

  if (cg.renderingThirdPerson) //hypov8 should we add crosshair??
    return;

  // set color based on health
  if (cg_crosshairHealth.integer)
  {
    vec4_t hcolor;
    CG_ColorForHealth(hcolor);
    trap_R_SetColor(hcolor);
  }
  else
  {
    trap_R_SetColor(NULL);
  }

  w = h = cg_crosshairSize.value;

  // pulse the size of the crosshair when picking up items
  f = cg.time - cg.itemPickupBlendTime;
  if (f > 0 && f < ITEM_BLOB_TIME)
  {
    f /= ITEM_BLOB_TIME;
    w *= (1 + f);
    h *= (1 + f);
  }

  x = cg_crosshairX.integer;
  y = cg_crosshairY.integer;
  CG_AdjustFrom640(&x, &y, &w, &h);

  ca = cg_drawCrosshair.integer;

  if (ca < 0)
    ca = 0;

  hShader = cgs.media.crosshairShader[ca % NUM_CROSSHAIRS];

  trap_R_DrawStretchPic(x + cg.refdef.x + 0.5 * (cg.refdef.width - w),
                        y + cg.refdef.y + 0.5 * (cg.refdef.height - h),
                        w, h, 0, 0, 1, 1, hShader);
}

/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity(void)
{
  trace_t trace;
  vec3_t start, end;
  int content;

  VectorCopy(cg.refdef.vieworg, start);
  VectorMA(start, 131072, cg.refdef.viewaxis[0], end);

  CG_Trace(&trace, start, vec3_origin, vec3_origin, end,
           cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY);
  if (trace.entityNum >= MAX_CLIENTS)
    return;

  // if the player is in fog, don't show it
  content = trap_CM_PointContents(trace.endpos, 0);
  if (content & CONTENTS_FOG)
    return;
#ifdef GT_USE_TA_TYPES
  // if the player is invisible, don't show it
  if (cg_entities[trace.entityNum].currentState.powerups & (1 << PW_INVIS))
    return;
#endif
  // update the fade timer
  cg.crosshairClientNum  = trace.entityNum;
  cg.crosshairClientTime = cg.time;
}

/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames(void)
{
  float *color;
  char *name;
  float w;

  if (!cg_drawCrosshair.integer)
    return;

  if (!cg_drawCrosshairNames.integer)
    return;

  if (cg.renderingThirdPerson)
    return;

  // scan the known entities to see if the crosshair is sighted on one
  CG_ScanForCrosshairEntity();

  // draw the name of the player being looked at
  color = CG_FadeColor(cg.crosshairClientTime, 1000);
  if (!color)
  {
    trap_R_SetColor(NULL);
    return;
  }

  name = cgs.clientinfo[cg.crosshairClientNum].name;
  color[3] *= 0.5f;
  w         = CG_Text_Width(name, 0.3f, 0, NULL_FONT);
#if 0
  CG_Text_Paint(320 - w / 2, 190, 0.3f, color, name, 0, 0, ITEM_TEXTSTYLE_SHADOWED, NULL_FONT);
#else
  CG_Text_Paint(320 - w / 2, 450, 0.3f, color, name, 0, 0, ITEM_TEXTSTYLE_SHADOWED, NULL_FONT);
#endif

  trap_R_SetColor(NULL);
}

//==============================================================================
/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void)
{
  static const char spec[] = "SPECTATOR";
  static const char info[] = "press ESC and use the JOIN menu to play";
  static const char info2[] = "waiting to play";
  int w = 0;
  float scale = .5f;

  w = CG_Text_Width(spec, scale, 0, NULL_FONT);
  CG_Text_Paint(320 - w / 2, 40, scale, colorWhite, spec, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);

  if (cgs.gametype == GT_TOURNAMENT)
  {
    w = CG_Text_Width(info2, scale, 0, NULL_FONT);
    CG_Text_Paint(320 - w / 2, 60, scale, colorWhite, info2, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);
  }
  else if (cgs.gametype >= GT_TEAM)
  {
    scale = 0.3f;
    w = CG_Text_Width(info, scale, 0, NULL_FONT);
    CG_Text_Paint(320 - w / 2, 60, scale, colorWhite, info, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);
  }
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void)
{
  char *s;
  int sec;

  if (!cgs.voteTime)
    return;

  // play a talk beep whenever it is modified
  if (cgs.voteModified)
  {
    cgs.voteModified = qfalse;
    trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
  }

  sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;

  if (sec < 0)
    sec = 0;

  s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
  CG_DrawSmallString(0, 58, s, 1.0F);      // FIXME:  0xA5EA CG_Text_Paint( 600, y + 20, .25, color, s, 0, 0, 0);

  s = "or press ESC then click Vote";
  CG_DrawSmallString(0, 58 + SMALLCHAR_HEIGHT + 2, s, 1.0F);

}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void)
{
  char *s;
  int sec, cs_offset;

  switch(cgs.clientinfo->team)
  {
  case TEAM_DRAGONS: cs_offset = 0;   break;
  case TEAM_NIKKIS:  cs_offset = 1;   break;
  default: return;
  }

  if (!cgs.teamVoteTime[cs_offset])
    return;

  // play a talk beep whenever it is modified
  if (cgs.teamVoteModified[cs_offset])
  {
    cgs.teamVoteModified[cs_offset] = qfalse;
    trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
  }

  sec = (VOTE_TIME - (cg.time - cgs.teamVoteTime[cs_offset])) / 1000;

  if (sec < 0)
    sec = 0;

  s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
         cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset]);
  CG_DrawSmallString(0, 90, s, 1.0F);          // FIXME: 0xA5EA	CG_Text_Paint( 600, y + 20, .25, color, s, 0, 0, 0);
}

/*
=================
CG_DrawScoreboard
=================
*/
static qboolean CG_DrawScoreboard(void)
{
  static qboolean firstTime = qtrue;
  float fade, *fadeColor;

  if (menuScoreboard)
    menuScoreboard->window.flags &= ~WINDOW_FORCED;

  if (cg_paused.integer)
  {
    cg.deferredPlayerLoading = 0;
    firstTime                = qtrue;
    return qfalse;
  }

#ifdef USE_GT_SINGLEPLAYER
  // should never happen in Team Arena
  if (cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION)
  {
    cg.deferredPlayerLoading = 0;
    firstTime                = qtrue;
    return qfalse;
  }
#endif

  // don't draw scoreboard during death while warmup up
  if (cg.warmup && !cg.showScores)
    return qfalse;

  if (cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD || cg.predictedPlayerState.pm_type == PM_INTERMISSION)
  {
    fade      = 1.0;
    fadeColor = colorWhite;
  }
  else
  {
    fadeColor = CG_FadeColor(cg.scoreFadeTime, FADE_TIME);
    if (!fadeColor)
    {
      // next time scoreboard comes up, don't print killer
      cg.deferredPlayerLoading = 0;
      cg.killerName[0]         = 0;
      firstTime                = qtrue;
      return qfalse;
    }
    fade = *fadeColor;
  }

  (void)fade; //shutup compiler

  if (menuScoreboard == NULL)
  {
    switch(cgs.gametype)
    {
    default:
    case  GT_FFA:
    case  GT_TOURNAMENT:
      menuScoreboard = Menus_FindByName("score_menu");
      break;
    case  GT_TEAM:
    case  GT_CTF:
    case  GT_1FCTF:
#ifdef GT_USE_TA_TYPES
    case  GT_OBELISK:
    case  GT_HARVESTER:
#endif
      menuScoreboard = Menus_FindByName("teamscore_menu");
      break;
#ifdef WITH_BAGMAN_MOD
    case  GT_BAGMAN:
      menuScoreboard = Menus_FindByName("teambagmanscore_menu"); //hypov8 todo: these can be the same
      break;
#endif
    }
  }

  if (menuScoreboard)
  {
    if (firstTime)
    {
      CG_SetScoreSelection(menuScoreboard);
      firstTime = qfalse;
    }
    Menu_Paint(menuScoreboard, qtrue);
  }

  // load any models that have been deferred
  if (++cg.deferredPlayerLoading > 10)
    CG_LoadDeferredPlayers();

  return qtrue;
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission(void)
{
#ifdef USE_GT_SINGLEPLAYER
  if (cg_singlePlayer.integer) {
  	CG_DrawCenterString();
  	return;
  }
#endif
  cg.scoreFadeTime     = cg.time;
  cg.scoreBoardShowing = CG_DrawScoreboard();
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow(void)
{
  float scale = .3f;
  int w = 0;
  const char *name;
  static const char* info = "following";

  if (!(cg.snap->ps.pm_flags & PMF_FOLLOW))
    return qfalse;

  // draw following
  w = CG_Text_Width(info, scale, 0, NULL_FONT);
  CG_Text_Paint(320 - w/2, 400, scale, colorWhite, info, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);

  // draw clientname
  scale = .4f;
  name = cgs.clientinfo[cg.snap->ps.clientNum].name;
  w = CG_Text_Width(name, scale, 0, NULL_FONT);
  CG_Text_Paint(320 - w/2, 420, scale, colorWhite, name, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);

  return qtrue;
}

/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning(void)
{
  const char *s;
  int w;

  if (cg_drawAmmoWarning.integer == 0)
    return;

  if (!cg.lowAmmoWarning)
    return;

  if (cg.lowAmmoWarning == 2)
    s = "OUT OF AMMO";
  else
    s = "LOW AMMO WARNING";

  w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
  //CG_DrawBigString(320 - w / 2, 64, s, 1.0F);
  CG_DrawSmallString(320 - w / 2, 64, s, 1.0F); /* 0xA5EA, FIXME: CG_Text_Paint( 600, y + 20, .25, color, s, 0, 0, 0); hud smaller test */
}


#if 0                                             //def MISSIONPACK 0xA5EA
/*
=================
CG_DrawProxWarning
=================
*/
static void CG_DrawProxWarning(void)
{
  char s [32];
  int w;
  static int proxTime;
  static int proxCounter;
  static int proxTick;

  if (!(cg.snap->ps.eFlags & EF_TICKING))
  {
    proxTime = 0;
    return;
  }

  if (proxTime == 0)
  {
    proxTime    = cg.time + 5000;
    proxCounter = 5;
    proxTick    = 0;
  }

  if (cg.time > proxTime)
  {
    proxTick = proxCounter--;
    proxTime = cg.time + 1000;
  }

  if (proxTick != 0)
  {
    Com_sprintf(s, sizeof(s), "INTERNAL COMBUSTION IN: %i", proxTick);
  }
  else
  {
    Com_sprintf(s, sizeof(s), "YOU HAVE BEEN MINED");
  }

  w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
  CG_DrawBigStringColor(320 - w / 2, 64 + BIGCHAR_HEIGHT, s, g_color_table[ColorIndex(COLOR_RED)]);
}
#endif


/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup(void)
{
  int w;
  int sec;
  int i;
  float scale;
  clientInfo_t *ci1, *ci2;
  int cw;
  const char *s;

  sec = cg.warmup;
  if (!sec)
    return;

  if (sec < 0)
  {
    s = "Waiting for players";
    w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
    CG_DrawBigString(320 - w / 2, 24, s, 1.0F);
    cg.warmupCount = 0;
    return;
  }

  if (cgs.gametype == GT_TOURNAMENT)
  {
    // find the two active players
    ci1 = NULL;
    ci2 = NULL;
    for (i = 0; i < cgs.maxclients; i++)
    {
      if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE)
      {
        if (!ci1)
          ci1 = &cgs.clientinfo[i];
        else
          ci2 = &cgs.clientinfo[i];
      }
    }

    if (ci1 && ci2)
    {
      s = va("%s vs %s", ci1->name, ci2->name);
      w = CG_Text_Width(s, 0.6f, 0, NULL_FONT);
      CG_Text_Paint(320 - w / 2, 60, 0.6f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);
    }
  }
  else
  {
    if (cgs.gametype == GT_FFA)
      s = "Gangbang";
    else if (cgs.gametype == GT_TEAM)
      s = "Team Deathmatch";
    else if (cgs.gametype == GT_CTF)
      s = "Capture the Flag";
#ifdef WITH_BAGMAN_MOD
    else if (cgs.gametype == GT_BAGMAN)
      s = "Bagman";
#endif
    else if (cgs.gametype == GT_1FCTF)
      s = "One Flag CTF";
#ifdef GT_USE_TA_TYPES
    else if (cgs.gametype == GT_OBELISK)
      s = "Overload";
    else if (cgs.gametype == GT_HARVESTER)
      s = "Harvester";
#endif
    else
      s = "";

    w = CG_Text_Width(s, 0.6f, 0, NULL_FONT);
    CG_Text_Paint(320 - w / 2, 90, 0.6f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);
  }

  sec = (sec - cg.time) / 1000;
  if (sec < 0)
  {
    cg.warmup = 0;
    sec       = 0;
  }
  s = va("Starts in: %i", sec + 1);
  if (sec != cg.warmupCount)
  {
    cg.warmupCount = sec;
    switch (sec)
    {
    case 0:
      trap_S_StartLocalSound(cgs.media.count1Sound, CHAN_ANNOUNCER);
      break;
    case 1:
      trap_S_StartLocalSound(cgs.media.count2Sound, CHAN_ANNOUNCER);
      break;
    case 2:
      trap_S_StartLocalSound(cgs.media.count3Sound, CHAN_ANNOUNCER);
      break;
    default:
      break;
    }
  }
  scale = 0.45f;
  switch (cg.warmupCount)
  {
  case 0:
    cw    = 28;
    scale = 0.54f;
    break;
  case 1:
    cw    = 24;
    scale = 0.51f;
    break;
  case 2:
    cw    = 20;
    scale = 0.48f;
    break;
  default:
    cw    = 16;
    scale = 0.45f;
    break;
  }
  (void)cw; //shutup compiler
  w = CG_Text_Width(s, scale, 0, NULL_FONT);
  CG_Text_Paint(320 - w / 2, 125, scale, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, NULL_FONT);
}

//==================================================================================
/*
=================
CG_DrawTimedMenus
=================
*/
void CG_DrawTimedMenus(void)
{
  if (cg.voiceTime)
  {
    int t = cg.time - cg.voiceTime;
    if (t > 2500)
    {
      Menus_CloseByName("voiceMenu");
      trap_Cvar_Set("cl_conXOffset", "0");
      cg.voiceTime = 0;
    }
  }
}

/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D(void)
{
  if (cgs.orderPending && cg.time > cgs.orderTime)
  {
    CG_CheckOrderPending();
  }
  // if we are taking a levelshot for the menu, don't draw anything
  if (cg.levelShot)
    return;

  if (cg_draw2D.integer == 0)
    return;

  if (cg.snap->ps.pm_type == PM_INTERMISSION)
  {
    CG_DrawIntermission();
    // draw the chat during intermission
    CG_DrawTeamInfo(cgs.gametype);
    return;
  }

  /*
      if (cg.cameraMode) {
          return;
      }
  */
  if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
  {
    CG_DrawSpectator();
    CG_DrawCrosshair();
    CG_DrawCrosshairNames();
    CG_DrawTeamInfo(cgs.gametype);
  }
  else
  {
    // don't draw any status if dead or the scoreboard is being explicitly shown
    if (/*!cg.showScores && */ cg.snap->ps.stats[STAT_HEALTH] > 0)
    {
      if (cg_drawStatus.integer)
      {
        Menu_PaintAll();
        CG_DrawTimedMenus();
      }

      CG_DrawAmmoWarning();
      //FIXME (0xA5EA):commented out ammo waring

#if 0       //def MISSIONPACK
      CG_DrawProxWarning();
#endif
      CG_DrawCrosshair();
      CG_DrawCrosshairNames();
      CG_DrawWeaponSelect();
	  CG_DrawPickupItem(); //hypov8 pickup_item display in hud
      CG_DrawReward();
    }
    CG_DrawTeamInfo(cgs.gametype);
  }

  CG_DrawVote();
  CG_DrawTeamVote();
  CG_DrawLagometer();

  if (!cg_paused.integer)
    CG_DrawUpperRight();

  if (!CG_DrawFollow())
    CG_DrawWarmup();

  // don't draw center string if scoreboard is up
  cg.scoreBoardShowing = CG_DrawScoreboard();
  if (!cg.scoreBoardShowing)
    CG_DrawCenterString();
}

/*
=====================
CG_DrawActive
Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive(stereoFrame_t stereoView)
{
  float separation;
  vec3_t baseOrg;

  // optionally draw the info screen instead
  if (!cg.snap)
  {
    CG_DrawInformation();
    //Menu_Paint(&Menus[3], qfalse);
    return;
  }
#if 0       // 0xA5EA, funktion is nicht definiert ? deshalb entfernt
  // optionally draw the tournement scoreboard instead
  if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
      (cg.snap->ps.pm_flags & PMF_SCOREBOARD))
  {
    CG_DrawTourneyScoreboard();
    return;
  }
#endif
  switch (stereoView)
  {
  case STEREO_CENTER:
    separation = 0;
    break;
  case STEREO_LEFT:
    separation = -cg_stereoSeparation.value / 2;
    break;
  case STEREO_RIGHT:
    separation = cg_stereoSeparation.value / 2;
    break;
  default:
    separation = 0;
    CG_Error("CG_DrawActive: Undefined stereoView");
  }

  // clear around the rendered view if sized down
  CG_TileClear();

  CG_PB_RenderPolyBuffers();

  // offset vieworg appropriately if we're doing stereo separation
  VectorCopy(cg.refdef.vieworg, baseOrg);

  if (separation != 0)
    VectorMA(cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg);

  // draw 3D view
  trap_R_RenderScene(&cg.refdef);

  // restore original viewpoint if running stereo
  if (separation != 0)
    VectorCopy(baseOrg, cg.refdef.vieworg);

  // draw status bar and other floating elements
  CG_Draw2D();

}

/*
=====================
CG_Text_PaintAligned
=====================
*/
void CG_Text_PaintAligned(int x, int y, char *s, float scale, int style, vec4_t color, fontInfo_t *font)
{
  int w, h;

  w = CG_Text_Width(s, scale, 0, font);
  h = CG_Text_Height(s, scale, 0, font);

  if (style & UI_CENTER)
    CG_Text_Paint(x - w / 2, y + h / 2, scale, color, s, 0, 0, style, font);
  else if (style & UI_RIGHT)
    CG_Text_Paint(x - w, y + h / 2, scale, color, s, 0, 0, style, font);
  else
    CG_Text_Paint(x, y + h / 2, scale, color, s, 0, 0, style, font);  // UI_LEFT
}
