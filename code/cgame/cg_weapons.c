/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2010 Lars '0xA5EA' Kandler
Copyright (C) 2011-2012 Ari 'KRYPTYK' Mirles

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
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

static refSkeleton_t gunSkeleton;
static refSkeleton_t oldGunSkeleton;
//FIXME(0xA5EA): merge, where is this used

#ifdef USE_FLAMEGUN
// Ridah
/*
===============
CG_FlamethrowerFlame
===============
*/
static void CG_FlamethrowerFlame( centity_t *cent, vec3_t origin)
{
  if (cent->currentState.weapon != WP_FLAMER)
    return;

  CG_FireFlameChunks(cent, origin, cent->lerpAngles, 1.0, qtrue);
}
#endif
/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass(centity_t *cent)
{
  localEntity_t *le;
  refEntity_t *re;
  vec3_t velocity, xvelocity;
  vec3_t offset, xoffset;
  float waterScale = 1.0f;
  vec3_t v[3];

  if (cg_brassTime.integer <= 0)
    return;

  le = CG_AllocLocalEntity();
  re = &le->refEntity;

  velocity[0] = 0;
  velocity[1] = -50 + 40 * crandom();
  velocity[2] = 100 + 50 * crandom();

  le->leType    = LE_FRAGMENT;
  le->startTime = cg.time;
  le->endTime   = le->startTime + cg_brassTime.integer + (cg_brassTime.integer / 4) * random();

  le->pos.trType = TR_GRAVITY;
  le->pos.trTime = cg.time - (rand() & 15);

  AnglesToAxis(cent->lerpAngles, v);

  offset[0] = 8;
  offset[1] = -4;
  offset[2] = 24;

  xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
  xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
  xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
  VectorAdd(cent->lerpOrigin, xoffset, re->origin);

  VectorCopy(re->origin, le->pos.trBase);

  if (CG_PointContents(re->origin, -1) & CONTENTS_WATER)
    waterScale = 0.10f;

  xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
  xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
  xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
  VectorScale(xvelocity, waterScale, le->pos.trDelta);

  AxisCopy(axisDefault, re->axis);

  re->hModel = cgs.media.machinegunBrassModel; //note hypov8 eject brass bullet

  le->bounceFactor = 0.4 * waterScale;

  le->angles.trType     = TR_LINEAR;
  le->angles.trTime     = cg.time;
  le->angles.trBase[0]  = rand() & 31;
  le->angles.trBase[1]  = rand() & 31;
  le->angles.trBase[2]  = rand() & 31;
  le->angles.trDelta[0] = 2;
  le->angles.trDelta[1] = 1;
  le->angles.trDelta[2] = 0;

  le->leFlags           = LEF_TUMBLE;
  le->leBounceSoundType = LEBS_BRASS;
  le->leMarkType        = LEMT_NONE;
}

/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass(centity_t *cent)
{
  localEntity_t *le;
  refEntity_t *re;
  vec3_t velocity, xvelocity;
  vec3_t offset, xoffset;
  vec3_t v[3];
  //int i;

  if (cg_brassTime.integer <= 0)
    return;

  //for (i = 0; i < 1; i++)   //war 2
  {
    float waterScale = 1.0f;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    velocity[0] = 20 + 10 * crandom(); //forward
    velocity[1] = -80 + 10 * crandom(); //right
    velocity[2] = 100 + 20 * crandom(); //up

    le->leType    = LE_FRAGMENT;
    le->startTime = cg.time;
    le->endTime   = le->startTime + cg_brassTime.integer * 3 + cg_brassTime.integer *random();

    le->pos.trType = TR_GRAVITY;
    le->pos.trTime = cg.time;

    AnglesToAxis(cent->lerpAngles, v);

    offset[0] = 8.78f;
    offset[1] = -4.7f;
    offset[2] = 26.0f;

    xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
    xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
    xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

    VectorAdd(cent->lerpOrigin, xoffset, re->origin);
    VectorCopy(re->origin, le->pos.trBase);

    if (CG_PointContents(re->origin, -1) & CONTENTS_WATER)
      waterScale = 0.10f;

    xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
    xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
    xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];

    VectorScale(xvelocity, waterScale, le->pos.trDelta);

    AxisCopy(axisDefault, re->axis);
    re->hModel       = cgs.media.shotgunBrassModel; //note hypov8 eject brass shotty
    le->bounceFactor = 0.3f;

    le->angles.trType = TR_LINEAR;
    le->angles.trTime = cg.time;

    le->angles.trBase[0] = rand() & 31;
    le->angles.trBase[1] = rand() & 31;
    le->angles.trBase[2] = rand() & 31;

    le->angles.trDelta[0] = 1;
    le->angles.trDelta[1] = 0.5;
    le->angles.trDelta[2] = 0;

    le->leFlags           = LEF_TUMBLE;
    le->leMarkType        = LEMT_NONE;
    le->leBounceSoundType = LEBS_BRASS;
  }
}


#if 0 //def MISSIONPACK
/*
==========================
CG_NailgunEjectBrass
==========================
*/
static void CG_NailgunEjectBrass(centity_t *cent)
{
  localEntity_t *smoke;
  vec3_t origin;
  vec3_t v[3];
  vec3_t offset;
  vec3_t xoffset;
  vec3_t up;

  AnglesToAxis(cent->lerpAngles, v);

  offset[0] = 0;
  offset[1] = -12;
  offset[2] = 24;

  xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
  xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
  xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
  VectorAdd(cent->lerpOrigin, xoffset, origin);

  VectorSet(up, 0, 0, 64);

  smoke = CG_SmokePuff(origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader);
  // use the optimized local entity add
  smoke->leType = LE_SCALE_FADE;
}
#endif


/*
==========================
CG_RailTrail
==========================
*/
void CG_RailTrail(clientInfo_t *ci, vec3_t start, vec3_t end)
{
  vec3_t axis[36], move, move2, next_move, vec, temp;
  float len;
  int i, j, skip;

  localEntity_t *le;
  refEntity_t *re;

#define RADIUS 4
#define ROTATION 1
#define SPACING 5

  start[2] -= 4;
  VectorCopy(start, move);
  VectorSubtract(end, start, vec);
  len = VectorNormalize(vec);
  PerpendicularVector(temp, vec);
  for (i = 0; i < 36; i++)
  {
    RotatePointAroundVector(axis[i], vec, temp, i * 10); //banshee 2.4 was 10
  }

  le = CG_AllocLocalEntity();
  re = &le->refEntity;

  le->leType    = LE_FADE_RGB;
  le->startTime = cg.time;
  le->endTime   = cg.time + cg_railTrailTime.value;
  le->lifeRate  = 1.0 / (le->endTime - le->startTime);

  re->shaderTime   = cg.time / 1000.0f;
  re->reType = RT_BEAM; // was RT_RAILCORE;
  re->customShader = cgs.media.railCoreShader;

  VectorCopy(start, re->origin);
  VectorCopy(end, re->oldorigin);

  re->shaderRGBA[0] = ci->color1[0] * 255;
  re->shaderRGBA[1] = ci->color1[1] * 255;
  re->shaderRGBA[2] = ci->color1[2] * 255;
  re->shaderRGBA[3] = 255;

  le->color[0] = ci->color1[0] * 0.75;
  le->color[1] = ci->color1[1] * 0.75;
  le->color[2] = ci->color1[2] * 0.75;
  le->color[3] = 1.0f;

  AxisClear(re->axis);

  VectorMA(move, 20, vec, move);
  VectorCopy(move, next_move);
  VectorScale(vec, SPACING, vec);

  skip = -1;

  j = 18;
  for (i = 0; i < len; i += SPACING)
  {
    if (i != skip)
    {
      skip = i + SPACING;
      le = CG_AllocLocalEntity();
      re = &le->refEntity;
      le->leFlags = LEF_PUFF_DONT_SCALE;
      le->leType  = LE_MOVE_SCALE_FADE;
      le->startTime = cg.time;
      le->endTime   = cg.time + (i >> 1) + 600;
      le->lifeRate  = 1.0 / (le->endTime - le->startTime);

      re->shaderTime = cg.time / 1000.0f;
      re->reType = RT_SPRITE;
      re->radius = 1.1f;
      re->customShader = cgs.media.railRingsShader;

      re->shaderRGBA[0] = ci->color2[0] * 255;
      re->shaderRGBA[1] = ci->color2[1] * 255;
      re->shaderRGBA[2] = ci->color2[2] * 255;
      re->shaderRGBA[3] = 255;

      le->color[0] = ci->color2[0] * 0.75;
      le->color[1] = ci->color2[1] * 0.75;
      le->color[2] = ci->color2[2] * 0.75;
      le->color[3] = 1.0f;

      le->pos.trType = TR_LINEAR;
      le->pos.trTime = cg.time;

      VectorCopy(move, move2);
      VectorMA(move2, RADIUS, axis[j], move2);
      VectorCopy(move2, le->pos.trBase);

      le->pos.trDelta[0] = axis[j][0] * 6;
      le->pos.trDelta[1] = axis[j][1] * 6;
      le->pos.trDelta[2] = axis[j][2] * 6;
    }
    VectorAdd(move, vec, move);
    j = j + ROTATION < 36 ? j + ROTATION : (j + ROTATION) % 36;
  }
}

/*
==========================
CG_RocketTrail
==========================
*/
static void CG_RocketTrail(centity_t *ent, const weaponInfo_t *wi)
{
  int step;
  vec3_t origin, lastPos;
  int t;
  int startTime, contents;
  int lastContents;
  entityState_t *es;
  vec3_t up;
  localEntity_t *smoke;
  qhandle_t smokePuffshader;

  if (cg_noProjectileTrail.integer)
    return;

  up[0] = 0;
  up[1] = 0;
  up[2] = 0;

  step = 50;

  es = &ent->currentState;
  startTime = ent->trailTime;
  t = step * ((startTime + step) / step);

  BG_EvaluateTrajectory(&es->pos, cg.time, origin);
  contents = CG_PointContents(origin, -1);

  // if object (e.g. grenade) is stationary, don't toss up smoke
  if (es->pos.trType == TR_STATIONARY)
  {
    ent->trailTime = cg.time;
    return;
  }

  BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos);
  lastContents = CG_PointContents(lastPos, -1);

  ent->trailTime = cg.time;

  if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
  {
    if (contents & lastContents & CONTENTS_WATER)
      CG_BubbleTrail(lastPos, origin, 8);
    return;
  }

  if( wi->customSmokePuffshader )
    smokePuffshader = wi->customSmokePuffshader;
  else
    smokePuffshader = cgs.media.smokePuffShader;

  for (; t <= ent->trailTime; t += step)
  {
    BG_EvaluateTrajectory(&es->pos, t, lastPos);

    smoke = CG_SmokePuff(lastPos, up,
                         wi->trailRadius,
                         1, 1, 1, 0.33f,
                         wi->wiTrailTime,
                         t,
                         0,
                         0,
                         smokePuffshader);
    // use the optimized local entity add
    smoke->leType = LE_SCALE_FADE;
  }
}

/*
==========================
CG_NailTrail
==========================
*/
static void CG_NailTrail(centity_t *ent, const weaponInfo_t *wi)
{
  int step;
  vec3_t origin, lastPos;
  int t;
  int startTime, contents;
  int lastContents;
  entityState_t *es;
  vec3_t up;
  localEntity_t *smoke;

  if (cg_noProjectileTrail.integer)
    return;

  up[0] = 0;
  up[1] = 0;
  up[2] = 0;

  step = 50;

  es        = &ent->currentState;
  startTime = ent->trailTime;
  t         = step * ((startTime + step) / step);

  BG_EvaluateTrajectory(&es->pos, cg.time, origin);
  contents = CG_PointContents(origin, -1);

  // if object (e.g. grenade) is stationary, don't toss up smoke
  if (es->pos.trType == TR_STATIONARY)
  {
    ent->trailTime = cg.time;
    return;
  }

  BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos);
  lastContents = CG_PointContents(lastPos, -1);

  ent->trailTime = cg.time;

  if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
  {
    if (contents & lastContents & CONTENTS_WATER)
    {
      CG_BubbleTrail(lastPos, origin, 8);
    }
    return;
  }

  for (; t <= ent->trailTime; t += step)
  {
    BG_EvaluateTrajectory(&es->pos, t, lastPos);
    smoke = CG_SmokePuff
        (
            lastPos, up,
            wi->trailRadius,
            1, 1, 1, 0.33f,
            wi->wiTrailTime,
            t,
            0,
            0,
            cgs.media.smokePuffShader
          //  cgs.media.nailPuffShader
        );
    // use the optimized local entity add
    smoke->leType = LE_SCALE_FADE;
  }

}
#if 0 //def MISSIONPACK
/*
==========================
CG_PlasmaTrail
==========================
*/
static void CG_PlasmaTrail(centity_t *cent, const weaponInfo_t *wi)
{
  localEntity_t *le;
  refEntity_t *re;
  entityState_t *es;
  vec3_t velocity, xvelocity, origin;
  vec3_t offset, xoffset;
  vec3_t v[3];
  int t, startTime, step;

  float waterScale = 1.0f;

  if (cg_noProjectileTrail.integer)
    return;

  step = 50;

  es        = &cent->currentState;
  startTime = cent->trailTime;
  t         = step * ((startTime + step) / step);

  BG_EvaluateTrajectory(&es->pos, cg.time, origin);

  le = CG_AllocLocalEntity();
  re = &le->refEntity;

  velocity[0] = 60 - 120 * crandom();
  velocity[1] = 40 - 80 * crandom();
  velocity[2] = 100 - 200 * crandom();

  le->leType            = LE_MOVE_SCALE_FADE;
  le->leFlags           = LEF_TUMBLE;
  le->leBounceSoundType = LEBS_NONE;
  le->leMarkType        = LEMT_NONE;

  le->startTime = cg.time;
  le->endTime   = le->startTime + 600;

  le->pos.trType = TR_GRAVITY;
  le->pos.trTime = cg.time;

  AnglesToAxis(cent->lerpAngles, v);

  offset[0] = 2;
  offset[1] = 2;
  offset[2] = 2;

  xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
  xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
  xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

  VectorAdd(origin, xoffset, re->origin);
  VectorCopy(re->origin, le->pos.trBase);

  if (CG_PointContents(re->origin, -1) & CONTENTS_WATER)
  {
    waterScale = 0.10f;
  }

  xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
  xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
  xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
  VectorScale(xvelocity, waterScale, le->pos.trDelta);

  AxisCopy(axisDefault, re->axis);
  re->shaderTime   = cg.time / 1000.0f;
  re->reType       = RT_SPRITE;
  re->radius       = 0.25f;
  re->customShader = cgs.media.railRingsShader;
  le->bounceFactor = 0.3f;

  re->shaderRGBA[0] = wi->flashDlightColor[0] * 63;
  re->shaderRGBA[1] = wi->flashDlightColor[1] * 63;
  re->shaderRGBA[2] = wi->flashDlightColor[2] * 63;
  re->shaderRGBA[3] = 63;

  le->color[0] = wi->flashDlightColor[0] * 0.2;
  le->color[1] = wi->flashDlightColor[1] * 0.2;
  le->color[2] = wi->flashDlightColor[2] * 0.2;
  le->color[3] = 0.25f;

  le->angles.trType     = TR_LINEAR;
  le->angles.trTime     = cg.time;
  le->angles.trBase[0]  = rand() & 31;
  le->angles.trBase[1]  = rand() & 31;
  le->angles.trBase[2]  = rand() & 31;
  le->angles.trDelta[0] = 1;
  le->angles.trDelta[1] = 0.5;
  le->angles.trDelta[2] = 0;

}
#endif
/*
==========================
CG_GrappleTrail
==========================
*/
void CG_GrappleTrail(centity_t *ent, const weaponInfo_t *wi)
{
  vec3_t origin;
  entityState_t *es;
  vec3_t forward, up;
  refEntity_t beam;

  es = &ent->currentState;

  BG_EvaluateTrajectory(&es->pos, cg.time, origin);
  ent->trailTime = cg.time;

  Com_Memset(&beam, 0, sizeof(beam));
  //FIXME adjust for muzzle position
  VectorCopy(cg_entities[ent->currentState.otherEntityNum].lerpOrigin, beam.origin);
  beam.origin[2] += 26;
  AngleVectors(cg_entities[ent->currentState.otherEntityNum].lerpAngles, forward, NULL, up);
  VectorMA(beam.origin, -6, up, beam.origin);
  VectorCopy(origin, beam.oldorigin);

  if (Distance(beam.origin, beam.oldorigin) < 64)
    return;                                                 // Don't draw if close

  beam.reType       = RT_BEAM; // was RT_LIGHTNING //hypov8 merge: ok?
  beam.customShader = cgs.media.lightningShader;

  AxisClear(beam.axis);
  beam.shaderRGBA[0] = 0xff;
  beam.shaderRGBA[1] = 0xff;
  beam.shaderRGBA[2] = 0xff;
  beam.shaderRGBA[3] = 0xff;
  trap_R_AddRefEntityToScene(&beam);
}

