/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// edit hypov8 21 aug 2013. working md5, try combine md3
// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"
//#include "cg_xppm2.c"

// debugging
int   debug_anim_current;
int   debug_anim_old;
float debug_anim_blend;

static refSkeleton_t legsSkeleton;
static refSkeleton_t torsoSkeleton;
static refSkeleton_t oldSkeleton;

typedef struct {
  vec3_t delta;
  quat_t rot;
}

delta_t;

delta_t deltas[ WP_NUM_WEAPONS ][ MAX_BONES ];

char           *cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
  "*death1.ogg",
  "*death2.ogg",
  "*death3.ogg",
  "*jump1.ogg",
  "*jump2.ogg",
  "*jump3.ogg",
  "*pain25_1.ogg",
  "*pain50_1.ogg",
  "*pain75_1.ogg",
  "*pain100_1.ogg",
  "*falling1.ogg",
  "*gasp.ogg",
  "*drown.ogg",
  "*fall1.ogg",
  "*fall2.ogg",
  "*taunt1.ogg",
  "*taunt2.ogg",
  "*taunt3.ogg",
  "*taunt4.ogg",
  "*taunt5.ogg",
  "*taunt6.ogg",
  "*taunt7.ogg",
  "*taunt8.ogg",
  "*taunt9.ogg",
  "*taunt10.ogg",
  "*taunt11.ogg",
  "*taunt12.ogg"
};
//hypov8 add 12 custom taunts. to match player 'model' sounds, thug, bitch etc
//it removed the numCustomTaunts1 numCustomTaunts2
//FIXME(0xA5EA): this is not correct

/*
================
CG_CustomSound

================
*/
sfxHandle_t CG_CustomSound(int clientNum, const char *soundName)
{
  clientInfo_t *ci;
  int i;

  if (soundName[0] != '*')
  {
    return trap_S_RegisterSound(soundName, qfalse);
  }

  if (clientNum < 0 || clientNum >= MAX_CLIENTS)
  {
    clientNum = 0;
  }

  ci = &cgs.clientinfo[clientNum];

  for (i = 0; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i]; i++)
  {
    if (!qstrcmp(soundName, cg_customSoundNames[i]))
      return ci->sounds[i];
  }

  CG_Error("Unknown custom sound: %s", soundName);
  return 0;
}

/*
=============================================================================

CLIENT INFO

=============================================================================
*/

/*
======================
CG_ParseCharacterFile

Read a configuration file containing body.md5mesh custom
models/players/visor/character.cfg, etc
note hypov8 load MD5
======================
*/
static qboolean CG_ParseCharacterFile(const char *filename, clientInfo_t * ci)
{
  char           *text_p, *prev;
  size_t          len;
  int             i;
  char           *token;
  char            text[20000];
  fileHandle_t    f;

  // load the file
  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if(len <= 0)
  {
    return qfalse;
  }
  if(len >= sizeof(text) - 1)
  {
    CG_Printf("File %s too long\n", filename);
    trap_FS_FCloseFile(f);
    return qfalse;
  }
  trap_FS_Read(text, len, f);
  text[len] = 0;
  trap_FS_FCloseFile(f);

  // parse the text
  text_p = text;

  ci->footsteps = FOOTSTEP_NORMAL;
  VectorClear(ci->headOffset);
  ci->gender = GENDER_MALE;
  ci->fixedlegs = qfalse;
  ci->fixedtorso = qfalse;
  ci->modelScale = 1;
  //ci->modelScale[1] = 1;
  //ci->modelScale[2] = 1;

  ci->numLegBones = 0;
  ci->modelScale = 1.0f;
  //ci->leftShoulderBone = 0;
  //ci->rightShoulderBone = 0;

  ci->firstTorsoControlBone = 0; //add hypov8
  ci->lastTorsoControlBone = 0; //add hypov8
  ci->neckControlBone = 0; //add hypov8
  ci->torsoControlBone = 0;

  // read optional parameters
  while(1)
  {
    prev = text_p;			// so we can unget
    (void)prev;         //shutup compiler
    token = Com_Parse(&text_p);
    if(!token[0])
    {
      break;
    }

    if(!Q_stricmp(token, "footsteps"))
    {
      token = Com_Parse(&text_p);
      if(!token)
      {
        break;
      }
      if(!Q_stricmp(token, "default") || !Q_stricmp(token, "normal") || !Q_stricmp(token, "marble"))
      {
        ci->footsteps = FOOTSTEP_NORMAL;
      }
      else if(!Q_stricmp(token, "boot"))
      {
        ci->footsteps = FOOTSTEP_BOOT;
      }
      else if(!Q_stricmp(token, "flesh"))
      {
        ci->footsteps = FOOTSTEP_FLESH;
      }
      else if(!Q_stricmp(token, "mech"))
      {
        ci->footsteps = FOOTSTEP_MECH;
      }
      else if(!Q_stricmp(token, "energy"))
      {
        ci->footsteps = FOOTSTEP_ENERGY;
      }
      else
      {
        CG_Printf("Bad footsteps parm in %s: %s\n", filename, token);
      }
      continue;
    }
    else if(!Q_stricmp(token, "headoffset"))
    {
      for(i = 0; i < 3; i++)
      {
        token = Com_Parse(&text_p);
        if(!token)
        {
          break;
        }
        ci->headOffset[i] = atof(token);
      }
      continue;
    }
    else if(!Q_stricmp(token, "sex"))
    {
      token = Com_Parse(&text_p);
      if(!token)
      {
        break;
      }
      if(token[0] == 'f' || token[0] == 'F')
      {
        ci->gender = GENDER_FEMALE;
      }
      else if(token[0] == 'n' || token[0] == 'N')
      {
        ci->gender = GENDER_NEUTER;
      }
      else
      {
        ci->gender = GENDER_MALE;
      }
      continue;
    }
    else if(!Q_stricmp(token, "fixedlegs"))
    {
      ci->fixedlegs = qtrue;
      continue;
    }
    else if(!Q_stricmp(token, "fixedtorso"))
    {
      ci->fixedtorso = qtrue;
      continue;
    }
    else if(!Q_stricmp(token, "firstTorsoBoneName"))
    {
      token = Com_Parse(&text_p);
      ci->firstTorsoControlBone = trap_R_BoneIndex( ci->bodyModel, token );
      continue;
    }
    else if(!Q_stricmp(token, "lastTorsoBoneName"))
    {
      token = Com_Parse(&text_p);
      ci->lastTorsoControlBone = trap_R_BoneIndex( ci->bodyModel, token );
      continue;
    }
    else if(!Q_stricmp(token, "torsoControlBoneName"))
    {
      token = Com_Parse(&text_p);
      ci->torsoControlBone = trap_R_BoneIndex( ci->bodyModel, token );
      continue;
    }

    else if(!Q_stricmp(token, "neckControlBoneName"))
    {
      token = Com_Parse(&text_p);
      Q_strncpyz(ci->neckControlBoneName, token, sizeof(ci->neckControlBoneName));
      //strcpy(, ); //hypov8 keep name 4 lerp
      ci->neckControlBone = trap_R_BoneIndex( ci->bodyModel, token );
      continue;
    }
    else if(!Q_stricmp(token, "modelScale"))
    {
      token = COM_ParseExt( &text_p, qfalse );

      if ( token )
      {
        ci->modelScale = atof( token );
      }
      continue;
    }

    Com_Printf("unknown token '%s' is %s\n", token, filename);
  }

  return qtrue;
}


static qboolean CG_RegisterPlayerAnimation( clientInfo_t *ci, const char *modelName, int anim, const char *animName,
                                            qboolean loop, qboolean reversed, qboolean clearOrigin, qboolean iqm )
{
  char filename[MAX_QPATH]; // , newModelName[MAX_QPATH];
  int  frameRate;


  //Q_strncpyz( newModelName, modelName, sizeof( newModelName ) );

  if ( iqm )
  {
    Com_sprintf( filename, sizeof( filename ), "models/players/%s/%s.iqm:%s", //hypov8 todo: file name??
      modelName, modelName, animName);
    ci->animations[ anim ].handle = trap_R_RegisterAnimation( filename );
  }
  else
  {
    Com_sprintf(filename, sizeof(filename), "models/players/%s/ani/%s.md5anim", modelName, animName);
    ci->animations[ anim ].handle = trap_R_RegisterAnimation( filename );
  }

  if ( !ci->animations[ anim ].handle )
  {
    Com_Printf(S_COLOR_YELLOW "Failed to load animation file %s\n", filename );
    return qfalse;
  }

  ci->animations[ anim ].firstFrame = 0;
  ci->animations[ anim ].numFrames = trap_R_AnimNumFrames( ci->animations[ anim ].handle );
  frameRate = trap_R_AnimFrameRate( ci->animations[ anim ].handle );

  if ( frameRate == 0 )
  {
    frameRate = 1;
  }

  //hypov8 todo: use bg_timmers for wep drop/raise
  ci->animations[ anim ].frameLerp = 1000 / frameRate;
  ci->animations[ anim ].initialLerp = 1000 / frameRate;

  if ( loop )
  {
    ci->animations[ anim ].loopFrames = ci->animations[ anim ].numFrames;
  }
  else
  {
    ci->animations[ anim ].loopFrames = 0;
  }

  ci->animations[ anim ].reversed = reversed;
  ci->animations[ anim ].clearOrigin = clearOrigin;

  return qtrue;
}
#if 0
static qboolean CG_DeriveAnimationDelta( const char *modelName, weapon_t weapon, clientInfo_t *ci, qboolean iqm )
{
  int handle, i;
  static refSkeleton_t base, delta;

  if ( iqm )
  {
    handle = trap_R_RegisterAnimation( va( "models/players/%s/%s.iqm:%s_delta", modelName, modelName, "FIXME") ); // /*BG_Weapon( weapon )->name )*/
  }
  else
  {
    handle = trap_R_RegisterAnimation( va( "models/players/%s/%s_delta.md5anim", modelName, "FIXME" /*BG_Weapon( weapon )->name )*/) );
  }

  if ( !handle )
  {
    return qfalse;
  }

  ci->weaponAdjusted |= 1 << weapon;

  trap_R_BuildSkeleton( &delta, handle, 1, 1, 0, qfalse );
  trap_R_BuildSkeleton( &base, ci->animations[ TORSO_STAND ].handle, 1, 1, 0, qfalse );

  for ( i = 0; i < ci->numHandBones; i++ )
  {
    VectorSubtract( delta.bones[ ci->handBones[ i ] ].t.trans, base.bones[ ci->handBones[ i ] ].t.trans, deltas[ weapon ][ ci->handBones[ i ] ].delta );

    QuatInverse( base.bones[ ci->handBones[ i ] ].t.rot );

    QuatMultiply1( base.bones[ ci->handBones[ i ] ].t.rot, delta.bones[ ci->handBones[ i ] ].t.rot, deltas[ weapon ][ ci->handBones[ i ] ].rot );
  }

  return qtrue;
}
#endif

//note hypov8 start unnessary skin.. for xppm
// probly usefull to asigne a team skin????????
/*
==========================
CG_RegisterClientSkin
==========================
*/
#if 0 // not needed???
static qboolean CG_RegisterClientSkin(clientInfo_t *ci, const char *teamName, const char *modelName, const char *skinName)
{
  char filename[MAX_QPATH];
 /*
  if ( ci->md5 )
   {*/
    if (CG_FindClientModelFile(filename, sizeof(filename), ci, teamName, modelName, skinName, "body", "skin"))
      ci->bodySkin = trap_R_RegisterSkin(filename);
    if (!ci->bodySkin)
      Com_Printf("MD5 Body skin load failure: %s\n", filename);

  /*if (!ci->bodySkin)
    return qfalse;
  }
   else
   {
  if (CG_FindClientModelFile(filename, sizeof(filename), ci, teamName, modelName, skinName, "lower", "skin"))
    ci->legsSkin = trap_R_RegisterSkin(filename);
  if (!ci->legsSkin)
    Com_Printf("Leg skin load failure: %s\n", filename);

  if (CG_FindClientModelFile(filename, sizeof(filename), ci, teamName, modelName, skinName, "upper", "skin"))
    ci->torsoSkin = trap_R_RegisterSkin(filename);
  if (!ci->torsoSkin)
    Com_Printf("Torso skin load failure: %s\n", filename);

  if (CG_FindClientHeadFile(filename, sizeof(filename), ci, teamName, headModelName, headSkinName, "head", "skin"))
    ci->headSkin = trap_R_RegisterSkin(filename);
  if (!ci->headSkin)
    Com_Printf("Head skin load failure: %s\n", filename);
*/

  // if any skins failed to load
 // if (!ci->legsSkin || !ci->torsoSkin || !ci->headSkin || !ci->bodySkin)
  //if (!ci->bodySkin)
   // return qfalse;
   /*
}*/
  return qtrue;
}
#endif
//end hypov8 skin

/*
==========================
CG_FileExists
==========================
*/
static qboolean CG_FileExists(const char *filename)
{
  int             len;

  len = trap_FS_FOpenFile(filename, 0, FS_READ);
  if(len > 0)
  {
    return qtrue;
  }
  return qfalse;
}


/*
==========================
CG_FindClientModelFile
dragons thug default
==========================
*/
qboolean CG_FindClientModelFile(char *filename, int length, clientInfo_t * ci, const char *teamName, const char *modelName,
  const char *skinName, const char *headModelName, const char *base, const char *ext)
{
  char           *team;

  if (cgs.gametype >= GT_TEAM)
  {
    switch (ci->team)
    {
    case TEAM_NIKKIS:
    {
      team = TEAM_NAME_NIKKIS;
      break;
    }
    default:
    {
      team = TEAM_NAME_DRAGONS;
      break;
    }
    }
  }
  else
  {
    team = "default";
  }



  if (cgs.gametype >= GT_TEAM)
  {

    if (!Q_stricmp(base, "icon"))
    {//                               "models/players/thug/team_icon_area.tga"
      Com_sprintf(filename, length, "models/players/%s/team_icon_%s.%s", modelName, /*headModelName*/ skinName, ext);
    }
    else
    {//                               "models/players/thug/team_area_nikkis.skin"
      Com_sprintf(filename, length, "models/players/%s/team_%s_%s.%s", modelName, headModelName, team, ext);
    }
  }
  else
  {
    if (!Q_stricmp(base, "icon"))
    {//                            "models/players/thug/body_leory.tga"
      Com_sprintf(filename, length, "models/players/%s/%s_%s.%s", modelName, base, skinName, ext);
    }
    else
    { //                            "models/players/thug/body_leory.skin"
      Com_sprintf(filename, length, "models/players/%s/%s_%s.%s", modelName, base, skinName, ext);
    }
  }

  if (CG_FileExists(filename))
  {
    return qtrue;
  }

  return qfalse;
}

