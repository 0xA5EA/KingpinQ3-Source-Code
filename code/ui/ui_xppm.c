/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

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

// ui_xppm.c -- handle md5 model stuff ( instead of ui_players.c )
//#include "ui_local.h" //remove hypov8

#include "ui_local.h"

static int      dp_realtime;

#define SWINGSPEED				0.3f

#ifdef XPPM

/*
======================
UI_XPPMParseCharacterFile

Read a configuration file containing body.md5mesh custom
models/players/thug/character.cfg, etc
======================
*/
static qboolean UI_XPPMParseCharacterFile(const char *filename, playerInfo_t * pi)
{
  char           *text_p, *prev;
  int             len;
  int             i;
  char           *token;
  char            text[20000];
  fileHandle_t    f;

  Com_Printf("UI_XPPM: Loading configuration %s\n", filename);

  // load the file
  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if (len <= 0)
  {
    return qfalse;
  }
  if (len >= sizeof(text) - 1)
  {
    Com_Printf("File %s too long\n", filename);
    trap_FS_FCloseFile(f);
    return qfalse;
  }

  trap_FS_Read(text, len, f);
  text[len] = 0;
  trap_FS_FCloseFile(f);

  // parse the text
  text_p = text;

  pi->gender = GENDER_MALE;
  pi->firstTorsoBoneName[0] = '\0';
  pi->lastTorsoBoneName[0] = '\0';
  pi->torsoControlBoneName[0] = '\0';
  pi->neckControlBoneName[0] = '\0';
  pi->modelScale = 1;

#ifdef COMPAT_KPQ3
  VectorSet(pi->wepRotate, 0, 0, 0);
  VectorSet(pi->flagRotate, 0, 0, 0);
#endif

  // read optional parameters
  while (1)
  {
    prev = text_p;			// so we can unget
    (void)prev;         // shutup compiler
    token = Com_Parse(&text_p);
    if (!token[0])
    {
      break;
    }

    if (!Q_stricmp(token, "sex"))
    {
      token = Com_Parse(&text_p);
      if (!token)
      {
        break;
      }
      if (token[0] == 'f' || token[0] == 'F')
      {
        pi->gender = GENDER_FEMALE;
      }
      else if (token[0] == 'n' || token[0] == 'N')
      {
        pi->gender = GENDER_NEUTER;
      }
      else
      {
        pi->gender = GENDER_MALE;
      }
      continue;
    }
    else if(!Q_stricmp(token, "footsteps"))
    {
      token = Com_Parse(&text_p);
      if(!token)
        break;
      continue;
    }
    else if(!Q_stricmp(token, "headoffset"))
    {
      for(i = 0; i < 3; i++)
      {
        token = Com_Parse(&text_p);
        if(!token)
          break;
      }
      continue;
    }
    else if (!Q_stricmp(token, "firstTorsoBoneName"))
    {
      token = Com_Parse(&text_p);
      if (!token)
        break;
      continue;
    }
    else if (!Q_stricmp(token, "torsoControlBoneName"))
    {
      token = Com_Parse(&text_p);
      if (!token)
        break;
      //Q_strncpyz(pi->torsoControlBoneName, token, sizeof(pi->torsoControlBoneName));
      continue;
    }
    else if (!Q_stricmp(token, "neckControlBoneName"))
    {
      token = Com_Parse(&text_p);
      if (!token)
        break;
      //Q_strncpyz(pi->neckControlBoneName, token, sizeof(pi->neckControlBoneName));
      continue;
    }
    else if (!Q_stricmp(token, "modelScale"))
    {
      token = Com_Parse(&text_p);
      if (!token)
        break;
      pi->modelScale = atof(token); //todo update to vec_t
      continue;
    }
#ifdef COMPAT_KPQ3
    //copy cg_players.c CG_ParseCharacterFile
    else if (!Q_stricmp(token, "wepRotate"))
    {
      for(i = 0; i < 3; i++)
      {
        token = Com_ParseExt(&text_p, qfalse);
        if(!token)
          break;
        pi->wepRotate[i] = atof(token);
      }
      continue;
    }
    else if (!Q_stricmp(token, "flagRotate"))
    {
      for(i = 0; i < 3; i++)
      {
        token = Com_ParseExt(&text_p, qfalse);
        if(!token)
          break;
        pi->flagRotate[i] = atof(token);
      }
      continue;
    }
#endif

    Com_Printf(S_COLOR_YELLOW"unknown token '%s' is %s\n", token, filename);
  }

  return qtrue;
}

