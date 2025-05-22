/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-xyyz Lars '0xA5EA' Kandler
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
along with KingpinQ3 source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// ui_players.c

#include "ui_local.h"



#define UI_TIMER_GESTURE 2300
#define UI_TIMER_JUMP 1000
#define UI_TIMER_LAND 130
#define UI_TIMER_WEAPON_SWITCH 300
#define UI_TIMER_ATTACK 500
#define UI_TIMER_MUZZLE_FLASH 20
#define UI_TIMER_WEAPON_DELAY 250

#define JUMP_HEIGHT				56 //note hypov8 to test

#define SWINGSPEED				0.3f

#define SPIN_SPEED				0.9f
#define COAST_TIME				1000


//static int dp_realtime; //remove hypov8
//static float jumpHeight; //remove hypov8
//sfxHandle_t weaponChangeSound; //remove hypov8


/*
===============
UI_PlayerInfo_SetWeapon
===============
*/
/*
static void UI_PlayerInfo_SetWeapon(playerInfo_t *pi, weapon_t weaponNum)
{
  gitem_t *item;
  char path[MAX_QPATH];

  pi->currentWeapon = weaponNum;
tryagain:
  pi->realWeapon  = weaponNum;
  pi->weaponModel = 0;
  pi->barrelModel = 0;
  pi->flashModel  = 0;

  if (weaponNum == WP_NONE)
    return;

  for (item = bg_itemlist + 1; item->classname; item++)
  {
    if (item->giType != IT_WEAPON)
      continue;

    if (item->giTag == weaponNum)
      break;
  }

  if (item->classname)
  {
    pi->weaponModel = trap_R_RegisterModel(item->world_model[0]);
  }

  if (pi->weaponModel == 0)
  {
    if (weaponNum == WP_MACHINEGUN)
    {
      weaponNum = WP_NONE;
      goto tryagain;
    }
    weaponNum = WP_MACHINEGUN;
    goto tryagain;
  }

  qstrcpy(path, item->world_model[0]);
  COM_StripExtension(path, path, sizeof(path));
  qstrcat(path, "_flash.md3");
  pi->flashModel = trap_R_RegisterModel(path);

  switch (weaponNum)
  {
  case WP_CROWBAR:
    MAKERGB(pi->flashDlightColor, 0.6f, 0.6f, 1);
    break;
  case WP_PISTOL:
    MAKERGB(pi->flashDlightColor, 1, 0.7f, 1);
    break;
  case WP_MACHINEGUN:
    MAKERGB(pi->flashDlightColor, 1, 1, 0);
    break;
  case WP_SHOTGUN:
    MAKERGB(pi->flashDlightColor, 1, 1, 0);
    break;
  case WP_GRENADE_LAUNCHER:
    MAKERGB(pi->flashDlightColor, 1, 0.7f, 0.5f);
    break;
  case WP_ROCKET_LAUNCHER:
    MAKERGB(pi->flashDlightColor, 1, 0.75f, 0);
    break;
  case WP_HMG:
    MAKERGB(pi->flashDlightColor, 1, 0.5f, 0);
    break;
	// Added -KRYPTYK
   case WP_FLAMER:
	MAKERGB( pi->flashDlightColor, 0.6, 0.6, 1 );
	break;
  case WP_GRAPPLING_HOOK:
    MAKERGB(pi->flashDlightColor, 0.6f, 0.6f, 1);
    break;
  default:
    MAKERGB(pi->flashDlightColor, 1, 1, 1);
    break;
  }
}
*/

/*
===============
UI_ForceLegsAnim
===============
*/
/*
static void UI_ForceLegsAnim(playerInfo_t *pi, int anim)
{
  pi->legsAnim = ((pi->legsAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT) | anim;

  if (anim == LEGS_JUMP)
  {
    pi->legsAnimationTimer = UI_TIMER_JUMP;
  }
}
*/

/*
===============
UI_SetLegsAnim
===============
*/
/*
static void UI_SetLegsAnim(playerInfo_t *pi, int anim)
{
  if (pi->pendingLegsAnim)
  {
    anim                = pi->pendingLegsAnim;
    pi->pendingLegsAnim = 0;
  }
  UI_ForceLegsAnim(pi, anim);
}
*/