/*
==========================
CG_RegisterClientModel          md5
note CG_RegisterClientModelname md3
note hypov8
==========================
*/                                                           //thug                   default               area                       dragon
qboolean CG_RegisterClientModel(clientInfo_t * ci, const char *modelName, const char *skinName, const char *headModelName, const char *teamName)
{
  int             i;
  char            filename[MAX_QPATH * 2];
  //char        newTeamName[MAX_QPATH];

/* Com_sprintf( filename, sizeof( filename ), "models/players/%s/%s.iqm", modelName, modelName );
  if ( CG_FileExists( filename ) )
  {
    ci->bodyModel = trap_R_RegisterModel( filename );
    ci->iqm = qtrue;
  }
  */
  Com_sprintf(filename, sizeof(filename), "models/players/%s/body.md5mesh", modelName);
  ci->bodyModel = trap_R_RegisterModel(filename);

  if (!ci->bodyModel)
  {
    Com_Printf(S_COLOR_YELLOW"Failed to load body mesh file  %s\n", filename);
    return qfalse;
  }

  if (ci->bodyModel)
  {
    ci->md5 = qtrue;
    // load the animations
    Com_sprintf(filename, sizeof(filename), "models/players/%s/ani/character.cfg", modelName);
    if (!CG_ParseCharacterFile(filename, ci))
    {
      Com_Printf(S_COLOR_YELLOW"Failed to load character file %s\n", filename);
      return qfalse;
    }


    if (!CG_RegisterPlayerAnimation(ci, modelName, LEGS_IDLE, "leg_idle", qtrue, qfalse, qfalse, qfalse))
    {
      Com_Printf(S_COLOR_YELLOW"Failed to load idle animation file %s\n", filename);
      return qfalse;
    }

#define doLoop qtrue
#define noLoop qfalse

#define doRevs qtrue
#define noRevs qfalse

#define doClaer qtrue
#define noClear qfalse

    // FIXME add bunny hop animations??
    //combined animations
    CG_RegisterPlayerAnimation(ci, modelName, BOTH_DEATH1,   "death1",    noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, BOTH_DEATH2,   "death2",    noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, BOTH_DEATH3,   "death3",    noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, BOTH_DEAD1,    "dead1",     noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, BOTH_DEAD2,    "dead2",     noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, BOTH_DEAD3,    "dead3",     noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, BOTH_LADDER,   "ladder",    noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, BOTH_LDR_UP,   "ladder_up", doLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, BOTH_LDR_DN,   "ladder_up", doLoop, doRevs, qfalse, qfalse);
    //torso
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_GESTURE, "tor_gesture",  noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_GESTURE2,"tor_gesture2", noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_GESTURE3,"tor_gesture3", noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_ATTACK,  "tor_attack",   noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_ATTACK2, "tor_attack2",  noLoop, noRevs, qfalse, qfalse); //crowbar
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_ATTACK3, "tor_attack3",  noLoop, noRevs, qfalse, qfalse); //pistol
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_DROP,    "tor_wep_drop", noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_RAISE,   "tor_wep_raise",noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_STAND,   "tor_idle",     doLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_STAND2,  "tor_idle2",    doLoop, noRevs, qfalse, qfalse); //crowbar
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_STAND3,  "tor_idle3",    doLoop, noRevs, qfalse, qfalse); //pistol
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_RUN,     "tor_run",      doLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, TORSO_WALK,    "tor_walk",     doLoop, noRevs, qfalse, qfalse);
    //legs
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_WALK,     "leg_walk",     doLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_WALK_BACK,"leg_walk",     doLoop, doRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_CR_IDLE,  "leg_crch_idle",doLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_CR_WALK,  "leg_crch_walk",doLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_CR_BACK,  "leg_crch_walk",doLoop, doRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_RUN,      "leg_run",      doLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_RUN_BACK, "leg_run",      doLoop, doRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_SWIM,     "leg_swim",     doLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_JUMP,     "leg_jump",     noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_LAND,     "leg_land",     noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_JUMPB,    "leg_jump",     noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_LANDB,    "leg_land",     noLoop, noRevs, qfalse, qfalse);
    CG_RegisterPlayerAnimation(ci, modelName, LEGS_TURN,     "leg_step",     noLoop, noRevs, qfalse, qfalse);

    // hypov8 LEGS_IDLE for any failed handle
    for (i = 0; i < MAX_TOTALANIMATIONS; i++)
    {
      if (i == LEGS_IDLE)
        continue;

      //failed
      if (!ci->animations[i].handle)
        ci->animations[i] = ci->animations[LEGS_IDLE];
    }


    if (CG_FindClientModelFile(filename, sizeof(filename), ci, teamName, modelName, skinName, headModelName, "body", "skin"))
    {
      Com_Printf("loading skin %s\n", filename);

      ci->bodySkin = trap_R_RegisterSkin(filename);
    }

    if (!ci->bodySkin)
    {
      Com_Printf("Body skin load failure: %s\n", filename);
      return qfalse;
    }
  }

  if (CG_FindClientModelFile(filename, sizeof(filename), ci, teamName, modelName, skinName, headModelName, "icon", "tga"))
  {
    ci->modelIcon = trap_R_RegisterShaderNoMip(filename);
  }
  else if (CG_FindClientModelFile(filename, sizeof(filename), ci, teamName, modelName, skinName, headModelName, "icon", "png"))
  {
    ci->modelIcon = trap_R_RegisterShaderNoMip(filename);
  }

  if (!ci->modelIcon)
  {
    Com_Printf(S_COLOR_YELLOW "Failed to load icon file %s\n", filename);
    return qfalse;
  }

  return qtrue;
}

/*
====================
CG_ColorFromString
note hypov8 old ref, works xppm
====================
*/
static void CG_ColorFromString(const char *v, vec3_t color)
{
  int val;

  VectorClear(color);

  val = atoi(v);

  if (val < 1 || val > 7)
  {
    VectorSet(color, 1, 1, 1);
    return;
  }

  if (val & 1)
    color[2] = 1.0f;

  if (val & 2)
    color[1] = 1.0f;

  if (val & 4)
    color[0] = 1.0f;
}

/*
===================
CG_LoadClientInfo

Load it now, taking the disk hits.
This will usually be deferred to a safe time
note hypov8 xppm version
===================
*/
static void CG_LoadClientInfo(clientInfo_t * ci)
{
  const char     *dir, *fallback;
  int             i, modelloaded;
  const char     *s;
  int             clientNum;
  char            teamname[MAX_QPATH];

  teamname[0] = 0;
  if (cgs.gametype >= GT_TEAM)
  {
    if (ci->team == TEAM_NIKKIS)
      Q_strncpyz(teamname, cg_NikkiTeamName.string, sizeof(teamname));
    else
      Q_strncpyz(teamname, cg_DragonTeamName.string, sizeof(teamname));
  }
  if (teamname[0])
    strcat(teamname, "/");

  //add hypov8 if spectating or first load. its reset to 0
  if (ci->modelName[0] == '\0')
    strcat(ci->modelName, DEFAULT_TEAM_MODEL);
  //ci->modelName[0] = *DEFAULT_TEAM_MODEL;


  modelloaded = qtrue;
  if (!CG_RegisterClientModel(ci, ci->modelName, ci->skinName, ci->headModelName, teamname))
  {
    if (cg_buildScript.integer)
    {
      CG_Error("CG_RegisterClientModel( %s, %s, %s ) failed", ci->modelName, ci->skinName, teamname);
    }

    // fall back to default team name
    if (cgs.gametype >= GT_TEAM)
    {
      if (ci->team == TEAM_NIKKIS)
        Q_strncpyz(teamname, TEAM_NAME_NIKKIS, sizeof(teamname));
      else
        Q_strncpyz(teamname, TEAM_NAME_DRAGONS, sizeof(teamname));

      if (!CG_RegisterClientModel(ci, DEFAULT_TEAM_MODEL, DEFAULT_TEAM_MODEL, DEFAULT_TEAM_MODEL, teamname))
        CG_Error("DEFAULT_TEAM_MODEL / skin (%s/%s/%s) failed to register", DEFAULT_TEAM_MODEL, DEFAULT_TEAM_MODEL, teamname); //hypov8 ci->skinName
    }
    else
    {
      if (!CG_RegisterClientModel(ci, DEFAULT_MODEL, "default", DEFAULT_MODEL, teamname))
        CG_Error("DEFAULT_MODEL (%s) failed to register", DEFAULT_MODEL);
    }
    modelloaded = qfalse;
  }

  ci->newAnims = qfalse;

  // sounds
  dir = ci->modelName;
  fallback = DEFAULT_MODEL; //<--xppm //md3 -->fallback = (cgs.gametype >= GT_TEAM) ? DEFAULT_TEAM_MODEL : DEFAULT_MODEL;
  //hypov8 start duplicate XPPM
  for (i = 0; i < MAX_CUSTOM_SOUNDS; i++)
  {
    s = cg_customSoundNames[i];

    if (!s)
      break;

    ci->sounds[i] = 0;
    // if the model didn't load use the sounds of the default model
    if (modelloaded)
      ci->sounds[i] = trap_S_RegisterSound(va("sound/player/%s/%s", dir, s + 1), qfalse);

    if (!ci->sounds[i])
      ci->sounds[i] = trap_S_RegisterSound(va("sound/player/%s/%s", fallback, s + 1), qfalse);
  }

  ci->deferred = qfalse;

  // reset any existing players and bodies, because they might be in bad
  // frames for this new model
  clientNum = ci - cgs.clientinfo;
  for (i = 0; i < MAX_GENTITIES; i++)
  {
    if (cg_entities[i].currentState.clientNum == clientNum && cg_entities[i].currentState.eType == ET_PLAYER)
      CG_ResetPlayerEntity(&cg_entities[i]);
  } //hypov8 end duplicate XPPM
}

/*
======================
CG_CopyClientInfoModel
note hypov8 xppm version
======================
*/
static void CG_CopyClientInfoModel(clientInfo_t * from, clientInfo_t * to)
{
  VectorCopy( from->headOffset, to->headOffset );
  to->modelScale = from->modelScale;
  to->footsteps = from->footsteps;
  to->gender = from->gender;

  to->numLegBones = from->numLegBones;
  to->numHandBones = from->numHandBones;
  to->torsoControlBone = from->torsoControlBone;
  to->weaponAdjusted = from->weaponAdjusted;
  //to->rightShoulderBone = from->rightShoulderBone;
  //to->leftShoulderBone = from->leftShoulderBone;


  to->firstTorsoControlBone  = from->firstTorsoControlBone; //add hypov8
   to->lastTorsoControlBone = from->lastTorsoControlBone; //add hypov8
   to->neckControlBone = from->neckControlBone; //add hypov8
  Q_strncpyz(to->neckControlBoneName, from->neckControlBoneName, sizeof(to->neckControlBoneName));

  to->legsModel  = from->legsModel;
  to->legsSkin   = from->legsSkin;
  to->torsoModel = from->torsoModel;
  to->torsoSkin  = from->torsoSkin;
  to->headModel  = from->headModel;
  to->headSkin   = from->headSkin;
  to->nonSegModel = from->nonSegModel;
  to->nonSegSkin = from->nonSegSkin;
  to->nonsegmented = from->nonsegmented;
  to->modelIcon = from->modelIcon;
  to->bodyModel = from->bodyModel;
  to->bodySkin = from->bodySkin;
  to->md5 = from->md5;

  memcpy( to->animations, from->animations, sizeof( to->animations ) );
  memcpy( to->sounds, from->sounds, sizeof( to->sounds ) );
  memcpy( to->customFootsteps, from->customFootsteps, sizeof( to->customFootsteps ) );
  memcpy( to->customMetalFootsteps, from->customMetalFootsteps, sizeof( to->customMetalFootsteps ) );
  memcpy( to->legBones, from->legBones, sizeof( to->legBones ) );
  memcpy( to->handBones, from->handBones, sizeof( to->legBones ) );
}

/*
======================
CG_GetCorpseNum
======================
*/
#if 0
static int CG_GetCorpseNum( class_t class_ )
{
  int          i;
  clientInfo_t *match;
  char         *modelName;
  char         *skinName;

  modelName = BG_ClassModelConfig( class_ )->modelName;
  skinName = BG_ClassModelConfig( class_ )->skinName;

  for ( i = PCL_NONE + 1; i < PCL_NUM_CLASSES; i++ )
  {
    match = &cgs.corpseinfo[ i ];

    if ( !match->infoValid )
    {
      continue;
    }

    if ( !Q_stricmp( modelName, match->modelName ) &&
         !Q_stricmp( skinName, match->skinName ) )
    {
      // this clientinfo is identical, so use its handles
      return i;
    }
  }

  //something has gone horribly wrong
  return -1;
}
#endif
//FIXME(0xA5EA): merge

/*
======================
CG_ScanForExistingClientInfo
note hypov8 old ref, works xppm
======================
*/
static qboolean CG_ScanForExistingClientInfo(clientInfo_t * ci)
{
  int             i;
  clientInfo_t   *match;

  for(i = 0; i < cgs.maxclients; i++)
  {
    match = &cgs.clientinfo[i];
    if(!match->infoValid)
    {
      continue;
    }
    if(match->deferred)
    {
      continue;
    }
    if(!Q_stricmp(ci->modelName, match->modelName)
       && !Q_stricmp(ci->skinName, match->skinName)
     && !Q_stricmp(ci->headModelName, match->headModelName) //note hypov8
         && !Q_stricmp(ci->headSkinName, match->headSkinName)   //note hypov8
       && !Q_stricmp(ci->nikkiTeam, match->nikkiTeam)
       && !Q_stricmp(ci->dragonTeam, match->dragonTeam)
       && (cgs.gametype < GT_TEAM || ci->team == match->team))
    {
      // this clientinfo is identical, so use it's handles

      ci->deferred = qfalse;

      CG_CopyClientInfoModel(match, ci);

      return qtrue;
    }
  }

  // nothing matches, so defer the load
  return qfalse;
}