static qboolean UI_XPPM_RegisterPlayerAnimation(playerInfo_t * pi, const char *modelName, int anim, const char *animName,
    qboolean loop, qboolean reversed, qboolean clearOrigin)
{
  char            filename[MAX_QPATH];
  int             frameRate;


  Com_sprintf(filename, sizeof(filename), "models/players/%s/ani/%s.md5anim", modelName, animName);

  // Com_Printf("UI_XPPM: Loading animation %s\n", filename); hypov8 debug print .md5anim loaded

  pi->animations[anim].handle = trap_R_RegisterAnimation(filename);


  if (!pi->animations[anim].handle)
  {
    Com_Printf(S_COLOR_YELLOW"Failed to load animation file %s\n", filename);
    return qfalse;
  }

  pi->animations[anim].firstFrame = 0;
  pi->animations[anim].numFrames = trap_R_AnimNumFrames(pi->animations[anim].handle);
  frameRate = trap_R_AnimFrameRate(pi->animations[anim].handle);

  if (frameRate == 0)
  {
    frameRate = 1;
  }
  pi->animations[anim].frameLerp = 1000 / frameRate;
  pi->animations[anim].initialLerp = 1000 / frameRate;

  if (loop)
  {
    pi->animations[anim].loopFrames = pi->animations[anim].numFrames;
  }
  else
  {
    pi->animations[anim].loopFrames = 0;
  }

  pi->animations[anim].reversed = reversed;
  pi->animations[anim].clearOrigin = clearOrigin;

  return qtrue;
}



/*
==========================
UI_XPPM_RegisterModel
==========================
*/

qboolean UI_XPPM_RegisterModel(playerInfo_t * pi, const char *modelName, const char *skinName, const char *clanName)
{
  char            filename[MAX_QPATH * 2];

  Com_sprintf(filename, sizeof(filename), "models/players/%s/body.md5mesh", modelName);

  Com_Printf("UI_XPPM: Loading model %s\n", filename);

  pi->bodyModel = trap_R_RegisterModel(filename); //remove hypov8 , qfalse);

  if (!pi->bodyModel)
  {
    Com_Printf(S_COLOR_YELLOW"Failed to load body mesh file  %s\n", filename);
    return qfalse;
  }

  // load the animations
  Com_sprintf(filename, sizeof(filename), "models/players/%s/ani/character.cfg", modelName);
  if (!UI_XPPMParseCharacterFile(filename, pi))
  {
    Com_Printf(S_COLOR_YELLOW"Failed to load character file %s\n", filename);
    return qfalse;
  }

  if (!UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_IDLE, "tor_idle", qtrue, qfalse, qfalse)) //tommy
  {
    Com_Printf(S_COLOR_YELLOW"Failed to load idle animation file %s\n", filename);
    return qfalse;
  }

  // make LEGS_IDLE the default animation
  /*for (i = 0; i < MAX_TOTALANIMATIONS; i++)
  {
    if (i == LEGS_IDLE)
      continue;

    pi->animations[i] = pi->animations[LEGS_IDLE];
  }*/

  UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_GESTURE,  "tor_gesture",  qfalse, qfalse, qfalse);
  UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_GESTURE2, "tor_gesture2", qfalse, qfalse, qfalse);
  UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_GESTURE3, "tor_gesture3", qfalse, qfalse, qfalse);
  //UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_STAND,    "tor_idle",     qtrue, qfalse, qfalse); //tommy
  //UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_STAND2,   "tor_idle2",    qtrue, qfalse, qfalse); //pistol
  //UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_STAND3,   "tor_idle3",    qtrue, qfalse, qfalse); //crowbar

  //team skin
  if (clanName[0] != '\0')
    //Com_sprintf(filename, sizeof(filename), "models/players/%s/team_%s_%s.skin", modelName, skinName, teamName);
    Com_sprintf(filename, sizeof(filename), "models/players/%s/clans/%s/%s.skin", modelName, clanName, TEAM_SKIN_DRAGONS); //default to red  in menu
  else
    Com_sprintf(filename, sizeof(filename), "models/players/%s/body_%s.skin", modelName, skinName);

  Com_Printf("UI_XPPM: Loading skin %s\n", filename);

  pi->bodySkin = trap_R_RegisterSkin(filename);

  if (!pi->bodySkin)
  {
    Com_Printf("Body skin load failure: %s\n", filename);
    return qfalse;
  }

  return qtrue;
}