/*
==========================
CG_GrenadeTrail
==========================
*/
static void CG_GrenadeTrail(centity_t *ent, const weaponInfo_t *wi)
{
  CG_RocketTrail(ent, wi);
}


/*
================
CG_WeaponReloadTimeOffset

add hypov8
match times used in bg_

time to complete an animation cycle
used to set fps bassed on frame count instead of using animation.cfg "fps" value
================
*/
static int CG_WeaponReloadTimeOffset(int weapon)
{
	switch (weapon)
	{
	default:
	case WP_PISTOL:
		return WP_TIME_RELOAD_PISTOL;
	case WP_SHOTGUN:
		return WP_TIME_RELOAD_SHOTGUN;
	case WP_MACHINEGUN:
		return WP_TIME_RELOAD_MACHINEGUN;
	case WP_GRENADE_LAUNCHER:
		return WP_TIME_RELOAD_GRENADE_LAUNCHER;
	case WP_ROCKET_LAUNCHER:
		return WP_TIME_RELOAD_ROCKET_LAUNCHER;
	case WP_HMG:
		return WP_TIME_RELOAD_HMG;
#ifdef USE_FLAMEGUN
	case WP_FLAMER:
		return WP_TIME_RELOAD_FLAMEGUN;
#endif
	}
}

/*
================
CG_WeaponFireTimeOffset

time to complete an animation cycle
used to set fps bassed on frame count instead of using animation.cfg "fps" value

guns with mods will run full the animation time
but this gets terminated by a new event  
================
*/
static int CG_WeaponFireTimeOffset(int weaponIndex)
{
	switch (weaponIndex)
	{
	default:
	case WP_CROWBAR:
		return WP_TIME_FIRE_CROWBAR;
	case WP_PISTOL:
		return WP_TIME_FIRE_PISTOL;
	case WP_SHOTGUN:
		return WP_TIME_FIRE_SHOTGUN;
	case WP_MACHINEGUN:
		return WP_TIME_FIRE_MACHINEGUN;
	case WP_GRENADE_LAUNCHER:
		return WP_TIME_FIRE_GRENADE_LAUNCHER;
	case WP_ROCKET_LAUNCHER:
		return WP_TIME_FIRE_ROCKET_LAUNCHER;
	case WP_HMG:
		return WP_TIME_FIRE_HMG_LAST;
#ifdef USE_FLAMEGUN
	case WP_FLAMER:
		return WP_TIME_FIRE_FLAMEGUN;
#endif
	case WP_GRAPPLING_HOOK:
		return WP_TIME_FIRE_GRAPPLING_HOOK;
	}
}

/*
================
CG_WeaponChangeTimeOffset

add hypov8
match times used in bg_

time to complete an animation cycle
used to set fps bassed on frame count instead of using animation.cfg "fps" value
================
*/
static int CG_WeaponChangeTimeOffset(int weapon)
{
	switch (weapon)
	{
		case WP_CROWBAR:
		case WP_GRAPPLING_HOOK:
			return WP_TIME_CHANGE_MELEE;
	}
	return WP_TIME_CHANGE_GUNS;
}

/*
================
CG_WeaponModTimeOffset

add hypov8
match times used in bg_

time to complete an animation cycle
used to set fps bassed on frame count instead of using animation.cfg "fps" value
================
*/
static int CG_WeaponModTimeOffset(int weapon)
{
	switch (weapon)
	{
	case WP_SHOTGUN:
			return WP_TIME_MOD_SHOTGUN*2;
	}
	return 0; //todo other mods
}

static qboolean CG_ParseWeaponAnimationFile(const char *filename, animation_t *animations, int weaponNum)
{
  char *text_p;
  unsigned int len;
  int ws;
  char *token;
  float fps;
  char text[1024];
  fileHandle_t f;
  int maxWepStates = WEAPON_RELOAD_MOD;

  if ( weaponNum == WP_SHOTGUN || weaponNum == WP_CROWBAR )
	maxWepStates = MAX_WEAPON_STATES; //hypov8 todo: add WEAPON_MOD to all animation.cfg
  
  Com_Memset(animations, 0, sizeof(animation_t) * MAX_WEAPON_STATES);

  if (cg_debugWeaponAnim.integer)
    Com_Printf("loading animation file: %s\n", filename);

  // load the file
  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if (len <= 0)
  {
    Com_Printf(S_COLOR_YELLOW"Failed to load animation file %s\n", filename);
    return qfalse;
  }
  if (len >= (sizeof(text) - 1))
  {
    Com_Printf("File %s too long\n", filename);

    trap_FS_FCloseFile(f);
    return qfalse;
  }
  trap_FS_Read(text, len, f);
  text[len] = 0;
  trap_FS_FCloseFile(f);

  COM_Compress(text);

  // parse the text
  text_p = text;

  // read information for each frame
  for (ws = WEAPON_READY; ws < maxWepStates; ws++) //hypov8 add: beta2
  {
    token = COM_Parse(&text_p);

    if (!token)
      break;

    animations[ws].firstFrame = atoi(token);

    token = COM_Parse(&text_p);
    if (!token)
      break;

    animations[ws].numFrames = atoi(token);

    token = COM_Parse(&text_p);

    if (!token)
      break;

	if (ws == WEAPON_READY && atoi(token) > 0) // disable loop on all but idle
		animations[ws].loopFrames = animations[ws].numFrames;//hypov8 force looping whole sequence
	else
		animations[ws].loopFrames = 0;

    token = COM_Parse(&text_p);
    if (!token)
      break;

    fps = atof(token);
    if (fps == 0)
    {
      Com_Printf(S_COLOR_RED"Error parsing animation frames per second = : %d, defaulting to 15", (int)fps);
      fps = 15;
    }

	//add hypov8 sync animation using a #defined total animation time in engine, not animation.cfg
	//this allows the complte animation sequence to run in the set timeframe
	if (ws == WEAPON_RELOADING && animations[ws].numFrames > 0)
	{
		animations[ws].frameLerp = floor(CG_WeaponReloadTimeOffset(weaponNum) / animations[ws].numFrames);
		animations[ws].initialLerp = floor(CG_WeaponReloadTimeOffset(weaponNum) / animations[ws].numFrames);
	}
	else if (ws == WEAPON_RELOAD_MOD && animations[ws].numFrames > 0)
	{
		animations[ws].frameLerp = floor(CG_WeaponModTimeOffset(weaponNum) / animations[ws].numFrames);
		animations[ws].initialLerp = floor(CG_WeaponModTimeOffset(weaponNum) / animations[ws].numFrames);
	}
	else if (ws == WEAPON_FIRING && animations[ws].numFrames > 0)
	{
		animations[ws].frameLerp = floor(CG_WeaponFireTimeOffset(weaponNum) / animations[ws].numFrames);
		animations[ws].initialLerp = floor(CG_WeaponFireTimeOffset(weaponNum) / animations[ws].numFrames);
	}
	else if ((ws == WEAPON_DROPPING || ws == WEAPON_RAISING) && animations[ws].numFrames > 0)
	{
		animations[ws].frameLerp = floor(CG_WeaponChangeTimeOffset(weaponNum) / animations[ws].numFrames);
		animations[ws].initialLerp = floor(CG_WeaponChangeTimeOffset(weaponNum) / animations[ws].numFrames);
	}
	else //WEAPON_READY
	{
		animations[ws].frameLerp  = 1000 / fps;
		animations[ws].initialLerp = 1000 / fps;
	}


    if (cg_debugWeaponAnim.integer)
       Com_Printf(S_COLOR_WHITE"Mode %d: %d %d %d %f\n", ws, animations[ws].firstFrame, animations[ws].numFrames, animations[ws].loopFrames , fps );
  }

  if (ws != maxWepStates)
  {
    Com_Printf(S_COLOR_RED"Error parsing animation file: %s\n", filename);
    return qfalse;
  }

  return qtrue;
}