/*
======================
CG_SetDeferredClientInfo

We aren't going to load it now, so grab some other
client's info to use until we have some spare time.
note hypov8 old ref, works xppm
======================
*/
static void CG_SetDeferredClientInfo(clientInfo_t * ci)
{
  int             i;
  clientInfo_t   *match;

  // if someone else is already the same models and skins we
  // can just load the client info
  for(i = 0; i < cgs.maxclients; i++)
  {
    match = &cgs.clientinfo[i];
    if(!match->infoValid || match->deferred)
    {
      continue;
    }
    if(Q_stricmp(ci->skinName, match->skinName) || Q_stricmp(ci->modelName, match->modelName) ||
//           Q_stricmp( ci->headModelName, match->headModelName ) ||
//           Q_stricmp( ci->headSkinName, match->headSkinName ) ||
       (cgs.gametype >= GT_TEAM && ci->team != match->team))
    {
      continue;
    }
    // just load the real info cause it uses the same models and skins
    CG_LoadClientInfo(ci);
    return;
  }

  // if we are in teamplay, only grab a model if the skin is correct
  if(cgs.gametype >= GT_TEAM)
  {
    for(i = 0; i < cgs.maxclients; i++)
    {
      match = &cgs.clientinfo[i];
      if(!match->infoValid || match->deferred)
      {
        continue;
      }
      if(Q_stricmp(ci->skinName, match->skinName) || (cgs.gametype >= GT_TEAM && ci->team != match->team))
      {
        continue;
      }
      ci->deferred = qtrue;
      CG_CopyClientInfoModel(match, ci);
      return;
    }
    // load the full model, because we don't ever want to show
    // an improper team skin.  This will cause a hitch for the first
    // player, when the second enters.  Combat shouldn't be going on
    // yet, so it shouldn't matter
    CG_LoadClientInfo(ci);
    return;
  }

  // find the first valid clientinfo and grab its stuff
  for(i = 0; i < cgs.maxclients; i++)
  {
    match = &cgs.clientinfo[i];
    if(!match->infoValid)
    {
      continue;
    }

    ci->deferred = qtrue;
    CG_CopyClientInfoModel(match, ci);
    return;
  }

  // we should never get here...
  CG_Printf("CG_SetDeferredClientInfo: no valid clients!\n");

  CG_LoadClientInfo(ci);
}


/*
======================
CG_NewClientInfo
note hypov8 old ref, works xppm
======================
*/
void CG_NewClientInfo(int clientNum)
{
  clientInfo_t   *ci;
  clientInfo_t    newInfo;
  const char     *configstring;
  const char     *v;
  char           *slash;

  ci = &cgs.clientinfo[clientNum];

  configstring = CG_ConfigString(clientNum + CS_PLAYERS);
  if(!configstring[0])
  {
    memset(ci, 0, sizeof(*ci));
    return;					// player just left
  }

  // build into a temp buffer so the defer checks can use
  // the old value
  memset(&newInfo, 0, sizeof(newInfo));

  if(cg.progress != 0)
    CG_LoadingString("info", qfalse);

  // isolate the player's name
  v = Info_ValueForKey(configstring, "n");
  Q_strncpyz(newInfo.name, v, sizeof(newInfo.name));

  // colors
  v = Info_ValueForKey(configstring, "c1");
  CG_ColorFromString(v, newInfo.color1);

  v = Info_ValueForKey(configstring, "c2");
  CG_ColorFromString(v, newInfo.color2);

  // bot skill
  v = Info_ValueForKey(configstring, "skill");
  newInfo.botSkill = atoi(v);

  // handicap
  v = Info_ValueForKey(configstring, "hc");
  newInfo.handicap = atoi(v);

  // wins
  v = Info_ValueForKey(configstring, "w");
  newInfo.wins = atoi(v);

  // losses
  v = Info_ValueForKey(configstring, "l");
  newInfo.losses = atoi(v);

  // team
  v = Info_ValueForKey(configstring, "t");
  newInfo.team = (team_t)atoi(v);

  // team task
  v = Info_ValueForKey(configstring, "tt");
  newInfo.teamTask = atoi(v);

  // team leader
  v = Info_ValueForKey(configstring, "tl");
  newInfo.teamLeader = atoi(v);

  v = Info_ValueForKey(configstring, "g_dragonTeam");
  Q_strncpyz(newInfo.dragonTeam, v, MAX_TEAMNAME);

  v = Info_ValueForKey(configstring, "g_nikkiTeam");
  Q_strncpyz(newInfo.nikkiTeam, v, MAX_TEAMNAME);

  if(cg.progress != 0)
    CG_LoadingString("model", qfalse);

  // model
  v = Info_ValueForKey(configstring, "model");
  if(cg_forceModel.integer)
  {
    // forcemodel makes everyone use a single model
    // to prevent load hitches
    char            modelStr[MAX_QPATH];
    char           *skin = "";

    if(cgs.gametype >= GT_TEAM)
    {
      Q_strncpyz(newInfo.modelName, DEFAULT_TEAM_MODEL, sizeof(newInfo.modelName));
      Q_strncpyz(newInfo.skinName, "default", sizeof(newInfo.skinName)); //TEAM_NAME_NIKKIS//
    }
    else
    {							//	thug/red	thug/blue
      trap_Cvar_VariableStringBuffer("model", modelStr, sizeof(modelStr));

      skin = strchr(modelStr, '/');
      if(!skin)
        skin = "default";
      else
        *skin++ = 0;

      Q_strncpyz(newInfo.skinName, skin, sizeof(newInfo.skinName)); //DEFAULT_MODEL
      Q_strncpyz(newInfo.modelName, modelStr, sizeof(newInfo.modelName));

    }

    if (cgs.gametype >= GT_TEAM)
    {
      // keep skin name
      slash = (char*)strchr(v, '/');
      if (slash)
      Q_strncpyz(newInfo.skinName, slash + 1, sizeof(newInfo.skinName));
    }
  }
  else	//hypov8 end forced body models
  {
    Q_strncpyz(newInfo.modelName, v, sizeof(newInfo.modelName));
    slash = strchr(newInfo.modelName, '/');
    if (!slash)
    {
      // modelName didn not include a skin name
      Q_strncpyz(newInfo.skinName, "default", sizeof(newInfo.skinName));
    }
    else
    {
      Q_strncpyz(newInfo.skinName, slash + 1, sizeof(newInfo.skinName));
      // truncate modelName
      *slash = 0;
    }
  }

  // head model
  v = Info_ValueForKey(configstring, "hmodel");
    if (cg_forceModel.integer)
  {
    // forcemodel makes everyone use a single model
    // to prevent load hitches
    char modelStr[MAX_QPATH];
    char *skin;

    if (cgs.gametype >= GT_TEAM)
    {
      Q_strncpyz(newInfo.headModelName, DEFAULT_TEAM_HEAD, sizeof(newInfo.headModelName));
      Q_strncpyz(newInfo.headSkinName, "default", sizeof(newInfo.headSkinName));
    }
    else
    {
      trap_Cvar_VariableStringBuffer("headmodel", modelStr, sizeof(modelStr));
      skin = strchr(modelStr, '/');
      if (!skin)
        skin = "default";
      else
        *skin++ = 0;

      Q_strncpyz(newInfo.headSkinName, skin, sizeof(newInfo.headSkinName));
      Q_strncpyz(newInfo.headModelName, modelStr, sizeof(newInfo.headModelName));
    }

    if ( cgs.gametype >= GT_TEAM )
    {
      // keep skin name
      slash = (char*)strchr( v, '/' );
      if ( slash )
      {
        Q_strncpyz( newInfo.headSkinName, slash + 1, sizeof( newInfo.headSkinName ) );
      }
    }
  }
  else //end fixed head model
  {
    Q_strncpyz(newInfo.headModelName, v, sizeof(newInfo.headModelName));

    slash = strchr(newInfo.headModelName, '/');
    if (!slash)
    {
      // modelName didn not include a skin name
      Q_strncpyz(newInfo.headSkinName, "default", sizeof(newInfo.headSkinName));
    }
    else
    {
      Q_strncpyz(newInfo.headSkinName, slash + 1, sizeof(newInfo.headSkinName));
      // truncate modelName
      *slash = 0;
    }
  }

  // scan for an existing clientinfo that matches this modelname
  // so we can avoid loading checks if possible
  if (!CG_ScanForExistingClientInfo(&newInfo))
  {
    qboolean forceDefer;

    forceDefer = trap_MemoryRemaining() < 4000000;

    // if we are defering loads, just have it pick the first valid
    if (forceDefer || (cg_deferPlayers.integer && !cg_buildScript.integer && !cg.loading))
    {
      // keep whatever they had if it won't violate team skins
      CG_SetDeferredClientInfo(&newInfo);
      // if we are low on memory, leave them with this model
      if (forceDefer)
      {
        CG_Printf("Memory is low.  Using deferred model.\n");
        newInfo.deferred = qfalse;
      }
    }
    else
  {
      CG_LoadClientInfo(&newInfo);
    }
  }

  // replace whatever was there with the new one
  newInfo.infoValid = qtrue;
  *ci = newInfo;
}



/*
======================
CG_LoadDeferredPlayers

Called each frame when a player is dead
and the scoreboard is up
so deferred players can be loaded
======================
*/
void CG_LoadDeferredPlayers(void)
{
  int             i;
  clientInfo_t   *ci;

  // scan for a deferred player to load
  for(i = 0, ci = cgs.clientinfo; i < cgs.maxclients; i++, ci++)
  {
    if(ci->infoValid && ci->deferred)
    {
      // if we are low on memory, leave it deferred
      if(trap_MemoryRemaining() < 4000000)
      {
        CG_Printf("Memory is low.  Using deferred model.\n");
        ci->deferred = qfalse;
        continue;
      }
      CG_LoadClientInfo(ci);
//          break;
    }
  }
}

/*
=============================================================================

PLAYER ANIMATION

=============================================================================
*/

static void CG_CombineLegSkeleton( refSkeleton_t *dest, refSkeleton_t *legs, int *legBones, int numBones )
{
  int i;

  for ( i = 0; i < numBones; i++ )
  {
    dest->bones[ legBones[ i ] ] = legs->bones[ legBones[ i ] ];
  }
}



/*
===============
CG_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetLerpFrameAnimation( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, refSkeleton_t *skel )
{
  animation_t *anim;

  // save old animation
  lf->old_animationNumber = lf->animationNumber;
  lf->old_animation = lf->animation;

  lf->animationNumber = newAnimation;
  newAnimation &= ~ANIM_TOGGLEBIT;

  if (newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS)
  {
    CG_Error("Bad animation number: %i", newAnimation);
  }

  anim = &ci->animations[newAnimation];

  lf->animation = anim;
  if(cg_debugPlayerAnim.integer)
  {
    CG_Printf("player anim: %i", newAnimation); //animation (name) and \n added later
  }

  // if (ci->md5) //hypov8 always md5
  {
    debug_anim_current = lf->animationNumber;
    debug_anim_old = lf->old_animationNumber;

    if (lf->old_animationNumber <= 0)
    {
      // skip initial / invalid blending
      lf->blendlerp = 0.0f;
      return;
    }

    if (lf->old_animationNumber <= 0)
    {
      // skip initial / invalid blending
      lf->blendlerp = 0.0f;
      return;
    }

    // TODO: blend through two blendings!

    if ((lf->blendlerp <= 0.0f))
    {
      lf->blendlerp = 1.0f;
    }
    else
    {
      lf->blendlerp = 1.0f - lf->blendlerp; // use old blending for smooth blending between two blended animations
    }

    oldSkeleton = *skel;


    if (lf->old_animation->handle /*&& oldSkeleton.numBones == skel->numBones*/) //daemon .5 no check for match
    {
      if (!trap_R_BuildSkeleton(&oldSkeleton, lf->old_animation->handle,
        lf->oldFrame, lf->frame, lf->blendlerp,
        lf->old_animation->clearOrigin))
      {
        CG_Printf("Can't blend skeleton\n");
        return;
      }
    }

    lf->animationTime = cg.time + anim->initialLerp;

    lf->oldFrame = lf->frame = 0;
    lf->oldFrameTime = lf->frameTime = 0;
  }  //end md5


  if (cg_debugAnim.integer)
  {
    CG_Printf("Anim: %i\n", newAnimation);
  }

}

/*
===============
CG_RunPlayerLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunPlayerLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, refSkeleton_t *skel, float speedScale )
{
  qboolean animChanged = qfalse;

    // see if the animation sequence is switching
  if ( newAnimation != lf->animationNumber || !lf->animation )
  {
    CG_SetLerpFrameAnimation( ci, lf, newAnimation, skel );
    animChanged = qtrue;
  }

  CG_RunLerpFrame( lf, speedScale );

  if ( ci->md5 )
  {
    CG_RunMD5LerpFrame( lf, speedScale, animChanged );

    // blend old and current animation
    CG_BlendLerpFrame( lf );

    if( ci->team != TEAM_SPECTATOR )
      CG_BuildAnimSkeleton( lf, skel, &oldSkeleton );
  }
}

static void CG_RunCorpseLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation )
{
  animation_t    *anim;
  qboolean        animChanged UNUSED;

  // debugging tool to get no animations
  if(cg_animSpeed.integer == 0)
  {
    lf->oldFrame = lf->frame = lf->backlerp = 0;
    return;
  }

  // see if the animation sequence is switching
  if (newAnimation != lf->animationNumber || !lf->animation)
  {
    CG_SetLerpFrameAnimation( ci, lf, newAnimation, NULL );

    if(!lf->animation)
    {
      oldSkeleton = legsSkeleton;
    }

    animChanged = qtrue;
  }
  else
  {
    animChanged = qfalse;
  }

  anim = lf->animation;

  if ( !anim || !anim->frameLerp )
  {
    return;				// shouldn't happen
  }

  // blend old and current animation
  CG_BlendLerpFrame( lf );

  if ( lf->animation )
  {
    if ( !trap_R_BuildSkeleton( &legsSkeleton, lf->animation->handle, anim->numFrames, anim->numFrames, 0, lf->animation->clearOrigin ) )
    {
      CG_Printf("Can't build lf->skeleton\n");
    }
  }
}

/*
===============
CG_ClearLerpFrame
===============
*/
static void CG_ClearLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int animationNumber, refSkeleton_t *skel )
{
  lf->frameTime = lf->oldFrameTime = cg.time;
  CG_SetLerpFrameAnimation( ci, lf, animationNumber, skel );
  lf->oldFrame = lf->frame = lf->animation->firstFrame;
}