/*
===============
UI_ForceTorsoAnim
===============
*/
/*
static void UI_ForceTorsoAnim(playerInfo_t *pi, int anim)
{
  pi->torsoAnim = ((pi->torsoAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT) | anim;

  if (anim == TORSO_GESTURE)
  {
    pi->torsoAnimationTimer = UI_TIMER_GESTURE;
  }

  if (anim == TORSO_ATTACK || anim == TORSO_ATTACK2)
  {
    pi->torsoAnimationTimer = UI_TIMER_ATTACK;
  }
}
*/

/*
===============
UI_SetTorsoAnim
===============
*/
/*
static void UI_SetTorsoAnim(playerInfo_t *pi, int anim)
{
  if (pi->pendingTorsoAnim)
  {
    anim                 = pi->pendingTorsoAnim;
    pi->pendingTorsoAnim = 0;
  }

  UI_ForceTorsoAnim(pi, anim);
}
*/

/*
===============
UI_TorsoSequencing
===============
*/
/*
static void UI_TorsoSequencing(playerInfo_t *pi)
{
  int currentAnim;

  currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

  if (pi->weapon != pi->currentWeapon)
  {
    if (currentAnim != TORSO_DROP)
    {
      pi->torsoAnimationTimer = UI_TIMER_WEAPON_SWITCH;
      UI_ForceTorsoAnim(pi, TORSO_DROP);
    }
  }

  if (pi->torsoAnimationTimer > 0)
  {
    return;
  }

  if (currentAnim == TORSO_GESTURE)
  {
    UI_SetTorsoAnim(pi, TORSO_STAND);
    return;
  }

  if (currentAnim == TORSO_ATTACK || currentAnim == TORSO_ATTACK2)
  {
    UI_SetTorsoAnim(pi, TORSO_STAND);
    return;
  }

  if (currentAnim == TORSO_DROP)
  {
    UI_PlayerInfo_SetWeapon(pi, pi->weapon);
    pi->torsoAnimationTimer = UI_TIMER_WEAPON_SWITCH;
    UI_ForceTorsoAnim(pi, TORSO_RAISE);
    return;
  }

  if (currentAnim == TORSO_RAISE)
  {
    UI_SetTorsoAnim(pi, TORSO_STAND);
    return;
  }
}
*/

/*
===============
UI_LegsSequencing
===============
*/
/*
static void UI_LegsSequencing(playerInfo_t *pi)
{
  int currentAnim;

  currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;

  if (pi->legsAnimationTimer > 0)
  {
    if (currentAnim == LEGS_JUMP)
    {
      jumpHeight = JUMP_HEIGHT * sin(M_PI * (UI_TIMER_JUMP - pi->legsAnimationTimer) / UI_TIMER_JUMP);
    }
    return;
  }

  if (currentAnim == LEGS_JUMP)
  {
    UI_ForceLegsAnim(pi, LEGS_LAND);
    pi->legsAnimationTimer = UI_TIMER_LAND;
    jumpHeight             = 0;
    return;
  }

  if (currentAnim == LEGS_LAND)
  {
    UI_SetLegsAnim(pi, LEGS_IDLE);
    return;
  }
}
*/

/*
======================
UI_PositionEntityOnTag
======================
*/
/*
static void UI_PositionEntityOnTag(refEntity_t *entity, const refEntity_t *parent, clipHandle_t parentModel, char *tagName)
{
  int i;
  orientation_t lerped;

  // lerp the tag
  trap_CM_LerpTag(&lerped, parentModel, parent->oldframe, parent->frame,
                  1.0 - parent->backlerp, tagName);

  // FIXME: allow origin offsets along tag?
  VectorCopy(parent->origin, entity->origin);
  for (i = 0; i < 3; i++)
  {
    VectorMA(entity->origin, lerped.origin[i], parent->axis[i], entity->origin);
  }

  AxisMultiply(lerped.axis, ((refEntity_t *)parent)->axis, entity->axis);

  entity->backlerp = parent->backlerp;
}
*/