/*
==========================
CG_RegisterWeaponAnimation
1st person MD5 weapon regester
==========================
*/
#if 0 //FIXME 0xA5EA: currently unused
static qboolean CG_RegisterWeaponAnimation(animation_t * anim, const char *filename, qboolean loop, qboolean reversed,
    qboolean clearOrigin)
{
  int             frameRate;

  anim->handle = trap_R_RegisterAnimation(filename);
  if (!anim->handle)
  {
    Com_Printf("Failed to load animation file %s\n", filename);
    return qfalse;
  }

  anim->firstFrame = 0;
  anim->numFrames = trap_R_AnimNumFrames(anim->handle);
  frameRate = trap_R_AnimFrameRate(anim->handle);

  if (frameRate == 0)
  {
    frameRate = 1;
  }

	anim->frameLerp = 1000 / frameRate; //hypov8 merge: frameTime
  anim->initialLerp = 1000 / frameRate;

  if (loop)
  {
    anim->loopFrames = anim->numFrames;
  }
  else
  {
    anim->loopFrames = 0;
  }

  anim->reversed = reversed;
  anim->clearOrigin = clearOrigin;

  return qtrue;
}
#endif
/*
=================
CG_RegisterWeapon
The server says this item is used on this level
=================
*/
void CG_RegisterWeapon(int weaponNum)
{
  weaponInfo_t *weaponInfo;
  gitem_t *item, *ammo;
  vec3_t mins, maxs;
  char anim_file[MAX_QPATH];
  int i;

  weaponInfo = &cg_weapons[weaponNum];

  if (weaponNum == 0)
    return;

  if (weaponInfo->registered)
    return;

  Com_Memset(weaponInfo, 0, sizeof(*weaponInfo));
  weaponInfo->registered = qtrue;

  for (item = bg_itemlist + 1; item->classname; item++)
  {
    if (item->giType == IT_WEAPON && item->giTag == weaponNum)
    {
      weaponInfo->item = item;
      break;
    }
  }
  if (!item->classname)
    CG_Error("Couldn't find weapon %i", weaponNum);

  CG_RegisterItemVisuals(item - bg_itemlist);

	// load cmodel before model so filecache works
	if (item->world_model[WORLD_GUNMODEL_POS])
		weaponInfo->weaponModel  = trap_R_RegisterModel(item->world_model[WORLD_GUNMODEL_POS]); // 0xA5EA, test, only worldmodel works ?
	else
	  CG_Error("Missing view wep for index %i", weaponNum);

	if (item->world_model[WORLD_HANDMODEL_POS])
		weaponInfo->handModel = trap_R_RegisterModel(item->world_model[WORLD_HANDMODEL_POS]);
	if (item->world_model[WORLD_FLASHMODEL_POS])
		weaponInfo->flashModel = trap_R_RegisterModel(item->world_model[WORLD_FLASHMODEL_POS]);
	if (item->world_model[WORLD_CLIPMODEL_POS])
		weaponInfo->ammoClipModel = trap_R_RegisterModel(item->world_model[WORLD_CLIPMODEL_POS]);
	if (item->world_model[WORLD_PLAYERWEAPONMODEL_POS])
		weaponInfo->worldPlayerWeaponModel = trap_R_RegisterModel(item->world_model[WORLD_PLAYERWEAPONMODEL_POS]);
	if (item->world_model[WORLD_WEAPONMODEL_POS])
		weaponInfo->worldWeaponModel = trap_R_RegisterModel(item->world_model[WORLD_WEAPONMODEL_POS]);

  // calc midpoint for rotation
  trap_R_ModelBounds(weaponInfo->weaponModel, mins, maxs);
  for (i = 0; i < 3; i++)
  {
    weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * (maxs[i] - mins[i]);
  }

  // calc midpoint of for rotation
  trap_R_ModelBounds(weaponInfo->worldPlayerWeaponModel, mins, maxs);
  for (i = 0; i < 3; i++)
  {
    weaponInfo->eweaponMidpoint[i] = mins[i] + 0.5 * (maxs[i] - mins[i]);
  }

  weaponInfo->weaponIcon = trap_R_RegisterShader(item->icon);
  weaponInfo->ammoIcon = trap_R_RegisterShader(item->icon);

  for (ammo = bg_itemlist + 1; ammo->classname; ammo++)
  {
	  if (weaponNum == WP_PISTOL) //add hypov8. use machinegun ammo as pistol
	  {
		  if (ammo->giType == IT_AMMO && ammo->giTag == WP_MACHINEGUN)
			  break;
	  }
	  else
	  { 
		 if (ammo->giType == IT_AMMO && ammo->giTag == weaponNum)
			break;
	  }
  }

  if (ammo->classname && ammo->world_model[0])
    weaponInfo->ammoModel = trap_R_RegisterModel(ammo->world_model[WORLD_GUNMODEL_POS]);

  //weaponInfo->loopFireSound = qfalse;

  weaponInfo->tagModel = trap_R_RegisterModel("models/weapons/v_tag.md3"); //add tag to all guns

  switch (weaponNum) //hypov8 todo: clean this up. 
  {
  //case WP_PIPE: /hypov8 todo: blackjack

  case WP_CROWBAR:
    Com_sprintf(anim_file, MAX_QPATH, "%s","models/weapons/crowbar/animation.cfg");
    weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/melee/swing.ogg", qfalse);
    break;

  case WP_PISTOL:
	  MAKERGB(weaponInfo->flashDlightColor, 0.5f, 0.5f, 0); //add hypov8
	  Com_sprintf(anim_file, MAX_QPATH, "%s", "models/weapons/colt/animation.cfg");
	  //load mods(complete gun+mod)
	  weaponInfo->mod1WeaponModel = trap_R_RegisterModel("models/weapons/colt/v_wep_mag.md3");  //PW_WPMOD_PISTOLMAGNUM
	  weaponInfo->mod2WeaponModel = trap_R_RegisterModel("models/weapons/colt/v_wep_sil.md3");	//PW_WPMOD_SILENCER
	  weaponInfo->mod3WeaponModel = trap_R_RegisterModel("models/weapons/colt/v_wep_mag_sil.md3"); //PW_WPMOD_PISTOLMAGNUM && PW_WPMOD_SILENCER
	  //load moded sounds
	  weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/pistol/ColtPistollouder.ogg", qfalse);
	  weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/pistol/silencer.ogg", qfalse);

	  //FIXME: 0xA5EA shader and stuff
	  //		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
	  weaponInfo->reloadSound = trap_S_RegisterSound("sound/weapons/pistol/clip_in.ogg", qfalse);
	  weaponInfo->missileTrailFunc = CG_NailTrail;
	  //FIXME(0xA5EA): nailtrail ok ??
	  weaponInfo->wiTrailTime = 700;
	  weaponInfo->trailRadius = 4;
	  weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
    break;

  case WP_MACHINEGUN:
    MAKERGB(weaponInfo->flashDlightColor, 0.5f, 0.5f, 0.0f);
    Com_sprintf(anim_file, MAX_QPATH, "%s","models/weapons/tomgun/animation.cfg");
    weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/machinegun/shot1.ogg", qfalse);
    weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/machinegun/shot2.ogg", qfalse);
    weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/machinegun/shot3.ogg", qfalse);
    weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/machinegun/shot4.ogg", qfalse);
    weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
    weaponInfo->reloadSound = trap_S_RegisterSound("sound/weapons/machinegun/machgcock.ogg", qfalse);
//		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" ); //0xA5EA, oa shader looks crap
    //cgs.media.bulletExplosionShader =  trap_R_RegisterShader( "smokePuff" );
    break;

  case WP_SHOTGUN:
	MAKERGB(weaponInfo->flashDlightColor, 0.5f, 0.5f, 0);
    Com_sprintf(anim_file, MAX_QPATH, "%s","models/weapons/shotgun/animation.cfg");
    weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/shotgun/fire1.ogg", qfalse);
	weaponInfo->reloadSound    = trap_S_RegisterSound("sound/weapons/shotgun/reload.ogg", qfalse);
    weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
    break;

  case WP_ROCKET_LAUNCHER:
	//models
    Com_sprintf(anim_file, MAX_QPATH, "%s","models/weapons/rocketlauncher/animation.cfg");
    weaponInfo->missileModel = trap_R_RegisterModel("models/weapons/rocketlauncher/rocket.md3");
	//sounds
    weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/rocket_launcher/rl_rocket-gverb.ogg", qfalse);
    weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket_launcher/rl_fire.ogg", qfalse);
    weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/rocket_launcher/rl_fire.ogg", qfalse);
    weaponInfo->reloadSound = trap_S_RegisterSound("sound/weapons/rocket_launcher/reload.ogg", qfalse);

    weaponInfo->missileTrailFunc = CG_RocketTrail;
    weaponInfo->missileDlight = 300;
    weaponInfo->wiTrailTime   = 2000;
    weaponInfo->trailRadius   = 128;
    weaponInfo->customSmokePuffshader = cgs.media.smokePuffRlShader;
	//light
    MAKERGB(weaponInfo->missileDlightColor[0],   1.f, 0.75f,   0.f);
    MAKERGB(weaponInfo->missileDlightColor[1], 0.75f,   1.f,   0.f);
    MAKERGB(weaponInfo->missileDlightColor[2],   0.f,   1.f, 0.75f);
    MAKERGB(weaponInfo->flashDlightColor, 0.6f, 0.45f, 0.0f); //hypov8 , 1, 0.75f, 0);

    cgs.media.rocketExplosionShader = trap_R_RegisterShader("rocketExplosion");

    break;

  case WP_GRENADE_LAUNCHER:
    MAKERGB(weaponInfo->flashDlightColor, 1, 0.70f, 0);
	//models
    Com_sprintf(anim_file, MAX_QPATH, "%s","models/weapons/grenadelauncher/animation.cfg");
    weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/grenade1.md3");
    weaponInfo->missileTrailFunc = CG_GrenadeTrail;
    weaponInfo->wiTrailTime = 700;
    weaponInfo->trailRadius = 32;

    weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/grenade_launcher/gl_fire.ogg", qfalse);
    weaponInfo->reloadSound = trap_S_RegisterSound("sound/weapons/grenade_launcher/reload.ogg", qfalse);
    cgs.media.grenadeExplosionShader = trap_R_RegisterShader("grenadeExplosion");
    break;

  case WP_HMG:
	MAKERGB(weaponInfo->flashDlightColor, 0.6f, 0.3f, 0.0f);
    Com_sprintf(anim_file, MAX_QPATH, "%s","models/weapons/hmg/animation.cfg");
    weaponInfo->mod1WeaponModel = trap_R_RegisterModel("models/weapons/hmg/hmgcool.md3");
	weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/hmg/hmg_one.ogg", qfalse);                /* 0xA5EA */
    weaponInfo->reloadSound    = trap_S_RegisterSound("sound/weapons/hmg/hmgcock.ogg", qfalse);            /* 0xA5EA */
    weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;

    //FIXME(0xA5EA): use bulletExplosion shader
//    cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
    //cgs.media.railExplosionShader = trap_R_RegisterShader( "railExplosion" );
    //cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
    //cgs.media.railCoreShader = trap_R_RegisterShader( "railCore" );
    break;

#ifdef USE_FLAMEGUN
  case WP_FLAMER:
	//models
    Com_sprintf(anim_file, MAX_QPATH, "%s","models/weapons/flamegun/animation.cfg");   
#ifdef HYPODEBUG//	CG_Flamer(cent); //debug
	weaponInfo->missileModel = trap_R_RegisterModel("models/weapons/rocketlauncher/rocket.md3");
#endif
    //sounds
	weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/flame_thrower/flame1.ogg", qfalse);
    weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/flame_thrower/flame2.ogg", qfalse);
    weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/flame_thrower/flame3.ogg", qfalse);
    //weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/flame_thrower/flameend.ogg", qfalse);
    //weaponInfo->firingSound //hypov8 todo: looping sound
    weaponInfo->readySound    = trap_S_RegisterSound("sound/weapons/flame_thrower/flamepilot.ogg", qfalse);
	//light
    MAKERGB(weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f);
    //FIXME: 0xA5EA, shader
    break;
#endif

  case WP_GRAPPLING_HOOK:
    MAKERGB(weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f);
    //weaponInfo->tagModel = trap_R_RegisterModel("models/weapons/xx/tag.md3"); //static model?
    weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/hook.md3");
    weaponInfo->missileTrailFunc = CG_GrappleTrail;
    weaponInfo->missileDlight    = 200;
    weaponInfo->wiTrailTime      = 2000;
    weaponInfo->trailRadius      = 64;
    MAKERGB(weaponInfo->missileDlightColor[0], 1, 0.75f, 0);
    MAKERGB(weaponInfo->missileDlightColor[1], 1, 0.75f, 0);
    MAKERGB(weaponInfo->missileDlightColor[2], 1, 0.75f, 0);
    //FIXME(0xA5EA): this files are currently missing
    //weaponInfo->readySound  = trap_S_RegisterSound("sound/weapons/melee/fsthum.ogg", qfalse);
    //weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/melee/fstrun.ogg", qfalse);
    break;

  default:
    MAKERGB(weaponInfo->flashDlightColor, 1, 1, 1);
    weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg", qfalse);
    break;
  }

  if (!CG_ParseWeaponAnimationFile(anim_file, weaponInfo->animations, weaponNum))
     Com_Printf(S_COLOR_RED"Error parsing animation file: %s\n", anim_file);
}