/*
===============
CG_PlayerAnimation
===============
*/
#if 0
//hypov8 merge: md3 alien
static void CG_PlayerAnimation( centity_t *cent, int *legsOld, int *legs, float *legsBackLerp,
                                int *torsoOld, int *torso, float *torsoBackLerp )
{
  clientInfo_t *ci;
  int          clientNum;
  float        speedScale = 1.0f;

  clientNum = cent->currentState.clientNum;

  if ( cg_noPlayerAnims.integer )
  {
    *legsOld = *legs = *torsoOld = *torso = 0;
    return;
  }

  ci = &cgs.clientinfo[ clientNum ];

  // do the shuffle turn frames locally
  if ( cent->pe.legs.yawing && ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_IDLE )
  {
    CG_RunPlayerLerpFrame( ci, &cent->pe.legs, LEGS_TURN, NULL, speedScale );
  }
  else
  {
    CG_RunPlayerLerpFrame( ci, &cent->pe.legs, cent->currentState.legsAnim, NULL, speedScale );
  }

  *legsOld = cent->pe.legs.oldFrame;
  *legs = cent->pe.legs.frame;
  *legsBackLerp = cent->pe.legs.backlerp;

  CG_RunPlayerLerpFrame( ci, &cent->pe.torso, cent->currentState.torsoAnim, NULL, speedScale );

  *torsoOld = cent->pe.torso.oldFrame;
  *torso = cent->pe.torso.frame;
  *torsoBackLerp = cent->pe.torso.backlerp;
}
#endif
/*
===============
CG_PlayerNonSegAnimation
===============
*/
#if 0
//md3 alien
static void CG_PlayerNonSegAnimation( centity_t *cent, int *nonSegOld, int *nonSeg, float *nonSegBackLerp )
{
  clientInfo_t *ci;
  int          clientNum;
  float        speedScale = 1.0f;

  clientNum = cent->currentState.clientNum;

  if ( cg_noPlayerAnims.integer )
  {
    *nonSegOld = *nonSeg = 0;
    return;
  }

  ci = &cgs.clientinfo[ clientNum ];

  // do the shuffle turn frames locally
  if ( cent->pe.nonseg.yawing && ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == NSPA_STAND )
  {
    CG_RunPlayerLerpFrame( ci, &cent->pe.nonseg, NSPA_TURN, NULL, speedScale );
  }
  else
  {
    CG_RunPlayerLerpFrame( ci, &cent->pe.nonseg, cent->currentState.legsAnim, NULL, speedScale );
  }

  *nonSegOld = cent->pe.nonseg.oldFrame;
  *nonSeg = cent->pe.nonseg.frame;
  *nonSegBackLerp = cent->pe.nonseg.backlerp;
}
#endif
//FIXME(0xA5EA): merge

/*
===============
CG_PlayerMD5Animation
===============
*/

static void CG_PlayerMD5Animation( centity_t *cent )
{
  clientInfo_t   *ci;
  int             clientNum;
  float           speedScale;
  int legAnim= cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
  int bodyAnim= cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT;
  int aniChangedLeg = 0;
  int aniChangedBody = 0;

  clientNum = cent->currentState.clientNum;

  if(cg_noPlayerAnims.integer)
  {
    return;
  }

    speedScale = 1;

  ci = &cgs.clientinfo[clientNum];

  //show animation region name
  if(cg_debugPlayerAnim.integer)
  {	//legs
    if ( cent->pe.legs.yawing && legAnim == LEGS_IDLE )
    {
      if ( cent->currentState.legsAnim != LEGS_TURN || !cent->pe.legs.animation )
        aniChangedLeg = 1;
    }
    else
    {
      if ( cent->currentState.legsAnim != cent->pe.legs.animationNumber || !cent->pe.legs.animation )
        aniChangedLeg = 1;
    }

    //torso
    if ( cent->currentState.torsoAnim != cent->pe.torso.animationNumber || !cent->pe.torso.animation )
      aniChangedBody = 1;
  }


  // do the shuffle turn frames locally
  if(cent->pe.legs.yawing && legAnim == LEGS_IDLE)
  {
    CG_RunPlayerLerpFrame( ci, &cent->pe.legs, LEGS_TURN, &legsSkeleton, speedScale );
  }
  else
  {
    CG_RunPlayerLerpFrame( ci, &cent->pe.legs, cent->currentState.legsAnim, &legsSkeleton, speedScale );
    }

  if ( aniChangedLeg  )
    CG_Printf(" (legs)\n");


  //TORSO
  //sync run legs with run torso
  //hypov8 todo: thug.md5anim walk/run animation is not in sync
  if ( ( bodyAnim == TORSO_RUN && ( legAnim == LEGS_RUN || legAnim == LEGS_RUN_BACK ) ) ||
    ( bodyAnim == TORSO_WALK && ( legAnim == LEGS_WALK || legAnim == LEGS_WALK_BACK ) ) )
  {
    cent->pe.torso.frame = cent->pe.legs.frame;
    cent->pe.torso.frameTime = cent->pe.legs.frameTime;
    cent->pe.torso.oldFrame = cent->pe.legs.oldFrame;
    cent->pe.torso.oldFrameTime = cent->pe.legs.oldFrameTime;
    cent->pe.torso.backlerp = cent->pe.legs.backlerp;
    cent->pe.torso.animationTime = cent->pe.legs.animationTime;
    cent->pe.torso.blendlerp = cent->pe.legs.blendlerp;
    cent->pe.torso.blendtime = cent->pe.legs.blendtime;
  }

  CG_RunPlayerLerpFrame(ci, &cent->pe.torso, cent->currentState.torsoAnim, &torsoSkeleton, speedScale);

  if (aniChangedBody)
    CG_Printf(" (torso)\n");

  }

 /*
===============
CG_PlayerMD5AlienAnimation
===============
*/
#if 0 //hypov8 merge:
static void CG_PlayerMD5AlienAnimation( centity_t *cent )
{
  clientInfo_t  *ci;
  int           clientNum;
  float         speedScale;
  static refSkeleton_t blend;

  clientNum = cent->currentState.clientNum;

  if ( cg_noPlayerAnims.integer )
  {
    return;
}

  speedScale = 1;

  ci = &cgs.clientinfo[ clientNum ];

  // If we are attacking/taunting, and in motion, allow blending the two skeletons
  if ( ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) >= NSPA_ATTACK1 &&
         ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) <= NSPA_ATTACK3 ) ||
       ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == NSPA_GESTURE )
  {
    blend.type = SK_RELATIVE; // Tell game to blend

    if( ( cent->pe.nonseg.animationNumber & ~ANIM_TOGGLEBIT ) <= NSPA_TURN &&
      ( cent->pe.nonseg.animationNumber & ~ANIM_TOGGLEBIT ) != NSPA_GESTURE )
    {
      cent->pe.legs = cent->pe.nonseg;
    }
  }
  else
  {
    blend.type = SK_INVALID;
  }

  // do the shuffle turn frames locally
  if ( cent->pe.nonseg.yawing && ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == NSPA_STAND )
  {
    CG_RunPlayerLerpFrame( ci, &cent->pe.nonseg, NSPA_TURN, &legsSkeleton, speedScale );
  }
  else
  {
    CG_RunPlayerLerpFrame( ci, &cent->pe.nonseg, cent->currentState.legsAnim, &legsSkeleton, speedScale );
  }

  if ( blend.type == SK_RELATIVE )
  {
    CG_RunPlayerLerpFrame( ci, &cent->pe.legs, cent->pe.legs.animationNumber, &blend, speedScale );
    trap_R_BlendSkeleton( &legsSkeleton, &blend, 0.5 );
  }
}
#endif
//FIXME(0xA5EA):merges
/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

/*
==================
CG_SwingAngles
==================
*/
void CG_SwingAngles( float destination, float swingTolerance, float clampTolerance, float speed, float *angle, qboolean *swinging )
{
  float           swing;
  float           move;
  float           scale;

  if(!*swinging)
  {
    // see if a swing should be started
    swing = AngleSubtract(*angle, destination);

    if(swing > swingTolerance || swing < -swingTolerance)
    {
      *swinging = qtrue;
    }
  }

  if(!*swinging)
  {
    return;
  }

  // modify the speed depending on the delta
  // so it doesn't seem so linear
  swing = AngleSubtract(destination, *angle);
  scale = fabs(swing);

  if(scale < swingTolerance * 0.5)
  {
    scale = 0.5;
  }
  else if(scale < swingTolerance)
  {
    scale = 1.0;
  }
  else
  {
    scale = 2.0;
  }

  // swing towards the destination angle
  if(swing >= 0)
  {
    move = cg.frametime * scale * speed;

    if(move >= swing)
    {
      move = swing;
      *swinging = qfalse;
    }

    *angle = AngleMod( *angle + move );
  }
  else if(swing < 0)
  {
    move = cg.frametime * scale * -speed;

    if(move <= swing)
    {
      move = swing;
      *swinging = qfalse;
    }

    *angle = AngleMod( *angle + move );
  }

  // clamp to no more than tolerance
  swing = AngleSubtract(destination, *angle);

  if(swing > clampTolerance)
  {
    *angle = AngleMod( destination - ( clampTolerance - 1 ) );
  }
  else if(swing < -clampTolerance)
  {
    *angle = AngleMod( destination + ( clampTolerance - 1 ) );
  }
}

/*
=================
CG_AddPainTwitch
=================
*/
void CG_AddPainTwitch( centity_t *cent, vec3_t torsoAngles )
{
  int             t;
  float           f;

  t = cg.time - cent->pe.painTime;

  if(t >= PAIN_TWITCH_TIME)
  {
    return;
  }

  f = 1.0 - (float)t / PAIN_TWITCH_TIME;

  torsoAngles[PITCH] -= 20 * f;

  if(cent->pe.painDirection)
  {
    torsoAngles[YAW] += 20 * f;
  }
  else
  {
    torsoAngles[YAW] -= 20 * f;
  }
}

/*
===============
CG_PlayerAngles

Handles separate torso motion

  legs pivot based on direction of movement

  head always looks exactly at cent->lerpAngles

  if motion < 20 degrees, show in head only
  if < 45 degrees, also show in torso
===============
*/
static void CG_PlayerAngles(centity_t * cent, const vec3_t sourceAngles, vec3_t legsAngles, vec3_t torsoAngles, vec3_t headAngles)
{
  float           dest;
  static int      movementOffsets[8];
  vec3_t          velocity;
  float           speed;
  int             dir, clientNum;
  clientInfo_t   *ci;

  int	idleAnimLeg, idleAnimTor;//add hypov8
  int anim;// add hypov8

  //hypov8, body seperation angles
  movementOffsets[0] = 0;		/*forward*/
  movementOffsets[1] = 22;	/*fwd+left*/
  movementOffsets[2] = 45;	/*left*/
  movementOffsets[3] = -22;	/*back+left*/
  movementOffsets[4] = 0;		/*back*/
  movementOffsets[5] = 22;	/*back+right*/
  movementOffsets[6] = -45;	/*right*/
  movementOffsets[7] = -22;	/*forward+right*/

/*
  VectorCopy(cent->lerpAngles, headAngles);
  headAngles[YAW] = AngleMod(headAngles[YAW]);
  VectorClear( legsAngles );
  VectorClear( torsoAngles );*/


  /* allow free rotate 3rd person camers. could also use in spectate mode */
  if (cg_thirdPersonFixed.integer) /* cg_thirdPerson.integer*/
  {
    VectorCopy(sourceAngles, headAngles);
  }
  else
  {
    headAngles[0] = 0.0f;
    headAngles[1] = 0.0f;
    headAngles[2] = 0.0f;
  }

  headAngles[YAW] = AngleNormalize360(headAngles[YAW]);
  VectorClear(legsAngles);
  VectorClear(torsoAngles);

  // --------- yaw -------------
 //hypov8
    idleAnimLeg = 0;
  idleAnimTor = 0;
  anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
  if (anim == LEGS_IDLE || anim == LEGS_CR_IDLE)
    idleAnimLeg = 1;

  anim = cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT;
  if (anim == TORSO_STAND || anim == TORSO_STAND2 || anim == TORSO_STAND3)
    idleAnimTor = 1;

  if (idleAnimLeg != 1 || idleAnimTor != 1)
  {
  // if not standing still, always point all in the same direction
    cent->pe.torso.yawing = qtrue;	// always center
    cent->pe.torso.pitching = qtrue;	// always center
    cent->pe.legs.yawing = qtrue;	// always center
  }

  //different player movemnts on legs, for strafe roate

  // adjust legs for movement dir
  if(cent->currentState.eFlags & EF_DEAD)
  {
    // don't let dead bodies twitch
    dir = 0;
  }
  else
  {
    anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
    // did use angles2.. now uses time2.. looks a bit funny but time2 isn't used othwise
    if (anim == LEGS_LAND || anim == LEGS_JUMP)
      dir = 0; //hypov8 straight ahead
    else
    dir = cent->currentState.time2;

    if(dir < 0 || dir > 7)
    {
      CG_Error("Bad player movement angle");
      dir = 0;
    }
  }
  legsAngles[YAW] = headAngles[YAW] + (vec_t)movementOffsets[dir];
  torsoAngles[YAW] = headAngles[YAW] + 0.25 * (vec_t)movementOffsets[dir];

  // torso
  CG_SwingAngles(torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing);
  CG_SwingAngles(legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing);


  torsoAngles[YAW] = cent->pe.torso.yawAngle;
  legsAngles[YAW] = cent->pe.legs.yawAngle;

  // --------- pitch -------------

  // only show a fraction of the pitch angle in the torso
  if(headAngles[PITCH] > 180)
  {
    dest = (-360 + headAngles[PITCH]) * 0.75f;
  }
  else
  {
    dest = headAngles[PITCH] * 0.75f;
  }

  CG_SwingAngles(dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching);
  torsoAngles[PITCH] = cent->pe.torso.pitchAngle;

  //
  clientNum = cent->currentState.clientNum;

  if(clientNum >= 0 && clientNum < MAX_CLIENTS)
  {
    ci = &cgs.clientinfo[clientNum];

    if(ci->fixedtorso)
    {
      torsoAngles[PITCH] = 0.0f;
    }
  }

  // --------- roll -------------

  // lean towards the direction of travel
  VectorCopy( cent->currentState.pos.trDelta, velocity );
  speed = VectorNormalize(velocity);

  if(speed)
  {
    vec3_t          axis[3];
    float           side;

    speed *= 0.02f;

    AnglesToAxis(legsAngles, axis);
    side = speed * DotProduct(velocity, axis[1]);
    legsAngles[ROLL] -= side;

    side = speed * DotProduct(velocity, axis[0]);
    legsAngles[PITCH] += side;
  }

  //
  clientNum = cent->currentState.clientNum;

  if(clientNum >= 0 && clientNum < MAX_CLIENTS)
  {
    ci = &cgs.clientinfo[clientNum];

    if(ci->fixedlegs)
    {
      legsAngles[YAW] = torsoAngles[YAW];
      legsAngles[PITCH] = 0.0f;
      legsAngles[ROLL] = 0.0f;
    }
  }

  // pain twitch
  CG_AddPainTwitch(cent, torsoAngles);

  // pull the angles back out of the hierarchial chain
  AnglesSubtract(headAngles, torsoAngles, headAngles);
  AnglesSubtract(torsoAngles, legsAngles, torsoAngles);
}
#if 0
//md3 alien
static void CG_PlayerAxis( centity_t *cent, const vec3_t srcAngles,
                           vec3_t legs[ 3 ], vec3_t torso[ 3 ], vec3_t head[ 3 ] )
{
  vec3_t legsAngles, torsoAngles, headAngles;
  CG_PlayerAngles( cent, srcAngles, legsAngles, torsoAngles, headAngles );
  AnglesToAxis( legsAngles, legs );
  AnglesToAxis( torsoAngles, torso );
  AnglesToAxis( headAngles, head );
}
#endif
#define MODEL_WWSMOOTHTIME 200