/*
======================
UI_PositionRotatedEntityOnTag
======================
*/
/*
static void UI_PositionRotatedEntityOnTag(refEntity_t *entity, const refEntity_t *parent, clipHandle_t parentModel, char *tagName)
{
  int i;
  orientation_t lerped;
  vec3_t tempAxis[3];

  // lerp the tag
  trap_CM_LerpTag(&lerped, parentModel, parent->oldframe, parent->frame,
                  1.0 - parent->backlerp, tagName);

  // FIXME: allow origin offsets along tag?
  VectorCopy(parent->origin, entity->origin);
  for (i = 0; i < 3; i++)
  {
    VectorMA(entity->origin, lerped.origin[i], parent->axis[i], entity->origin);
  }

  // cast away const because of compiler problems
  AxisMultiply(entity->axis, ((refEntity_t *)parent)->axis, tempAxis);
  AxisMultiply(lerped.axis, tempAxis, entity->axis);
}
*/

/*
===============
UI_SetLerpFrameAnimation
===============
*/
/*
static void UI_SetLerpFrameAnimation(playerInfo_t *ci, lerpFrame_t *lf, int newAnimation)
{
  animation_t *anim;

  lf->animationNumber = newAnimation;
  newAnimation       &= ~ANIM_TOGGLEBIT;

  if (newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS)
  {
    trap_Error(va("Bad animation number: %i", newAnimation));
  }

  anim = &ci->animations[newAnimation];

  lf->animation     = anim;
  lf->animationTime = lf->frameTime + anim->initialLerp;
}
*/

/*
===============
UI_RunLerpFrame
===============
*/
/*
static void UI_RunLerpFrame(playerInfo_t *ci, lerpFrame_t *lf, int newAnimation)
{
  int f;
  animation_t *anim;

  // see if the animation sequence is switching
  if (newAnimation != lf->animationNumber || !lf->animation)
  {
    UI_SetLerpFrameAnimation(ci, lf, newAnimation);
  }

  // if we have passed the current frame, move it to
  // oldFrame and calculate a new frame
  if (dp_realtime >= lf->frameTime)
  {
    lf->oldFrame     = lf->frame;
    lf->oldFrameTime = lf->frameTime;

    // get the next frame based on the animation
    anim = lf->animation;
    if (dp_realtime < lf->animationTime)
    {
      lf->frameTime = lf->animationTime;      // initial lerp
    }
    else
    {
      lf->frameTime = lf->oldFrameTime + anim->frameTime;
    }
    f = (lf->frameTime - lf->animationTime) / anim->frameTime;
    if (f >= anim->numFrames)
    {
      f -= anim->numFrames;
      if (anim->loopFrames)
      {
        f %= anim->loopFrames;
        f += anim->numFrames - anim->loopFrames;
      }
      else
      {
        f = anim->numFrames - 1;
        // the animation is stuck at the end, so it
        // can immediately transition to another sequence
        lf->frameTime = dp_realtime;
      }
    }
    lf->frame = anim->firstFrame + f;
    if (dp_realtime > lf->frameTime)
    {
      lf->frameTime = dp_realtime;
    }
  }

  if (lf->frameTime > dp_realtime + 200)
  {
    lf->frameTime = dp_realtime;
  }

  if (lf->oldFrameTime > dp_realtime)
  {
    lf->oldFrameTime = dp_realtime;
  }
  // calculate current lerp value
  if (lf->frameTime == lf->oldFrameTime)
  {
    lf->backlerp = 0;
  }
  else
  {
    lf->backlerp = 1.0 - (float)(dp_realtime - lf->oldFrameTime) / (lf->frameTime - lf->oldFrameTime);
  }
}
*/