//daemon
/*
===============
CG_InitWeapons

Precaches weapons
===============
*/
void CG_InitWeapons()
{
	int i;

	Com_Memset( cg_weapons, 0, sizeof( cg_weapons ) );

	for ( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
	{
		CG_RegisterWeapon( i );
	}
}



/*
=================
CG_RegisterItemVisuals
The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals(int itemNum)
{
  itemInfo_t *itemInfo;
  gitem_t *item;

  if (itemNum < 0 || itemNum >= bg_numItems)
    CG_Error("CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems - 1);

  itemInfo = &cg_items[itemNum];

  if (itemInfo->registered)
    return;

  item = &bg_itemlist[itemNum];

 // CG_Printf(S_COLOR_BLUE"%s\n", item->classname);
  Com_Memset(itemInfo, 0, sizeof(&itemInfo));
  itemInfo->registered = qtrue;

  itemInfo->models[0] = trap_R_RegisterModel(item->world_model[WORLD_WEAPONMODEL_POS]);

  itemInfo->icon = trap_R_RegisterShader(item->icon);

  if (item->giType == IT_WEAPON)
  {
    //CG_RegisterWeapon(item->giTag); //hypov8 disable
    //FIXME (0xA5EA): loops with CG_RegisterWeapon ?
    //FIXME (0xA5EA): done twice in cg_main
  }

  // powerups have an accompanying ring or sphere
  switch(item->giType)
  {
  case IT_POWERUP:
  case IT_HEALTH:
  case IT_ARMOR:
  case IT_HOLDABLE:
    if (item->world_model[1])
      itemInfo->models[1] = trap_R_RegisterModel(item->world_model[1]);
    break;
  default:;
  }
}

/*
========================================================================================
VIEW WEAPON
========================================================================================
*/

/*
===============
CG_SetWeaponLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
#if 1 //unvan
static void CG_SetWeaponLerpFrameAnimation( weapon_t weapon, lerpFrame_t *lf, int newAnimation )
{
	animation_t *anim;
	qboolean toggle = qfalse;

	lf->animationNumber = newAnimation;
	toggle = newAnimation & ANIM_TOGGLEBIT;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_WEAPON_STATES )
	{
		CG_Error( "Bad animation number: %i", newAnimation );
	}

	anim = &cg_weapons[ weapon ].animations[ newAnimation ];

	lf->animation = anim;

	//animation changed. choose blend mode.
	if (newAnimation == WEAPON_FIRING || newAnimation == WEAPON_RAISING || lf->animationNumber == -1) //hypov8 add: dont tween animations
	{
		lf->animationTime = lf->frameTime = cg.time;
		lf->frame = lf->oldFrame = anim->firstFrame; //hypov8 md3 is not 0
	}
	else
	{
		lf->oldFrame = lf->frame;
		lf->animationTime = lf->frameTime = cg.time + anim->initialLerp; //hypov8 use ani tween time?
		lf->frame = anim->firstFrame; //hypov8 md3 is not 0
	}
	//lf->animationTime = lf->frameTime + anim->initialLerp; //unvan .5
	//lf->frame = lf->oldFrame = 0; //unvan .5

	if (cg_debugAnim.integer)
	{
		CG_Printf( "Anim: %i\n", newAnimation );
	}

	if ( &cg_weapons[ weapon ].md5 && !toggle && lf && lf->old_animation && lf->old_animation->handle )
	{
		if ( !trap_R_BuildSkeleton( &oldGunSkeleton, lf->old_animation->handle, lf->oldFrame, lf->frame, lf->backlerp, lf->old_animation->clearOrigin ) )
		{
			CG_Printf( "CG_SetWeaponLerpFrameAnimation: can't build old gunSkeleton\n" );
			return;
		}
	}
}

/*
===============
CG_WeaponAnimation
===============
*/
static void CG_WeaponAnimation( centity_t *cent, int *old, int *now, float *backLerp )
{
	lerpFrame_t   *lf = &cent->pe.weapon;
	entityState_t *es = &cent->currentState;
	int  newAnimation;

	// see if the animation sequence is switching
	if ( es->weaponAnim != lf->animationNumber || !lf->animation || ( cg_weapons[ es->weapon ].md5 && !lf->animation->handle ) )
	{
		CG_SetWeaponLerpFrameAnimation( (weapon_t) es->weapon, lf, es->weaponAnim );
	}

	CG_RunLerpFrame( lf, 1.0f );

	*old = lf->oldFrame;
	*now = lf->frame;
	*backLerp = lf->backlerp;

	if ( cg_weapons[ es->weapon ].md5 )
	{
		CG_BlendLerpFrame( lf );
		CG_BuildAnimSkeleton( lf, &gunSkeleton, &oldGunSkeleton );
	}
}
#endif
/*
=================
CG_MapTorsoToWeaponFrame

hypov8 merge: not used previously+ using old ver
=================
*/
#if 1
static int CG_MapTorsoToWeaponFrame(clientInfo_t *ci, int frame, int anim, int weaponNum)
{
	if (anim == -1)
		return 0;

	if (0 /*!cg_highPolyPlayerModels.integer*/) //FIXME(0xA5EA):merge //hypov8 IQM models? with 1 animation file
	{
		// change weapon
		if (frame >= ci->animations[TORSO_DROP].firstFrame
			&& frame < ci->animations[TORSO_DROP].firstFrame + 9)
		{
			return frame - ci->animations[TORSO_DROP].firstFrame + 6;
		}

		// stand attack
		if (frame >= ci->animations[TORSO_ATTACK].firstFrame
			&& frame < ci->animations[TORSO_ATTACK].firstFrame + 6)
		{
			return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
		}

		// stand attack 2
		if (frame >= ci->animations[TORSO_ATTACK2].firstFrame && frame < ci->animations[TORSO_ATTACK2].firstFrame + 6)
		{
			return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
		}

		// stand attack 3
		if (frame >= ci->animations[TORSO_ATTACK3].firstFrame && frame < ci->animations[TORSO_ATTACK3].firstFrame + 6)
		{
			return 1 + frame - ci->animations[TORSO_ATTACK3].firstFrame;
		}
	}
	else // MD5 animations all start at 0, so there is no way to differentiate them with first frame alone
	{
		weaponInfo_t *weapon = &cg_weapons[weaponNum];
		weapon->weaponAnimState;
		// change weapon
		if (anim == TORSO_DROP && frame < 9)
		{
			return frame + weapon->animations[WEAPON_DROPPING].firstFrame;
		}
		else if (anim == TORSO_RAISE && frame < 9)
		{
			return frame + weapon->animations[WEAPON_RAISING].firstFrame;;
		}
		else if ((anim == TORSO_ATTACK || anim == TORSO_ATTACK2 || anim == TORSO_ATTACK3))	// stand attack
		{
			return frame + weapon->animations[WEAPON_FIRING].firstFrame;
		}
	}

	return 0;
}
#endif

/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition(vec3_t origin, vec3_t angles)
{
  float scale;
  int delta;
  float fracsin;

  VectorCopy(cg.refdef.vieworg, origin);
  VectorCopy(cg.refdefViewAngles, angles);

  // on odd legs, invert some angles
  if (cg.bobcycle & 1)
    scale = -cg.xyspeed;
  else
    scale = cg.xyspeed;

  // gun angles from bobbing //edit hypov8
  angles[ROLL]  += scale * cg.bobfracsin * 0.001;		//hypov8 was 0.005
  angles[YAW]   += scale * cg.bobfracsin * 0.001;		//hypov8 was 0.01
  angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.001;	//hypov8 was 0.005

  // drop the weapon when landing
  delta = cg.time - cg.landTime;

  if (delta < LAND_DEFLECT_TIME)
    origin[2] += cg.landChange * 0.25 * delta / LAND_DEFLECT_TIME;
  else if (delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME)
    origin[2] += cg.landChange*0.25*(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta)/LAND_RETURN_TIME;

#if 0
  // drop the weapon when stair climbing
  delta = cg.time - cg.stepTime;
  if (delta < STEP_TIME / 2)
  {
    origin[2] -= cg.stepChange*0.25*delta/(STEP_TIME/2);
  }
  else if (delta < STEP_TIME)
  {
    origin[2] -= cg.stepChange*0.25*(STEP_TIME - delta)/(STEP_TIME/2);
  }
#endif

  // idle drift //note hypov8 weapon while standing still. scale was 120
  scale          =  20;  //+ 40;cg.xyspeed //causes weapon to go down with speed increasing, sux
  fracsin        = sin(cg.time * 0.001);
  angles[ROLL]  += scale * fracsin * 0.01;
  angles[YAW]   += scale * fracsin * 0.01;
  angles[PITCH] += scale * fracsin * 0.01;
}


//FIXME(0xA5EA): use this for flamegun ?
#if 1
/*
===============
CG_LightningBolt
Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/
static void CG_LightningBolt(centity_t *cent, vec3_t origin)
{
  trace_t trace;
  refEntity_t beam;
  vec3_t forward;
  vec3_t muzzlePoint, endPoint;

  Com_Memset(&beam, 0, sizeof(beam));

//unlagged - attack prediction #1
	// if the entity is us, unlagged is on server-side, and we've got it on for the lightning gun
	if ( (cent->currentState.number == cg.predictedPlayerState.clientNum) && cgs.delagHitscan &&
			( cg_delag.integer & 1 || cg_delag.integer & 8 ) ) {
		// always shoot straight forward from our current position
		AngleVectors( cg.predictedPlayerState.viewangles, forward, NULL, NULL );
		VectorCopy( cg.predictedPlayerState.origin, muzzlePoint );
	}
	else
//unlagged - attack prediction #1
  // CPMA  "true" lightning
  if ((cent->currentState.number == cg.predictedPlayerState.clientNum) && (cg_trueLightning.value != 0))
  {
    vec3_t angle;
    int i;
//unlagged - true lightning
		// might as well fix up true lightning while we're at it
		vec3_t viewangles;
		VectorCopy( cg.predictedPlayerState.viewangles, viewangles );
//unlagged - true lightning
    for (i = 0; i < 3; i++)
    {
      float a = cent->lerpAngles[i] - cg.refdefViewAngles[i];

      if (a > 180)
        a -= 360;

      if (a < -180)
        a += 360;

      angle[i] = cg.refdefViewAngles[i] + a * (1.0 - cg_trueLightning.value);

      if (angle[i] < 0)
        angle[i] += 360;

      if (angle[i] > 360)
        angle[i] -= 360;
    }

    AngleVectors(angle, forward, NULL, NULL);
//unlagged - true lightning
//    VectorCopy(cent->lerpOrigin, muzzlePoint);
//		VectorCopy(cg.refdef.vieworg, muzzlePoint );
		// *this* is the correct origin for true lightning
		VectorCopy(cg.predictedPlayerState.origin, muzzlePoint );
    //VectorCopy(cent->lerpOrigin, muzzlePoint); //kpq3 unlag
//		VectorCopy(cg.refdef.vieworg, muzzlePoint ); //old code kpq3 unlag
//unlagged - true lightning
  }
  else
  {
    // !CPMA
    AngleVectors(cent->lerpAngles, forward, NULL, NULL);
    VectorCopy(cent->lerpOrigin, muzzlePoint);
  }

  // FIXME: crouch
  muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

  VectorMA(muzzlePoint, 14, forward, muzzlePoint);

  // project forward by the lightning range
  VectorMA(muzzlePoint, FLAMETHROWER_RANGE, forward, endPoint);

  // see if it hit a wall
  CG_Trace(&trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, cent->currentState.number, MASK_SHOT);

  // this is the endpoint
  VectorCopy(trace.endpos, beam.oldorigin);

  // use the provided origin, even though it may be slightly
  // different than the muzzle origin
  VectorCopy(origin, beam.origin);

  beam.reType = RT_BEAM; // was RT_LIGHTNING; //hypov8 merge: ok?
  beam.customShader = cgs.media.lightningShader;
  trap_R_AddRefEntityToScene(&beam);

  // add the impact flare if it hit something
  if (trace.fraction < 1.0)
  {
    vec3_t angles;
    vec3_t dir;

    VectorSubtract(beam.oldorigin, beam.origin, dir);
    VectorNormalize(dir);

    Com_Memset(&beam, 0, sizeof(beam));
    beam.hModel = cgs.media.lightningExplosionModel;

    VectorMA(trace.endpos, -16, dir, beam.origin);

    // make a random orientation
    angles[0] = rand() % 360;
    angles[1] = rand() % 360;
    angles[2] = rand() % 360;
    AnglesToAxis(angles, beam.axis);
    trap_R_AddRefEntityToScene(&beam);
  }
}
#endif
/*

static void CG_LightningBolt( centity_t *cent, vec3_t origin ) {
    trace_t		trace;
    refEntity_t		beam;
    vec3_t			forward;
    vec3_t			muzzlePoint, endPoint;

    if ( cent->currentState.weapon != WP_LIGHTNING ) {
        return;
    }

    Com_Memset( &beam, 0, sizeof( beam ) );

    // find muzzle point for this frame
    VectorCopy( cent->lerpOrigin, muzzlePoint );
    AngleVectors( cent->lerpAngles, forward, NULL, NULL );

    // FIXME: crouch
    muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

    VectorMA( muzzlePoint, 14, forward, muzzlePoint );

    // project forward by the lightning range
    VectorMA( muzzlePoint, LIGHTNING_RANGE, forward, endPoint );

    // see if it hit a wall
    CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint,
        cent->currentState.number, MASK_SHOT );

    // this is the endpoint
    VectorCopy( trace.endpos, beam.oldorigin );

    // use the provided origin, even though it may be slightly
    // different than the muzzle origin
    VectorCopy( origin, beam.origin );

    beam.reType = RT_LIGHTNING;
    beam.customShader = cgs.media.lightningShader;
    trap_R_AddRefEntityToScene( &beam );

    // add the impact flare if it hit something
    if ( trace.fraction < 1.0 ) {
        vec3_t	angles;
        vec3_t	dir;

        VectorSubtract( beam.oldorigin, beam.origin, dir );
        VectorNormalize( dir );

        Com_Memset( &beam, 0, sizeof( beam ) );
        beam.hModel = cgs.media.lightningExplosionModel;

        VectorMA( trace.endpos, -16, dir, beam.origin );

        // make a random orientation
        angles[0] = rand() % 360;
        angles[1] = rand() % 360;
        angles[2] = rand() % 360;
        AnglesToAxis( angles, beam.axis );
        trap_R_AddRefEntityToScene( &beam );
    }
}
*/

/*
===============
CG_SpawnRailTrail
Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
===============
//hypov8 removed
*//*
static void CG_SpawnRailTrail(centity_t *cent, vec3_t origin)
{
  clientInfo_t *ci;
  //FIXME(0xA5EA): do i need this function ?

//	if ( cent->currentState.weapon != WP_RAILGUN ) {  //0xA5EA, no railgun
  return;
//	}
  if (!cent->pe.railgunFlash)
  {
    return;
  }
  cent->pe.railgunFlash = qtrue;
  ci = &cgs.clientinfo[cent->currentState.clientNum];
  CG_RailTrail(ci, origin, cent->pe.railgunImpact);
}
*/
/*
======================
CG_MachinegunSpinAngle
======================
*/
#if 0 //0xA5EA FIXME: currently unused
#define     SPIN_SPEED 0.9
#define     COAST_TIME 1000
static float CG_MachinegunSpinAngle(centity_t *cent)
{
  int delta;
  float angle;
  float speed;

  delta = cg.time - cent->pe.barrelTime;
  if (cent->pe.barrelSpinning)
  {
    angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
  }
  else
  {
    if (delta > COAST_TIME)
    {
      delta = COAST_TIME;
    }

    speed = 0.5 * (SPIN_SPEED + (float)(COAST_TIME - delta) / COAST_TIME);
    angle = cent->pe.barrelAngle + delta * speed;
  }

  if (cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING))
  {
    cent->pe.barrelTime     = cg.time;
    cent->pe.barrelAngle    = AngleMod(angle);
    cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
    /* //FIXME: 0xA5EA,
//     #ifdef MISSIONPACK
            if ( cent->currentState.weapon == WP_CHAINGUN && !cent->pe.barrelSpinning ) {
                trap_S_StartSound( NULL, cent->currentState.number, CHAN_WEAPON, trap_S_RegisterSound( "sound/weapons/vulcan/wvulwind.ogg", qfalse ) );
            }
    #endif
    */
  }
  return angle;
}
#endif

/*
========================
CG_AddWeaponWithPowerups
========================
*/
#if 0 //FIXME 0xA5EA: currently unused
static void CG_AddWeaponWithPowerups(refEntity_t *gun, int powerups)
{
  // add powerup effects
  trap_R_AddRefEntityToScene(gun);
#if 0
  if (powerups & (1 << PW_INVIS))
  {
    gun->customShader = cgs.media.invisShader;
    trap_R_AddRefEntityToScene(gun);
  }
  else
  {
    trap_R_AddRefEntityToScene(gun);

    if (powerups & (1 << PW_BATTLESUIT))
    {
      gun->customShader = cgs.media.battleWeaponShader;
      trap_R_AddRefEntityToScene(gun);
    }
    //0xA5EA, removed
    if (powerups & (1 << PW_QUAD))
    {
      gun->customShader = cgs.media.quadWeaponShader;
      trap_R_AddRefEntityToScene(gun);
    }
  }
#endif
}
#endif

static void CG_AddViewFlashlight(void) //note hypov8 add thirdperson view.. also add zoom fov
{
#if 0 // hypov8 disable flashlight in 1st person view, should we enable this and use 3rd person seperate
  refLight_t light;
  vec_t fov_x;
  Com_Memset(&light, 0, sizeof(refLight_t));

  if (!cgs.media.flashLightShader)
  {
    CG_Printf(S_COLOR_RED"no flashlight shader\n");
    return;
  }

  light.attenuationShader = cgs.media.flashLightShader;

  light.rlType = RL_PROJ;

  VectorCopy(cg.refdef.vieworg, light.origin);

  light.color[0] = 1.0f;
  light.color[1] = 1.0f;
  light.color[2] = 0.8f;

  QuatClear(light.rotation);
  fov_x = tan((cg.refdef.fov_x * 0.5f)*M_DEG2RAD);
  VectorCopy(cg.refdef.viewaxis[0], light.projTarget);

  fov_x /= 2;

  VectorScale(cg.refdef.viewaxis[1], -fov_x, light.projRight);
  VectorScale(cg.refdef.viewaxis[2], fov_x,  light.projUp);
  VectorScale(cg.refdef.viewaxis[0], 1,     light.projStart);
  VectorScale(cg.refdef.viewaxis[0], 1000,   light.projEnd);
  trap_R_AddRefLightToScene(&light);
#endif
}