#if 0
void UI_XPPM_TransformSkeleton(refSkeleton_t * skel, const vec3_t scale)
{

  int             i;
  refBone_t      *bone;

  switch (skel->type)
  {
  case SK_INVALID:
  case SK_ABSOLUTE:
    return;

  default:
    break;
  }

  // calculate absolute transforms
  for (i = 0, bone = &skel->bones[0]; i < skel->numBones; i++, bone++)
  {
    if (bone->parentIndex >= 0)
    {
      vec3_t          rotated;
      quat_t          quat;

      refBone_t      *parent;

      parent = &skel->bones[bone->parentIndex];

      QuatTransformVector(parent->t.rot, bone->t.trans, rotated);

      if (scale)
      {
        rotated[0] *= scale[0];
        rotated[1] *= scale[1];
        rotated[2] *= scale[2];
      }

      VectorAdd(parent->t.trans, rotated, bone->t.trans);

      QuatMultiply1(parent->t.rot, bone->t.rot, quat);
      QuatCopy(quat, bone->t.rot);
    }
  }

  skel->type = SK_ABSOLUTE;

  if (scale)
  {
    VectorCopy(scale, skel->scale);
  }
  else
  {
    VectorSet(skel->scale, 1, 1, 1);
  }
}
#else
void UI_XPPM_TransformSkeleton(refSkeleton_t *skel, const vec_t scale)
{
  int       i;
  refBone_t *bone;

  switch (skel->type)
  {
  case SK_INVALID:
  case SK_ABSOLUTE:
    return;

  default:
    break;
  }

  // calculate absolute transforms
  for (i = 0, bone = &skel->bones[0]; i < skel->numBones; i++, bone++)
  {
    if (bone->parentIndex >= 0)
    {
      refBone_t *parent;

      parent = &skel->bones[bone->parentIndex];

      TransCombine(&bone->t, &parent->t, &bone->t);
    }
  }

  skel->type = SK_ABSOLUTE;
  skel->scale = scale;
}
#endif
static void UI_XPPM_SwingAngles(float destination, float swingTolerance, float clampTolerance,
                                float speed, float *angle, qboolean * swinging)
{
  float           swing;
  float           move;
  float           scale;

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
    move = uiInfo.uiDC.frameTime * scale * speed; //rename .uis hypov8 //rename hypov8 ftametime
    if (move >= swing)
    {
      move = swing;
      *swinging = qfalse;
    }
    *angle = AngleMod(*angle + move);
  }
  else if (swing < 0)
  {
    move = uiInfo.uiDC.frameTime * scale * -speed; //rename hypov8 was = move = uis.frametime * scale * -speed;
    if (move <= swing)
    {
      move = swing;
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

static float UI_XPPM_MovedirAdjustment(playerInfo_t * pi)
{
  vec3_t          relativeAngles;
  vec3_t          moveVector;

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

static void UI_XPPM_PlayerAngles(playerInfo_t * pi, vec3_t legsAngles, vec3_t torsoAngles, vec3_t headAngles)
{
  float           dest;
  float           adjust;

  VectorCopy(pi->viewAngles, headAngles);
  headAngles[YAW] = AngleMod(headAngles[YAW]);
  VectorClear(legsAngles);
  VectorClear(torsoAngles);

  // --------- yaw -------------

  // allow yaw to drift a bit
  if ((pi->legsAnim & ~ANIM_TOGGLEBIT) != LEGS_IDLE || (pi->torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND)
  {
    // if not standing still, always point all in the same direction
    pi->torso.yawing = qtrue;	// always center
    pi->torso.pitching = qtrue;	// always center
    pi->legs.yawing = qtrue;	// always center
  }

  // adjust legs for movement dir
  adjust = UI_XPPM_MovedirAdjustment(pi);
  legsAngles[YAW] = headAngles[YAW] + adjust;
  torsoAngles[YAW] = headAngles[YAW] + 0.25 * adjust;


  // torso
  UI_XPPM_SwingAngles(torsoAngles[YAW], 25, 90, SWINGSPEED, &pi->torso.yawAngle, &pi->torso.yawing);
  UI_XPPM_SwingAngles(legsAngles[YAW], 40, 90, SWINGSPEED, &pi->legs.yawAngle, &pi->legs.yawing);

  torsoAngles[YAW] = pi->torso.yawAngle;
  legsAngles[YAW] = pi->legs.yawAngle;

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
  UI_XPPM_SwingAngles(dest, 15, 30, 0.1f, &pi->torso.pitchAngle, &pi->torso.pitching);
  torsoAngles[PITCH] = pi->torso.pitchAngle;

  // pull the angles back out of the hierarchial chain
  AnglesSubtract(headAngles, torsoAngles, headAngles);
  AnglesSubtract(torsoAngles, legsAngles, torsoAngles);
}


#if 0 // solves unused compiler warning
static void UI_XPPM_PositionEntityOnTag(refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName)
{
  int i;
  orientation_t lerped;

  Q_UNUSED(parentModel);

  // lerp the tag
#if defined(COMPAT_KPQ3) || defined(COMPAT_ET)
  trap_R_LerpTag( &lerped, parent, tagName, 0 );
#else
  trap_R_LerpTag(&lerped, parentModel, parent->oldframe, parent->frame,  1.0 - parent->backlerp, tagName);
 #endif
  // FIXME: allow origin offsets along tag?
  VectorCopy(parent->origin, entity->origin);

  for (i = 0; i < 3; i++)
  {
    VectorMA(entity->origin, lerped.origin[i], parent->axis[i], entity->origin);
  }

  // had to cast away the const to avoid compiler problems...
  AxisMultiply(lerped.axis, ((refEntity_t *)parent)->axis, entity->axis);
  entity->backlerp = parent->backlerp;
}
#endif


/*
======================
UI_XPPM_PositionRotatedSelfOnTag

hypov8 add: 
offset gun using internal tag. so 1 model can be used for w_map and w_player
======================
*/
void UI_XPPM_PositionRotatedSelfOnTag(refEntity_t *entity, char *tagSelfName)
{
  int i, tagIndex;  
  orientation_t offsetTag;

  if (tagSelfName[0] != '\0')
  {
    tagIndex = trap_R_LerpTag( &offsetTag, entity, tagSelfName, 0 ); //"tag_weapon"
    if (tagIndex > -1)
    {
      matrix_t mat1;
      vec3_t axisOut[3], tempAxis[3];

      MatrixFromVectorsFLU(mat1, offsetTag.axis[0], offsetTag.axis[1], offsetTag.axis[2]); 
      MatrixInverse(mat1);
      MatrixToVectorsFLU(mat1, axisOut[0], axisOut[1], axisOut[2]);
      AxisMultiply(axisOut, entity->axis, tempAxis);
      AxisCopy(tempAxis, entity->axis);

      //add inverted offsest.
      for (i = 0; i < 3; i++)
      {
        VectorMA(entity->origin, -offsetTag.origin[i], entity->axis[i], entity->origin);
      }
    }
  }
}


/*
======================
UI_XPPM_PositionRotatedEntityOnTag
Modifies the entities position and axis by the given
tag location
======================
*/
void UI_XPPM_PositionRotatedEntityOnTag(refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName)
{
  int i;
  orientation_t lerped;
  vec3_t tempAxis[3];

  Q_UNUSED(parentModel);
//AxisClear( entity->axis );
  // lerp the tag
#if defined(COMPAT_KPQ3) || defined(COMPAT_ET)
  trap_R_LerpTag( &lerped, parent, tagName, 0 );
#else
  trap_R_LerpTag(&lerped, parentModel, parent->oldframe, parent->frame, 1.0 - parent->backlerp, tagName);
#endif

  // FIXME: allow origin offsets along tag?
  VectorCopy(parent->origin, entity->origin);

  for (i = 0; i < 3; i++)
  {
    VectorMA(entity->origin, lerped.origin[i], parent->axis[i], entity->origin);
  }

  // had to cast away the const to avoid compiler problems...
  AxisMultiply(entity->axis, lerped.axis, tempAxis);
  AxisMultiply(tempAxis, ((refEntity_t *)parent)->axis, entity->axis);
}

//add hypov8
void UI_XPPM_AddPlayerWeapon(refEntity_t *parent, playerInfo_t * pi)
{
  static refEntity_t gun;
  vec3_t angles;

  // add the weapon
  Com_Memset(&gun, 0, sizeof(gun));
  gun.shadowPlane = parent->shadowPlane;
  gun.renderfx    = parent->renderfx;
  gun.hModel = trap_R_RegisterModel("models/weapons/tomgun/w_wep.md3"); //gitem_t bg_itemlist[] "weapon_tommygun"

  if (!gun.hModel)
    return;

  angles[PITCH] = pi->wepRotate[PITCH] ;
  angles[YAW] = pi->wepRotate[YAW]; // 90
  angles[ROLL] = pi->wepRotate[ROLL]; // 90
  AnglesToAxis(angles, gun.axis);
  UI_XPPM_PositionRotatedEntityOnTag(&gun, parent, parent->hModel, "tag_weapon");
  UI_XPPM_PositionRotatedSelfOnTag(&gun, "tag_weapon"); //hypov8 add
  trap_R_AddRefEntityToScene(&gun);
}

static void UI_XPPM_CalculateLerpFrame(lerpFrame_t *lf)
{


  //copy from CG_RunLerpFrame
  //lf->animationEnded = qfalse;

  if (dp_realtime > lf->frameTime)
  {
    if (dp_realtime < lf->animationTime)
    {
      // initial lerp
      lf->frameTime = lf->oldFrameTime = lf->animationTime;
      lf->oldFrame = lf->frame;
    }
    else
    {
      lf->oldFrame = lf->frame;
      lf->oldFrameTime = lf->frameTime;

      animation_t *anim = lf->animation;
      int numFrames = anim->numFrames;
      float frameLength = anim->frameLerp;

      int relativeFrame = ceil((dp_realtime - lf->animationTime) / frameLength);
      if (relativeFrame >= numFrames)
      {
        qboolean looping = !!anim->loopFrames;
        if (looping)
        {
          assert(anim->loopFrames == numFrames);//hypov8
          lf->animationTime += relativeFrame / numFrames * (numFrames * frameLength);
          relativeFrame %= numFrames;
          lf->frameTime = lf->animationTime + relativeFrame * frameLength;
        }
        else
        {
          relativeFrame = numFrames - 1;
          // the animation is stuck at the end, so it
          // can immediately transition to another sequence
          lf->frameTime = dp_realtime;
          //lf->animationEnded = qtrue;
        }
      }
      else
      {
        lf->frameTime = lf->animationTime + relativeFrame * frameLength;
      }
      if (anim->reversed)
      {
        lf->frame = anim->firstFrame + numFrames - 1 - relativeFrame;
      }
      else
      {
        lf->frame = anim->firstFrame + relativeFrame;
      }
    }
  }

  if (lf->frameTime > lf->oldFrameTime)
  {
    lf->backlerp = 1.0 - (float)(dp_realtime - lf->oldFrameTime) / (lf->frameTime - lf->oldFrameTime);
  }
  else
  {
    lf->backlerp = 0;
  }
}

void UI_XPPM_ResetLerpAnim(lerpFrame_t *lf)
{
  lf->frame = lf->oldFrame = 0;
  lf->frameTime = lf->oldFrameTime = dp_realtime;
}

void UI_XPPM_BuildAnimSkeleton(lerpFrame_t *lf, refSkeleton_t *skel)
{

  if (!lf->animation || !lf->animation->handle)
  {
    // initialize skeleton if animation handle is invalid
    int i;

    skel->type = SK_ABSOLUTE;
    skel->numBones = MAX_BONES;
    for (i = 0; i < MAX_BONES; i++)
    {
      skel->bones[i].parentIndex = -1;
      TransInit(&skel->bones[i].t);
    }
    return;
  }

  if (!trap_R_BuildSkeleton(skel, lf->animation->handle, lf->oldFrame, lf->frame, 1.0 - lf->backlerp, qfalse))
  {
    Com_Printf("Can't build animation\n");
    return;
  }
}

static void CG_SetLerpFrameAnimation( playerInfo_t *pi, lerpFrame_t *lf, refSkeleton_t *skel)
{

  lf->animationNumber = LEGS_IDLE;
  lf->animation = &pi->animations[LEGS_IDLE]; 
  lf->backlerp = 0;

  if (!trap_R_BuildSkeleton(skel, lf->animation->handle, lf->oldFrame, lf->frame, lf->backlerp, qfalse))
  {
    Com_Printf("Can't build animation\n");
    return;
  }

  lf->animationTime = dp_realtime + lf->animation->initialLerp;
  lf->oldFrame = lf->frame = 0;
  lf->oldFrameTime = lf->frameTime = 0;
}


void UI_XPPM_RunLerpFrame(playerInfo_t * pi, lerpFrame_t *lf, refSkeleton_t *skel)
{
  // see if the animation sequence is switching
  if (!lf->animation )
  {
    CG_SetLerpFrameAnimation(pi, lf, skel);
  }

  UI_XPPM_CalculateLerpFrame(lf);
  //CG_BlendLerpFrame( lf );
  UI_XPPM_BuildAnimSkeleton( lf, skel );//oldSkeleton
}

void UI_XPPM_Player(float x, float y, float w, float h, playerInfo_t * pi, int time,
  int mouseX, int mouseY)
{
  refEntity_t     body;
  refEntity_t     podium;
  refdef_t        refdef;

  vec3_t          legsAngles;
  //vec3_t          torsoAngles;
  //vec3_t          headAngles;

  //vec3_t          podiumAngles;

  vec3_t          origin;
  int             renderfx;

  vec3_t          mins = { -16, -16, -0 };
  vec3_t          maxs = { 16, 16, 72 };
  float           len;
  float           xx;
  int             minXY[2], maxXY[2];
  static float    rotAngle = 160.0f;


  if (!pi->bodyModel)
    return;

  if (!pi->bodySkin)
    return;

  dp_realtime = time;
  
  minXY[0] = x;
  minXY[1] = y;
  maxXY[0] = x+w;
  maxXY[1] = y+h;

#if  HYPODEBUG
  UI_DrawRect(x, y, w, h, colorYellow); //hypov8 enabled to test
#endif

  UI_AdjustFrom640(&x, &y, &w, &h);

  Com_Memset(&refdef, 0, sizeof(refdef));
  Com_Memset(&body, 0, sizeof(body));
  Com_Memset(&podium, 0, sizeof(podium));

  refdef.rdflags = RDF_NOWORLDMODEL | RDF_NOSHADOWS;

  AxisClear(refdef.viewaxis);

  refdef.x = x;
  refdef.y = y;
  refdef.width = w;
  refdef.height = h;

  refdef.fov_x = (int)((float)refdef.width / 640.0f * 60.0f);
  xx = refdef.width / tan(refdef.fov_x / 360 * M_PI);
  refdef.fov_y = atan2(refdef.height, xx);
  refdef.fov_y *= (360 / M_PI);

  // calculate distance so the player nearly fills the box
  len = 0.5 * (maxs[2] - mins[2]); //hypov8 was 0.7*
  origin[0] = len / tan(DEG2RAD(refdef.fov_x) * 0.5) + 10;
  origin[1] = 0.5 * (mins[1] + maxs[1]);
  origin[2] = -0.5 * (mins[2] + maxs[2]);
  //origin[2] -= len /*- 20*/;

  refdef.time = dp_realtime;

  trap_R_ClearScene();

  // get the rotation information
//#if 0 //remove hypov8
  //UI_XPPM_PlayerAngles(pi, legsAngles, torsoAngles, headAngles);

  if (mouseX > minXY[0] && mouseX < maxXY[0] &&
      mouseY > minXY[1] && mouseY < maxXY[1])
  {
    rotAngle = AngleNormalize360(160.0f + ((mouseX-320) * 0.3f));
  }

  legsAngles[PITCH] = 0;
  legsAngles[YAW] = rotAngle;
  legsAngles[ROLL] = 0;

//#else //remove hypov8
#if 0
  // Quake 2 style
//legsAngles[YAW] = AngleNormalize360((float)(uis.realtime / 30.0)); //edit hypov8
  legsAngles[YAW] = AngleNormalize360((float)(uiInfo.uiDC.realTime / 30.0)); //180 - 30;
  legsAngles[PITCH] = 0;
  legsAngles[ROLL] = 0;
#endif

  AnglesToAxis(legsAngles, body.axis);

  renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW |RF_MINLIGHT;

  // add the body
  VectorCopy(origin, body.origin);
  VectorCopy(body.origin, body.oldorigin);

  body.hModel = pi->bodyModel;
  body.customSkin = pi->bodySkin;
  body.shaderTime = 1.0f;

  body.renderfx = renderfx;
  VectorCopy(origin, body.lightingOrigin);
  body.lightingOrigin[0] -= 150;			// + = behind, - = in front
  body.lightingOrigin[1] += 150;			// + = left, - = right
  body.lightingOrigin[2] += 300;			// + = above, - = below

  //hypov8 simple lerp idle animation
  UI_XPPM_RunLerpFrame(pi, &pi->legs, &body.skeleton);

  if (body.skeleton.type == SK_RELATIVE)
  {
    // transform relative bones to absolute ones required for vertex skinning
    UI_XPPM_TransformSkeleton(&body.skeleton, pi->modelScale);
  }

  UI_XPPM_AddPlayerWeapon(&body, pi);
  //UI_PlayerFloatSprite(pi, origin, trap_R_RegisterShaderNoMip("sprites/balloon3"));
  trap_R_AddRefEntityToScene(&body);

  //origin[0] += 150;			// + = behind, - = in front
  //origin[1] += 150;			// + = left, - = right
  //origin[2] += 150;			// + = above, - = below
  VectorSet(origin, body.origin[0]-150, body.origin[1]+150, body.origin[2]+150);
  trap_R_AddAdditiveLightToScene(origin, 800, 1.0, 1.0, 1.0);
  VectorSet(origin, body.origin[0]-150, body.origin[1]-150, body.origin[2]-150);
  trap_R_AddAdditiveLightToScene(origin, 400, 1.0, 1.0, 1.0);

  trap_R_RenderScene(&refdef);
}


#endif