/*
===============
UI_PlayerAnimation
===============
*/
/*
static void UI_PlayerAnimation(playerInfo_t *pi, int *legsOld, int *legs, float *legsBackLerp, int *torsoOld, int *torso, float *torsoBackLerp)
{

  // legs animation
  pi->legsAnimationTimer -= uiInfo.uiDC.frameTime;
  if (pi->legsAnimationTimer < 0)
  {
    pi->legsAnimationTimer = 0;
  }

  UI_LegsSequencing(pi);

  if (pi->legs.yawing && (pi->legsAnim & ~ANIM_TOGGLEBIT) == LEGS_IDLE)
  {
    UI_RunLerpFrame(pi, &pi->legs, LEGS_TURN);
  }
  else
  {
    UI_RunLerpFrame(pi, &pi->legs, pi->legsAnim);
  }
  *legsOld      = pi->legs.oldFrame;
  *legs         = pi->legs.frame;
  *legsBackLerp = pi->legs.backlerp;

  // torso animation
  pi->torsoAnimationTimer -= uiInfo.uiDC.frameTime;
  if (pi->torsoAnimationTimer < 0)
  {
    pi->torsoAnimationTimer = 0;
  }

  UI_TorsoSequencing(pi);

  UI_RunLerpFrame(pi, &pi->torso, pi->torsoAnim);
  *torsoOld      = pi->torso.oldFrame;
  *torso         = pi->torso.frame;
  *torsoBackLerp = pi->torso.backlerp;
}
*/

/*
==================
UI_SwingAngles
==================
*/
/*
static void UI_SwingAngles(float destination, float swingTolerance, float clampTolerance, float speed, float *angle, qboolean *swinging)
{
  float swing;
  float move;
  float scale;

  if (!*swinging)
  {
    // see if a swing should be started
    swing = AngleSubtract(*angle, destination);
    if (swing > swingTolerance || swing < -swingTolerance)
    {
      *swinging = qtrue;
    }
  }

  if (!*swinging)
  {
    return;
  }

  // modify the speed depending on the delta
  // so it doesn't seem so linear
  swing = AngleSubtract(destination, *angle);
  scale = fabs(swing);
  if (scale < swingTolerance * 0.5)
  {
    scale = 0.5;
  }
  else if (scale < swingTolerance)
  {
    scale = 1.0;
  }
  else
  {
    scale = 2.0;
  }

  // swing towards the destination angle
  if (swing >= 0)
  {
    move = uiInfo.uiDC.frameTime * scale * speed;
    if (move >= swing)
    {
      move      = swing;
      *swinging = qfalse;
    }
    *angle = AngleMod(*angle + move);
  }
  else if (swing < 0)
  {
    move = uiInfo.uiDC.frameTime * scale * -speed;
    if (move <= swing)
    {
      move      = swing;
      *swinging = qfalse;
    }
    *angle = AngleMod(*angle + move);
  }

  // clamp to no more than tolerance
  swing = AngleSubtract(destination, *angle);
  if (swing > clampTolerance)
  {
    *angle = AngleMod(destination - (clampTolerance - 1));
  }
  else if (swing < -clampTolerance)
  {
    *angle = AngleMod(destination + (clampTolerance - 1));
  }
}
*/

/*
======================
UI_MovedirAdjustment
======================
*/
/*
static float UI_MovedirAdjustment(playerInfo_t *pi)
{
  vec3_t relativeAngles;
  vec3_t moveVector;

  VectorSubtract(pi->viewAngles, pi->moveAngles, relativeAngles);
  AngleVectors(relativeAngles, moveVector, NULL, NULL);
  if (Q_fabs(moveVector[0]) < 0.01)
  {
    moveVector[0] = 0.0;
  }
  if (Q_fabs(moveVector[1]) < 0.01)
  {
    moveVector[1] = 0.0;
  }

  if (moveVector[1] == 0 && moveVector[0] > 0)
  {
    return 0;
  }
  if (moveVector[1] < 0 && moveVector[0] > 0)
  {
    return 22;
  }
  if (moveVector[1] < 0 && moveVector[0] == 0)
  {
    return 45;
  }
  if (moveVector[1] < 0 && moveVector[0] < 0)
  {
    return -22;
  }
  if (moveVector[1] == 0 && moveVector[0] < 0)
  {
    return 0;
  }
  if (moveVector[1] > 0 && moveVector[0] < 0)
  {
    return 22;
  }
  if (moveVector[1] > 0 && moveVector[0] == 0)
  {
    return -45;
  }

  return -22;
}
*/