/*
=============
CG_AddPlayerWeapon
Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon(refEntity_t *parent, playerState_t *ps, centity_t *cent)
{
	static refEntity_t gun;
	static refEntity_t hand;
	static refEntity_t ammo;
	static refEntity_t flash;
	vec3_t       angles;
	weapon_t weaponNum;
	weaponInfo_t *weapon;
	qboolean firing;
	qboolean    noGunModel;

	weaponNum = (weapon_t)cent->currentState.weapon;

	weapon = &cg_weapons[weaponNum];
	if ( !weapon->registered )
	{
		Com_Printf( S_COLOR_RED "CG_AddPlayerWeapon: weapon %d (ss) "
			"is not registered", weaponNum/*, BG_Weapon( weaponNum )->name*/ );
		return;
	}

  firing = ((cent->currentState.eFlags & EF_FIRING) != 0);

  // add the weapon
  Com_Memset(&gun, 0, sizeof(refEntity_t)); //1st/3rd
  Com_Memset(&hand, 0, sizeof(refEntity_t));
  Com_Memset(&ammo, 0, sizeof(refEntity_t));
  Com_Memset(&flash, 0, sizeof(refEntity_t));

  //copy state from parent
  VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
  gun.renderfx    = parent->renderfx;

  //gun.shadowPlane = parent->shadowPlane;


  //hand.shadowPlane = ammo.shadowPlane = hand.shadowPlane;
  //hand.renderfx = ammo.renderfx = parent->renderfx;

	if (!ps) //third person 
	{
		gun.hModel = weapon->worldPlayerWeaponModel;
	}
	else //1st person (firstperson)
	{
		Vector4Set(gun.shaderRGBA, (byte)255, (byte)255, (byte)255, (byte)255);

		if (weapon->item->giTag == WP_HMG)
		{
			if(ps->stats[STAT_WEAP_MODS] & (1<<PW_WPMOD_COOLING))
			{
				gun.hModel = weapon->mod1WeaponModel;
			}
			else
			{
				gun.hModel = weapon->weaponModel;
			}  
		}		//FIXME (0xA5EA): pistol
		else if (weapon->item->giTag == WP_PISTOL)
		{
			if ((ps->stats[STAT_WEAP_MODS] & (1<<PW_WPMOD_PISTOLMAGNUM)) && (ps->stats[STAT_WEAP_MODS] & (1<<PW_WPMOD_SILENCER)))
			{
				gun.hModel = weapon->mod3WeaponModel;
			}
			else if (!(ps->stats[STAT_WEAP_MODS] & (1<<PW_WPMOD_PISTOLMAGNUM)) && (ps->stats[STAT_WEAP_MODS] & (1<<PW_WPMOD_SILENCER)))
			{
				gun.hModel = weapon->mod2WeaponModel;
			}
			else if ((ps->stats[STAT_WEAP_MODS] & (1<<PW_WPMOD_PISTOLMAGNUM)) && !(ps->stats[STAT_WEAP_MODS] & (1<<PW_WPMOD_SILENCER)))
			{
				gun.hModel = weapon->mod1WeaponModel;
			}
			else
			{
				gun.hModel = weapon->weaponModel;
			}
		}
		else //no mods
		{
			gun.hModel = weapon->weaponModel;
		}
	} //end 1st person

  	noGunModel = ( ( !ps || cg.renderingThirdPerson ) && 0 /*weapon->disableIn3rdPerson*/  )|| !gun.hModel;

 // if (!gun.hModel)
  //  return;

	if (!ps) //third person
	{
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
		if ((cent->currentState.eFlags & EF_FIRING) && weapon->firingSound)
		{
			// lightning gun and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound);
			cent->pe.lightningFiring = qtrue;
		}
		else if (weapon->readySound)
		{
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound);
		}
	}

	if (!noGunModel)
	{
		if ( ps )	//first person
		{
			CG_PositionEntityOnTag(&gun, parent, parent->hModel, "tag_weapon");
			CG_WeaponAnimation( cent, &gun.oldframe, &gun.frame, &gun.backlerp );
		}
		else
		{
			angles[PITCH] = 0;
			angles[YAW] = 90;  //hypov8 todo: fix thug player model tag error. remove this
			angles[ROLL] = 90;
			AnglesToAxis(angles, gun.axis);
			CG_PositionRotatedEntityOnTag(&gun, parent, parent->hModel, "tag_weapon");
		}

#if 0	//use md5?
		if ( weapon->md5 )
		{

			gun.skeleton = gunSkeleton;

			if ( weapon->rotationBone[ 0 ] && ps )
			{
				int    boneIndex = trap_R_BoneIndex( gun.hModel, weapon->rotationBone );
				quat_t rotation;
				matrix_t mat;
				vec3_t   nBounds[ 2 ];

				if ( boneIndex < 0 )
				{
					Log::Warn( "Cannot find bone index %s, using root bone",
								weapon->rotationBone );
					weapon->rotationBone[ 0 ] = '\0'; // avoid repeated warnings
					boneIndex = 0;
				}

				QuatFromAngles( rotation, weapon->rotation[ 0 ], weapon->rotation[ 1 ], weapon->rotation[ 2 ] );
				QuatMultiply2( gun.skeleton.bones[ boneIndex ].t.rot, rotation );

				// Update bounds to reflect rotation
				MatrixFromAngles( mat, weapon->rotation[ 0 ], weapon->rotation[ 1 ], weapon->rotation[ 2 ] );

				MatrixTransformBounds(mat, gun.skeleton.bounds[0], gun.skeleton.bounds[1], nBounds[0], nBounds[1]);

				BoundsAdd( gun.skeleton.bounds[ 0 ], gun.skeleton.bounds[ 1 ], nBounds[ 0 ], nBounds[ 1 ] );
			}

			CG_TransformSkeleton( &gun.skeleton, weapon->scale );
		} //end md5
#endif

		trap_R_AddRefEntityToScene(&gun);

		//add extra models
		if (ps)	//first person
		{
			//add ammo model //hypov8 todo: remove. combine with gun model
			ammo.hModel = weapon->ammoClipModel;
			if (ammo.hModel)
			{
				CG_PositionEntityOnTag(&ammo, parent, parent->hModel, "tag_weapon");

				//copy state from parent
				VectorCopy( parent->lightingOrigin, ammo.lightingOrigin );
				ammo.renderfx = parent->renderfx;
				ammo.oldframe = gun.oldframe;
				ammo.frame = gun.frame;
				ammo.backlerp = gun.backlerp;

				trap_R_AddRefEntityToScene(&ammo);
			}

			//add hand model
			hand.hModel = weapon->handModel;
			if (hand.hModel)
			{
				CG_PositionEntityOnTag(&hand, parent, parent->hModel, "tag_weapon");

				//copy state from parent
				VectorCopy( parent->lightingOrigin, hand.lightingOrigin );
				hand.renderfx = parent->renderfx;
				hand.oldframe = gun.oldframe;
				hand.frame = gun.frame;
				hand.backlerp = gun.backlerp;

				trap_R_AddRefEntityToScene(&hand);
			}
		}
	}


//hypov8 merge: store origin 4 later
  //VectorCopy(parent->lightingOrigin, tag_flash_origin);
  //tag_flash_shadowPlane = parent->shadowPlane;
  //tag_flash_renderfx   = parent->renderfx;

  // make sure we aren't looking at cg.predictedPlayerEntity for LG
  //nonPredictedCent = &cg_entities[cent->currentState.clientNum];

  // if the index of the nonPredictedCent is not the same as the clientNum
  // then this is a fake player (like on teh single player podiums), so
  // go ahead and use the cent
  //if ((nonPredictedCent - cg_entities) != cent->currentState.clientNum)
    //nonPredictedCent = cent;


	// add the flash
	if ((weaponNum == WP_CROWBAR || weaponNum == WP_GRAPPLING_HOOK))/*&& (nonPredictedCent->currentState.eFlags & EF_FIRING)*/
	{
		//intermittent flash
		if (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME /*&& !cent->pe.railgunFlash*/)
			return;
	}
	else if (weaponNum == WP_FLAMER)
	{
		if (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME2 /*&& !cent->pe.railgunFlash*/)
			return;
	}  
	else
	{   // impulse flash
		if (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME /*&& !cent->pe.railgunFlash*/)
			return;
	}


	flash.hModel = weapon->flashModel;
	if (flash.hModel)
	{
		VectorCopy(parent->lightingOrigin, flash.lightingOrigin);
		flash.shadowPlane = parent->shadowPlane;
		flash.renderfx    = parent->renderfx;
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = crandom() * 10;
		AnglesToAxis(angles, flash.axis);
		if (noGunModel)
			CG_PositionRotatedEntityOnTag(&flash, parent, parent->hModel, "tag_weapon");
		else 
			CG_PositionRotatedEntityOnTag(&flash, &gun, gun.hModel, "tag_flash");

		trap_R_AddRefEntityToScene(&flash);
	}

	if (ps || cg.renderingThirdPerson || cent->currentState.number != cg.predictedPlayerState.clientNum)
	{
		if (firing && !(cent->currentState.eFlags & (EF_DEAD)))
		{
			CG_FlamethrowerFlame(cent, flash.origin);
		}

		if (weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2])
		{
			trap_R_AddAdditiveLightToScene(flash.origin, 300 + (rand() & 31),
			weapon->flashDlightColor[0], weapon->flashDlightColor[1], weapon->flashDlightColor[2]);
		}
    }
}

#if 0
void CG_AddPlayerWeaponLight(playerState_t *ps, centity_t *cent, refEntity_t parent)
{
	qboolean firing;
	weapon_t weaponNum;
	centity_t *nonPredictedCent;
	static refEntity_t flash;
	weaponInfo_t *weapon;
	vec3_t angles;


	weaponNum = (weapon_t)cent->currentState.weapon;
	weapon = &cg_weapons[weaponNum];
	firing = ((cent->currentState.eFlags & EF_FIRING) != 0);

	Com_Memset(&flash, 0, sizeof(flash));

#if 0
	static refEntity_t gun;
	static refEntity_t mod;
	qboolean firing;
	CG_RegisterWeapon(weaponNum);

	firing = ((cent->currentState.eFlags & EF_FIRING) != 0);
	// add the weapon
	Com_Memset(&gun, 0, sizeof(gun));
	Com_Memset(&mod, 0, sizeof(mod));
	Com_Memset(&flash, 0, sizeof(flash));
	VectorCopy(parent->lightingOrigin, gun.lightingOrigin);
	VectorCopy(parent->lightingOrigin, mod.lightingOrigin);

	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;
	mod.shadowPlane = gun.shadowPlane;
	mod.renderfx = gun.renderfx;

	if (ps) //1st person (firstperson)
	{
		Vec4_Set(gun.shaderRGBA, (byte)255, (byte)255, (byte)255, (byte)255);
		Vec4_Set(mod.shaderRGBA, (byte)255, (byte)255, (byte)255, (byte)255);
		gun.hModel = weapon->weaponModel;
		mod.hModel = weapon->barrelModel;
	}
	else //third person 
		gun.hModel = weapon->worldPlayerWeaponModel;

	if (!gun.hModel)
		return;

	if (!ps) //third person (gun) (thirdperson)
	{
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
		if ((cent->currentState.eFlags & EF_FIRING) && weapon->firingSound)
		{
			// lightning gun and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound);
			cent->pe.lightningFiring = qtrue;
		}
		else if (weapon->readySound)
		{
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound);
		}

		CG_PositionRotatedEntityOnBone(&gun, parent, parent->hModel, "tag_weapon");

		trap_R_AddRefEntityToScene(&gun);
	}
	else //1st person (gun+mods)
	{
		VectorCopy(parent->origin, gun.origin);
		Axis_Copy(parent->axis, gun.axis);

		if (mod.hModel)
		{
			VectorCopy(parent->origin, mod.origin);
			Axis_Copy(parent->axis, mod.axis);

		}

		if (weapon->item->giTag == WP_HMG)
		{
			if (ps->stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_COOLING))
			{
				gun.hModel = weapon->mod1WeaponModel;
			}
			//FIXME (0xA5EA): pistol
		}
		else if (weapon->item->giTag == WP_PISTOL)
		{
#if 0
			PW_WPMOD_PISTOLMAGNUM,
				PW_WPMOD_PISTOLDOUBLEFIRE,
				PW_WPMOD_SILENCER,
#endif
				if ((ps->stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_PISTOLMAGNUM)) && (ps->stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_SILENCER)))
				{
					gun.hModel = weapon->mod3WeaponModel;
				}
				else if (!(ps->stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_PISTOLMAGNUM)) && (ps->stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_SILENCER)))
				{
					gun.hModel = weapon->mod2WeaponModel;
				}
				else if ((ps->stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_PISTOLMAGNUM)) && !(ps->stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_SILENCER)))
				{
					gun.hModel = weapon->mod1WeaponModel;
				}
				else
				{
					gun.hModel = weapon->weaponModel;
				}
		}

		// Com_Printf("handframe1: %i\n", parent->frame );
		//Com_Printf("weaponstate: %i\n", ps->weaponstate );

		gun.oldframe = parent->oldframe;
		gun.frame = parent->frame;
		gun.backlerp = parent->backlerp;

		if (mod.hModel)
		{
			mod.oldframe = parent->oldframe;
			mod.frame = parent->frame;
			mod.backlerp = parent->backlerp;
		}

		trap_R_AddRefEntityToScene(&gun);

		if (mod.hModel)
			trap_R_AddRefEntityToScene(&mod);

	}
#endif
	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if ((nonPredictedCent - cg_entities) != cent->currentState.clientNum)
		nonPredictedCent = cent;


	//--
	// add the flash
	if ((weaponNum == WP_CROWBAR || weaponNum == WP_GRAPPLING_HOOK/* || weaponNum == WP_FLAMER*/) //test hypov8 removed flamer. cause flash to work at wrong time
		&& (nonPredictedCent->currentState.eFlags & EF_FIRING))
	{
		if (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && !cent->pe.railgunFlash)
			return;
		//hypov8 not!!! continuous flash
	}
	else if (weaponNum == WP_FLAMER) //add hypov8 longer muzzel flash
	{
		if (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME2 && !cent->pe.railgunFlash)
			return;
	}
	else
	{   // impulse flash
		if (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && !cent->pe.railgunFlash)
			return;
	}

	Com_Memset(&flash, 0, sizeof(flash));
	VectorCopy(tag_flash_origin, flash.lightingOrigin);

	flash.shadowPlane = tag_flash_shadowPlane;
	flash.renderfx = tag_flash_renderfx;


#ifdef USE_REFENTITY_NOSHADOWID
	flash.noShadowID = parent->noShadowID; //add hypov8
#endif

	flash.hModel = weapon->flashModel;
	//if(!flash.hModel)	 {  return;   }

	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;
	AnglesToAxis(angles, flash.axis);

	// CG_PositionRotatedEntityOnTag(&flash, &gun, weapon->weaponModel, "tag_flash");
	CG_PositionRotatedEntityOnTag(&flash, &parent, parent.hModel, "tag_flash");
	trap_R_AddRefEntityToScene(&flash);

	//todo: hypo, missing light in 1st person
	if ( ps || cg.renderingThirdPerson || cent->currentState.number != cg.predictedPlayerState.clientNum)
	{
		if (firing && !(cent->currentState.eFlags & (EF_DEAD)))
		{
			CG_FlamethrowerFlame(cent, flash.origin);

			if (weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2])
			{
				trap_R_AddAdditiveLightToScene(flash.origin, 100 /*300 + (rand() & 31*/, //hypov8 const size
					weapon->flashDlightColor[0], weapon->flashDlightColor[1], weapon->flashDlightColor[2]);
			}
		}
	}
}
#endif



/*
===============
CG_SetWeaponLerpFrameAnimation
may include ANIM_TOGGLEBIT
===============
*/
#if 0 //kpq3
static void CG_SetWeaponLerpFrameAnimation(weaponInfo_t * wi, lerpFrame_t * lf, int weaponNumber, int weaponAnimation, int weaponTime)
{
	animation_t    *anim;
	int             shouldTime, wouldTime;

	// save old animation
	lf->old_animationNumber = lf->animationNumber;
	lf->old_animation = lf->animation;
	lf->old_weaponNumber = lf->weaponNumber;

	lf->weaponNumber = weaponNumber;
	lf->animationNumber = weaponAnimation;

	weaponAnimation &= ~ANIM_TOGGLEBIT;

	if(weaponAnimation < 0 || weaponAnimation >= MAX_WEAPON_STATES)
		CG_Error("bad weapon animation number: %i", weaponAnimation);

	anim = &wi->animations[weaponAnimation];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	shouldTime = weaponTime;
	wouldTime = anim->numFrames * anim->frameLerp; //hypov8 merge: ok?

	 if(cg_debugWeaponAnim.integer)
	   Com_Printf("(%d, %d) would, should\n", wouldTime, shouldTime);

	// if the time for a animation is smaller than the timeoffset between weapon fires
	// scale the animation
	if(shouldTime <= wouldTime && shouldTime > 0)
	{
		lf->animationScale = (float)wouldTime / shouldTime;

		 if(cg_debugWeaponAnim.integer)
		   Com_Printf("lf->animationScale %f\n",lf->animationScale);
	}
	else
	{
		lf->animationScale = 1.0f;
	}

  if(cg_debugWeaponAnim.integer)
    Com_Printf("CG_SetWeaponLerpFrameAnimation: weapon=%i new anim=%i old anim=%i time=%i\n", weaponNumber, lf->animationNumber, lf->old_animationNumber, weaponTime);

	if(lf->old_animationNumber <= 0 || lf->old_weaponNumber != lf->weaponNumber)
	{
		// skip initial / invalid blending
		lf->blendlerp = 0.0f;
		return;
	}

	// TODO: blend through two blendings!
	if((lf->blendlerp <= 0.0f))
		lf->blendlerp = 1.0f;
	else
		lf->blendlerp = 1.0f - lf->blendlerp;	// use old blending for smooth blending between two blended animations

#if 0
#if 0
	Com_Memcpy(&lf->oldSkeleton, &lf->skeleton, sizeof(refSkeleton_t));
#else
	if(!trap_R_BuildSkeleton(&lf->oldSkeleton, lf->old_animation->handle, lf->oldFrame, lf->frame, lf->blendlerp, lf->old_animation->clearOrigin))
	{
		CG_Printf("CG_SetWeaponLerpFrameAnimation: can't build old skeleton\n");
		return;
	}
#endif
#endif

	if(cg_debugWeaponAnim.integer)//hypov8 merge: ok?
	{
		Com_Printf("CG_SetWeaponLerpFrameAnimation: weapon=%i new anim=%i old anim=%i time=%i\n", weaponNumber, weaponAnimation, lf->old_animationNumber, weaponTime);
	}

}
#elif 0 //ET
static void CG_SetWeapLerpFrameAnimation( weaponInfo_t *wi, lerpFrame_t *lf, int newAnimation ) {
	animation_t *anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_WEAPON_STATES ) {
		CG_Error( "Bad animation number (CG_SWLFA): %i", newAnimation );
	}

	anim = &wi->animations[ newAnimation ];

	lf->animation       = anim;
	lf->animationTime   = lf->frameTime + anim->initialLerp;

	if ( cg_debugAnim.integer & 2 ) {
		CG_Printf( "Weap Anim: %d\n", newAnimation );
	}
}
#endif