/*
===============
CG_PlayerWWSmoothing

Smooth the angles of transitioning wall walkers
===============
*/
#if 0
static void CG_PlayerWWSmoothing(centity_t * cent, vec3_t in[3], vec3_t out[3])
{
  entityState_t  *es = &cent->currentState;
  int             i;
  vec3_t          surfNormal, rotAxis, temp;
  vec3_t          refNormal = { 0.0f, 0.0f, 1.0f };
  vec3_t          ceilingNormal = { 0.0f, 0.0f, -1.0f };
  float           stLocal, sFraction, rotAngle;
  vec3_t          inAxis[3], lastAxis[3], outAxis[3];

  // set surfNormal
  if ( !( es->eFlags & EF_WALLCLIMB ) )
  {
    VectorCopy( refNormal, surfNormal );
  }
  else if ( !( es->eFlags & EF_WALLCLIMBCEILING ) )
  {
    VectorCopy( es->angles2, surfNormal );
  }
  else
  {
    VectorCopy( ceilingNormal, surfNormal );
  }

  AxisCopy(in, inAxis);

  if(!Vec3_Compare(surfNormal, cent->pe.lastNormal))
  {
    // if we moving from the ceiling to the floor special case
    // ( x product of colinear vectors is undefined)
    if(Vec3_Compare(ceilingNormal, cent->pe.lastNormal) && Vec3_Compare(refNormal, surfNormal))
    {
      VectorCopy(in[1], rotAxis);
      rotAngle = 180.0f;
    }
    else
    {
      AxisCopy(cent->pe.lastAxis, lastAxis);
      rotAngle = DotProduct(inAxis[0], lastAxis[0]) +
        DotProduct(inAxis[1], lastAxis[1]) + DotProduct(inAxis[2], lastAxis[2]);

      rotAngle = RAD2DEG(acos((rotAngle - 1.0f) / 2.0f));

      CrossProduct(lastAxis[0], inAxis[0], temp);
      VectorCopy(temp, rotAxis);
      CrossProduct(lastAxis[1], inAxis[1], temp);
      VectorAdd(rotAxis, temp, rotAxis);
      CrossProduct(lastAxis[2], inAxis[2], temp);
      VectorAdd(rotAxis, temp, rotAxis);

      VectorNormalize(rotAxis);
    }

    // iterate through smooth array
    for(i = 0; i < MAXSMOOTHS; i++)
    {
      //found an unused index in the smooth array
      if(cent->pe.sList[i].time + MODEL_WWSMOOTHTIME < cg.time)
      {
        //copy to array and stop
        VectorCopy(rotAxis, cent->pe.sList[i].rotAxis);
        cent->pe.sList[i].rotAngle = rotAngle;
        cent->pe.sList[i].time = cg.time;
        break;
      }
    }
  }

  // iterate through ops
  for(i = MAXSMOOTHS - 1; i >= 0; i--)
  {
    //if this op has time remaining, perform it
    if(cg.time < cent->pe.sList[i].time + MODEL_WWSMOOTHTIME)
    {
      stLocal = 1.0f - (((cent->pe.sList[i].time + MODEL_WWSMOOTHTIME) - cg.time) / MODEL_WWSMOOTHTIME);
      sFraction = -(cos(stLocal * M_PI) + 1.0f) / 2.0f;

      RotatePointAroundVector(outAxis[0], cent->pe.sList[i].rotAxis, inAxis[0], sFraction * cent->pe.sList[i].rotAngle);
      RotatePointAroundVector(outAxis[1], cent->pe.sList[i].rotAxis, inAxis[1], sFraction * cent->pe.sList[i].rotAngle);
      RotatePointAroundVector(outAxis[2], cent->pe.sList[i].rotAxis, inAxis[2], sFraction * cent->pe.sList[i].rotAngle);

      AxisCopy(outAxis, inAxis);
    }
  }

  // outAxis has been copied to inAxis
  AxisCopy(inAxis, out);
}
#endif

/*
===============
CG_HasteTrail
===============
*/
#if 0
 static void CG_HasteTrail(centity_t * cent)
{
  localEntity_t  *smoke;
  vec3_t          origin;
  int             anim;

  if(cent->trailTime > cg.time)
  {
    return;
  }
  anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
  if(anim != LEGS_RUN && anim != LEGS_BACK)
  {
    return;
  }

  cent->trailTime += 100;
  if(cent->trailTime < cg.time)
  {
    cent->trailTime = cg.time;
  }

  Vec3_Copy(cent->lerpOrigin, origin);
  origin[2] -= 16;

  smoke = CG_SmokePuff(origin, vec3_origin, 8, 1, 1, 1, 1, 500, cg.time, 0, 0, cgs.media.hastePuffShader);

  // use the optimized local entity add
  smoke->leType = LE_SCALE_FADE;
}
#endif