/*
===============
UI_PlayerAngles
===============
*/
/*
static void UI_PlayerAngles(playerInfo_t *pi, vec3_t legs[3], vec3_t torso[3], vec3_t head[3])
{
  vec3_t legsAngles, torsoAngles, headAngles;
  float dest;
  float adjust;

  VectorCopy(pi->viewAngles, headAngles);
  headAngles[YAW] = AngleMod(headAngles[YAW]);
  VectorClear(legsAngles);
  VectorClear(torsoAngles);

  // --------- yaw -------------

  // allow yaw to drift a bit
  if ((pi->legsAnim & ~ANIM_TOGGLEBIT) != LEGS_IDLE
      || (pi->torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND)
  {
    // if not standing still, always point all in the same direction
    pi->torso.yawing   = qtrue;    // always center
    pi->torso.pitching = qtrue;    // always center
    pi->legs.yawing    = qtrue;    // always center
  }

  // adjust legs for movement dir
  adjust           = UI_MovedirAdjustment(pi);
  legsAngles[YAW]  = headAngles[YAW] + adjust;
  torsoAngles[YAW] = headAngles[YAW] + 0.25 * adjust;


  // torso
  UI_SwingAngles(torsoAngles[YAW], 25, 90, SWINGSPEED, &pi->torso.yawAngle, &pi->torso.yawing);
  UI_SwingAngles(legsAngles[YAW], 40, 90, SWINGSPEED, &pi->legs.yawAngle, &pi->legs.yawing);

  torsoAngles[YAW] = pi->torso.yawAngle;
  legsAngles[YAW]  = pi->legs.yawAngle;

  // --------- pitch -------------

  // only show a fraction of the pitch angle in the torso
  if (headAngles[PITCH] > 180)
  {
    dest = (-360 + headAngles[PITCH]) * 0.75;
  }
  else
  {
    dest = headAngles[PITCH] * 0.75;
  }
  UI_SwingAngles(dest, 15, 30, 0.1f, &pi->torso.pitchAngle, &pi->torso.pitching);
  torsoAngles[PITCH] = pi->torso.pitchAngle;

  // pull the angles back out of the hierarchial chain
  AnglesSubtract(headAngles, torsoAngles, headAngles);
  AnglesSubtract(torsoAngles, legsAngles, torsoAngles);
  AnglesToAxis(legsAngles, legs);
  AnglesToAxis(torsoAngles, torso);
  AnglesToAxis(headAngles, head);
}
*/

/*
===============
UI_PlayerFloatSprite
===============
*/
/*
static void UI_PlayerFloatSprite(playerInfo_t *pi, vec3_t origin, qhandle_t shader)
{
  refEntity_t ent;

  Com_Memset(&ent, 0, sizeof(ent));
  VectorCopy(origin, ent.origin);
  ent.origin[2]   += 48;
  ent.reType       = RT_SPRITE;
  ent.customShader = shader;
  ent.radius       = 10;
  ent.renderfx     = 0;
  trap_R_AddRefEntityToScene(&ent);
}
*/

/*
======================
UI_MachinegunSpinAngle
======================
*/
/*
float UI_MachinegunSpinAngle(playerInfo_t *pi)
{
  int delta;
  float angle;
  float speed;
  int torsoAnim;

  delta = dp_realtime - pi->barrelTime;
  if (pi->barrelSpinning)
  {
    angle = pi->barrelAngle + delta * SPIN_SPEED;
  }
  else
  {
    if (delta > COAST_TIME)
    {
      delta = COAST_TIME;
    }

    speed = 0.5 * (SPIN_SPEED + (float)(COAST_TIME - delta) / COAST_TIME);
    angle = pi->barrelAngle + delta * speed;
  }

  torsoAnim = pi->torsoAnim  & ~ANIM_TOGGLEBIT;
  if (torsoAnim == TORSO_ATTACK2)
  {
    torsoAnim = TORSO_ATTACK;
  }
  if (pi->barrelSpinning == !(torsoAnim == TORSO_ATTACK))
  {
    pi->barrelTime     = dp_realtime;
    pi->barrelAngle    = AngleMod(angle);
    pi->barrelSpinning = !!(torsoAnim == TORSO_ATTACK);
  }

  return angle;
}
*/