/*
===============
CG_ClearWeapLerpFrame
===============
*/
#if 0
void CG_ClearWeapLerpFrame( weaponInfo_t *wi, lerpFrame_t *lf, int animationNumber ) //hypov8 ET
{
	if (cg_debugWeaponAnim.integer)	
		Com_Printf("newAnim Firing\n");

	lf->frameTime = lf->oldFrameTime = cg.time;
	CG_SetWeapLerpFrameAnimation( wi, lf, animationNumber );
	lf->oldFrame = lf->frame = lf->animation->firstFrame;
	//lf->oldFrame = lf->frame = lf->animation->mdxFile;
}
#endif
/*
===============
CG_RunWeaponLerpFrame
Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
#if 0
static void CG_RunWeaponLerpFrame(weaponInfo_t * wi, lerpFrame_t * lf, int weaponNumber, int weaponAnimation, int weaponTime,  float speedScale)
{
	int f = 0, numFrames = 0;
	animation_t *anim;
	qboolean animChanged = qfalse;

	// debugging tool to get no animations
	if(cg_animSpeed.integer == 0)
	{
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if ( !lf->animation ) 
	{
		CG_ClearWeapLerpFrame( wi, lf, weaponAnimation );
	} 
	else if (weaponAnimation != lf->animationNumber)
	{
		if (cg_debugWeaponAnim.integer)	
			Com_Printf("wep anim changed. From %d to %d\n", lf->animationNumber, (weaponAnimation & ~ANIM_TOGGLEBIT));

		//animation changed. choose blend mode.
		if ((weaponAnimation & ~ANIM_TOGGLEBIT) == WEAPON_FIRING || (weaponAnimation & ~ANIM_TOGGLEBIT) == WEAPON_RAISING)
			CG_ClearWeapLerpFrame(wi, lf, weaponAnimation); //instant
		else	
			CG_SetWeapLerpFrameAnimation(wi, lf, weaponAnimation); //smooth
	}

	//hypov8 todo: md5? md3 is lossy (vertex wobble!!!)
	//loss is not noticable if model frames changes are greatly differnt. idle animations are mostly obvious
	//md2/mdx is very obvious and conversions should not use idle sequence
	//hypov8 todo: remove idle frames? replace with code idle/sway
	//copy ET

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( cg.time >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( !anim->frameLerp ) {
			return;     // shouldn't happen
		}
		if ( cg.time < lf->animationTime ) {
			lf->frameTime = lf->animationTime;      // initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
		f *= speedScale;        // adjust for haste, etc
		if ( f >= anim->numFrames ) {
			f -= anim->numFrames;
			if ( anim->loopFrames ) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = anim->numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}
		lf->frame = anim->firstFrame + f;
		//lf->frameModel = anim->mdxFile;
		if ( cg.time > lf->frameTime ) {
			lf->frameTime = cg.time;
			if ( cg_debugAnim.integer ) {
				CG_Printf( "Clamp lf->frameTime\n" );
			}
		}
	}

	if ( lf->frameTime > cg.time + 200 ) {
		lf->frameTime = cg.time;
	}

	if ( lf->oldFrameTime > cg.time ) {
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}
#endif

/*
=================
CG_WeaponAnimation
=================
*/
#if 0 //old kpq3
static void CG_WeaponAnimation(centity_t * cent, weaponInfo_t * weapon, int weaponNumber, int weaponState, int weaponTime)
{
	//int clientNum;
	float speedScale = 1;

	//clientNum = cent->currentState.clientNum;

	// change weapon animation
	CG_RunWeaponLerpFrame(weapon, &cent->pe.weapon, weaponNumber, weaponState, weaponTime, speedScale);
}
#endif

#if 0 //not used anymore
/*
==============
CG_CalcWeaponState
==============
*/
static weaponstate_t CG_CalcWeaponState(centity_t * cent, playerState_t *ps)
{
#if 0
  typedef enum
  {
    WEAPON_READY,
    WEAPON_RAISING,
    WEAPON_DROPPING,
    WEAPON_FIRING,
    WEAPON_RELOADING,
    MAX_WEAPON_STATES,
    WEAPON_HM_LOCK = 0x10,  // is used as a flag
    WEAPON_HM_AUTOSWITCH
    //FIXME: 0xA5EA, weaponreload states
  } weaponstate_t;

#endif
  //FIXME(0xA5EA): reload states
  int torsoAnim = (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT);
  int weaponState = ps->weaponstate;
  int animToggleBit = cent->currentState.torsoAnim & ANIM_TOGGLEBIT;

	if (torsoAnim == TORSO_ATTACK && weaponState == WEAPON_FIRING)
		return (weaponstate_t)(WEAPON_FIRING | animToggleBit);

	if (torsoAnim == TORSO_ATTACK2 && weaponState == WEAPON_FIRING) //add hypov8 
		return (weaponstate_t)(WEAPON_FIRING | animToggleBit);

	if (torsoAnim == TORSO_ATTACK3 && weaponState == WEAPON_FIRING) //add hypov8 allow pistol to re'shoot animations
		return (weaponstate_t)(WEAPON_FIRING | animToggleBit);

  return (weaponstate_t)ps->weaponstate;
}
#endif

/*
==============
CG_AddViewWeapon
Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon(playerState_t *ps)
{
	static refEntity_t  hand; // static for proper alignment in QVMs
	centity_t *cent;
	clientInfo_t *ci;
	float fovOffset;
	vec3_t angles;
	int weaponNum= ps->weapon;
	//int weaponState;
	//int weaponTime;
	weaponInfo_t *weapon;
  	qboolean     drawGun = qtrue;

 
	if (ps->persistant[PERS_TEAM] == TEAM_SPECTATOR)
		return;

	if (ps->pm_type == PM_INTERMISSION)
		return;

	if (weaponNum == WP_NONE || cg.predictedPlayerState.stats[STAT_HEALTH] <= 0) //hypov8 add
		return;

  	weapon = &cg_weapons[weaponNum];


	if(! cg_drawGun.integer )
		drawGun = qfalse;

	if ( !weapon->registered )
	{
		    Com_DPrintf(S_COLOR_RED"CG_AddViewWeapon: weapon %d (ss) "
		            "is not registered", weapon/*, BG_Weapon( weapon )->name */);
		return;
	}

	cent = &cg.predictedPlayerEntity; // &cg_entities[cg.snap->ps.clientNum];

	if ( ps->pm_type == PM_INTERMISSION )
	{
		return;
	}

  // no gun if in third person view
  if (cg.renderingThirdPerson)
    return;


  // allow the gun to be completely removed
  if (!drawGun)
  {
    vec3_t origin;

    if (cg.predictedPlayerState.eFlags & EF_FIRING)
    {
      if (weaponNum == WP_FLAMER)
      {
        // special hack for flamethrower...
        VectorCopy(cg.refdef.vieworg, origin);

        VectorMA(origin, 18, cg.refdef.viewaxis[0], origin);
        VectorMA(origin, -7, cg.refdef.viewaxis[1], origin);
        VectorMA(origin, -4, cg.refdef.viewaxis[2], origin);

        // Ridah, Flamethrower effect
        CG_FlamethrowerFlame(&cg.predictedPlayerEntity, origin);
      }
      else
      {
        // special hack for lightning gun...
        VectorCopy(cg.refdef.vieworg, origin);
        VectorMA(origin, -8, cg.refdef.viewaxis[2], origin);

        //FIXME(0xA5EA): this is the lightning shit when shooting
        //FIXME(0xA5EA): do i need this ??
        CG_LightningBolt(&cg_entities[ps->clientNum], origin);
      }
    }
    return;
  }


  // don't draw if testing a gun model
  if (cg.testGun)
    return;

  // drop gun lower at higher fov
  if (cg_fov.integer > 90)                      // 0xA5EA, waffe hiermit kleiner machen
  {
    //fovOffset = -0.2 * ( cg_fov.integer - 90 );  //orginal q3 way
    //fovOffset = -0.1 * (cg_fov.integer - 90); // gl passt dann
      fovOffset = -0.05* (cg_fov.integer - 90);
    //  fovOffset = -cg_larshelp.value* (cg_fov.integer - 90);
     //fovOffset = 0;
  }
  else
  {
    fovOffset = 0;
  }



  /////////////////////////hypov8
	//if ( ps->weapon > WP_NONE ) 
  //cent = &cg.predictedPlayerEntity;             // &cg_entities[cg.snap->ps.clientNum];

	//weaponTime = ps->weaponTime;
	//CG_RegisterWeapon(weaponNum); //hypov8 disable
	//FIXME (0xA5EA): weapons registered every time ???

  
	Com_Memset(&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition(hand.origin, angles);

	VectorMA(hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin);
	VectorMA(hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin);
	VectorMA(hand.origin, (cg_gun_z.value + fovOffset), cg.refdef.viewaxis[2], hand.origin);

	AnglesToAxis(angles, hand.axis);


	//weaponState = CG_CalcWeaponState(cent, ps);
	//CG_WeaponAnimation(cent, weapon, weaponNum, weaponState, weaponTime);
	//FIXME(0xA5EA): merge !!!!!
	//FIXME (0xA5EA): removed hitmen mask from weaponstate
	// map torso animations to weapon animations
	if ( cg_gun_frame.integer )
	{
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	}
	else
	{	// get clientinfo for animation map
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		hand.frame = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.frame, !weapon->md5 ? cent->pe.torso.animationNumber & ~ANIM_TOGGLEBIT : -1, weaponNum);
		hand.oldframe = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.oldFrame, !weapon->md5 ? cent->pe.torso.animationNumber & ~ANIM_TOGGLEBIT : -1, weaponNum);
		hand.backlerp = cent->pe.torso.backlerp; //hypov8 use player lerp value?
	}
	//tag.frame    = cent->pe.weapon.frame;
	//tag.oldframe = cent->pe.weapon.oldFrame;
	//tag.backlerp = cent->pe.weapon.backlerp;

	// if((weaponState & 0xF) != WEAPON_READY)
	//  Com_Printf("tag.frame %d, oldframe = %d, backlerp = %f\n",  tag.frame, tag.oldframe, tag.backlerp);
	hand.hModel   = weapon->tagModel;
	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	// add everything onto the tag
	if (weapon)
	{
		CG_AddPlayerWeapon(&hand, ps, &cg.predictedPlayerEntity);
		//CG_AddPlayerWeaponLight(ps, &cg.predictedPlayerEntity); //hypov8 todo:
	}

	if (ps->powerups[PW_FLASHLIGHT])
		CG_AddViewFlashlight();

#if 0
  else
  {
    refEntity_t     hand;
    Com_Memset(&hand, 0, sizeof(hand));

  // set up gun position
    CG_CalculateWeaponPosition(hand.origin, angles);

    VectorMA(hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin);
    VectorMA(hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin);
    VectorMA(hand.origin, (cg_gun_z.value + fovOffset), cg.refdef.viewaxis[2], hand.origin);

//	VectorMA( weap.origin, cg_gun_x.value, cg.refdef.viewaxis[0], weap.origin );
//	VectorMA( weap.origin, cg_gun_y.value, cg.refdef.viewaxis[1], weap.origin );
//	VectorMA( weap.origin, (cg_gun_z.value+fovOffset), cg.refdef.viewaxis[2], weap.origin );

    AnglesToAxis(angles, hand.axis);
    if (cg_gun_frame.integer)
    {
      // development tool
      hand.frame    = hand.oldframe = cg_gun_frame.integer;
      hand.backlerp = 0;

    }
    else
    {

    // get clientinfo for animation map
    //ps->weaponstate;
#if 0

    typedef enum
    {
      WEAPON_READY,
      WEAPON_RAISING,
      WEAPON_DROPPING,
      WEAPON_FIRING,
      WEAPON_RELOADING,
      MAX_WEAPON_STATES,
      WEAPON_HM_LOCK = 0x10,
      WEAPON_HM_AUTOSWITCH
      //FIXME: 0xA5EA, weaponreload states
    } weaponstate_t;
#endif
  //  Com_Printf(" cg.time %d\n",  cg.time);
    //Com_Printf("weaponTime %d\n", ps->weaponTime);

    ci            = &cgs.clientinfo[cent->currentState.clientNum];
    #if 1
    hand.frame    = CG_MapTorsoToWeaponFrame(ci, weapon, cent->pe.torso.frame);
    hand.oldframe = CG_MapTorsoToWeaponFrame(ci, weapon, cent->pe.torso.oldFrame);
    #else
    CG_ModifiyWeaponFrame(ps, &hand, weapon);

   // void CG_ModifiyWeaponFrame(centity_t* cent, clientInfo_t* ci, playerState_t *ps, refEntity_t* weap,  weaponInfo_t* weapInfo)
    #endif
   // hand.backlerp = cent->pe.torso.backlerp;
    hand.backlerp = 0;
   // Com_Printf("cent->pe.torso.backlerp %f\n",  cent->pe.torso.backlerp);
    }
      hand.hModel   = weapon->tagModel;
  hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

  // add everything onto the hand
  CG_AddPlayerWeapon(&hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM]);
  }
#endif
}