/*
===============
CG_BreathPuffs
===============
*/
void CG_BreathPuffs(centity_t * cent, const vec3_t headOrigin, const vec3_t headDirection)
{
  clientInfo_t   *ci;
  vec3_t          up, origin;
  int             contents;

  ci = &cgs.clientinfo[cent->currentState.number];

  if(!cg_enableBreath.integer)
  {
    return;
  }
  if(cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
  {
    return;
  }
  if(cent->currentState.eFlags & EF_DEAD)
  {
    return;
  }
  contents = trap_CM_PointContents(headOrigin, 0);
  if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
  {
    return;
  }
  if(ci->breathPuffTime > cg.time)
  {
    return;
  }

  VectorSet(up, 0, 0, 8);
  //Vec3_MA(headOrigin, 8, headDirection, origin);
  //Vec3_MA(origin, -4, head->axis[2], origin);

  CG_SmokePuff(origin, up, 16, 1, 1, 1, 0.66f, 1500, cg.time, cg.time + 400, LEF_PUFF_DONT_SCALE,
         cgs.media.shotgunSmokePuffShader);
  ci->breathPuffTime = cg.time + 2000;
}

/*
===============
CG_DustTrail
===============
*/
void CG_DustTrail(centity_t * cent)
{
  int             anim;
  localEntity_t  *dust;
  vec3_t          end, vel;
  trace_t         tr;

  if(!cg_enableDust.integer)
    return;

  if(cent->dustTrailTime > cg.time)
  {
    return;
  }

  anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
  if(anim != LEGS_LANDB && anim != LEGS_LAND)
  {
    return;
  }

  cent->dustTrailTime += 40;
  if(cent->dustTrailTime < cg.time)
  {
    cent->dustTrailTime = cg.time;
  }

  VectorCopy(cent->currentState.pos.trBase, end);
  end[2] -= 64;
  CG_Trace(&tr, cent->currentState.pos.trBase, NULL, NULL, end, cent->currentState.number, MASK_PLAYERSOLID);

  if(!(tr.surfaceFlags & SURF_DUST))
    return;

  VectorCopy(cent->currentState.pos.trBase, end);
  end[2] -= 16;

  VectorSet(vel, 0, 0, 5); //hypov8 -30 not on md5
  dust = CG_SmokePuff(end, vel, 24, .8f, .8f, 0.7f, 0.33f, 500, cg.time, 0, 0, cgs.media.shotgunSmokePuffShader);	//cgs.media.dustPuffShader);

  (void)dust; //shutup compiler
}



/*
===============
CG_TrailItem
===============
*/
#if 0 // disabled hypov8, use flag instead
static void CG_TrailItem(centity_t * cent, qhandle_t hModel) //, qhandle_t hSkin)
{
  refEntity_t     ent;
  vec3_t          angles;
  vec3_t          axis[3];

  Vec3_Copy(cent->lerpAngles, angles);
  angles[PITCH] = 0;
  angles[ROLL] = 0;
  AnglesToAxis(angles, axis);

  memset(&ent, 0, sizeof(ent));
  VectorMA(cent->lerpOrigin, -16, axis[0], ent.origin);
  ent.origin[2] += 16;
  angles[YAW] += 90;
  AnglesToAxis(angles, ent.axis);

  ent.hModel = hModel;
  //ent.customSkin = hSkin;
  trap_R_AddRefEntityToScene(&ent);
}

#endif


//FIXME(0xA5EA): merge
/*
===============
CG_RunLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
#if 0
void CG_RunLerpFrame( lerpFrame_t *lf, float scale )
{
  int         f, numFrames;
  animation_t *anim;

  // debugging tool to get no animations
  if ( cg_animSpeed.integer == 0 )
  {
    lf->oldFrame = lf->frame = lf->backlerp = 0;
    return;
  }

  // if we have passed the current frame, move it to
  // oldFrame and calculate a new frame
  if ( cg.time >= lf->frameTime )
  {
    lf->oldFrame = lf->frame;
    lf->oldFrameTime = cg.time;

    // get the next frame based on the animation
    anim = lf->animation;

    if ( !anim->frameLerp )
    {
      return; // shouldn't happen
    }

    if ( cg.time < lf->animationTime )
    {
      lf->frameTime = lf->animationTime; // initial lerp
    }
    else
    {
      lf->frameTime = lf->oldFrameTime + anim->frameLerp;
    }

    f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
    f *= scale;
    numFrames = anim->numFrames;

    if ( anim->flipflop )
    {
      numFrames *= 2;
    }

    if ( f >= numFrames )
    {
      f -= numFrames;

      if ( anim->loopFrames )
      {
        f %= anim->loopFrames;
        f += anim->numFrames - anim->loopFrames;
      }
      else
      {
        f = numFrames - 1;
        // the animation is stuck at the end, so it
        // can immediately transition to another sequence
        lf->frameTime = cg.time;
      }
    }

    if ( anim->reversed )
    {
      lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
    }
    else if ( anim->flipflop && f >= anim->numFrames )
    {
      lf->frame = anim->firstFrame + anim->numFrames - 1 - ( f % anim->numFrames );
    }
    else
    {
      lf->frame = anim->firstFrame + f;
    }

    if ( cg.time > lf->frameTime )
    {
      lf->frameTime = cg.time;

      if ( cg_debugAnim.integer ) //hypov8 merge: cg_debugPlayerAnim ??
      {
        CG_Printf( "Clamp lf->frameTime\n" );
      }
    }
  }

  if ( lf->frameTime > cg.time + 200 )
  {
    lf->frameTime = cg.time;
  }

  if ( lf->oldFrameTime > cg.time )
  {
    lf->oldFrameTime = cg.time;
  }

  // calculate current lerp value
  if ( lf->frameTime == lf->oldFrameTime )
  {
    lf->backlerp = 0;
  }
  else
  {
    lf->backlerp = 1.0 - ( float )( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
  }
}

#if 0
static void CG_PlayerUpgrades( centity_t *cent, refEntity_t *torso )
{
  // These are static because otherwise we have >32K of locals, and lcc doesn't like that.
  // Also, jetpack and battpack are never both in use together, so just #define.
  refEntity_t jetpack;
  refEntity_t flash;

#	define battpack jetpack

  int held, publicFlags;
  entityState_t *es = &cent->currentState;

  held = es->modelindex;
  publicFlags = es->modelindex2;

  // jetpack model and effects
  if (held & (1 << UP_JETPACK))
  {
    memset(&jetpack, 0, sizeof(jetpack));
    VectorCopy(torso->lightingOrigin, jetpack.lightingOrigin);
    jetpack.shadowPlane = torso->shadowPlane;
    jetpack.renderfx = torso->renderfx;

    jetpack.hModel = cgs.media.jetpackModel;

    // identity matrix
    AxisCopy(axisDefault, jetpack.axis);

    // FIXME: change to tag_back when it exists
    CG_PositionRotatedEntityOnTag(&jetpack, torso, torso->hModel, "tag_head");

    trap_R_AddRefEntityToScene(&jetpack);

    if (publicFlags & PF_JETPACK_ACTIVE)
    {
      // spawn ps if necessary
      if (cent->jetPackState != JPS_ACTIVE)
      {
        if (CG_IsParticleSystemValid(&cent->jetPackPS))
        {
          CG_DestroyParticleSystem(&cent->jetPackPS);
        }

        cent->jetPackPS = CG_SpawnNewParticleSystem(cgs.media.jetPackThrustPS);

        cent->jetPackState = JPS_ACTIVE;
      }

      // play thrust sound
      trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin,
          vec3_origin, cgs.media.jetpackThrustLoopSound);

      // Add flash tag (?)
      {
        memset(&flash, 0, sizeof(flash));
        VectorCopy(torso->lightingOrigin, flash.lightingOrigin);
        flash.shadowPlane = torso->shadowPlane;
        flash.renderfx = torso->renderfx;

        flash.hModel = cgs.media.jetpackFlashModel;

        if (!flash.hModel)
        {
          return;
        }

        AxisCopy(axisDefault, flash.axis);

        CG_PositionRotatedEntityOnTag(&flash, &jetpack, jetpack.hModel,
            "tag_flash");
        trap_R_AddRefEntityToScene(&flash);
      }

      // attach ps
      if (CG_IsParticleSystemValid(&cent->jetPackPS))
      {
        CG_SetAttachmentTag(&cent->jetPackPS->attachment, &jetpack,
            jetpack.hModel, "tag_flash");
        CG_SetAttachmentCent(&cent->jetPackPS->attachment, cent);
        CG_AttachToTag(&cent->jetPackPS->attachment);
      }
    }
    else if (CG_IsParticleSystemValid(&cent->jetPackPS))
    {
      // disable jetpack ps when not thrusting anymore
      CG_DestroyParticleSystem(&cent->jetPackPS);
      cent->jetPackState = JPS_INACTIVE;
    }
  }
  else if (CG_IsParticleSystemValid(&cent->jetPackPS))
  {
    // disable jetpack ps when not carrying it anymore
    CG_DestroyParticleSystem(&cent->jetPackPS);
    cent->jetPackState = JPS_INACTIVE;
  }

  // battery pack
  if (held & (1 << UP_BATTPACK))
  {
    memset(&battpack, 0, sizeof(battpack));
    VectorCopy(torso->lightingOrigin, battpack.lightingOrigin);
    battpack.shadowPlane = torso->shadowPlane;
    battpack.renderfx = torso->renderfx;

    battpack.hModel = cgs.media.battpackModel;

    //identity matrix
    AxisCopy(axisDefault, battpack.axis);

    //FIXME: change to tag_back when it exists
    CG_PositionRotatedEntityOnTag(&battpack, torso, torso->hModel, "tag_head");

    trap_R_AddRefEntityToScene(&battpack);
  }

  // creep below bloblocked players
  if (es->eFlags & EF_BLOBLOCKED)
  {
    vec3_t temp, origin, up = { 0.0f, 0.0f, 1.0f };
    trace_t tr;
    float size;

    VectorCopy(es->pos.trBase, temp);
    temp[2] -= 4096.0f;

    CG_Trace(&tr, es->pos.trBase, NULL, NULL, temp, es->number, MASK_SOLID);
    VectorCopy(tr.endpos, origin);

    size = 32.0f;

    if (size > 0.0f)
    {
      CG_ImpactMark(cgs.media.creepShader, origin, up, 0.0f, 1.0f, 1.0f, 1.0f,
          1.0f, qfalse, size, qtrue);
    }
  }

#	undef battpack
}
#endif
#endif
//FIXME(0xA5EA):merge

static ID_INLINE qboolean CG_PlayerHasTeamItem(int powerups)
{
  return (powerups & ((1 << PW_DRAGONFLAG)
            | (1 << PW_NIKKIFLAG)
            | (1 << PW_NEUTRALFLAG)
            | (1 << PW_STOLENSCASH))); // add hypov8
}


/*
===============
CG_PlayerFlag
xreal flag
===============
*/
static void CG_PlayerFlag(centity_t * cent, qhandle_t hSkin, qhandle_t hModel, refEntity_t * body)
{
  //clientInfo_t *ci;
  static refEntity_t     flag;
  vec3_t          angles;

  int clientNum;
  clientNum = cent->currentState.clientNum;
  if(clientNum < 0 || clientNum >= MAX_CLIENTS)
    CG_Error("Bad clientNum on player entity");

  //ci = &cgs.clientinfo[clientNum];

  memset(&flag, 0, sizeof(flag));

  //setup model
  flag.hModel = hModel;
  flag.customSkin = hSkin;
  flag.shadowPlane = body->shadowPlane;
  flag.renderfx = body->renderfx;
  VectorCopy(body->lightingOrigin, flag.lightingOrigin);

  //VectorCopy(cent->lerpAngles, angles);
  //AnglesToAxis(angles, flag.axis);

  angles[PITCH] = 0;
  angles[YAW] = 90;  //hypov8 todo: fix thug player model tag error. remove this
  angles[ROLL] = 90;
  AnglesToAxis(angles, flag.axis);
  CG_PositionRotatedEntityOnTag(&flag, body, body->hModel, "tag_flag");
  //CG_PositionEntityOnTag(&flag, body, body->hModel, "tag_flag");

  trap_R_AddRefEntityToScene(&flag);
}

/*
===============
CG_PlayerModelAddons
===============
*/
static void CG_PlayerModelAddons(centity_t * cent, refEntity_t * torso)
{
  int             powerups;
  //refLight_t      light;
  //float           radius;

  powerups = cent->currentState.powerups;
  if (powerups == PW_NONE)
    return;

  if (CG_PlayerHasTeamItem(powerups))
  {
    qhandle_t skin = 0;
    qhandle_t model = 0;
    //vec_t* color = 0;
    //int	nolight = 0;

    if (powerups & (1 << PW_NIKKIFLAG))
    {
      skin = cgs.media.nikkiFlagFlapSkin;
      model = cgs.media.nikkiFlagModel;
      //color = colortmgreenyellow;
    }
    else if (powerups & (1 << PW_DRAGONFLAG))
    {
      skin = cgs.media.dragonFlagFlapSkin;
      model = cgs.media.dragonFlagModel;
      //color = colortmRed;
    }
    else if (powerups & (1 << PW_NEUTRALFLAG))
    {
      skin = cgs.media.neutralFlagFlapSkin;
      model = cgs.media.neutralFlagModel;
      //color = colortmgreenyellow;
    }
    else if (powerups & (1 << PW_STOLENSCASH)) // add hypov8 bm bag backpack
    {
      //skin = cgs.media.cashStolenSkin;
      model = cgs.media.cashStolenModel;
      //color = colortmgreenyellow;
      //nolight = 1;
    }

     //hypov8 Todo: add a model linking between flag and torso

    //add the flag/bag model
    CG_PlayerFlag(cent, skin, model, torso);

    //add dynamic player light
    /*if (!nolight)
    {
      Com_Memset(&light, 0, sizeof(refLight_t));
      light.rlType = RL_OMNI;
      VectorCopy(color, light.color);
      VectorCopy(cent->lerpOrigin, light.origin);
      radius = 200 + (rand() & 31);
      light.radius[0] = radius;
      light.radius[1] = radius;
      light.radius[2] = radius;
      QuatClear(light.rotation);
      light.noShadows = qtrue; // hypov8 disabled shadows on dynamic player light
      trap_R_AddRefLightToScene(&light);
    }*/
  }

/////////////////////////////////////////////////////////////////////////////
  if (powerups & (1 << PW_FLASHLIGHT)) // add hypov8 (makes light work in 3rd person)
  {
    ;/* todo: add flashlight model */
  }
}


/*
===============
CG_PlayerModelAddonsLights
hypov8
render lights after shadow caster
===============
*/
static void CG_PlayerModelAddonsLights(centity_t * cent, refEntity_t * torso)
{
  int             powerups;
  refLight_t      light;
  float           radius;

  powerups = cent->currentState.powerups;
  if (powerups == PW_NONE) //impliment flashlight
    return;

  if (CG_PlayerHasTeamItem(powerups))
  {
    vec_t* color = 0;
    qboolean addLight = qtrue;

    if (powerups & (1 << PW_NIKKIFLAG))
      color = colortmgreenyellow;
    else if (powerups & (1 << PW_DRAGONFLAG))
      color = colortmRed;
    else if (powerups & (1 << PW_NEUTRALFLAG))
      color = colorWhite;
    else if (powerups & (1 << PW_STOLENSCASH)) // add hypov8 bm bag backpack
      addLight = qfalse;

    //add dynamic player light
    if (addLight)
    {
      Com_Memset(&light, 0, sizeof(refLight_t));
      light.rlType = RL_OMNI;
      VectorCopy(color, light.color);
      VectorCopy(cent->lerpOrigin, light.origin);
      radius = 200 + (rand() & 31);
      light.radius[0] = radius;
      light.radius[1] = radius;
      light.radius[2] = radius;
      QuatClear(light.rotation);
      light.noShadows = qtrue; // hypov8 disabled shadows on dynamic player light. view option??
      trap_R_AddRefLightToScene(&light);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (powerups & (1 << PW_FLASHLIGHT)) // add hypov8 (makes light work in 3rd person)
  {
    vec3_t headAxis;
    vec_t fov_x;
    vec3_t originMoveUp;
    vec3_t head_axis[3];

    if (!cgs.media.flashLightShader)
    {
      CG_Printf(S_COLOR_RED"no flashlight shader\n");
      return;
    }

#if 1
    /* add flashlight light */
    Com_Memset(&light, 0, sizeof(refLight_t));

    light.attenuationShader = cgs.media.flashLightShader;
    light.rlType = RL_PROJ;
    light.color[0] = 1.0f;
    light.color[1] = 1.0f;
    light.color[2] = 0.8f;

    //if (!CG_PositionRotatedLightOnBone(&light, torso, torso->hModel, "thug_kpq3.Head")) //hypov8 attatch to head ToDo: re'locate
    //{
    //cent->lerpOrigin[2] += 36;

#if 0 //use skelton bone for light
    int boneIndex;
    //orientation_t lerped;

    //trap_R_LerpTag(&lerped, torso->hModel, torso->oldframe, torso->frame, 1.0 - torso->backlerp, "thug_kpq3.Head");
    boneIndex = trap_R_BoneIndex(torso->hModel, "thug_kpq3.Head");
    Vec3_Copy(torso->skeleton.bones[boneIndex].origin, originMoveUp);
    Vec3_Add(torso->origin, originMoveUp, light.origin);

#else //use calculated origin for light
    VectorCopy(cent->lerpOrigin, originMoveUp);

    //hypov8 move flashlight origin up to suit player head height(and when crouched)
    if ((cent->currentState.legsAnim & ~ANIM_TOGGLEBIT) == LEGS_CR_IDLE ||
      (cent->currentState.legsAnim & ~ANIM_TOGGLEBIT) == LEGS_CR_WALK ||
      (cent->currentState.legsAnim & ~ANIM_TOGGLEBIT) == LEGS_CR_BACK)
      originMoveUp[2] += 12;
    else
      originMoveUp[2] += 36;

    VectorCopy(originMoveUp, light.origin); //default body light if bone missing

#endif
    //}

    //fov_x = tan((cg.refdef.fov_x * 0.5f)*M_DEG2RAD);
    fov_x = tan((90 * 0.5f)*M_DEG2RAD);
    fov_x /= 2;

    VectorCopy(cent->lerpAngles, headAxis); //hypo change this. smooth it some how
    AnglesToAxis(headAxis, head_axis);
    VectorCopy(head_axis[0], light.projTarget);
    VectorScale(head_axis[1], -fov_x, light.projRight);
    VectorScale(head_axis[2], fov_x, light.projUp);
    VectorScale(head_axis[0], -14, light.projStart);
    VectorScale(head_axis[0], 1000, light.projEnd);

    light.noShadows = qtrue; //disable shadows testing (player will cause self shadows)
    light.isForcedDynLight = qtrue;

    trap_R_AddRefLightToScene(&light);
#endif
  }

}


/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
static void CG_PlayerFloatSprite(centity_t * cent, refEntity_t * body, qhandle_t shader)
{
  int             rf, boneIndex;
  refEntity_t     ent;
  vec3_t          surfNormal;

  clientInfo_t   *ci;
  int             clientNum;

  if(cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
  {
    rf = RF_THIRD_PERSON;	// only show in mirrors
  }
  else
  {
    rf = 0;
  }

//hypov8 add get bone origin
  clientNum = cent->currentState.clientNum;
  if(clientNum < 0 || clientNum >= MAX_CLIENTS)
  {
    CG_Error("Bad clientNum on player entity");
  }
  ci = &cgs.clientinfo[clientNum];
//hypov8 end

  memset(&ent, 0, sizeof(ent));

//hypov8 add
  if (cent->currentState.eFlags & EF_TALK && !(cent->currentState.eFlags & EF_DEAD) && !(cg.intermissionStarted))
  {
    if (cent->currentState.eFlags & EF_TALK)
      VectorSet(surfNormal, 0.0f, 0.0f, -1.0f);
    else
      VectorCopy(cent->currentState.angles2, surfNormal);
  }
  else
  {
    VectorSet(surfNormal, 0.0f, 0.0f, 1.0f);
  }

  //VectorMA(cent->lerpOrigin, 64.0f, surfNormal, ent.origin);

  //boneIndex = trap_R_BoneIndex(body->hModel, "tag_flag");
  boneIndex = /*trap_R_BoneIndex(body->hModel,*/ ci->neckControlBone/*)*/;

  if (boneIndex >= 0 && boneIndex < body->skeleton.numBones /*cent->pe.torso..skeleton.numBones*/) //hypov8 merge:
  {
    orientation_t lerped;
    int i;

    CG_PositionEntityOnTag(&ent, body, body->hModel, ci->neckControlBoneName);

    //trap_R_LerpTag(&lerped, body->hModel, body->oldframe, body->frame, 1.0 - body->backlerp, ci->neckControlBoneName);
    VectorCopy(body->skeleton.bones[boneIndex].t.trans, lerped.origin);

    // FIXME: allow origin offsets along tag?
    VectorCopy(body->origin, ent.origin);
    for (i = 0; i < 3; i++)
    {
      VectorMA(ent.origin, lerped.origin[i], body->axis[i], ent.origin);
    }

  }
  else
    VectorCopy(cent->lerpOrigin, ent.origin);

  ent.origin[2] += 16; // 46; //add hypov8 offset for md5 bone
//hypov8 end
  ent.reType = RT_SPRITE;
  ent.customShader = shader;
  ent.radius = 10;
  ent.renderfx = rf;
  ent.shaderRGBA[0] = 255;
  ent.shaderRGBA[1] = 255;
  ent.shaderRGBA[2] = 255;
  ent.shaderRGBA[3] = 255;
  trap_R_AddRefEntityToScene(&ent);
}

/*
===============
CG_PlayerSprites
hypov8 todo: sprites on player head. fix bone

Float sprites over the player's head
===============
*/
void CG_PlayerSprites(centity_t *cent, refEntity_t * body)
{
  int team;

  if (cent->currentState.eFlags & EF_CONNECTION)
  {
    CG_PlayerFloatSprite(cent,body, cgs.media.connectionShader);
    return;
  }

  if (cent->currentState.eFlags & EF_TALK)
  {
    CG_PlayerFloatSprite(cent, body, cgs.media.balloonShader);
    return;
  }
#if 0
  if (cent->currentState.eFlags & EF_AWARD_IMPRESSIVE)
  {
    CG_PlayerFloatSprite(cent, cgs.media.medalImpressive);
    return;
  }
#endif
  if (cent->currentState.eFlags & EF_AWARD_EXCELLENT)
  {
    CG_PlayerFloatSprite(cent, body, cgs.media.medalExcellent);
    return;
  }
#if 0
  if (cent->currentState.eFlags & EF_AWARD_GAUNTLET)
  {
    CG_PlayerFloatSprite(cent, cgs.media.medalGauntlet);
    return;
  }
#endif
  if (cent->currentState.eFlags & EF_AWARD_DEFEND)
  {
    CG_PlayerFloatSprite(cent, body, cgs.media.medalDefend);
    return;
  }

  if (cent->currentState.eFlags & EF_AWARD_ASSIST)
  {
    CG_PlayerFloatSprite(cent, body, cgs.media.medalAssist);
    return;
  }

  if (cent->currentState.eFlags & EF_AWARD_CAP)
  {
    CG_PlayerFloatSprite(cent, body, cgs.media.medalCapture);
    return;
  }

  team = cgs.clientinfo[cent->currentState.clientNum].team;
  if (!(cent->currentState.eFlags & EF_DEAD) &&
      cg.snap->ps.persistant[PERS_TEAM] == team &&
      cgs.gametype >= GT_TEAM)
  {
    if (cg_drawFriend.integer)
    {
    if (team == 2) //hypov8 bagman sprites
      CG_PlayerFloatSprite(cent, body, cgs.media.teamNikki);
    else
      CG_PlayerFloatSprite(cent, body, cgs.media.teamDragon);
    }
    return;
  }
}


/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

should it return a full plane instead of a Z?
===============
*/
#define SHADOW_DISTANCE 128
qboolean CG_PlayerShadow( centity_t *cent, float *shadowPlane)
{
  vec3_t        end;
  static const vec3_t mins = {-15, -15, 0}, maxs = {15, 15, 2};
  trace_t trace;
  float alpha;
  //entityState_t *es = &cent->currentState;
  vec3_t        surfNormal = { 0.0f, 0.0f, 1.0f };

  //FIXME(0xA5EA):merge

  *shadowPlane = 0;

  if ( cg_shadows.integer == SHADOWING_NONE )
  {
    return qfalse;
  }

  // send a trace down from the player to the ground
  VectorCopy( cent->lerpOrigin, end );
  VectorMA( cent->lerpOrigin, -SHADOW_DISTANCE, surfNormal, end );

  trap_CM_BoxTrace(&trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID);

  // no shadow if too high
  if (trace.fraction == 1.0 || trace.startsolid || trace.allsolid)
  {
    return qfalse;
  }

  // FIXME: stencil shadows will be broken for walls.
  //           Unfortunately there isn't much that can be
  //           done since Q3 references only the Z coord
  //           of the shadowPlane
  if (surfNormal[2] < 0.0f)
  {
    *shadowPlane = trace.endpos[2] - 1.0f;
  }
  else
  {
    *shadowPlane = trace.endpos[2] + 1.0f;
  }

  if ( cg_shadows.integer > SHADOWING_BLOB /* && cg_playerShadows.integer*/ ) //FIXME(0xA5EA):merge
  {
    // add inverse shadow map
    {
      CG_StartShadowCaster( cent->lerpOrigin, mins, maxs );
    }
  }

  if ( cg_shadows.integer != SHADOWING_BLOB ) // no mark for stencil or projection shadows
    return qtrue;

  // fade the shadow out with height
  alpha = 1.0 - trace.fraction;

  // add the mark as a temporary, so it goes directly to the renderer
  // without taking a spot in the cg_marks array
  CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal,
                 cent->pe.legs.yawAngle, 0.0f, 0.0f, 0.0f, alpha, qfalse,
                 24.0f * 1 /*BG_ClassModelConfig( class_ )->shadowScale*/, qtrue );

  //FIXME(0xA5EA): merge

  return qtrue;
}

static void CG_PlayerShadowEnd( void )
{
  if ( cg_shadows.integer == SHADOWING_NONE )
    return;

  if ( cg_shadows.integer > SHADOWING_BLOB /* && cg_playerShadows.integer*/ ) //FIXME(0xA5EA):merge
    CG_EndShadowCaster( );
}

/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
void CG_PlayerSplash( centity_t *cent)
{
//hypov8 merge: test? disabled in kpq3
#if 0
  vec3_t          start, end;
  static const vec3_t mins = {-15, -15, 0}, maxs = {15, 15, 2};
  trace_t         trace;
  int             contents;

  if ( cg_shadows.integer == SHADOWING_NONE )
    return;

  VectorCopy( cent->lerpOrigin, end );
  end[ 2 ] += mins[ 2 ];

  // if the feet aren't in liquid, don't make a mark
  // this won't handle moving water brushes, but they wouldn't draw right anyway...
  contents = trap_CM_PointContents(end, 0);

  if(!(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA)))
  {
    return;
  }

  VectorCopy( cent->lerpOrigin, start );
  start[2] += 32;

  // if the head isn't out of liquid, don't make a mark
  contents = trap_CM_PointContents(start, 0);

  if(contents & (CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
  {
    return;
  }

  // trace down to find the surface
  trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0,
                    ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) );

  if ( trace.fraction == 1.0f )
  {
    return;
  }

  CG_ImpactMark( cgs.media.wakeMarkShader, trace.endpos, trace.plane.normal,
                 cent->pe.legs.yawAngle, 1.0f, 1.0f, 1.0f, 1.0f, qfalse,
                 32.0f * 1/* BG_ClassModelConfig( class_ )->shadowScale*/, qtrue );
#endif
  //FIXME(0xA5EA): merge
}

/*
=================
CG_LightVerts
=================
*/
#if 0 //hypov8 merge: not used?
int CG_LightVerts(vec3_t normal, int numVerts, polyVert_t * verts)
{
  int             i, j;
  float           incoming;
  vec3_t          ambientLight;
  vec3_t          lightDir;
  vec3_t          directedLight;

  trap_R_LightForPoint(verts[0].xyz, ambientLight, directedLight, lightDir);

  for(i = 0; i < numVerts; i++)
  {
    incoming = DotProduct(normal, lightDir);
    if(incoming <= 0)
    {
      verts[ i ].modulate[ 0 ] = ambientLight[ 0 ];
      verts[ i ].modulate[ 1 ] = ambientLight[ 1 ];
      verts[ i ].modulate[ 2 ] = ambientLight[ 2 ];
      verts[i].modulate[3] = 255;
      continue;
    }
    j = (ambientLight[0] + incoming * directedLight[0]);

    if ( j > 255 )
    {
      j = 255;
    }

    verts[ i ].modulate[ 0 ] = j;

    j = (ambientLight[1] + incoming * directedLight[1]);

    if ( j > 255 )
    {
      j = 255;
    }

    verts[ i ].modulate[ 1 ] = j;

    j = (ambientLight[2] + incoming * directedLight[2]);

    if ( j > 255 )
    {
      j = 255;
    }

    verts[ i ].modulate[ 2 ] = j;

    verts[i].modulate[3] = 255;
  }

  return qtrue;
}
#endif

/*
=================
CG_LightFromDirection
=================
*/
#if 0 //hypove merge: not used?
int CG_LightFromDirection( vec3_t point, vec3_t direction )
{
  int    j;
  float  incoming;
  vec3_t ambientLight;
  vec3_t lightDir;
  vec3_t directedLight;
  vec3_t result;

  trap_R_LightForPoint( point, ambientLight, directedLight, lightDir );

  incoming = DotProduct( direction, lightDir );

  if ( incoming <= 0 )
  {
    result[ 0 ] = ambientLight[ 0 ];
    result[ 1 ] = ambientLight[ 1 ];
    result[ 2 ] = ambientLight[ 2 ];
    return ( int )( ( float )( result[ 0 ] + result[ 1 ] + result[ 2 ] ) / 3.0f );
  }

  j = ( ambientLight[ 0 ] + incoming * directedLight[ 0 ] );

  if ( j > 255 )
  {
    j = 255;
  }

  result[ 0 ] = j;

  j = ( ambientLight[ 1 ] + incoming * directedLight[ 1 ] );

  if ( j > 255 )
  {
    j = 255;
  }

  result[ 1 ] = j;

  j = ( ambientLight[ 2 ] + incoming * directedLight[ 2 ] );

  if ( j > 255 )
  {
    j = 255;
  }

  result[ 2 ] = j;

  return ( int )( ( float )( result[ 0 ] + result[ 1 ] + result[ 2 ] ) / 3.0f );
}
#endif

/*
=================
CG_AmbientLight
=================
*/
#if 0 //hypove merge: not used?
int CG_AmbientLight( vec3_t point )
{
  vec3_t ambientLight;
  vec3_t lightDir;
  vec3_t directedLight;
  vec3_t result;

  trap_R_LightForPoint( point, ambientLight, directedLight, lightDir );

  result[ 0 ] = ambientLight[ 0 ];
  result[ 1 ] = ambientLight[ 1 ];
  result[ 2 ] = ambientLight[ 2 ];
  return ( int )( ( float )( result[ 0 ] + result[ 1 ] + result[ 2 ] ) / 3.0f );
}
#endif

#define TRACE_DEPTH    32.0f

/*
===============
Statics for CG_Player & CG_Corpse
These are QVM-only to keep locals down below 32K
===============
*/
#ifdef Q3_VM
static refEntity_t legs, torso, body, head;
#endif

/*
===============
CG_Player
===============
*/
void CG_Player(centity_t * cent)
{
  clientInfo_t *ci;

  // NOTE: legs is used for nonsegmented models
  //       this helps reduce code to be changed
#ifndef Q3_VM
  refEntity_t /*legs, torso,*/ body/*, head*/;
#endif

  int clientNum;
  int renderfx;
  qboolean shadow UNUSED = qfalse;
  float shadowPlane = 0.0f;
  entityState_t *es = &cent->currentState;
  vec3_t tempAxis[3];
  vec3_t angles;
  vec3_t surfNormal = { 0.0f, 0.0f, 1.0f };

  //altShader_t   altShaderIndex;

  // the client number is stored in clientNum.  It can't be derived
  // from the entity number, because a single client may have
  // multiple corpses on the level using the same clientinfo
  clientNum = es->clientNum;

  if (clientNum < 0 || clientNum >= MAX_CLIENTS)
  {
    CG_Error("Bad clientNum on player entity");
  }

  ci = &cgs.clientinfo[clientNum];

  // it is possible to see corpses from disconnected players that may
  // not have valid clientinfo
  if (!ci->infoValid)
  {
    return;
  }

  //don't draw
  if (es->eFlags & EF_NODRAW)
    return;

#if 0
  if ( es->eFlags & EF_DEAD )
  {
    altShaderIndex = CG_ALTSHADER_DEAD;
  }
  else if ( !(es->eFlags & EF_B_POWERED) )
  {
    altShaderIndex = CG_ALTSHADER_UNPOWERED;
  }
  else
  {
    altShaderIndex = CG_ALTSHADER_DEFAULT;
  }
#endif
  //FIXME(0xA5EA): merge
  // get the player model information
  renderfx = 0;

  if (es->number == cg.snap->ps.clientNum)
  {
    if (!cg.renderingThirdPerson)
    {
      renderfx = RF_THIRD_PERSON; // only draw in mirrors
    }
  }

  if (cg_drawBBox.integer && cg.renderingThirdPerson )
  {
    vec3_t mins, maxs;
    VectorSet(mins, -16, -16, -24);
    VectorSet(maxs, 16, 16, 48);
    //BG_ClassBoundingBox( class_, mins, maxs, nullptr, nullptr, nullptr );
    CG_DrawBoundingBox( cg_drawBBox.integer, cent->lerpOrigin, mins, maxs );
  }

  //FIXME(0xA5EA):merge

  if (ci->md5)
  {
    Com_Memset(&body, 0, sizeof(body));
  }
 /* else
  {
    memset(&legs, 0, sizeof(legs));
    memset(&torso, 0, sizeof(torso));
    memset(&head, 0, sizeof(head));
  }
  */
  VectorCopy( cent->lerpAngles, angles);
  AnglesToAxis(cent->lerpAngles, tempAxis);

  // rotate lerpAngles to floor
  //if ( es->eFlags & EF_WALLCLIMB &&  BG_RotateAxis( es->angles2, tempAxis, tempAxis2, qtrue, es->eFlags & EF_WALLCLIMBCEILING ) )
  //{
  //	AxisToAngles(tempAxis2, angles);
  //}
  //else
  {
    VectorCopy( cent->lerpAngles, angles);
  }

  //normalise the pitch
  if (angles[PITCH] < -180.0f)
  {
    angles[PITCH] += 360.0f;
  }

  //if (ci->md5)
  {
    vec3_t legsAngles, torsoAngles, headAngles;
    int boneIndex;
    vec3_t playerOrigin;
    quat_t rotation;

    if (ci->gender != GENDER_NEUTER)
    {
      CG_PlayerAngles(cent, angles, legsAngles, torsoAngles, headAngles);
      AnglesToAxis(legsAngles, body.axis);
    }
    else  //alien
    {
      //CG_PlayerNonSegAxis( cent, angles, body.axis );
      //FIXME(0xA5EA): merge
    }

    AxisCopy(body.axis, tempAxis);


    //FIXME(0xA5EA): merge
    AxisCopy(tempAxis, cent->pe.lastAxis);

    // get the animation state (after rotation, to allow feet shuffle)
    if (ci->gender != GENDER_NEUTER)
    {
      CG_PlayerMD5Animation(cent);
    }
    else //alien
    {
      //CG_PlayerMD5AlienAnimation( cent );
      //FIXME(0xA5EA):merge
    }

    // add the shadow
    if ((es->number == cg.snap->ps.clientNum && cg.renderingThirdPerson)
      || es->number != cg.snap->ps.clientNum)
    {
      shadow = CG_PlayerShadow(cent, &shadowPlane);
    }


    renderfx |= RF_LIGHTING_ORIGIN; // use the same origin for all
    if (cgs.gametype >= GT_TEAM)
      renderfx |= RF_MINLIGHT;

    // add the body
    body.hModel = ci->bodyModel;
    body.customSkin = ci->bodySkin;

    if (!body.hModel)
    {
      CG_Printf("No body model for player %i\n", clientNum);
      return;
    }

    body.shadowPlane = shadowPlane;
    body.renderfx = renderfx;

    //BG_ClassBoundingBox(class_, mins, maxs, NULL, NULL, NULL);

    //FIXME(0xA5EA):merge
    {
      VectorCopy(cent->lerpOrigin, playerOrigin);
      VectorCopy(playerOrigin, body.origin);
      body.origin[0] -= ci->headOffset[0];
      body.origin[1] -= ci->headOffset[1];
      body.origin[2] -= 22 + ci->headOffset[2]; //hypov8 todo 24 offset?
    }

    VectorCopy(body.origin, body.lightingOrigin);
    VectorCopy(body.origin, body.oldorigin); // don't positionally lerp at all

    if (ci->gender != GENDER_NEUTER)
    {
      // copy legs skeleton to have a base
      body.skeleton = torsoSkeleton;
      if (torsoSkeleton.numBones != legsSkeleton.numBones)
      {
        // seems only to happen when switching from an MD3 model to an MD5 model
        // while spectating (switching between players on the human team)
        // - don't treat as fatal, but doing so will (briefly?) cause rendering
        // glitches if chasing; also, brief spam
        CG_Printf("[skipnotify]WARNING: cent->pe.legs.skeleton.numBones != cent->pe.torso.skeleton.numBones\n");
      }

      {	  //hypov8 add: assumes rootBone -> torsoBone are the first bones in model
        int i;

        ci->numLegBones = 0;
        boneIndex = ci->torsoControlBone;

        for (i = 0; i < boneIndex; i++)
        {
          ci->legBones[i] = i;// body.skeleton.bones[i];
          ci->numLegBones += 1;
        }
      }

      // combine legs and torso skeletons
      if (ci->numLegBones) //handBones?
      {
        CG_CombineLegSkeleton(&body.skeleton, &legsSkeleton, ci->legBones, ci->numLegBones);
      }

      if (ci->weaponAdjusted & (1 << es->weapon) && (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) >= TORSO_ATTACK)
      {
        int j;

        for (j = 0; j < ci->numHandBones; j++)
        {
          VectorAdd(deltas[es->weapon][ci->handBones[j]].delta,
          body.skeleton.bones[ci->handBones[j]].t.trans,
          body.skeleton.bones[ci->handBones[j]].t.trans);
          QuatMultiply0(body.skeleton.bones[ci->handBones[j]].t.rot,
          deltas[es->weapon][ci->handBones[j]].rot);
        }
      }


      // rotate torso
      boneIndex = ci->torsoControlBone;
      if (boneIndex >= 0 && boneIndex < torsoSkeleton.numBones)
      {
        // HACK: convert angles to bone system
        QuatFromAngles(rotation,torsoAngles[ROLL] , torsoAngles[PITCH], torsoAngles[YAW]);//torsoAngles[YAW]
        QuatMultiply0(body.skeleton.bones[boneIndex].t.rot, rotation);
      }

      //////////////
      // rotate head
      boneIndex = ci->neckControlBone;
      if (boneIndex >= 0 && boneIndex < torsoSkeleton.numBones)
      {
        //QuatFromAngles(rotation, -cent->lerpAngles[0], cent->lerpAngles[1], 0);
        QuatFromAngles(rotation, headAngles[ROLL], headAngles[PITCH], headAngles[YAW]);
        QuatMultiply0(body.skeleton.bones[boneIndex].t.rot, rotation);//rightShoulderBone
      }
    }
    //else
      //alien

    // transform relative bones to absolute ones required for vertex skinning and tag attachments
    CG_TransformSkeleton(&body.skeleton, ci->modelScale);

    // add the gun to player model
    if (es->weapon != WP_NONE)
    {
      CG_AddPlayerWeapon(&body, NULL, cent);
    }

    //CG_PlayerUpgrades( cent, &body );
    //FIXME(0xA5EA):merge

    // add player held items. flags, cash, light etc..
    CG_PlayerModelAddons(cent, &body);

    // add body to renderer
    body.altShaderIndex = 1; // altShaderIndex;
    //FIXME(0xA5EA):merge

    // add body to renderer
    trap_R_AddRefEntityToScene(&body);
    //hypov8 finish adding models, now add lights
    ///////////////////////////////////

    // add a water splash if partially in and out of water
    CG_PlayerSplash(cent);

    CG_DustTrail(cent);

      /* add player sprites above head */
    CG_PlayerSprites(cent, &body);

    //add light after player shadow
    CG_PlayerModelAddonsLights(cent, &body);

    //add muzzle flash sprite and dynamic light at tag origin
    //CG_AddPlayerWeaponLight(NULL, cent ,body);

    goto finish_up; //not needed..  yet
  }


finish_up:
  //sanity check that particle systems are stopped when dead
#if 0
  if (es->eFlags & EF_DEAD)
  {
    if (CG_IsParticleSystemValid(&cent->muzzlePS))
    {
      CG_DestroyParticleSystem(&cent->muzzlePS);
    }

    if (CG_IsParticleSystemValid(&cent->jetPackPS))
    {
      CG_DestroyParticleSystem(&cent->jetPackPS);
    }
  }
#endif
  //FIXME(0xA5EA):merge

  VectorCopy(surfNormal, cent->pe.lastNormal);
  CG_PlayerShadowEnd();
}

#if 0
/*
===============
CG_Corpse
===============
*/
void CG_Corpse( centity_t *cent )
{
#ifndef Q3_VM
  refEntity_t   legs, torso, head;
#endif
  clientInfo_t  *ci;
  entityState_t *es = &cent->currentState;
  int           corpseNum;
  int           renderfx;
  qboolean      shadow = qfalse;
  float         shadowPlane;
  vec3_t        origin, liveZ, deadZ, deadMax;
  float         scale;

  corpseNum = 0 ;// CG_GetCorpseNum( (class_t) es->clientNum );
  //FIXME(0xA5EA): merge
  if ( corpseNum < 0 || corpseNum >= MAX_CLIENTS )
  {
    CG_Error( "Bad corpseNum on corpse entity: %d", corpseNum );
  }

  ci = &cgs.corpseinfo[ corpseNum ];

  // it is possible to see corpses from disconnected players that may
  // not have valid clientinfo
  if ( !ci->infoValid )
  {
    return;
  }

  memset( &legs, 0, sizeof( legs ) );
  memset( &torso, 0, sizeof( torso ) );
  memset( &head, 0, sizeof( head ) );

  VectorCopy( cent->lerpOrigin, origin );
  BG_ClassBoundingBox( es->clientNum, liveZ, NULL, NULL, deadZ, deadMax );
  origin[ 2 ] -= ( liveZ[ 2 ] - deadZ[ 2 ] );

  if( ci->md5 )
  {
    origin[ 0 ] -= ci->headOffset[ 0 ];
    origin[ 1 ] -= ci->headOffset[ 1 ];
    origin[ 2 ] -= 19 + ci->headOffset[ 2 ];
  }
  VectorCopy( es->angles, cent->lerpAngles );

  // get the rotation information
  if ( !ci->nonsegmented )
  {
    CG_PlayerAxis( cent, cent->lerpAngles, legs.axis, torso.axis, head.axis );
  }
  else
  {
    //CG_PlayerNonSegAxis( cent, cent->lerpAngles, legs.axis );
    //FIXME(0xA5EA):merge
  }

  //set the correct frame (should always be dead)
  if ( cg_noPlayerAnims.integer )
  {
    legs.oldframe = legs.frame = torso.oldframe = torso.frame = 0;
  }
  else if ( ci->md5 )
  {
    if ( ci->gender == GENDER_NEUTER )
    {
      memset( &cent->pe.nonseg, 0, sizeof( lerpFrame_t ) );
      CG_RunCorpseLerpFrame( ci, &cent->pe.nonseg, NSPA_DEATH1 );
      legs.oldframe = cent->pe.nonseg.oldFrame;
      legs.frame = cent->pe.nonseg.frame;
      legs.backlerp = cent->pe.nonseg.backlerp;
    }
    else
    {
      memset( &cent->pe.legs, 0, sizeof( lerpFrame_t ) );
      CG_RunCorpseLerpFrame( ci, &cent->pe.legs, BOTH_DEATH1 );
      legs.oldframe = cent->pe.legs.oldFrame;
      legs.frame = cent->pe.legs.frame;
      legs.backlerp = cent->pe.legs.backlerp;
    }
  }
  else if ( !ci->nonsegmented )
  {
    memset( &cent->pe.legs, 0, sizeof( lerpFrame_t ) );
    CG_RunPlayerLerpFrame( ci, &cent->pe.legs, es->legsAnim, NULL, 1 );
    legs.oldframe = cent->pe.legs.oldFrame;
    legs.frame = cent->pe.legs.frame;
    legs.backlerp = cent->pe.legs.backlerp;

    memset( &cent->pe.torso, 0, sizeof( lerpFrame_t ) );
    CG_RunPlayerLerpFrame( ci, &cent->pe.torso, es->torsoAnim, NULL, 1 );
    torso.oldframe = cent->pe.torso.oldFrame;
    torso.frame = cent->pe.torso.frame;
    torso.backlerp = cent->pe.torso.backlerp;
  }
  else
  {
    memset( &cent->pe.nonseg, 0, sizeof( lerpFrame_t ) );
    CG_RunPlayerLerpFrame( ci, &cent->pe.nonseg, es->legsAnim, NULL, 1 );
    legs.oldframe = cent->pe.nonseg.oldFrame;
    legs.frame = cent->pe.nonseg.frame;
    legs.backlerp = cent->pe.nonseg.backlerp;
  }

  // add the shadow
  shadow = CG_PlayerShadow( cent, &shadowPlane, (class_t) es->clientNum );

  // get the player model information
  renderfx = RF_LIGHTING_ORIGIN; // use the same origin for all

  //
  // add the legs
  //
  if ( ci->md5 )
  {
    legs.hModel = ci->bodyModel;
    legs.customSkin = ci->bodySkin;
    legs.skeleton = legsSkeleton;
    CG_TransformSkeleton( &legs.skeleton, ci->modelScale );
  }
  else if ( !ci->nonsegmented )
  {
    legs.hModel = ci->legsModel;
    legs.customSkin = ci->legsSkin;
  }
  else
  {
    legs.hModel = ci->nonSegModel;
    legs.customSkin = ci->nonSegSkin;
  }

  VectorCopy( origin, legs.origin );

  VectorCopy( origin, legs.lightingOrigin );
  legs.shadowPlane = shadowPlane;
  legs.renderfx = renderfx;
  legs.origin[ 2 ] += BG_ClassModelConfig( es->clientNum )->zOffset;
  VectorCopy( legs.origin, legs.oldorigin );  // don't positionally lerp at all

  //rescale the model
  scale = BG_ClassModelConfig( es->clientNum )->modelScale;

  if ( scale != 1.0f && !ci->md5 )
  {
    VectorScale( legs.axis[ 0 ], scale, legs.axis[ 0 ] );
    VectorScale( legs.axis[ 1 ], scale, legs.axis[ 1 ] );
    VectorScale( legs.axis[ 2 ], scale, legs.axis[ 2 ] );

    legs.nonNormalizedAxes = qtrue;
  }

  legs.altShaderIndex = CG_ALTSHADER_DEAD;
  trap_R_AddRefEntityToScene( &legs );

  // if the model failed, allow the default nullmodel to be displayed. Also, if MD5, no need to add other parts
  if ( !legs.hModel || ci->md5 )
  {
    CG_PlayerShadowEnd( );
    return;
  }

  if ( !ci->nonsegmented )
  {
    //
    // add the torso
    //
    torso.hModel = ci->torsoModel;

    if ( !torso.hModel )
    {
      CG_PlayerShadowEnd( );
      return;
    }

    torso.customSkin = ci->torsoSkin;

    VectorCopy( origin, torso.lightingOrigin );

    CG_PositionRotatedEntityOnTag( &torso, &legs, ci->legsModel, "tag_torso" );

    torso.shadowPlane = shadowPlane;
    torso.renderfx = renderfx;

    torso.altShaderIndex = CG_ALTSHADER_DEAD;
    trap_R_AddRefEntityToScene( &torso );

    //
    // add the head
    //
    head.hModel = ci->headModel;

    if ( !head.hModel )
    {
      CG_PlayerShadowEnd( );
      return;
    }

    head.customSkin = ci->headSkin;

    VectorCopy( origin, head.lightingOrigin );

    CG_PositionRotatedEntityOnTag( &head, &torso, ci->torsoModel, "tag_head" );

    head.shadowPlane = shadowPlane;
    head.renderfx = renderfx;

    head.altShaderIndex = CG_ALTSHADER_DEAD;
    trap_R_AddRefEntityToScene( &head );
  }
}
#endif
//FIXME(0xA5EA): merge
//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity(centity_t * cent)
{
  cent->errorTime = -99999;	// guarantee no error decay added
  cent->extrapolated = qfalse;

  CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ],
                     &cent->pe.legs, cent->currentState.legsAnim, &legsSkeleton );
  CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ],
                     &cent->pe.torso, cent->currentState.torsoAnim, &torsoSkeleton );
  CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ],
                     &cent->pe.nonseg, cent->currentState.legsAnim, &legsSkeleton );

  BG_EvaluateTrajectory(&cent->currentState.pos, cg.time, cent->lerpOrigin);
  BG_EvaluateTrajectory(&cent->currentState.apos, cg.time, cent->lerpAngles);

  VectorCopy(cent->lerpOrigin, cent->rawOrigin);
  VectorCopy(cent->lerpAngles, cent->rawAngles);

  memset(&cent->pe.legs, 0, sizeof(cent->pe.legs));
  cent->pe.legs.yawAngle = cent->rawAngles[YAW];
  cent->pe.legs.yawing = qfalse;
  cent->pe.legs.pitchAngle = 0;
  cent->pe.legs.pitching = qfalse;

  memset(&cent->pe.torso, 0, sizeof(cent->pe.torso));
  cent->pe.torso.yawAngle = cent->rawAngles[YAW];
  cent->pe.torso.yawing = qfalse;
  cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
  cent->pe.torso.pitching = qfalse;

  memset( &cent->pe.nonseg, 0, sizeof( cent->pe.nonseg ) );
  cent->pe.nonseg.yawAngle = cent->rawAngles[ YAW ];
  cent->pe.nonseg.yawing = qfalse;
  cent->pe.nonseg.pitchAngle = cent->rawAngles[ PITCH ];
  cent->pe.nonseg.pitching = qfalse;

  if(cg_debugPosition.integer)
  {
    CG_Printf("%i ResetPlayerEntity yaw=%f\n", cent->currentState.number, cent->pe.torso.yawAngle);
  }
}