/*
===============
UI_DrawPlayer
===============
*/
void UI_DrawPlayer(float x, float y, float w, float h, playerInfo_t * pi, int time,
  int mouseX, int mouseY)
{
  UI_XPPM_Player(x, y, w, h, pi, time, mouseX, mouseY);
}

/*
==========================
UI_FileExists
==========================
*/
/*
static qboolean UI_FileExists(const char *filename)
{
  int len;

  len = trap_FS_FOpenFile(filename, NULL, FS_READ);
  if (len > 0)
  {
    return qtrue;
  }
  return qfalse;
}
*/

/*
==========================
UI_FindClientHeadFile
==========================
*/
/*
static qboolean UI_FindClientHeadFile(char *filename, int length, const char *teamName, const char *headModelName, const char *headSkinName, const char *base, const char *ext)
{
  char *team, *headsFolder;
  int i;

  team = "default";

  if (headModelName[0] == '*')
  {
    headsFolder = "heads/";
    headModelName++;
  }
  else
  {
    headsFolder = "";
  }
  while (1)
  {
    for (i = 0; i < 2; i++)
    {
      if (i == 0 && teamName && *teamName)
      {
        Com_sprintf(filename, length, "models/players/%s%s/%s/%s%s_%s.%s", headsFolder, headModelName, headSkinName, teamName, base, team, ext);
      }
      else
      {
        Com_sprintf(filename, length, "models/players/%s%s/%s/%s_%s.%s", headsFolder, headModelName, headSkinName, base, team, ext);
      }
      if (UI_FileExists(filename))
      {
        return qtrue;
      }
      if (i == 0 && teamName && *teamName)
      {
        Com_sprintf(filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, headSkinName, ext);
      }
      else
      {
        Com_sprintf(filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, headSkinName, ext);
      }
      if (UI_FileExists(filename))
      {
        return qtrue;
      }
      if (!teamName || !*teamName)
      {
        break;
      }
    }
    // if tried the heads folder first
    if (headsFolder[0])
    {
      break;
    }
    headsFolder = "heads/";
  }

  return qfalse;
}
*/

/*
==========================
UI_RegisterClientModelname
==========================
*/
qboolean UI_RegisterClientModelname(playerInfo_t *pi, const char *modelName, const char *skinName, const char *clanName, const char *teamName)
{
  char            model[MAX_QPATH];
  char            skin[MAX_QPATH];
  char           *slash;

  pi->torsoModel = 0;
  pi->headModel = 0;

  if(!modelName[0])
  {
	  return qfalse;
  }

  Q_strncpyz(model, modelName, sizeof(model));

  if (clanName[0] == '\0') //DM
  {
    slash = strchr(model, '/');
    if (!slash)
    {
      // modelName did not include a skin name
      Q_strncpyz(skin, "default", sizeof(skin));
    }
    else
    {
      Q_strncpyz(skin, slash + 1, sizeof(skin));
      // truncate modelName
      *slash = 0;
    }
  }
  else //TEAM
  {
    Q_strncpyz(skin, skinName, sizeof(skin));
  }

#ifdef XPPM
  return UI_XPPM_RegisterModel(pi, model, skin, clanName, teamName);
#endif

}


/*
===============
UI_PlayerInfo_SetModel
===============
*/
void UI_PlayerInfo_SetModel(playerInfo_t *pi, const char *model, const char *headName, const char *clanName, const char *teamName)
{
  Com_Memset(pi, 0, sizeof(*pi));
  UI_RegisterClientModelname(pi, model, headName, clanName, teamName);
//remove hypov8
/*
  pi->weapon        = WP_MACHINEGUN;
  pi->currentWeapon = pi->weapon;
  pi->lastWeapon    = pi->weapon;
  pi->pendingWeapon = (weapon_t)-1;
  pi->weaponTimer   = 0;
  pi->chat          = qfalse;
  pi->newModel      = qtrue;
  UI_PlayerInfo_SetWeapon(pi, pi->weapon);*/
//end remove hypov8
}