/*
==============================================================================
WEAPON SELECTION
==============================================================================
*/
/*
===================
CG_DrawWeaponSelect
hypov8 draw weapons picked up (disable crowbar and? hook?)
hypov8 todo: move and cycle 3 items
===================
*/
void CG_DrawWeaponSelect(void)
{
  int i;
  int bits;
  int count;
  int x, y;
  char *name;
  float *color;

  // don't display if dead
  if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
    return;

  color = CG_FadeColor(cg.weaponSelectTime, WEAPON_SELECT_TIME);
  if (!color)
    return;

  trap_R_SetColor(color);

  // showing weapon select clears pickup item display, but not the blend blob
  cg.itemPickupTime = 0;

  // count the number of weapons owned
  bits  = cg.snap->ps.stats[STAT_WEAPONS];
  count = 0;
  for (i = WP_CROWBAR; i <= WP_LAST; i++) //hypov8 was 16. use num weapons
  {
    if (bits & (1 << i))
      count++;
  }

  x = 320 - count * 25;
  y = 380;

  for (i = 1; i < WP_NUM_WEAPONS; i++) //hypov8 was 16. use num weapons
  {
    if (!(bits & (1 << i)))
      continue;

	if ((cg.weaponSelect !=WP_CROWBAR && i == WP_CROWBAR) ||
		(cg.weaponSelect != WP_GRAPPLING_HOOK && i == WP_GRAPPLING_HOOK)) //hypov8 hide these from view if not selected
		continue;


   // CG_RegisterWeapon(i); //hypov8 disable

    // draw weapon icon
    CG_DrawPic(x, y, 42, 18, cg_weapons[i].weaponIcon);

    // draw selection marker
    if (i == cg.weaponSelect)
      CG_DrawPic(x - 4, y - 4, 50, 26, cgs.media.selectShader);

    // no ammo cross on top
	if (!cg.snap->ps.ammo_all[BG_AmmoCombineCheck(i)] && !cg.snap->ps.ammo_mag[i]) // hypov8 ammo combine
      CG_DrawPic(x + 10, y, 16, 16, cgs.media.noammoShader);

    x += 48;
  }

  // draw the selected name
  if (cg_weapons[cg.weaponSelect].item)
  {
    name = (char*)cg_weapons[cg.weaponSelect].item->pickup_name;
    if (name)
    {
      //w = CG_DrawStrlen( name ) * SMALLCHAR_WIDTH;
      //w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;
      //x = ( SCREEN_WIDTH - w ) / 2;
      //CG_DrawSmallStringColor(x, y - 27, name, color);
      CG_Text_Paint(280, y - 20, .33f, color, name, 0, 0, ITEM_TEXTSTYLE_SHADOWED, NULL_FONT);  // FIXME: welcher scale value 0xA5EA
      //CG_DrawBigStringColor(x, y - 22, name, color);
    }
  }
  trap_R_SetColor(NULL);
}

void CG_DrawPickupItem(void /*int index, gitem_t *item*/)
{
	float *color;

	// don't display if dead
	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
		return;
//hypov8. CG_DrawPickupItem. check item registered???

	color = CG_FadeColor(cg.itemPickupTime, WEAPON_SELECT_TIME);
	if (!color)
		return;
	//value = cg.itemPickup;
	//cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	///////////CG_DrawPic(80, 80, 42, 18, cg_items[ITEM_INDEX(item)].icon);

	if (bg_itemlist[cg.itemPickup].giType == IT_WEAPON) /*weps are not square*/
		CG_DrawPic(320, 462, 32, 18,  cg_items[cg.itemPickup].icon);
	else
		CG_DrawPic(320, 448, 32, 32,  cg_items[cg.itemPickup].icon);
	
	trap_R_SetColor(color);
}
/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable(weapon_t weapon)
{
	if ( !BG_InventoryContainsWeapon( weapon, cg.snap->ps.stats ) )
		return qfalse; //no wep

	//if (!(cg.snap->ps.stats[STAT_WEAPONS] & (1 << i)))
	//	return qfalse; //no wep

	/*if ( cg.weaponSelect >= WP_FIRST && cg.weaponSelect < WP_LAST )
	{
		if ( !cg.snap->ps.ammo_all[ BG_AmmoCombineCheck(weapon) ] && !cg.snap->ps.ammo_mag[ weapon ] )
			return qfalse; //no ammo
	}*/

  return qtrue; //have wep+ammo
}

static qboolean CG_WeaponHasAmmo(weapon_t weapon)
{
	if ( BG_InventoryContainsAmmo(weapon, &cg.snap->ps) )
		return true;

	return false;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f(void)
{
  int i, original, newWep;

  newWep = original = cg.weaponSelect;

  if (!cg.snap)
    return;

  if (cg.snap->ps.pm_flags & PMF_FOLLOW)
    return;

  cg.weaponSelectTime = cg.time;
  //original = cg.weaponSelect; //hypov8 disabled

  for (i = 0; i < WP_NUM_WEAPONS; i++)
  {
	  newWep++;

    if (newWep > WP_LAST)
      newWep = WP_FIRST;

	if ( CG_WeaponSelectable(( weapon_t ) newWep) && CG_WeaponHasAmmo(( weapon_t ) newWep) )
	{ 
		if ( newWep != original )
		{
			cg.weaponSelect = newWep;
		}
		return;
	}
  }

  cg.weaponSelect = WP_CROWBAR;
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f(void)
{
  int i, j,k;
  int original, newWep;
  newWep = original = cg.weaponSelect;

  j = k = 0;
  if (!cg.snap)
    return;

  if (cg.snap->ps.pm_flags & PMF_FOLLOW)
    return;

  cg.weaponSelectTime = cg.time;
  //original = cg.weaponSelect; //hypov8 disabled

  for (i = 0; i < WP_NUM_WEAPONS; i++)
  {
	  newWep--;

	if (newWep < WP_FIRST)
      newWep = WP_LAST;

    if (CG_WeaponSelectable((weapon_t)newWep) && CG_WeaponHasAmmo((weapon_t)newWep))
	{
		if ( newWep != original )
		{
			cg.weaponSelect = newWep;
		}
      return;
    }
  }

  cg.weaponSelect = WP_CROWBAR;
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f(void)
{
  int num;
  static int lastTimeUsed = 0; //prevent

  if (!cg.snap)
    return;

  num = atoi(CG_Argv(1));

  if (num <= WP_NONE || num >= WP_NUM_WEAPONS)
    return;

  if ( lastTimeUsed > cg.time )
	  return;

  if ( cg.snap->ps.pm_flags & PMF_FOLLOW  || cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
  {
	if (num == TEAM_DRAGONS || num == TEAM_NIKKIS)
	  trap_SendClientCommand(va("team %i", num)); //join team with wep 1 or 2 key
	lastTimeUsed = cg.time + 1000; //prevent weaponkey working for short time
	return;
  }

  //display weapon selected
  cg.weaponSelectTime = cg.time;

  //do we have the weapon?
  if ( !CG_WeaponSelectable(( weapon_t ) num) )
    return;     // don't have the weapon

  //do we have ammo/rounds?
  if ( !CG_WeaponHasAmmo(( weapon_t ) num) )
	  return;

  if ( cg.weaponSelect == num )
	  return;

  cg.weaponSelect = num;
}

/*
===================================================================================================
WEAPON EVENTS
===================================================================================================
*/
/*
================
CG_FireWeapon
Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon(centity_t *cent, qboolean spistol)
{
  entityState_t *ent;
  int c;
  weaponInfo_t *weap;

  ent = &cent->currentState;

  if (ent->weapon == WP_NONE)
    return;

  if (ent->weapon >= WP_NUM_WEAPONS)
  {
    CG_Error("CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS");
    return;
  }

  weap = &cg_weapons[ent->weapon];




  // mark the entity as muzzle flashing, so when it is added it will
  // append the flash to the weapon model
  cent->muzzleFlashTime = cg.time;

 /* if ( ent->weapon == WP_FLAMER )
  {
	  if ( cg.shootTimeFlamer > cg.time)
		  return;
	  cg.shootTimeFlamer = cg.time+100; //hypov8 todo: sounds needs fixing
  }*/

  // play a sound
  for (c = 0; c < MAX_FLASH_SOUNDS; c++)
  {
    if (!weap->flashSound[c])
      break;
  }

  if (c > 0)
  {
    if (ent->weapon == WP_PISTOL)
    {
		if (spistol)	//PW_WPMOD_SILENCER
        c = 1;
      else
        c = 0;
    }
    else
    {
      c = rand() % c;
    }
    if (weap->flashSound[c])
      trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, weap->flashSound[c]);
  }

  // do brass ejection
  if (weap->ejectBrassFunc && cg_brassTime.integer > 0)
    weap->ejectBrassFunc(cent);


	 // wep = cent->currentState.weapon;

//unlagged - attack prediction #1
	//CG_PredictWeaponEffects( cent ); //hypov8 note: disable. do we really need this.
//unlagged - attack prediction #1
}

/*
=================
CG_ReloadWeapon
=================
*/
void CG_ReloadWeapon(centity_t *cent)
{
  entityState_t *ent;
  weaponInfo_t *weap;

  ent = &cent->currentState;

  if (ent->weapon == WP_NONE)
    return;

  if (ent->weapon >= WP_NUM_WEAPONS)
  {
    CG_Error("CG_ReloadWeapon: ent->weapon >= WP_NUM_WEAPONS");
    return;
  }

  weap = &cg_weapons[ent->weapon];
  
  if ( weap->reloadSound && ent->clientNum == cg.predictedPlayerState.clientNum )
    trap_S_StartSound(NULL, ent->number, CHAN_WEAPON,  weap->reloadSound);
}

/*
================
CG_ResetWeaponSwitch
Caused by an EV_FINISH_CHANGE_WEAPON event
hypo used to reset gun auto switch on sucseful wep switch
================
*/
void CG_ResetWeaponSwitch(centity_t *cent, int weapon)
{
	if (weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS)
	{
		CG_Error("CG_ResetWeaponSwitch: invalid weapon number");
		return;
	}
	cg.weaponSelect = weapon;
}

/*
=================
CG_MissileHitWall
Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType)
{
  qhandle_t mod;
  qhandle_t mark;
  qhandle_t shader;
  sfxHandle_t sfx;
  float radius;
  float light;
  vec3_t lightColor;
  localEntity_t *le;
  qboolean alphaFade;
  qboolean isSprite;
  int duration;
  vec3_t sprVel;
  const int random_int = rand();

  mark          = 0;
  radius        = 4;
  sfx           = 0;
  mod           = 0;
  shader        = 0;
  light         = 0;
  lightColor[0] = 1;
  lightColor[1] = 1;
  lightColor[2] = 0;

  // set defaults
  isSprite = qfalse;
  duration = 600;

  switch (weapon)
  {
  case WP_CROWBAR:
    switch (soundType)
    {
    case IMPACTSOUND_GLASS:
      mark = cgs.media.meleeMarkGlass[random_int & 1];
      sfx = cgs.media.sfx_Pipehtin;
      break;
    case IMPACTSOUND_METALLIGHT:
    case IMPACTSOUND_METAL:
      sfx = cgs.media.sfx_Pipehtin;
      mark = cgs.media.meleeMarkMetal[random_int & 1];
      break;

    case IMPACTSOUND_WOOD:
      sfx = cgs.media.sfx_Pipehwood;
      mark = cgs.media.meleeMarkWood[random_int & 1];
      break;
    case IMPACTSOUND_SNOW:
    case IMPACTSOUND_GRAVEL:
    case IMPACTSOUND_EARTH:
      sfx = cgs.media.sfx_Pipehgravel;
	  //mark = cgs.media.bloodExplosionShader;
      break;
    case IMPACTSOUND_FLESH:
      sfx = cgs.media.sfx_Pipehbody;
	  mark = cgs.media.bloodExplosionShader; //hypov8 todo:
      break;
	default:
    case IMPACTSOUND_DEFAULT:
      sfx = cgs.media.sfx_Pipehcement;
	  mark = cgs.media.meleeMarkBrick[random_int & 1];
	  CG_ParticleRick(origin, dir);
	  //CG_ParticleDirtBulletDebris_Core(origin,vec3_origin, 2222, 20.0f, 20.0f, 0.5f,cgs.media.grenadeExplosionShader  );
      break;
    }
    break;
  case WP_GRENADE_LAUNCHER:
    mod      = cgs.media.dishFlashModel;
    shader   = cgs.media.grenadeExplosionShader;
    sfx      = cgs.media.sfx_GrenExpl[random_int % 3];
    mark     = cgs.media.burnMarkShader;
		//	cgs.media.sparkFlareShader;
		//	cgs.media.sparkShader;
			radius   = 128;
    light    = 300;
    isSprite = qtrue;
			duration = 1000;
			lightColor[0] = 1;
			lightColor[1] = 0.75;
			lightColor[2] = 0.0;
    //VectorScale(dir, 100, sprVel);
			dir[0] = dir[1] = 0; dir[2] = -1;			//set straight up
			//sprVel[0] = sprVel[1] = 0; sprVel[2] = 100;	//set straight up

			origin[2] += 8;
			CG_ParticleGrenade(origin, dir);
			//CG_ParticleSparks(sprOrg, sprVel, 1400, 20, 30, 600);
			//CG_ParticleRick(origin, dir);
    break;
///////////////
//rocket impact
  case WP_ROCKET_LAUNCHER:
    mod = cgs.media.dishFlashModel;
    shader = cgs.media.rocketExplosionShader;

    switch (soundType)
    {
    case IMPACTSOUND_METAL:
      sfx = cgs.media.sfx_rockexp[1];
      break;
    default:
      sfx = cgs.media.sfx_rockexp[0];
    }

    mark = cgs.media.burnMarkShaderRl;
    radius = 64;
    light = 300;
    isSprite = qtrue;
    duration = 1000;
    lightColor[0] = 1;
    lightColor[1] = 0.75;
    lightColor[2] = 0.0;
    VectorScale(dir, 100, sprVel);
	CG_ParticleSparks(origin, sprVel, 1400, 20, 30, 600);
    break;
///////////////
//flamer impact
  case WP_FLAMER:
	  switch ( soundType )
	  {
	  case IMPACTSOUND_DEFAULT:
	  default:
		  sfx = cgs.media.sfx_rockexp[ 0 ];


		  /* mark = cgs.media.burnMarkShaderRl;
		   radius = 64;
		   light = 300;
		   isSprite = qtrue;
		   duration = 1000;
		   lightColor[0] = 1;
		   lightColor[1] = 0.75;
		   lightColor[2] = 0.0;
		   VectorScale(dir, 100, sprVel);
		   CG_ParticleSparks(origin, sprVel, 1400, 20, 30, 600);*/


		  mod = cgs.media.dishFlashModel;
		  shader = cgs.media.rocketExplosionShader;
		  sfx = 0; // cgs.media.flameSound;
		  mark = cgs.media.burnMarkShader;
		  radius = 64;
		  //FIXME(0xA5EA): flamegun: is this complete ?
	  }

	  break;
///////////////
//shotty impact
  case WP_SHOTGUN:
			//mod = cgs.media.bulletFlashModel; //old sparks
    isSprite = qfalse;
    switch (soundType)
    {
    case IMPACTSOUND_METAL:
      sfx = cgs.media.sfx_ShottyhitMetal[random_int % NUM_SHOTTY_METALIMPACTSND];
      mark = cgs.media.bulletMarkmetal[random_int&1];
      break;
    case IMPACTSOUND_WOOD:
      sfx = cgs.media.sfx_BullethWood[random_int % NUM_WOODIMPACTSND];
	    mark = cgs.media.bulletMarkwood[random_int&1];
      //sfx = cgs.media.sfx_ShottyhitWood[rand()%3];  //rand()&3  //werden nur 2 sounds initialisiert
      break;
    case IMPACTSOUND_GRAVEL: //ToDo: hypov8
    case IMPACTSOUND_EARTH:
					//sfx = cgs.media.sfx_ShottyhitEarth[random_int& 1]; //hypov8 ToDo: ising tommy copy
					//copy from tommy below
					sfx = cgs.media.sfx_BullethEarth[random_int % 3];
					mark = cgs.media.bulletMark[random_int & 1];
      break;
    case IMPACTSOUND_GLASS:
      mark = cgs.media.bulletMarkglas[random_int&1]; //note hypov8 not working on glass, using default:
    case IMPACTSOUND_SNOW:
      sfx = cgs.media.sfx_ShottyhitIce[random_int & 1];
      break;
    default:
      mark = cgs.media.bulletMark[random_int&1];
      sfx = cgs.media.sfx_Shottyhit[random_int & 7];
    }
    CG_ParticleRick(origin, dir);
    radius = 2;
    break;
/////////////////
// default bullet impact (add shotty?)
  default:
  case WP_PISTOL:
  case WP_HMG:
  case WP_MACHINEGUN:
		 radius = 1;
	  if (weapon == WP_HMG)
	    radius = 3;
			//mod = cgs.media.bulletFlashModel; old sparks
	  isSprite = qfalse;
    switch (soundType)
    {
    case IMPACTSOUND_METAL:
      sfx = cgs.media.sfx_BullethMetal[random_int % NUM_METALIMPACTSND];
      mark = cgs.media.bulletMarkmetal[random_int&1];
      break;
    case IMPACTSOUND_WOOD:
      sfx = cgs.media.sfx_BullethWood[random_int % NUM_WOODIMPACTSND];
      mark = cgs.media.bulletMarkwood[random_int&1];
      break;
    case IMPACTSOUND_GRAVEL: //hypov8 todo: gravel
    case IMPACTSOUND_EARTH:
      sfx = cgs.media.sfx_BullethEarth[random_int%3];
					mark = cgs.media.bulletMark[random_int & 1];
      break;
    case IMPACTSOUND_GLASS:
	case IMPACTSOUND_SNOW: //hypov8 todo: snow
      mark = cgs.media.bulletMarkglas[random_int&1];
      sfx = cgs.media.sfx_BullethIce[random_int&1];
      break;
    default:
      mark = cgs.media.bulletMark[random_int&1];
      sfx = cgs.media.sfx_ric[random_int&7];
    }
    CG_ParticleRick(origin, dir);
    break;

  case WP_GRAPPLING_HOOK:
      sfx = cgs.media.sfx_HookHitBrick;
	  break;
  }

  if (sfx)
    trap_S_StartSound(origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx);

  // create the explosion
  if (mod)
  {
    le = CG_MakeExplosion(origin, dir, mod, shader, duration, isSprite);
    le->light = light;
    VectorCopy(lightColor, le->lightColor);
  }

  // impact mark
  alphaFade = (mark == cgs.media.energyMarkShader);   // plasma fades alpha, all others fade color
  CG_ImpactMark(mark, origin, dir, random( ) * 360, 1, 1, 1, 1, alphaFade, radius, qfalse);
}