/*
===============
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups
Also called by CG_Missile for quad rockets, but nobody can tell...
===============
*/
void CG_AddRefEntityWithPowerups(refEntity_t *ent, entityState_t *state, int team)
{

#ifdef USE_KAMIKAZE
  if ( state->eFlags & EF_KAMIKAZE ) {
      if (team == TEAM_NIKKIS)
          ent->customShader = cgs.media.blueKamikazeShader;
      else
          ent->customShader = cgs.media.redKamikazeShader;
      trap_R_AddRefEntityToScene( ent );
  }
  else
#endif
  {
    trap_R_AddRefEntityToScene(ent);
  }
}


/*
=================
CG_DrawPlayerCollision

Draws a bounding box around a player.  Called from CG_Player.
=================
*/
#if 0
void CG_PlayerDisconnect( vec3_t org )
{
  particleSystem_t *ps;

  trap_S_StartSound( org, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.disconnectSound );

  ps = CG_SpawnNewParticleSystem( cgs.media.disconnectPS );

  if ( CG_IsParticleSystemValid( &ps ) )
  {
    CG_SetAttachmentPoint( &ps->attachment, org );
    CG_AttachToPoint( &ps->attachment );
  }
}

centity_t *CG_GetLocation( vec3_t origin )
{
  int i;
  centity_t *eloc, *best;
  float bestlen, len;

  best = NULL;
  bestlen = 3.0f * 8192.0f * 8192.0f;

  for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
  {
    eloc = &cg_entities[i];

    if (!eloc->valid || eloc->currentState.eType != ET_LOCATION)
      continue;

    len = DistanceSquared(origin, eloc->lerpOrigin);

    if (len > bestlen)
      continue;

    if (!trap_R_inPVS(origin, eloc->lerpOrigin))
      continue;

    bestlen = len;
    best = eloc;
  }

  return best;
}

centity_t *CG_GetPlayerLocation( void )
{
  vec3_t    origin;

  VectorCopy( cg.predictedPlayerState.origin, origin );
  return CG_GetLocation( origin );
}

void CG_InitClasses( void )
{
  int i;

  Com_Memset(cg_classes, 0, sizeof(cg_classes));

  for (i = PCL_NONE + 1; i < PCL_NUM_CLASSES; i++)
  {
    const char *icon = BG_Class(i)->icon;

    if (icon)
    {
      cg_classes[i].classIcon = trap_R_RegisterShader(icon, RSF_DEFAULT);

      if (!cg_classes[i].classIcon)
      {
        Com_Printf      ( S_ERROR "Failed to load class icon file %s\n", icon );
      }
    }
  }
}
#endif
//FIXME(0xA5EA): merge