/*
===============
UI_PlayerInfo_SetInfo
===============
*///removed hypov8
/*
void UI_PlayerInfo_SetInfo(playerInfo_t *pi, int legsAnim, int torsoAnim, vec3_t viewAngles, vec3_t moveAngles, weapon_t weaponNumber, qboolean chat)
{
  int currentAnim;
  weapon_t weaponNum;

  pi->chat = chat;

  // view angles
  VectorCopy(viewAngles, pi->viewAngles);

  // move angles
  VectorCopy(moveAngles, pi->moveAngles);

  if (pi->newModel)
  {
    pi->newModel = qfalse;

    jumpHeight          = 0;
    pi->pendingLegsAnim = 0;
    UI_ForceLegsAnim(pi, legsAnim);
    pi->legs.yawAngle = viewAngles[YAW];
    pi->legs.yawing   = qfalse;

    pi->pendingTorsoAnim = 0;
    UI_ForceTorsoAnim(pi, torsoAnim);
    pi->torso.yawAngle = viewAngles[YAW];
    pi->torso.yawing   = qfalse;

    if (weaponNumber != -1)
    {
      pi->weapon        = weaponNumber;
      pi->currentWeapon = weaponNumber;
      pi->lastWeapon    = weaponNumber;
      pi->pendingWeapon = (weapon_t)-1;
      pi->weaponTimer   = 0;
      UI_PlayerInfo_SetWeapon(pi, pi->weapon);
    }

    return;
  }

  // weapon
  if (weaponNumber == -1)
  {
    pi->pendingWeapon = (weapon_t)-1;
    pi->weaponTimer   = 0;
  }
  else if (weaponNumber != WP_NONE)
  {
    pi->pendingWeapon = weaponNumber;
    pi->weaponTimer   = dp_realtime + UI_TIMER_WEAPON_DELAY;
  }
  weaponNum  = pi->lastWeapon;
  pi->weapon = weaponNum;

  if (torsoAnim == BOTH_DEATH1 || legsAnim == BOTH_DEATH1)
  {
    torsoAnim  = legsAnim = BOTH_DEATH1;
    pi->weapon = pi->currentWeapon = WP_NONE;
    UI_PlayerInfo_SetWeapon(pi, pi->weapon);

    jumpHeight          = 0;
    pi->pendingLegsAnim = 0;
    UI_ForceLegsAnim(pi, legsAnim);

    pi->pendingTorsoAnim = 0;
    UI_ForceTorsoAnim(pi, torsoAnim);

    return;
  }

  // leg animation
  currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;
  if (legsAnim != LEGS_JUMP && (currentAnim == LEGS_JUMP || currentAnim == LEGS_LAND))
  {
    pi->pendingLegsAnim = legsAnim;
  }
  else if (legsAnim != currentAnim)
  {
    jumpHeight          = 0;
    pi->pendingLegsAnim = 0;
    UI_ForceLegsAnim(pi, legsAnim);
  }

  // torso animation
  if (torsoAnim == TORSO_STAND || torsoAnim == TORSO_STAND2)
  {
    if (weaponNum == WP_NONE || weaponNum == WP_CROWBAR)
    {
      torsoAnim = TORSO_STAND2;
    }
    else
    {
      torsoAnim = TORSO_STAND;
    }
  }

  if (torsoAnim == TORSO_ATTACK || torsoAnim == TORSO_ATTACK2)
  {
    if (weaponNum == WP_NONE || weaponNum == WP_CROWBAR)
    {
      torsoAnim = TORSO_ATTACK2;
    }
    else
    {
      torsoAnim = TORSO_ATTACK;
    }
    pi->muzzleFlashTime = dp_realtime + UI_TIMER_MUZZLE_FLASH;
    //FIXME play firing sound here
  }

  currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

  if (weaponNum != pi->currentWeapon || currentAnim == TORSO_RAISE || currentAnim == TORSO_DROP)
  {
    pi->pendingTorsoAnim = torsoAnim;
  }
  else if ((currentAnim == TORSO_GESTURE || currentAnim == TORSO_ATTACK) && (torsoAnim != currentAnim))
  {
    pi->pendingTorsoAnim = torsoAnim;
  }
  else if (torsoAnim != currentAnim)
  {
    pi->pendingTorsoAnim = 0;
    UI_ForceTorsoAnim(pi, torsoAnim);
  }
}
*/