/*
=================
CG_MissileHitPlayer //note hypov8 remove same team effects
=================
*/
void CG_MissileHitPlayer(int weapon, vec3_t origin, vec3_t dir, int entityNum)
{

//hypov8
#if 0
if (playerteam->dragons) //hypo TODO: fix what they are called..
	{
	if (enamy = dragons)
		{
			return;
		}
	}
if (playerteam->nikki)
	{
	if (enamy = nikki)
		{
			return;	
		}	
	}
#endif

  CG_Bleed(origin, entityNum);

  // some weapons will make an explosion with the blood, while
  // others will just make the blood
  switch (weapon)
  {
  //FIXME(0xA5EA): WP_PISTOL
  case WP_CROWBAR:
  case WP_FLAMER:
  case WP_GRENADE_LAUNCHER:
  case WP_ROCKET_LAUNCHER:
    CG_MissileHitWall(weapon, 0, origin, dir, IMPACTSOUND_FLESH);
    break;
  default:
    break;
  }
}

/*
============================================================================
SHOTGUN TRACING
============================================================================
*/
/*
================
CG_ShotgunPellet
================
*/
#if 0 //not used
static void CG_ShotgunPellet(vec3_t start, vec3_t end, int skipNum)
{
  trace_t tr;
  int sourceContentType, destContentType;

  CG_Trace(&tr, start, NULL, NULL, end, skipNum, MASK_SHOT);

  sourceContentType = trap_CM_PointContents(start, 0);
  destContentType   = trap_CM_PointContents(tr.endpos, 0);

  // FIXME: should probably move this cruft into CG_BubbleTrail
  if (sourceContentType == destContentType)
  {
    if (sourceContentType & CONTENTS_WATER)
      CG_BubbleTrail(start, tr.endpos, 32);
  }
  else if (sourceContentType & CONTENTS_WATER)
  {
    trace_t trace;
    trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
    CG_BubbleTrail(start, trace.endpos, 32);
  }
  else if (destContentType & CONTENTS_WATER)
  {
    trace_t trace;
    trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);
    CG_BubbleTrail(tr.endpos, trace.endpos, 32);
  }

  if (tr.surfaceFlags & SURF_NOIMPACT)
    return;

  //Com_Printf("Pattern tr.surfaceFlags = %d\n", tr.surfaceFlags);
  //Com_Printf("CG_ShotgunPellet\n");
  //aufruf bei shotty, nicht bei tommy
  if (cg_entities[tr.entityNum].currentState.eType == ET_PLAYER)
  {
    CG_MissileHitPlayer(WP_SHOTGUN, tr.endpos, tr.plane.normal, tr.entityNum);
  }
  else
  {
    if (tr.surfaceFlags & SURF_NOIMPACT)
      return; // SURF_NOIMPACT will not make a flame puff or a mark

    if (tr.surfaceFlags & SURF_METALSTEPS || tr.surfaceFlags & SURF_METALLIGHT || tr.surfaceFlags & SURF_TIN)
      CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL);
    else if (tr.surfaceFlags & SURF_WOOD)
      CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_WOOD);                                                                                                                                                                                                                                                                                                          /* 0xA5EA */
    else if (tr.surfaceFlags & SURF_GRAVEL || tr.surfaceFlags & SURF_GRASS)
      CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_EARTH);
    else if (tr.surfaceFlags & SURF_SNOW)
      CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_SNOW);
    else
      CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT);                                                                                                                                                                                                                                                                                                       //marble and others
    //TODO: marble and tin
  }
}

/*
================
CG_ShotgunPattern
Perform the same traces the server did to locate the
hit splashes
================
*/

//unlagged - attack prediction
// made this non-static for access from cg_unlagged.c
//static void CG_ShotgunPattern(vec3_t origin, vec3_t origin2, int seed, int otherEntNum) kpq3 unlag
void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, int otherEntNum ) 
{
  int i;
  float r, u;
  vec3_t end;
  vec3_t forward, right, up;

  // derive the right and up vectors from the forward vector, because
  // the client won't have any other information
  VectorNormalize2(origin2, forward);
  PerpendicularVector(right, forward);
  CrossProduct(forward, right, up);

  // generate the "random" spread pattern
  for (i = 0; i < DEFAULT_SHOTGUN_COUNT; i++)
  {
    r = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;
    u = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;

    VectorMA(origin, 8192 * 16, forward, end);
    VectorMA(end, r, right, end);
    VectorMA(end, u, up, end);

    CG_ShotgunPellet(origin, end, otherEntNum);
  }
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire(entityState_t *es)
{
  vec3_t v;
  int contents;
  vec3_t up;

  VectorSubtract(es->origin2, es->pos.trBase, v);
  VectorNormalize(v);
  VectorScale(v, 32, v);
  VectorAdd(es->pos.trBase, v, v);

  contents = trap_CM_PointContents(es->pos.trBase, 0);
  if (!(contents & CONTENTS_WATER))
  {
    VectorSet(up, 0, 0, 8);
    CG_SmokePuff(v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader);
  }
  CG_ShotgunPattern(es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum);
}
#endif
/*
============================================================================
BULLETS
============================================================================
*/
/*
===============
CG_Tracer
===============
*/
void CG_Tracer(vec3_t source, vec3_t dest)
{
  vec3_t forward, right;
  polyVert_t verts[4];
  vec3_t line;
  float len, begin, end;
  vec3_t start, finish;
  vec3_t midpoint;

  // tracer
  VectorSubtract(dest, source, forward);
  len = VectorNormalize(forward);

  // start at least a little ways from the muzzle
  if (len < 100)
    return;

  begin = 50 + random() * (len - 60);
  end   = begin + cg_tracerLength.value;

  if (end > len)
    end = len;

  VectorMA(source, begin, forward, start);
  VectorMA(source, end, forward, finish);

  line[0] = DotProduct(forward, cg.refdef.viewaxis[1]);
  line[1] = DotProduct(forward, cg.refdef.viewaxis[2]);

  VectorScale(cg.refdef.viewaxis[1], line[1], right);
  VectorMA(right, -line[0], cg.refdef.viewaxis[2], right);
  VectorNormalize(right);

  VectorMA(finish, cg_tracerWidth.value, right, verts[0].xyz);
  verts[0].st[0]       = 0;
  verts[0].st[1]       = 1;
  verts[0].modulate[0] = 255;
  verts[0].modulate[1] = 255;
  verts[0].modulate[2] = 255;
  verts[0].modulate[3] = 255;

  VectorMA(finish, -cg_tracerWidth.value, right, verts[1].xyz);
  verts[1].st[0]       = 1;
  verts[1].st[1]       = 0;
  verts[1].modulate[0] = 255;
  verts[1].modulate[1] = 255;
  verts[1].modulate[2] = 255;
  verts[1].modulate[3] = 255;

  VectorMA(start, -cg_tracerWidth.value, right, verts[2].xyz);
  verts[2].st[0]       = 1;
  verts[2].st[1]       = 1;
  verts[2].modulate[0] = 255;
  verts[2].modulate[1] = 255;
  verts[2].modulate[2] = 255;
  verts[2].modulate[3] = 255;

  VectorMA(start, cg_tracerWidth.value, right, verts[3].xyz);
  verts[3].st[0]       = 0;
  verts[3].st[1]       = 0;
  verts[3].modulate[0] = 255;
  verts[3].modulate[1] = 255;
  verts[3].modulate[2] = 255;
  verts[3].modulate[3] = 255;

  if (cgs.media.tracerShader)
    trap_R_AddPolyToScene(cgs.media.tracerShader, 4, verts);
  else
    Com_DPrintf(S_COLOR_RED"cgs.media.tracerShader = null");

  midpoint[0] = (start[0] + finish[0]) * 0.5;
  midpoint[1] = (start[1] + finish[1]) * 0.5;
  midpoint[2] = (start[2] + finish[2]) * 0.5;

  // add the tracer sound
  trap_S_StartSound(midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound);
}

/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean CG_CalcMuzzlePoint(int entityNum, vec3_t muzzle)
{
  vec3_t forward;
  centity_t *cent;
  int anim;

  if (entityNum == cg.snap->ps.clientNum)
  {
    VectorCopy(cg.snap->ps.origin, muzzle);
    muzzle[2] += cg.snap->ps.viewheight;
    AngleVectors(cg.snap->ps.viewangles, forward, NULL, NULL);
    VectorMA(muzzle, 14, forward, muzzle);
    return qtrue;
  }

  cent = &cg_entities[entityNum];

  if (!cent->currentValid)
    return qfalse;

  VectorCopy(cent->currentState.pos.trBase, muzzle);

  AngleVectors(cent->currentState.apos.trBase, forward, NULL, NULL);
  anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
  if (anim == LEGS_CR_WALK || anim == LEGS_CR_IDLE)
    muzzle[2] += CROUCH_VIEWHEIGHT;
  else
    muzzle[2] += DEFAULT_VIEWHEIGHT;

  VectorMA(muzzle, 14, forward, muzzle);

  return qtrue;
}

/*
======================
CG_Bullet
Renders bullet effects.
======================
*/
void CG_Bullet(int weapon, vec3_t end, int sourceEntityNum, vec3_t normal, int ImpactType, int fleshEntityNum)
{
  trace_t trace;
//	trace_t tr;
  int sourceContentType, destContentType;
  vec3_t start;

  //Com_Printf("CG_Bullet\n");
  //CG_Trace( &tr, start, NULL, NULL, end, skipNum, MASK_SHOT );
  // if the shooter is currently valid, calc a source point and possibly
  // do trail effects
  if (sourceEntityNum >= 0 && cg_tracerChance.value > 0)
  {
    if (CG_CalcMuzzlePoint(sourceEntityNum, start))
    {
      sourceContentType = trap_CM_PointContents(start, 0);
      destContentType   = trap_CM_PointContents(end, 0);

      // do a complete bubble trail if necessary
      if ((sourceContentType == destContentType) && (sourceContentType & CONTENTS_WATER))
        CG_BubbleTrail(start, end, 32);

      // bubble trail from water into air
      else if ((sourceContentType & CONTENTS_WATER))
      {
        trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
        CG_BubbleTrail(start, trace.endpos, 32);
      }
      // bubble trail from air into water
      else if ((destContentType & CONTENTS_WATER))
      {
        trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);
        CG_BubbleTrail(trace.endpos, end, 32);
      }

      // draw a tracer
      if (random() < cg_tracerChance.value)
        CG_Tracer(start, end);
    }
  }

  // impact splash and mark
  //Com_Printf("Bullet ImpactType = %d\n", ImpactType);
  switch (ImpactType)
  {
  case IMPACTSOUND_FLESH:
    CG_Bleed(end, fleshEntityNum);
    break;
  default:
    CG_MissileHitWall(weapon, 0, end, normal, (impactSound_t)ImpactType);
  }
//Fixme: wasn los mit bullitimapct ?
#if 0
  if (flesh)
  {
    CG_Bleed(end, fleshEntityNum);
  }
  else
  {
    if (trace.surfaceFlags & SURF_METALSTEPS)                                                                                                                                                    //if (eventParm & SURF_METALSTEPS)
      CG_MissileHitWall(WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_METAL);
    else if (trace.surfaceFlags & SURF_WOOD)
      CG_MissileHitWall(WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_WOOD);
    else
      CG_MissileHitWall(WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_DEFAULT);

  }
#endif
}
