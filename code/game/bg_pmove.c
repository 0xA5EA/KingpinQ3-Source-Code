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
// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate
#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

//#define HYPO_DEBUG_MOVE
const vec3_t playerMins = { -16, -16, -24 };
const vec3_t playerMaxs = { 16, 16, 48 }; 
//FIXME(0xA5EA): added different player min max, das that fit ??


pmove_t *pm;
pml_t pml;

// movement parameters
float pm_stopspeed = 50.0f; //hypov8 was 100
float pm_duckScale = 0.45f;
float pm_swimScale = 0.50f;
float pm_wadeScale = 0.70f;

//float	pm_accelerate = 10.0f;
//float	pm_airaccelerate = 1.0f;
float pm_airaccelerate    = 4.0f;        /* 0xA5EA */
float pm_wateraccelerate  = 4.0f;
float pm_flyaccelerate    = 8.0f;
float pm_ladderAccelerate = 3000;        // The acceleration to friction ratio is 1:1

#if 0
float pm_maxspeed     = 320;             /* 0xA5EA  */
float pm_maxwalkspeed = 160;             /* 0xA5EA */
float pm_duckspeed    = 100;             /* 0xA5EA  */
#endif

/*
#define	sv_stopspeed		100
#define sv_friction			6
#define sv_waterfriction	1
*/
float pm_friction       = 8;             // initially 6
float pm_waterfriction  = 1;
float pm_ladderfriction = 2000;          // war 3000 0xA5EA Friction is high enough so you don't slip down

//float	pm_friction = 6.0f;
//float	pm_waterfriction = 1.0f;

//float	pm_flightfriction = 3.0f;
float pm_flightfriction    = 6.0f;       /* 0xA5EA */
float pm_spectatorfriction = 5.0f;
int c_pmove                = 0;

float pm_airstopaccelerate = 2.5;
float pm_aircontrol        = 150;

float pm_wishspeed = 30;

float pm_jump_velocity = 280;
float dash_height      = 170;
float wj_height        = 300;
float dash_ups         = 450;

float wj_angle    = 45.0;
float wj_cooldown = 700;

int pm_dash_time = 1000; // delay in milliseconds
/* 0xA5EA  */
#define NEW_AIRMOVE
#if 0                    /* 0xA5EA, aus kp, rotationale Reibung */
#define sv_stopspeed 100
#define sv_friction 6
#define sv_waterfriction 1

void SV_AddRotationalFriction(edict_t *ent)
{
  int n;
  float adjustment;

  VectorMA(ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);
  adjustment = FRAMETIME * sv_stopspeed * sv_friction;
  for (n = 0; n < 3; n++)
  {
    if (ent->avelocity[n] > 0)
    {
      ent->avelocity[n] -= adjustment;
      if (ent->avelocity[n] < 0)
        ent->avelocity[n] = 0;
    }
    else
    {
      ent->avelocity[n] += adjustment;
      if (ent->avelocity[n] > 0)
        ent->avelocity[n] = 0;
    }
  }
}
#endif


/*
===============
PM_AddEvent
===============
*/
void PM_AddEvent(int newEvent)
{
  BG_AddPredictableEventToPlayerstate(newEvent, 0, pm->ps);
  //pm->ps->eventParms[];
}

void PM_AddEvent2(int newEvent, int eventParms)
{
  BG_AddPredictableEventToPlayerstate(newEvent, eventParms, pm->ps);
}


/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt(int entityNum)
{
  int i;

  if (entityNum == ENTITYNUM_WORLD)
    return;

  if (pm->numtouch == MAXTOUCH)
    return;

  // see if it is already added
  for (i = 0; i < pm->numtouch; i++)
  {
    if (pm->touchents[i] == entityNum)
      return;
  }

  // add it
  pm->touchents[pm->numtouch] = entityNum;
  pm->numtouch++;
}

/*
===================
PM_StartTorsoAnim
===================
*/
static void PM_StartTorsoAnim(int anim)
{
  if (pm->ps->pm_type >= PM_DEAD)
    return;

  pm->ps->torsoAnim = ((pm->ps->torsoAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT) | anim;
}
#if 1 //daemon .50
/*
===================
PM_StartWeaponAnim
===================
*/
static void PM_StartWeaponAnim( int anim )
{
  if (pm->ps->pm_type >= PM_DEAD)
    return;


	pm->ps->weaponAnim = ( ( pm->ps->weaponAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
	                     | anim;
}
#endif

static void PM_StartLegsAnim(int anim)
{
  if (pm->ps->pm_type >= PM_DEAD)
    return;

  if (pm->ps->legsTimer > 0)
    return;     // a high priority animation is running

  pm->ps->legsAnim = ((pm->ps->legsAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT) | anim;
}

static void PM_ContinueLegsAnim(int anim)
{
  if ((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == anim)
    return;

  if (pm->ps->legsTimer > 0)
    return;     // a high priority animation is running

  PM_StartLegsAnim(anim);
}

static void PM_ContinueTorsoAnim(int anim)
{
  if ((pm->ps->torsoAnim & ~ANIM_TOGGLEBIT) == anim)
    return;

  if (pm->ps->torsoTimer > 0)
    return;     // a high priority animation is running

  PM_StartTorsoAnim(anim);
}

#if 1 //daemon .50
/*
===================
PM_ContinueWeaponAnim
===================
*/
static void PM_ContinueWeaponAnim( int anim )
{
	if ( ( pm->ps->weaponAnim & ~ANIM_TOGGLEBIT ) == anim )
	{
		return;
	}

	PM_StartWeaponAnim( anim );
}
#endif

static void PM_ForceLegsAnim(int anim)
{
  pm->ps->legsTimer = 0;
  //PM_StartLegsAnim(anim); //hypov8 disable
  PM_ContinueLegsAnim(anim);
}
/*
//add hypov8 should stop previous animations?
static void PM_ForceTorsoAnim(int anim)
{
  pm->ps->torsoTimer = 0; 
  //PM_StartTorsoAnim(anim);// hypov8 disable
  PM_ContinueTorsoAnim(anim);
}
//end add
*/



/*
==================
PM_ClipVelocity
Slide off of the impacting surface
==================
*/
void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
  float backoff;
  float change;
  int i;

#if 0 //ndef KINGPIN_PLAYERMOVE //hypov8 causing wird walking constraints and stoping view up/down

#define STOP_EPSILON 0.1
  int blocked;
  blocked = 0;
  if (normal[2] > 0)
    blocked |= 1;                                                                                         // floor
  if (!normal[2])
    blocked |= 2;                                                                                         // step

  backoff = DotProduct(in, normal) * overbounce;
  for (i = 0; i < 3; i++)
  {
    change = normal[i] * backoff;
    out[i] = in[i] - change;
    if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
      out[i] = 0;
  }

  //return blocked;  //in Kingpin this return is not used, so comment it out
#elif 0

  backoff = DotProduct(in, normal);

  if (backoff < 0)
  {
    backoff *= overbounce;
  }
  else
  {
    backoff /= overbounce;
  }

  for (i = 0; i < 3; i++)
  {
    change = normal[i] * backoff;
    out[i] = in[i] - change;
  }

#else
	float t = -DotProduct( in, normal );//unvan .50
	VectorMA( in, t, normal, out );
#endif
}

/*
==================
PM_Friction (Reibung)
Handles both ground friction and water friction
==================
*/
static void PM_Friction(void)
{
  vec3_t vec;
  float *vel;
  float speed, newspeed, control;
  float drop;

  vel = pm->ps->velocity;

  VectorCopy(vel, vec);
  if (pml.walking)
  {
    vec[2] = 0;     // ignore slope movement // hab ich probiert, aendert nix
  }

  speed = VectorLength(vec);
  if (speed < 1)
  {
    vel[0] = 0;
    vel[1] = 0;     // allow sinking underwater
    // FIXME: still have z friction underwater?
    return;
  }

  drop = 0;

  // apply ground friction
  if (pm->waterlevel <= 1)
  {
    if (pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK))
    {
      // if getting knocked back, no friction
      if (!(pm->ps->pm_flags & PMF_TIME_KNOCKBACK))
      {
        control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop   += control * pm_friction * pml.frametime;
      }
    }
  }

  // apply water friction even if just wading
  if (pm->waterlevel)
    drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;

#ifdef  GT_USE_TA_TYPES
  // apply flying friction
  if (pm->ps->powerups[PW_FLIGHT])
    drop += speed * pm_flightfriction * pml.frametime;
#endif
  if (pml.ladder)                                         // If they're on a ladder...
    drop += speed * pm_ladderfriction * pml.frametime;  // Add ladder friction!

  if (pm->ps->pm_type == PM_SPECTATOR)
    drop += speed * pm_spectatorfriction * pml.frametime;

  // scale the velocity
  newspeed = speed - drop;
  if (newspeed < 0)
    newspeed = 0;

  newspeed /= speed;

  vel[0] = vel[0] * newspeed;
  vel[1] = vel[1] * newspeed;
  vel[2] = vel[2] * newspeed;
}

/*
==============
PM_Accelerate
Handles user intended acceleration
==============
*/
static void PM_Accelerate(vec3_t wishdir, float wishspeed, float accel)
{
#if 1
  // q2 style
  int i;
  float addspeed, accelspeed, currentspeed;

  currentspeed = DotProduct(pm->ps->velocity, wishdir);
  addspeed = wishspeed - currentspeed;

  if (addspeed <= 0)
    return;

  accelspeed = accel * pml.frametime * wishspeed;

  if (accelspeed > addspeed)
    accelspeed = addspeed;

  for (i = 0; i < 3; i++)
  {
    pm->ps->velocity[i] += accelspeed * wishdir[i];
  }
#else
  // proper way (avoids strafe jump maxspeed bug), but feels bad
  vec3_t wishVelocity;
  vec3_t pushDir;
  float pushLen;
  float canPush;

  VectorScale(wishdir, wishspeed, wishVelocity);
  VectorSubtract(wishVelocity, pm->ps->velocity, pushDir);
  pushLen = VectorNormalize(pushDir);

  canPush = accel * pml.frametime * wishspeed;
  if (canPush > pushLen)
  {
    canPush = pushLen;
  }

  VectorMA(pm->ps->velocity, canPush, pushDir, pm->ps->velocity);
#endif
}

/*
============
PM_CmdScale
Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale(usercmd_t *cmd)
{
  int max;
  float total;
  float scale;

  max = abs(cmd->forwardmove);
  if (abs(cmd->rightmove) > max)
    max = abs(cmd->rightmove);

  if (abs(cmd->upmove) > max)
    max = abs(cmd->upmove);

  if (!max)
    return 0;

  total = sqrt((float)(cmd->forwardmove * cmd->forwardmove + cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove));
  scale = (float)pm->ps->speed * max / (127.0 * total); //hypov8 todo: change to 127.9?
  //add hypov8 speed up noclips.. taking effect once enter level?
  	if(pm->ps->pm_type == PM_NOCLIP)
		scale *= 2;
	//else
		//scale *= 1;
		
//end add
  return scale;
}

/*
================
PM_SetMovementDir
Determine the rotation of the legs reletive
to the facing dir
================
*/
static void PM_SetMovementDir(void)
{
  if (pm->cmd.forwardmove || pm->cmd.rightmove)
  {
    if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0)
      pm->ps->movementDir = 0;
    else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0)
      pm->ps->movementDir = 1;
    else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0)
      pm->ps->movementDir = 2;
    else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0)
      pm->ps->movementDir = 3;
    else if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0)
      pm->ps->movementDir = 4;
    else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0)
      pm->ps->movementDir = 5;
    else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0)
      pm->ps->movementDir = 6;
    else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0)
      pm->ps->movementDir = 7;
  }
  else
  {
    // if they aren't actively going directly sideways,
    // change the animation to the diagonal so they
    // don't stop too crooked
    if (pm->ps->movementDir == 2)
      pm->ps->movementDir = 1;
    else if (pm->ps->movementDir == 6)
      pm->ps->movementDir = 7;
  }
}

/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump(void)
{
  
  if (pm->ps->pm_flags & PMF_RESPAWNED)
    return qfalse;      // don't allow jump until all buttons are up

  if (pm->cmd.upmove < 10)
    return qfalse;

  // must wait for jump to be released
  if (pm->ps->pm_flags & PMF_JUMP_HELD)
  {
    pm->cmd.upmove = 0;
    return qfalse;
  }

  if (!pml.groundPlane) //add hypov8
  {
    if (pm->debugLevel)
		Com_Printf("PM_CheckJump. Allready in air \n");
    return qfalse;
  }

  pml.groundPlane = qfalse;              // jumping away
  pml.walking     = qfalse;
  //FIXME: 0xA5EA, reset pml.ladder ?
  pm->ps->pm_flags |= PMF_JUMP_HELD;

  pm->ps->groundEntityNum = ENTITYNUM_NONE;

  // 0xA5EA, doublejump ?
  // add the to current velocity
  // hypov8 todo: add some forward velocity to?
  ////////////////////////////////////////////////////////////////////////////////////////
  // hypov8 slope jump 
  if (pm->ps->velocity[2] >= 0.1f)
  {											//hypo slow down double jump acceleration by half
	  pm->ps->velocity[2] = JUMP_VELOCITY + (pm->ps->velocity[2]*0.5f); //hypov8 add some height to ram jumps
	  if (pm->debugLevel)
		  Com_Printf("slope jump used\n");
  }
  else
  {
    pm->ps->velocity[2] = JUMP_VELOCITY;
	  if (pm->debugLevel)
  	    Com_Printf("regular jump used\n");
  }

  PM_AddEvent(EV_JUMP);

  return qtrue;
 }


/*
=============
PM_CheckLadder
=============
*/
static void PM_CheckLadder(void)
{
  vec3_t flatforward, spot;
  trace_t trace;
//int anim;

  pml.ladder = qfalse;

  VectorCopy(pml.forward, flatforward);
  flatforward[2] = 0;
  VectorNormalize(flatforward);
  VectorMA(pm->ps->origin, 1, flatforward, spot);
  pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, spot, pm->ps->clientNum, MASK_PLAYERSOLID);

  if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER))
 	  pml.ladder = qtrue;
	//else /* unvan */ 
	 // pml.ladder = qfalse; /* unvan */ 

  //Com_Printf("Surfladder: %i, pml.ladder: %i \n",(trace.surfaceFlags & SURF_LADDER), pml.ladder);
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean PM_CheckWaterJump(void)
{
  vec3_t spot;
  int cont;
  vec3_t flatforward;

  if (pm->ps->pm_time)
    return qfalse;

  // check for water jump
  if (pm->waterlevel != 2)
    return qfalse;

  flatforward[0] = pml.forward[0];
  flatforward[1] = pml.forward[1];
  flatforward[2] = 0;
  VectorNormalize(flatforward);

  VectorMA(pm->ps->origin, 30, flatforward, spot);
  spot[2] += 4;
  cont = pm->pointcontents(spot, pm->ps->clientNum);

  if (!(cont & CONTENTS_SOLID))
    return qfalse;

  spot[2] += 16;
  cont = pm->pointcontents(spot, pm->ps->clientNum);
  if (cont)
    return qfalse;

  // jump out of water
  VectorScale(pml.forward, 200, pm->ps->velocity);
  pm->ps->velocity[2] = 350;

  pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
  pm->ps->pm_time   = 2000;

  return qtrue;
}

//============================================================================
/*
===================
PM_WaterJumpMove
Flying out of the water
===================
*/
static void PM_WaterJumpMove(void)
{
  // waterjump has no control, but falls
  PM_StepSlideMove(qtrue, qfalse);

  pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  if (pm->ps->velocity[2] < 0)
  {
    // cancel as soon as we are falling down again
    pm->ps->pm_flags &= ~PMF_ALL_TIMES;
    pm->ps->pm_time   = 0;
  }
}

/*
===================
PM_WaterMove
===================
*/
static void PM_WaterMove(void)
{
  int i;
  vec3_t wishvel;
  float wishspeed;
  vec3_t wishdir;
  float scale;
  float vel;

  if (PM_CheckWaterJump())
  {
    PM_WaterJumpMove();
    return;
  }
#if 0
  // jump = head for surface
  if (pm->cmd.upmove >= 10)
  {
    if (pm->ps->velocity[2] > -300)
    {
      if (pm->watertype == CONTENTS_WATER)
      {
        pm->ps->velocity[2] = 100;
      }
      else if (pm->watertype == CONTENTS_SLIME)
      {
        pm->ps->velocity[2] = 80;
      }
      else
      {
        pm->ps->velocity[2] = 50;
      }
    }
  }
#endif
  PM_Friction();

  scale = PM_CmdScale(&pm->cmd);
  //
  // user intentions
  //
  if (!scale)
  {
    wishvel[0] = 0;
    wishvel[1] = 0;
    wishvel[2] = -60;       // sink towards bottom
  }
  else
  {
    for (i = 0; i < 3; i++)
      wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove + scale * pml.right[i] * pm->cmd.rightmove;

    wishvel[2] += scale * pm->cmd.upmove;
  }

  VectorCopy(wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);

  if (wishspeed > pm->ps->speed * pm_swimScale)
    wishspeed = pm->ps->speed * pm_swimScale;

  PM_Accelerate(wishdir, wishspeed, pm_wateraccelerate);

  // make sure we can go up slopes easily under water
  if (pml.groundPlane && DotProduct(pm->ps->velocity, pml.groundTrace.plane.normal) < 0)
  {
    vel = VectorLength(pm->ps->velocity);
    // slide along the ground plane
    PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);

    VectorNormalize(pm->ps->velocity);
    VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
  }

  PM_SlideMove(qfalse);
  
  //add hypov8======
 	if (pm->ps->velocity[2] >= 50 || pm->xyspeed >= 50)
	{
	PM_ContinueLegsAnim(LEGS_SWIM);
}
	else if (pm->ps->velocity[2] < 50 || pm->xyspeed < 50)
	{
      PM_StartLegsAnim(LEGS_IDLE);
	} 
	//==============
}

#if 0 // def MISSIONPACK
/*
===================
PM_InvulnerabilityMove
Only with the invulnerability powerup
===================
*/
static void PM_InvulnerabilityMove(void)
{
  pm->cmd.forwardmove = 0;
  pm->cmd.rightmove   = 0;
  pm->cmd.upmove      = 0;
  VectorClear(pm->ps->velocity);
}
#endif

/*
===================
PM_FlyMove
Geschwindigkeit = velocity
Spectator mode
===================
*/
static void PM_FlyMove(void)
{
  int i;
  vec3_t wishvel;
  float wishspeed;
  vec3_t wishdir;
  float scale;

  // normal slowdown
  PM_Friction();

  scale = PM_CmdScale(&pm->cmd);

  // user intentions
  if (!scale)
  {
    wishvel[0] = 0;
    wishvel[1] = 0;
    wishvel[2] = 0;
  }
  else
  {
    for (i = 0; i < 3; i++)
      wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove + scale * pml.right[i] * pm->cmd.rightmove;

    wishvel[2] += scale * pm->cmd.upmove;
  }

  VectorCopy(wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);

  PM_Accelerate(wishdir, wishspeed, pm_flyaccelerate);

  PM_StepSlideMove(qfalse, qfalse);
}

#if 0
//=============
//PM_CheckSpecialMovement
//=============
static void PM_CheckSpecialMovement(void)
{
  vec3_t spot;
  int cont;
  trace_t trace;

  if (pm->ps->pm_time)
    return;

  //if( pm->s.pm_time )
  //	return;

  pml.ladder = qfalse;

#if 0   //0xA5EA: FIXME:
  // check for ladder
  VectorMA(pml.origin, 1, pml.flatforward, spot);

  pm->trace(&trace, pml.origin, min, max, dir, pm->ps->clientNum, pm->tracemask);
  //pm->trace( &trace, pml.origin, min, max, dir, pm->passent, pm->tracemask );

  //GS_Trace( &trace, pml.origin, pm->mins, pm->maxs, spot, pm->passent, pm->contentmask );
  pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);
  if ((trace.fraction < 1) && (trace.surfFlags & SURF_LADDER))
    pml.ladder = qtrue;

  // check for water jump
  if (pm->waterlevel != 2)
    return;

  VectorMA(pml.origin, 30, pml.flatforward, spot);
  spot[2] += 4;
  cont     = GS_PointContents(spot);
  if (!(cont & CONTENTS_SOLID))
    return;

  spot[2] += 16;
  cont     = GS_PointContents(spot);
  if (cont)
    return;

  // jump out of water
  VectorScale(pml.flatforward, 50, pm->ps->velocity);
  pm->ps->velocity[2] = 350;

  pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
  pm->s.pm_time     = 255;
#endif
}
#endif
//=============
//PM_AddCurrents
//=============
#if 0
static void PM_AddCurrents(vec3_t wishvel)
{
  // account for ladders
  if (pml.ladder && fabs(pm->ps->velocity[2]) <= 200)
  {
    if ((pm->ps->viewangles[PITCH] <= -15) && (pml.fordwardPush > 0))
      wishvel[2] = 200;
    else if ((pm->ps->viewangles[PITCH] >= 15) && (pml.fordwardPush > 0))
      wishvel[2] = -200;
    else if (pml.upPush > 0)
      wishvel[2] = 200;
    else if (pml.upPush < 0)
      wishvel[2] = -200;
    else
      wishvel[2] = 0;

    // limit horizontal speed when on a ladder
    clamp(wishvel[0], -25, 25);
    clamp(wishvel[1], -25, 25);
  }
}
#endif
/*
=================================================
PM_Aircontrol
bunnyhop related 0xA5EA
=================================================
*/
static void PM_Aircontrol(pmove_t *pm, vec3_t wishdir, float wishspeed)
{
  float zspeed, speed, dot, k;
  int i;
  float smove;

  // accelerate
  //fmove = pml.fordwardPush;
  //smove = pml.sidePush;

  //fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  // can't control movement if not moveing forward or backward
  if ((smove > 0 || smove < 0) || (wishspeed == 0.0))
    return;

  zspeed              = pm->ps->velocity[2];
  pm->ps->velocity[2] = 0;
  //zspeed = pml.velocity[2];
  //pml.velocity[2] = 0;
  speed = VectorNormalize(pm->ps->velocity);


  dot = DotProduct(pm->ps->velocity, wishdir);
  k   = 32;
  k  *= pm_aircontrol * dot * dot * pml.frametime;


  if (dot > 0)                                                                // we can't change direction while slowing down
  {
    for (i = 0; i < 2; i++)
      pm->ps->velocity[i] = pm->ps->velocity[i] * speed + wishdir[i] * k;
    VectorNormalize(pm->ps->velocity);
  }

  for (i = 0; i < 2; i++)
    pm->ps->velocity[i] *= speed;

  pm->ps->velocity[2] = zspeed;
}

/*
=================================================
PM_NewAirMove
bunnyhop related 0xA5EA
=================================================
*/
static void PM_AirMove(void)
{
  int i;
  vec3_t wishvel;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;
//	float		maxspeed;
  float accel;
  float wishspeed2;

  fmove = pm->cmd.forwardmove;        // 127 forward, -127 backwards    //nur walk gedrückt
  smove = pm->cmd.rightmove;          // 127 righwards, -127 leftwards  // nur wenn links oder rechts gedrückt
  // d.h. bei bh ist fmove 0 und |smove| = 128
  // woher kommt dann der vorwärdsmove
  //fmove = pml.fordwardPush;
  //smove = pml.sidePush;
  // set the movementDir so clients can rotate the legs for strafing
  //PM_SetMovementDir(); //note hypov8
//	Com_Printf("NewAirmove: fmove %i; smove %i\n", pm->cmd.forwardmove, pm->cmd.rightmove );
  for (i = 0; i < 2; i++)
  {
    wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
  }
  wishvel[2] = 0.0f;                     //0xA5EA, test jump higher, auskommentieren nutzt nix

//	PM_AddCurrents( wishvel );		//FIXME: 0xA5EA, nur für ladders, also überflüssig

  VectorCopy(wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);


#if 0
  if (pm->ps->pm_flags & PMF_DUCKED)
  {
    maxspeed = pm_duckspeed;
  }
  else if (pm->cmd.buttons & BUTTON_WALKING)
  {
    maxspeed = pm_maxwalkspeed;
  }
  else
    maxspeed = pm_maxspeed;

  if (wishspeed > maxspeed)
  {
    wishspeed = maxspeed / wishspeed;
    VectorScale(wishvel, wishspeed, wishvel);
    wishspeed = maxspeed;
  }
#endif
  //Com_Printf("pm->ps->groundEntityNum %d\n");
#if 0
  if (pml.ladder)
  {
    PM_Accelerate(wishdir, wishspeed, PM_ACCELERATE);

    if (!wishvel[2])
    {
      if (pm->ps->velocity[2] > 0)
      {
        pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
        if (pm->ps->velocity[2] < 0)
          pm->ps->velocity[2] = 0;
      }
      else
      {
        pm->ps->velocity[2] += pm->ps->gravity * pml.frametime;
        if (pm->ps->velocity[2] > 0)
          pm->ps->velocity[2] = 0;
      }
    }

    PM_StepSlideMove(qtrue);
  }

  //else if( pm->ps->groundEntityNum != -1 )
  else if (pm->ps->groundEntityNum != ENTITYNUM_NONE)
  {                                                                                                                                                                                    // walking on ground
    if (pm->ps->velocity[2] > 0)
      pm->ps->velocity[2] = 0;                                                                                                                                                     //!!! this is before the accel
    PM_Accelerate(wishdir, wishspeed, PM_ACCELERATE);

    // fix for negative trigger_gravity fields
    if (pm->ps->gravity > 0)
    {
      if (pm->ps->velocity[2] > 0)
        pm->ps->velocity[2] = 0;
    }
    else
      pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;

    if (!pm->ps->velocity[0] && !pm->ps->velocity[1])
      return;

    PM_StepSlideMove(qtrue);
  }
  else
#endif

  { 
    // Air Control
    wishspeed2 = wishspeed;
    if (DotProduct(pm->ps->velocity, wishdir) < 0)
      accel = pm_airstopaccelerate;
    else
      accel = 1;                                                                                                                      //instead of pm_airaccelerate, (correcting the forward bug);

    if ((smove > 0 || smove < 0) && fmove == 0)                                                                                         //0xA5EA, bh-buttons pressed ?
    {
      if (wishspeed > pm_wishspeed)
        wishspeed = pm_wishspeed;

      accel = PM_STRAFE_ACCELERATE;
    }

	//add hypov8 makes player stop in mid air forwared/back, like kp1
	if ((fmove > 0 || fmove < 0) && smove == 0)                                                                                         //0xA5EA, bh-buttons pressed ?
	{
		if (wishspeed > pm_wishspeed)
			wishspeed = pm_wishspeed;

		accel = PM_STRAFE_ACCELERATE;
	}
	//end add hypov8


    // Air control
    PM_Accelerate(wishdir, wishspeed, accel);
    if (pm_aircontrol)
      PM_Aircontrol(pm, wishdir, wishspeed2);

    // add gravity
    //pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
    //PM_StepSlideMove();
	//trap_Cvar_Set(); sv_maxvelocity
	//int checkMaxSpeed = DotProduct(pm->ps->velocity, wishdir);
	
//	if (checkMaxSpeed > cvar sv_maxvelocity )

	PM_StepSlideMove(qtrue, qfalse);
  }//end airmove


}


/*
===================
PM_LadderMove()
by: Calrathan [Arthur Tomlin]
Right now all I know is that this works for VERTICAL ladders.
Ladders with angles on them (urban2 for AQ2) haven't been tested.
===================
*/
static void PM_LadderMove(void)
{
  int i;
  vec3_t wishvel;
  float wishspeed;
  vec3_t wishdir;
  float scale;
  float vel;

  PM_Friction();

  scale = PM_CmdScale(&pm->cmd);

  // user intentions [what the user is attempting to do]
  if (!scale)
  {
    VectorClear(wishvel);
    //	wishvel[0] = 0;
    //	wishvel[1] = 0;
    //	wishvel[2] = 0;
  }
  else
  {
    // if they're trying to move... lets calculate it
    for (i = 0; i < 3; i++)
    {
      wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove + scale * pml.right[i] * pm->cmd.rightmove;
    }
    wishvel[2] += scale * pm->cmd.upmove;
  }

  VectorCopy(wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);

  if (wishspeed > pm->ps->speed * PM_LADDERSCALE)
    wishspeed = pm->ps->speed * PM_LADDERSCALE;

  PM_Accelerate(wishdir, wishspeed, pm_ladderAccelerate);

  // This SHOULD help us with sloped ladders, but it remains untested.
  if (pml.groundPlane && DotProduct(pm->ps->velocity, pml.groundTrace.plane.normal) < 0)
  {
    vel = VectorLength(pm->ps->velocity);
    // slide along the ground plane [the ladder section under our feet]
    PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal,  pm->ps->velocity, OVERCLIP);

    VectorNormalize(pm->ps->velocity);
    VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
  }
  PM_SlideMove(qfalse); // move without gravity
  
if (pm->ps->velocity[2] <= -2)
     {
       PM_ContinueLegsAnim(BOTH_LDR_DN);
	   PM_ContinueTorsoAnim(BOTH_LDR_DN);
      }
      else if (pm->ps->velocity[2] >= 2)
      {
        PM_ContinueLegsAnim(BOTH_LDR_UP);
		PM_ContinueTorsoAnim(BOTH_LDR_UP);
      }
	 else      
	  {
		PM_ContinueLegsAnim(BOTH_LADDER); //note hypov8 can animations pause then resume..
		PM_ContinueTorsoAnim(BOTH_LADDER);
#ifdef HYPO_DEBUG_MOVE		
        Com_Printf("LAD both IDLE ladder\n");
#endif		
	 }
}

/*
===================
PM_GrappleMove
===================
*/
static void PM_GrappleMove(void)
{
  vec3_t vel, v;
  float vlen;

  VectorScale(pml.forward, -16, v);
  VectorAdd(pm->ps->grapplePoint, v, v);
  VectorSubtract(v, pm->ps->origin, vel);
  vlen = VectorLength(vel);
  VectorNormalize(vel);

  if (vlen <= 100)
    VectorScale(vel, 10 * vlen, vel);
  else
    VectorScale(vel, 800, vel);

  VectorCopy(vel, pm->ps->velocity);

  pml.groundPlane = qfalse;
}

/*
===================
PM_WalkMove
===================
*/
static void PM_WalkMove(void)
{
  int i;
  vec3_t wishvel;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;
  float scale;
  usercmd_t cmd;
  float accelerate;
  float vel;

  //Com_Printf("PM_WalkMove\n");
  if (pm->waterlevel > 2 && DotProduct(pml.forward, pml.groundTrace.plane.normal) > 0)
  {
    // begin swimming
    PM_WaterMove();
    return; //hypov8 merge: todo: this is disabled in kpq3. more checks?
  }

  if (PM_CheckJump())
  {
    // jumped away
    if (pm->waterlevel > 1)
    {
      PM_WaterMove();
    }
    else
    {
      PM_AirMove(); // NEW_AIRMOVE
    }
    return; // area hatte ich mal auskommentiert
  }

  PM_Friction();

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  cmd   = pm->cmd;
  scale = PM_CmdScale(&cmd);

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();

  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2]   = 0;

  // project the forward and right directions onto the ground plane
  PM_ClipVelocity(pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP);
  PM_ClipVelocity(pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);
  //
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  for (i = 0; i < 3; i++)
  {
    wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
  }
  // when going up or down slopes the wish velocity should Not be zero
//	wishvel[2] = 0;

  VectorCopy(wishvel, wishdir);
  wishspeed  = VectorNormalize(wishdir);
  wishspeed *= scale;

  // clamp the speed lower if ducking
  if (pm->ps->pm_flags & PMF_DUCKED)
  {
    if (wishspeed > pm->ps->speed * pm_duckScale)
      wishspeed = pm->ps->speed * pm_duckScale;
  }

  // clamp the speed lower if wading or walking on the bottom
  if (pm->waterlevel)
  {
    float waterScale;

    waterScale = pm->waterlevel / 3.0;
    waterScale = 1.0 - (1.0 - pm_swimScale) * waterScale;
    if (wishspeed > pm->ps->speed * waterScale)
      wishspeed = pm->ps->speed * waterScale;
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if ((pml.groundTrace.surfaceFlags & SURF_SLICK) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK)
    accelerate = pm_airaccelerate;
  else
    accelerate = PM_ACCELERATE;

  PM_Accelerate(wishdir, wishspeed, accelerate);

  //Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
  //Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

  if ((pml.groundTrace.surfaceFlags & SURF_SLICK) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK)
    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  else
  {
    // don't reset the z velocity for slopes
//		pm->ps->velocity[2] = 0;
  }

  //vel = VectorLength(pm->ps->velocity);

  // slide along the ground plane
  PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);

  // don't decrease velocity when going up or down a slope
  //VectorNormalize(pm->ps->velocity);
  //VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

  // don't do anything if standing still
  if (!pm->ps->velocity[0] && !pm->ps->velocity[1])
    return;

  PM_StepSlideMove(qfalse, qfalse);

  //Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));
}

/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove(void)
{
  float forward;

  if (!pml.walking)
    return;

  // extra friction

  forward  = VectorLength(pm->ps->velocity);
  forward -= 20;
  if (forward <= 0)
  {
    VectorClear(pm->ps->velocity);
  }
  else
  {
    VectorNormalize(pm->ps->velocity);
    VectorScale(pm->ps->velocity, forward, pm->ps->velocity);
  }
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove(void)
{
  float speed, drop, friction, control, newspeed;
  int i;
  vec3_t wishvel;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;
  float scale;

  pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

  // friction

  speed = VectorLength(pm->ps->velocity);
  if (speed < 1)
  {
    VectorCopy(vec3_origin, pm->ps->velocity);
  }
  else
  {
    drop = 0;

    friction = pm_friction * 1.5; // extra friction
    control  = speed < pm_stopspeed ? pm_stopspeed : speed;
    drop    += control * friction * pml.frametime;

    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0)
      newspeed = 0;
    newspeed /= speed;

    VectorScale(pm->ps->velocity, newspeed, pm->ps->velocity);
  }

  // accelerate
  scale = PM_CmdScale(&pm->cmd);

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  for (i = 0; i < 3; i++)
    wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
  wishvel[2] += pm->cmd.upmove;

  VectorCopy(wishvel, wishdir);
  wishspeed  = VectorNormalize(wishdir);
  wishspeed *= scale;

  PM_Accelerate(wishdir, wishspeed, PM_ACCELERATE);

  // move
  VectorMA(pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
    PM_StartLegsAnim(LEGS_IDLE);
	PM_StartTorsoAnim(TORSO_STAND);
  
}

//============================================================================

/*
================
PM_FootstepForSurface
Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface(void)
{
  if (pml.groundTrace.surfaceFlags & SURF_NOSTEPS)
    return 0;
  if (pml.groundTrace.surfaceFlags & SURF_METALSTEPS)
    return EV_FOOTSTEP_METAL;
  else if (pml.groundTrace.surfaceFlags & SURF_GRAVEL)
    return EV_FOOTSTEP_GRAVEL;
  else if (pml.groundTrace.surfaceFlags & SURF_METALLIGHT)
    return EV_FOOTSTEP_METALLIGHT;
  else if (pml.groundTrace.surfaceFlags & SURF_RUG)
    return EV_FOOTSTEP_RUG;
  else if (pml.groundTrace.surfaceFlags & SURF_SNOW)
    return EV_FOOTSTEP_SNOW;
  else if (pml.groundTrace.surfaceFlags & SURF_WOOD)
    return EV_FOOTSTEP_WOOD;
  else if (pml.groundTrace.surfaceFlags & SURF_MARBLE)
    return EV_FOOTSTEP_MARBLE;
  else if (pml.groundTrace.surfaceFlags & SURF_TIN)
    return EV_FOOTSTEP_TIN;
  else if (pml.groundTrace.surfaceFlags & SURF_GRASS)
    return EV_FOOTSTEP_CRASS;
  else if (pml.groundTrace.surfaceFlags & SURF_GLASS)
    return EV_FOOTSTEP_GLASS;
  else if (pml.groundTrace.surfaceFlags & SURF_LADDER)
    return EV_FOOTSTEP_LADDER;
  else
    return EV_FOOTSTEP;
}

/*
=================
PM_CrashLand
Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand(void)
{
  float delta;
  float dist;
  float vel, acc;
  float t;
  float a, b, c, den;

  // decide which landing animation to use
  if (pm->ps->pm_flags & PMF_BACKWARDS_JUMP)
    PM_ForceLegsAnim(LEGS_LANDB);
  else
    PM_ForceLegsAnim(LEGS_LAND);

  pm->ps->legsTimer = TIMER_LAND;

  // calculate the exact velocity on landing
  dist = pm->ps->origin[2] - pml.previous_origin[2];
  vel  = pml.previous_velocity[2];
  acc  = -pm->ps->gravity;
  a = acc / 2;
  b = vel;
  c = -dist;
  den =  b * b - 4 * a * c;

  if (den < 0)
    return;

  t = (-b - sqrt(den)) / (2 * a);

  delta = vel + t * acc;
  delta = delta * delta * 0.0001;

  // ducking while falling doubles damage
  if (pm->ps->pm_flags & PMF_DUCKED)
    delta *= 2;

  // never take falling damage if completely underwater
  if (pm->waterlevel == 3)
    return;

  // reduce falling damage if there is standing water
  if (pm->waterlevel == 2)
    delta *= 0.25;

  if (pm->waterlevel == 1)
    delta *= 0.5;

  if (delta < 1)
    return;

  // create a local entity event to play the sound

  // SURF_NODAMAGE is used for bounce pads where you don't ever
  // want to take damage or play a crunch sound
  if (!(pml.groundTrace.surfaceFlags & SURF_NODAMAGE))
  {
    if (delta > 200)
      BG_AddPredictableEventToPlayerstate(EV_FALL_FAR, pml.groundTrace.surfaceFlags, pm->ps);          /* 0xA5EA */ /*hypov8 fall damage close to old kp.*/
    else if (delta > 120)
    {
      // this is a pain grunt, so don't play it if dead
      if (pm->ps->stats[STAT_HEALTH] > 0)
        BG_AddPredictableEventToPlayerstate(EV_FALL_MEDIUM, pml.groundTrace.surfaceFlags, pm->ps);   /* 0xA5EA */ /*hypov8 fall damage close to old kp.*/
    }
    else if (delta > 45)                                                                                 /* war 7 */
    {
		BG_AddPredictableEventToPlayerstate(EV_FALL_SHORT, pml.groundTrace.surfaceFlags, pm->ps);        /* 0xA5EA */ /*hypov8 fall damage close to old kp.*/
    }
    else
    {
      PM_AddEvent(PM_FootstepForSurface());
    }
  }
  // start footstep cycle over
  pm->ps->bobCycle = 0;
}

/*
=============
PM_CheckStuck
=============
*/
/*
void PM_CheckStuck(void) {
    trace_t trace;

    pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
    if (trace.allsolid) {
        //int shit = qtrue;
    }
}
*/

/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid(trace_t *trace)
{
  int i, j, k;
  vec3_t point;

  if (pm->debugLevel)
    Com_Printf("%i:allsolid\n", c_pmove);

  // jitter around
  for (i = -1; i <= 1; i++)
  {
    for (j = -1; j <= 1; j++)
    {
      for (k = -1; k <= 1; k++)
      {
        VectorCopy(pm->ps->origin, point);
        point[0] += (float)i;
        point[1] += (float)j;
        point[2] += (float)k;
        pm->trace(trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
        if (!trace->allsolid)
        {
          point[0] = pm->ps->origin[0];
          point[1] = pm->ps->origin[1];
          point[2] = pm->ps->origin[2] - 0.25;

          pm->trace(trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
          pml.groundTrace = *trace;
          return qtrue;
        }
      }
    }
  }

  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  pml.groundPlane         = qfalse;
  pml.walking             = qfalse;

  return qfalse;
}

/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed(void)
{
  trace_t trace;
  vec3_t point;

  if (pm->ps->groundEntityNum != ENTITYNUM_NONE)
  {
    // we just transitioned into freefall
    if (pm->debugLevel)
      Com_Printf("%i:lift\n", c_pmove);

    // if they aren't in a jumping animation and the ground is a ways away, force into it
    // if we didn't do the trace, the player would be backflipping down staircases
    VectorCopy(pm->ps->origin, point);
    point[2] -= 64;

    pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
    if (trace.fraction == 1.0)
    {
      if (pm->cmd.forwardmove >= 0)
      {
        PM_ForceLegsAnim(LEGS_JUMP);
        pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
      }
      else
      {
        PM_ForceLegsAnim(LEGS_JUMPB);
        pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
      }
    }
  }

  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  pml.groundPlane         = qfalse;
  pml.walking             = qfalse;
}

/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace(void)
{
  vec3_t point;
  trace_t trace;
  vec3_t          refNormal = { 0.0f, 0.0f, 1.0f }; //add xreal

  point[0] = pm->ps->origin[0];
  point[1] = pm->ps->origin[1];
  point[2] = pm->ps->origin[2] - 0.25f;

  pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
  pml.groundTrace = trace;

  // do something corrective if the trace starts in a solid...
  if (trace.allsolid)
  {
    if (!PM_CorrectAllSolid(&trace))
      return;
  }

  // if the trace didn't hit anything, we are in free fall
  /* 0xA5EA, check this */
  if (trace.fraction == 1.0)
  {
	 //hypov8 add: xreal. makes player stick to ground when going down stairs
	qboolean        steppedDown = qfalse;

	  // try to step down
	  if (pml.groundPlane != qfalse && PM_PredictStepMove())
	  {
		  // step down
		  point[0] = pm->ps->origin[0];
		  point[1] = pm->ps->origin[1];
		  point[2] = pm->ps->origin[2] - STEPSIZE;
		  pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);

		  // if we hit something
		  if (trace.fraction < 1.0f)
		  {
			  PM_StepEvent(pm->ps->origin, trace.endpos, refNormal);
			  VectorCopy(trace.endpos, pm->ps->origin);
			  steppedDown = qtrue;
		  }
	  }

	  if (!steppedDown) //end xreal stair fix
	  {
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking     = qfalse;
		return;
	  }
  }

	// BUNNYHOP //hypov8 this stops velocity and not allow a jump+velocity
	//hypov8 max up velocity for bunyhop would be 270+180. this should stop small step double jump
	//todo: might need some more checks. water?

  // check if getting thrown off the ground
  //if (pm->ps->velocity[2] > 0 && DotProduct(pm->ps->velocity, trace.plane.normal) > 10)
  if (pm->ps->velocity[2] > 180 /*&& DotProduct(pm->ps->velocity, trace.plane.normal) > 10*/) //hypov8 add: was > 0
  {
    if (pm->debugLevel)
      Com_Printf("%i:kickoff\n", c_pmove);

    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane         = qfalse;
    pml.walking             = qfalse;
    return;
  }

  // slopes that are too steep will not be considered onground
  if (trace.plane.normal[2] < MIN_WALK_NORMAL)
  {
    if (pm->debugLevel)
      Com_Printf("%i:steep\n", c_pmove);

    // FIXME: if they can't slide down the slope, let them
    // walk (sharp crevices)
    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane         = qtrue;
    pml.walking             = qfalse;
    return;
  }

  pml.groundPlane = qtrue;
  pml.walking     = qtrue;

  // hitting solid ground will end a waterjump
  if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
  {
    pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
    pm->ps->pm_time   = 0;
  }

  if (pm->ps->groundEntityNum == ENTITYNUM_NONE)
  {
    // just hit the ground
    if (pm->debugLevel)
      Com_Printf("%i:Land\n", c_pmove);

    PM_CrashLand();

    // don't do landing time if we were just going down a slope
    if (pml.previous_velocity[2] < -200)
    {
      // don't allow another jump for a little while
      // FIXME: 0xA5EA, does this affect bunnyhop ??
      pm->ps->pm_flags &= ~PMF_TIME_LAND;
      //pm->ps->pm_flags |= PMF_TIME_LAND;
      //pm->ps->pm_time = 250;
    }
  }

  pm->ps->groundEntityNum = trace.entityNum;

  // don't reset the z velocity for slopes
//	pm->ps->velocity[2] = 0;

	  if (!pm->ps->bunnyHop) //hypov8 todo: fix this properly, can walk diaginal faster
	  {						//could also delay rejump
		  int maxSpeed;
		  
		  maxSpeed = G_SPEED_MAX; // pm->ps->maxSpeed; //hypov8 servers cvar g_speed //might use a global macro for forwardspeed

		  if (pm->ps->velocity[0] > G_SPEED_MAX)
			  pm->ps->velocity[0] = G_SPEED_MAX;
		  if (pm->ps->velocity[0] < -G_SPEED_MAX)
			  pm->ps->velocity[0] = -G_SPEED_MAX;

		  if (pm->ps->velocity[1] > G_SPEED_MAX)
			  pm->ps->velocity[1] = G_SPEED_MAX;
		  if (pm->ps->velocity[1] < -G_SPEED_MAX)
			  pm->ps->velocity[1] = -G_SPEED_MAX;

	  }

  PM_AddTouchEnt(trace.entityNum);
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel(void)
{
  vec3_t point;
  int cont;
  int sample1;
  int sample2;

  //
  // get waterlevel, accounting for ducking
  //
  pm->waterlevel = 0;
  pm->watertype  = 0;

  point[0] = pm->ps->origin[0];
  point[1] = pm->ps->origin[1];
  point[2] = pm->ps->origin[2] + MINS_Z + 1;
  cont     = pm->pointcontents(point, pm->ps->clientNum);

  if (cont & MASK_WATER)
  {
    sample2 = pm->ps->viewheight - MINS_Z;
    sample1 = sample2 / 2;

    pm->watertype  = cont;
    pm->waterlevel = 1;
    point[2]       = pm->ps->origin[2] + MINS_Z + sample1;
    cont           = pm->pointcontents(point, pm->ps->clientNum);
    if (cont & MASK_WATER)
    {
      pm->waterlevel = 2;
      point[2] = pm->ps->origin[2] + MINS_Z + sample2;
      cont = pm->pointcontents(point, pm->ps->clientNum);

      if (cont & MASK_WATER)
        pm->waterlevel = 3;
    }
  }
}

/*
==============
PM_CheckDuck
Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_CheckDuck(void)
{
  trace_t trace;

  VectorCopy(playerMins, pm->mins);
  VectorCopy(playerMaxs, pm->maxs);

  pm->mins[2] = MINS_Z;

  if (pm->ps->pm_type == PM_DEAD)
  {
    pm->maxs[2]        = -8;
    pm->ps->viewheight = DEAD_VIEWHEIGHT;
    return;
  }

  if (pm->cmd.upmove < 0)
    pm->ps->pm_flags |= PMF_DUCKED;  // duck
  else
  {
    // stand up if possible
    if (pm->ps->pm_flags & PMF_DUCKED)
    {
      // try to stand up
      pm->maxs[2] = 48;
      pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
      if (!trace.allsolid)
        pm->ps->pm_flags &= ~PMF_DUCKED;
    }
  }

  if (pm->ps->pm_flags & PMF_DUCKED)
  {
    pm->maxs[2]        = 24;
    pm->ps->viewheight = CROUCH_VIEWHEIGHT;
  }
  else
  {
	pm->maxs[2] = playerMaxs[2];
    pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
  }
}

//===================================================================
/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps(void)
{
  float bobmove;
  int old;
  qboolean footstep;

  // calculate speed and cycle to be used for
  // all cyclic walking effects
  pm->xyspeed = sqrt(pm->ps->velocity[0] * pm->ps->velocity[0] +  pm->ps->velocity[1] * pm->ps->velocity[1]);

  if (pm->ps->groundEntityNum == ENTITYNUM_NONE)
  {
    // airborne leaves position in cycle intact, but doesn't advance
    if (pm->waterlevel > 1)
     // PM_ContinueLegsAnim(LEGS_SWIM);
		PM_WaterMove(); //add hypov8

    return;
  }
    //-------------------
  // if not trying to move
  if (!pm->cmd.forwardmove && !pm->cmd.rightmove)
  {
    if (pm->xyspeed < 5)
    {
      pm->ps->bobCycle = 0;   // start at beginning of cycle again
      if (pm->ps->pm_flags & PMF_DUCKED)
      {
        PM_ContinueLegsAnim(LEGS_CR_IDLE);
      }
	   else
	   {
        PM_ContinueLegsAnim(LEGS_IDLE);
	   }
    }
    return;
  }

  footstep = qfalse;

  if (pm->ps->pm_flags & PMF_DUCKED)
  {
    bobmove = 0.5;  // ducked characters bob much faster
    if (pm->ps->pm_flags & PMF_BACKWARDS_RUN)
    {
      PM_ContinueLegsAnim(LEGS_CR_BACK);
    }
    else
    {
      PM_ContinueLegsAnim(LEGS_CR_WALK);
    }
    // ducked characters never play footsteps
    /*
    } else  if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
        if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
            bobmove = 0.4;	// faster speeds bob faster
            footstep = qtrue;
        } else {
            bobmove = 0.3;
        }
        PM_ContinueLegsAnim( LEGS_BACK );
    */
  }
  else
  {
    if (!(pm->cmd.buttons & BUTTON_WALKING))
    {
      bobmove = 0.4f; // faster speeds bob faster
      if (pm->ps->pm_flags & PMF_BACKWARDS_RUN)
      {
        PM_ContinueLegsAnim(LEGS_RUN_BACK);
      }
      else
      {
        PM_ContinueLegsAnim(LEGS_RUN);
	  }
      footstep = qtrue;
    }
    else
    {
      bobmove = 0.3f; // walking bobs slow
      if (pm->ps->pm_flags & PMF_BACKWARDS_RUN)
      {
		  PM_ContinueLegsAnim(LEGS_WALK_BACK);
	  }
      else
      {
	    PM_ContinueLegsAnim(LEGS_WALK);
      }
    }
  }
 //-------------------------3
  // check for footstep / splash sounds
  old = pm->ps->bobCycle;
  pm->ps->bobCycle = (int)(old + bobmove * pml.msec) & 255;

  // if we just crossed a cycle boundary, play an apropriate footstep event
  if (((old + 64) ^ (pm->ps->bobCycle + 64)) & 128)
  {
    if (pm->waterlevel == 0)
    {
      // on ground will only play sounds if running
      if (footstep && !pm->noFootsteps)
        PM_AddEvent(PM_FootstepForSurface());
    }
    else if (pm->waterlevel == 1)
      PM_AddEvent(EV_FOOTSPLASH);  // splashing
    else if (pm->waterlevel == 2)
      PM_AddEvent(EV_SWIM);   // wading / swimming at surface
    else if (pm->waterlevel == 3)
    {
      // no sound when completely underwater
    }
  }
}

/*
==============
PM_WaterEvents
Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents(void)
{
  // if just entered a water volume, play a sound
  if (!pml.previous_waterlevel && pm->waterlevel)
    PM_AddEvent(EV_WATER_TOUCH);

  // if just completely exited a water volume, play a sound
  if (pml.previous_waterlevel && !pm->waterlevel)
    PM_AddEvent(EV_WATER_LEAVE);

  // check for head just going under water
  if (pml.previous_waterlevel != 3 && pm->waterlevel == 3)
    PM_AddEvent(EV_WATER_UNDER);

  // check for head just coming out of water
  if (pml.previous_waterlevel == 3 && pm->waterlevel != 3)
    PM_AddEvent(EV_WATER_CLEAR);
}

/*
===============
PM_WeaponReloadTimeOffset
===============
*/
static int PM_WeaponReloadTimeOffset(int weapon, int wepState)
{
	//hypov8 use global timers. used in cg_weapons
	switch (weapon)
	{
		case WP_PISTOL:
			return WP_TIME_RELOAD_PISTOL;
		case WP_SHOTGUN:
			if (wepState == WEAPON_RELOAD_MOD)
				return WP_TIME_MOD_SHOTGUN*2;
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
	//return pistol if missing
	return WP_TIME_RELOAD_PISTOL;
}

/*
===============
PM_BeginWeaponReload
===============
*/
static qboolean PM_BeginWeaponReload(int weapon)
{
	if (!BG_IsReloadableWeapon(weapon))
		return false;
	if (!(pm->ps->stats[STAT_WEAPONS] & (1 << weapon)))
		return false;
	if (pm->ps->weaponstate == WEAPON_RAISING)
		return false;		
	if (pm->ps->weaponstate == WEAPON_DROPPING)
		return false;	
 
	//check if gun magazine is full
	if (pm->ps->ammo_mag[weapon] == BG_WeaponMaxMagCount(weapon))
		return false; // switch??
	//no ammo to reload
	if (!pm->ps->ammo_all[BG_AmmoCombineCheck(weapon)])
		return false;


	//pm->ps->pm_flags &= ~PMF_WEAPON_RELOAD;
	if ( weapon == WP_SHOTGUN && 
		(pm->ps->weaponstate == WEAPON_RELOADING ||pm->ps->weaponstate == WEAPON_RELOAD_MOD ))
	{
		pm->ps->weaponstate = WEAPON_RELOAD_MOD;
		PM_ContinueTorsoAnim( TORSO_DROP ); //dont change 3rd person animation.. yet
		PM_StartWeaponAnim(WEAPON_RELOAD_MOD);
		BG_AddPredictableEventToPlayerstate( EV_RELOAD_SHOTGUN, pm->ps->weapon, pm->ps );
	}
	else
	{
		pm->ps->weaponstate = WEAPON_RELOADING;
		PM_StartTorsoAnim( TORSO_DROP ); //hypov8 todo: TORSO_DROP animation looks odd. make new animations
		PM_StartWeaponAnim(WEAPON_RELOADING);
		if (weapon != WP_SHOTGUN) //stop sound event on shotty until reloading
			BG_AddPredictableEventToPlayerstate( EV_RELOAD_WEAPON, pm->ps->weapon, pm->ps );
	}

	pm->ps->weaponTime += PM_WeaponReloadTimeOffset(weapon, pm->ps->weaponstate);
	return true;
}

/*
===============
PM_FinishWeaponReload
===============
*/
static void PM_FinishWeaponReload(int weapon)
{
  int roundsMax, rounds_Avalible, rounds_needed;

  if (!(pm->ps->stats[STAT_WEAPONS] & (1 << weapon)))
    return;


  roundsMax = BG_WeaponMaxMagCount(weapon);
  rounds_needed = roundsMax - pm->ps->ammo_mag[weapon];
  rounds_Avalible = pm->ps->ammo_all[BG_AmmoCombineCheck(weapon)];
  
	//max ammo count
	if (roundsMax)
	{
		if (weapon == WP_SHOTGUN)
		{
			if ( rounds_needed > 0 && rounds_Avalible > 0)
			{
				int minus = 1; //did we reload 3 bullets?

				if ( rounds_needed > rounds_Avalible )
					rounds_needed = rounds_Avalible;

				// add 3 bullets per reload. hogie request
				if ( rounds_needed > 3 )
					rounds_needed = 3;
				rounds_needed = 1; //single reload

				pm->ps->ammo_mag[  weapon] += rounds_needed;
				pm->ps->ammo_all[BG_AmmoCombineCheck( weapon ) ] -= rounds_needed;

				minus = rounds_needed;

				//continue to reload shotty
				if ( pm->ps->ammo_mag[ weapon ] < roundsMax && ( rounds_Avalible - minus ) > 0 )
				{
					pm->ps->weaponTime = 0;
					PM_BeginWeaponReload(weapon);
					return;
				}
				else if ( pm->ps->weaponTime > 0 )
					return; //finish reload animation for last bullet
			}
			else if ( pm->ps->weaponTime > 0 )
				return; //finish reload animation for last bullet
		}
		else
		{
			if (rounds_Avalible >= rounds_needed)
			{
				pm->ps->ammo_mag[weapon] += rounds_needed;
				pm->ps->ammo_all[BG_AmmoCombineCheck(weapon)] -= rounds_needed;
			}
			else
			{
				pm->ps->ammo_mag[weapon] += rounds_Avalible;
				pm->ps->ammo_all[BG_AmmoCombineCheck(weapon)] = 0;
			}
		}
  }

  pm->ps->weaponstate = WEAPON_READY;
}


int PM_FindNextBestWeapon(playerState_t *ps)
{
	int i, back = qfalse;
	int	newWep = ps->weapon+1;
	for ( i = WP_FIRST; i <= WP_LAST; i++ )
	{
		if ( newWep > WP_LAST )
		{	//search back from last active wep
			newWep = ps->weapon - 1; // WP_FIRST;
			back = qtrue;
		}

		if ( BG_InventoryContainsWeapon(newWep, ps->stats) && BG_InventoryContainsAmmo(newWep, ps) )
			break; //found wep+ammo

		if (!back )
			newWep++;
		else
			newWep--;
	}
	if ( i > WP_LAST )
		newWep = WP_CROWBAR;

	return newWep;
}

/*
===============
PM_BeginWeaponChange
===============
*/
void PM_BeginWeaponChange(int weapon)
{
	if ( pm->ps->weaponstate == WEAPON_DROPPING )
		return;
	if (pm->ps->weaponstate == WEAPON_RELOADING || pm->ps->weaponstate == WEAPON_RELOAD_MOD)
		return;
	if (weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS)
		weapon = WP_CROWBAR;


	PM_AddEvent2(EV_CHANGE_WEAPON_DROP, weapon); //update client weapon key(incase it was hitmen forced)
	pm->ps->weaponstate = WEAPON_DROPPING;
	pm->ps->persistant[ PERS_NEWWEAPON ] = weapon;

	if (pm->ps->weapon == WP_CROWBAR || pm->ps->weapon == WP_GRAPPLING_HOOK)
		pm->ps->weaponTime += WP_TIME_CHANGE_MELEE;
	else
		pm->ps->weaponTime += WP_TIME_CHANGE_GUNS; //hypov8 note: kp1 is about 400-600

	PM_StartTorsoAnim(TORSO_DROP); //force wep change animations
	PM_StartWeaponAnim(WEAPON_DROPPING); //unvan .5
}

/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange(int weapon)
{
	if (weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS) //spec = none?
		weapon = WP_CROWBAR;

	if ( !BG_InventoryContainsWeapon( weapon, pm->ps->stats ) )
		weapon = PM_FindNextBestWeapon(pm->ps); //find another wep 

	pm->ps->weapon = weapon; // pm->cmd.weapon;
	pm->ps->weaponstate = WEAPON_RAISING;

	if (pm->ps->weapon == WP_CROWBAR || pm->ps->weapon == WP_GRAPPLING_HOOK)
		pm->ps->weaponTime += WP_TIME_CHANGE_MELEE;
	else
		pm->ps->weaponTime += WP_TIME_CHANGE_GUNS;

	PM_AddEvent2(EV_CHANGE_WEAPON_RAISE, weapon); //tell client wep to use/select
	PM_StartTorsoAnim(TORSO_RAISE);
	PM_StartWeaponAnim(WEAPON_RAISING); //unvan .5
}

/*
==============
PM_TorsoAnimation
==============
*/
static void PM_TorsoAnimation(void)
{
	pm->xyspeed = sqrt(pm->ps->velocity[0] * pm->ps->velocity[0] +  pm->ps->velocity[1] * pm->ps->velocity[1]);

	if (pm->ps->weaponstate == WEAPON_READY && !pml.ladder) 
	{
		if (!pml.groundPlane) //note: hypov8 can i make it deley groundtrace. hopping along terrain
		{	//bunnyhop animation
			if (pm->xyspeed >= 300)
				PM_StartTorsoAnim(TORSO_RUN); //note hypov8 startTorso. use first frame
			else
			{ 
				if (pm->ps->weapon == WP_CROWBAR || pm->ps->weapon == WP_GRAPPLING_HOOK)
					PM_ContinueTorsoAnim(TORSO_STAND2);
				else if (pm->ps->weapon == WP_PISTOL)
					PM_ContinueTorsoAnim(TORSO_STAND3);
				else PM_ContinueTorsoAnim(TORSO_STAND);
			}
		}
		else //pml.groundPlane true
		{
			if ( !( pm->ps->pm_flags & PMF_DUCKED ) && pm->xyspeed > 80 )
			{
				if (pm->cmd.buttons & BUTTON_WALKING )
					PM_ContinueTorsoAnim(TORSO_WALK);
				else if ( pm->xyspeed < 300 )
					PM_ContinueTorsoAnim(TORSO_RUN);
				else  //pushed faster by rocket, bunnyhop etc..
					PM_ContinueTorsoAnim(TORSO_RUN);
			}
			else
			{ // croutch or idle torso animation
				if (pm->ps->weapon == WP_CROWBAR || pm->ps->weapon == WP_GRAPPLING_HOOK)
					PM_ContinueTorsoAnim(TORSO_STAND2);
				else if (pm->ps->weapon == WP_PISTOL)
					PM_ContinueTorsoAnim(TORSO_STAND3);
				else
					PM_ContinueTorsoAnim(TORSO_STAND);
			}
		}

		PM_ContinueWeaponAnim( WEAPON_READY ); //daemon .50
	}

  return; //end torso... else must be shooting or on a ladder
}

	
/*
================
PM_WeaponFireTimeOffset
================
*/
static int PM_WeaponFireTimeOffset(int weapon, qboolean hmgLastBullet)
{
	switch (pm->ps->weapon)
	{
	default:
	case WP_CROWBAR:
		return (int)(WP_TIME_FIRE_CROWBAR/2);
	case WP_PISTOL:
		return WP_TIME_FIRE_PISTOL;
		//FIXME (0xA5EA): pistol offset ok ?, pistol mod
	case WP_SHOTGUN:
		return WP_TIME_FIRE_SHOTGUN;
	case WP_MACHINEGUN:
		return WP_TIME_FIRE_MACHINEGUN;
	case WP_GRENADE_LAUNCHER:
		return WP_TIME_FIRE_GRENADE_LAUNCHER;         // 0xA5EA
		break;
	case WP_ROCKET_LAUNCHER:
		return WP_TIME_FIRE_ROCKET_LAUNCHER;         // 0xA5EA
	case WP_HMG:
		if (hmgLastBullet)
			if (pm->ps->stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_COOLING))
				return (int)(WP_TIME_FIRE_HMG_LAST / 2);
			else
				return WP_TIME_FIRE_HMG_LAST;
		else
			return WP_TIME_FIRE_HMG;

#ifdef USE_FLAMEGUN
	case WP_FLAMER:
		return WP_TIME_FIRE_FLAMEGUN;
#endif
	case WP_GRAPPLING_HOOK:
		return WP_TIME_FIRE_GRAPPLING_HOOK;
	}
}

/*
==============
PM_Weapon
Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon(void)
{
	int      addTime = 200; //default addTime - should never be used
	qboolean attack1 = ( pm->cmd.buttons & BUTTON_ATTACK );

	// Ignore weapons in some cases
	if (pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR)
		return;

	// check for dead player
	if (pm->ps->stats[STAT_HEALTH] <= 0)
	{
		pm->ps->weapon = WP_NONE;
		return;
	}

	// don't allow attack until all buttons are up
	if (pm->ps->pm_flags & PMF_RESPAWNED)
		return;

	// make weapon function
	if (pm->ps->weaponTime > 0)
		pm->ps->weaponTime -= pml.msec;

	//reset hmg counter
	if (pm->ps->weaponstate != WEAPON_FIRING)
		pm->ps->hmgBulletNum = 0;


	// check for weapon change
	// can't change if weapon is firing and counter > 0
	// can change again if lowering or raising or reloading
	if ( BG_PlayerCanChangeWeapon( pm->ps ) )
	{
#if 0 //not currently used
		// check for item using
		if (pm->cmd.buttons & BUTTON_USE_HOLDABLE)
		{
			if (!(pm->ps->pm_flags & PMF_USE_ITEM_HELD))
			{
				if (bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag == HI_MEDKIT && pm->ps->stats[STAT_HEALTH] >= (pm->ps->stats[STAT_MAX_HEALTH] + PLAYER_RESPAWN_HEALTH_ADD))
				{
					;// don't use medkit if at max health
				}
				else
				{
					pm->ps->pm_flags |= PMF_USE_ITEM_HELD;
					PM_AddEvent(EV_USE_ITEM0 + bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag);
					pm->ps->stats[STAT_HOLDABLE_ITEM] = 0;
				}
				return;
			}
		}
		else
		{
			pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
		}
#endif
		//something thinks a weapon change is necessary
		if ( pm->ps->pm_flags & PMF_WEAPON_SWITCH || 
			(pm->ps->weapon != pm->cmd.weapon && BG_InventoryContainsWeapon(pm->cmd.weapon, pm->ps->stats) ) )
		{
			int wepNew = (pm->ps->pm_flags & PMF_WEAPON_SWITCH )? pm->ps->persistant[ PERS_NEWWEAPON ]: pm->cmd.weapon ;
				
			if ( pm->ps->weapon != WP_NONE && pm->ps->persistant[ PERS_NEWWEAPON ] != WP_NONE )
			{
				PM_BeginWeaponChange(wepNew);
			}
			else
			{	// no current weapon, so just raise the new one
				PM_FinishWeaponChange(wepNew);
			}
			pm->ps->pm_flags &= ~PMF_WEAPON_SWITCH;
		}
	}



	if ( pm->ps->weaponstate == WEAPON_HM_LOCK ) //fix client sync
	{
		/*if ( pm->ps->pm_flags & PMF_WEAPON_LOCK )
		{
			pm->ps->pm_flags &= ~PMF_WEAPON_LOCK;
			PM_AddEvent( EV_HITMEN_INFOMSG );
			return;
		}*/
		return;
	}

	//FIXME (0xA5EA): hitmen weapon lock
#if 0
	if (pm->ps->weaponstate == WEAPON_HM_AUTOSWITCH)
	{
		//pm->ps->weapon      = level.time;
		pm->ps->weaponTime = 250; //hypov8 todo: hitman
		PM_ContinueTorsoAnim(TORSO_RAISE);
	}
#endif

	if (pm->ps->weaponTime > 0)
	{
		//allow shotgun attack while reloading
		if (pm->ps->weapon == WP_SHOTGUN && (pm->ps->weaponstate == WEAPON_RELOADING||pm->ps->weaponstate == WEAPON_RELOAD_MOD))
		{
			if ( attack1 && pm->ps->ammo_mag[ pm->ps->weapon ] > 0 )
			{
				pm->ps->weaponstate = WEAPON_FIRING;
				pm->ps->weaponTime = 0; //reset so reload time is removed
			}
			else if ( pm->ps->weaponstate == WEAPON_RELOADING && pm->ps->weaponTime < WP_TIME_MOD_SHOTGUN * 2 ) //inital reload
			{
				pm->ps->weaponTime = 0;
				PM_BeginWeaponReload(pm->ps->weapon);
			}
			else if ( pm->ps->weaponTime < WP_TIME_MOD_SHOTGUN) //2nd+ bullet
				PM_FinishWeaponReload(pm->ps->weapon);

			return;
		}
		else
		{	//busy... dont check any more events.
			return;
		}
	}


	// change weapon if time
	if (pm->ps->weaponstate == WEAPON_DROPPING)
	{
		int wepNew = (pm->ps->pm_flags & PMF_WEAPON_SWITCH )? pm->ps->persistant[ PERS_NEWWEAPON ]: pm->cmd.weapon ;	
		PM_FinishWeaponChange(wepNew);
		return;
	}

	// Set proper animation
	if (pm->ps->weaponstate == WEAPON_RAISING)
	{
		pm->ps->weaponstate = WEAPON_READY;

		if (pm->ps->weapon == WP_CROWBAR)
			PM_ContinueTorsoAnim(TORSO_STAND2);
		else if (pm->ps->weapon == WP_PISTOL)
			PM_ContinueTorsoAnim(TORSO_STAND3);
		else
			PM_ContinueTorsoAnim(TORSO_STAND);

		PM_ContinueWeaponAnim( WEAPON_READY ); //daemon .50

		return;
	}

	// check for out of ammo
	if ( !(pm->ps->ammo_mag[ pm->ps->weapon ] == INFINITE_AMMO) && !BG_InventoryContainsAmmo(pm->ps->weapon, pm->ps))
	{

		int newWep = PM_FindNextBestWeapon(pm->ps);

		//auto switch to next avalible wep		
		PM_BeginWeaponChange(newWep);
		return;
	}

	//done reloading so give em some ammo
	if ( pm->ps->weaponstate == WEAPON_RELOADING || pm->ps->weaponstate == WEAPON_RELOAD_MOD)
	{
		PM_FinishWeaponReload(pm->ps->weapon);
		return;
	}


	//start reloading if we have ammo
	if (!(pm->ps->ammo_mag[ pm->ps->weapon ] == INFINITE_AMMO) &&
		(pm->ps->ammo_mag[pm->ps->weapon ] <= 0 || pm->cmd.buttons & BUTTON_RELOAD &&
		pm->ps->ammo_all[BG_AmmoCombineCheck(pm->ps->weapon) ]> 0 ))
	{
		if ((pm->ps->weapon != WP_SHOTGUN)||
			(pm->ps->weapon == WP_SHOTGUN && !attack1 && pm->ps->weaponstate != WEAPON_FIRING))
		{
			if (PM_BeginWeaponReload(pm->ps->weapon))
				return;
		}
	}


	// fire events for non auto weapons
	if ( attack1 || (pm->ps->hmgBulletNum && pm->ps->weapon == WP_HMG))
	{
		if ( pm->ps->weapon == WP_PISTOL && pm->ps->stats[ STAT_WEAP_MODS ] & ( 1 << PW_WPMOD_SILENCER ) )
			PM_AddEvent(EV_FIRE_SPISTOL);
		else
			PM_AddEvent(EV_FIRE_WEAPON);
	}
	else
	{	// Idle
		pm->ps->hmgBulletNum = 0;
		pm->ps->weaponTime = 0;
		pm->ps->weaponstate = WEAPON_READY;
		return; //no attack
	}


	if (pm->ps->weapon)
	{
		switch (pm->ps->weapon)
		{
		case WP_CROWBAR:
			if ( pm->ps->hmgBulletNum == 0 )
			{
				pm->ps->hmgBulletNum = 1;
				PM_StartTorsoAnim(BG_AttackTorsoAnim(pm->ps->weapon));
				PM_StartWeaponAnim(WEAPON_FIRING);
			}
			else 
			{
				pm->ps->hmgBulletNum = 0;
				PM_StartTorsoAnim(BG_AttackTorsoAnim(pm->ps->weapon));
				PM_ContinueWeaponAnim(WEAPON_FIRING);
			}
			addTime = PM_WeaponFireTimeOffset(pm->ps->weapon, qfalse);
			break;

		case WP_PISTOL:
			PM_StartTorsoAnim(BG_AttackTorsoAnim(pm->ps->weapon));
			addTime = PM_WeaponFireTimeOffset(pm->ps->weapon, qfalse);
			break;

		case WP_HMG:
			PM_StartTorsoAnim(BG_AttackTorsoAnim(pm->ps->weapon));
			if (pm->ps->hmgBulletNum == 0)
			{
				pm->ps->hmgBulletNum = 1;
				addTime = PM_WeaponFireTimeOffset(pm->ps->weapon, qfalse);
			}
			else if (pm->ps->hmgBulletNum == 1)
			{
				pm->ps->hmgBulletNum = 2;
				addTime = PM_WeaponFireTimeOffset(pm->ps->weapon, qfalse);
			}
			else if (pm->ps->hmgBulletNum == 2)
			{
				pm->ps->hmgBulletNum = 0;
				addTime = PM_WeaponFireTimeOffset(pm->ps->weapon, qtrue);
			}

			break;

		default:
		//case WP_SHOTGUN:
				PM_StartTorsoAnim(BG_AttackTorsoAnim(pm->ps->weapon));
				addTime = PM_WeaponFireTimeOffset(pm->ps->weapon, qfalse);
			break;
		}

		if ( pm->ps->weapon != WP_CROWBAR )
			PM_StartWeaponAnim(WEAPON_FIRING);
	}

	pm->ps->weaponstate = WEAPON_FIRING;

	//make view bob with bullet recoil
	pm->ps->viewShootBob++; 
	if ( pm->ps->viewShootBob > 255 )
		pm->ps->viewShootBob = 1;

	// take an ammo away if not infinite
	if (pm->ps->ammo_mag[pm->ps->weapon] != INFINITE_AMMO)
	{
		pm->ps->ammo_mag[pm->ps->weapon]--;
		if (pm->ps->ammo_mag[pm->ps->weapon] < 0)
			pm->ps->ammo_mag[pm->ps->weapon] = 0;
	}

	pm->ps->weaponTime += addTime; //+
}

/*
================
PM_Animate
================
*/
static void PM_Animate(void)
{
  if (pm->cmd.buttons & BUTTON_GESTURE)
  {
    if (pm->ps->torsoTimer == 0)
    {
      shortbytes_t index;

      index.b[0] = rand() % 3;
      switch (index.b[0])
      {
	  default:
      case 0: 
			PM_ContinueTorsoAnim(TORSO_GESTURE); //edit hypov8 load different taunts on different sounds
			pm->ps->torsoTimer = TIMER_GESTURE;
			index.b[1] = rand() % numCustomTaunts1;
			//Com_Printf("TAUNT 1\n");	//note hypov8 getting multiple nags from fast frame rate
        break;							// compared to server frame rate.
										//g_synchronousClients "0"
      case 1: 
	  		PM_ContinueTorsoAnim(TORSO_GESTURE2); //add hypov8 taunt2
			pm->ps->torsoTimer = TIMER_GESTURE;
			index.b[1] = rand() % numCustomTaunts2;
			//Com_Printf("TAUNT 2\n"); 
        break;
		
	//---------adding 3rd taunt, need sounds?---------------
      case 2: 
	  		PM_ContinueTorsoAnim(TORSO_GESTURE3); //add hypov8 taunt3
			pm->ps->torsoTimer = TIMER_GESTURE;
			index.b[1] = rand() % numCustomTaunts1; //note hypov8, do we need 3rd sounds?
			//Com_Printf("TAUNT 3\n");
        break;
		
      }
	  //pm->ps->torsoTimer = TIMER_GESTURE;
      PM_AddEvent2(EV_TAUNT, index.s);
	  
     // PM_AddEvent(EV_TAUNT);
      //BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
    }
  }
  else if (pm->cmd.buttons & BUTTON_GETFLAG)
  {
    if (pm->ps->torsoTimer == 0)
    {
      PM_StartTorsoAnim(TORSO_GETFLAG);
      pm->ps->torsoTimer = 600;   //TIMER_GESTURE;
    }
  }
  else if (pm->cmd.buttons & BUTTON_GUARDBASE)
  {
    if (pm->ps->torsoTimer == 0)
    {
      PM_StartTorsoAnim(TORSO_GUARDBASE);
      pm->ps->torsoTimer = 600;   //TIMER_GESTURE;
    }
  }
  else if (pm->cmd.buttons & BUTTON_PATROL)
  {
    if (pm->ps->torsoTimer == 0)
    {
      PM_StartTorsoAnim(TORSO_PATROL);
      pm->ps->torsoTimer = 600;   //TIMER_GESTURE;
    }
  }
  else if (pm->cmd.buttons & BUTTON_FOLLOWME)
  {
    if (pm->ps->torsoTimer == 0)
    {
      PM_StartTorsoAnim(TORSO_FOLLOWME);
      pm->ps->torsoTimer = 600;   //TIMER_GESTURE;
    }
  }
  else if (pm->cmd.buttons & BUTTON_AFFIRMATIVE)
  {
    if (pm->ps->torsoTimer == 0)
    {
      PM_StartTorsoAnim(TORSO_AFFIRMATIVE);
      pm->ps->torsoTimer = 600;   //TIMER_GESTURE;
    }
  }
  else if (pm->cmd.buttons & BUTTON_NEGATIVE)
  {
    if (pm->ps->torsoTimer == 0)
    {
      PM_StartTorsoAnim(TORSO_NEGATIVE);
      pm->ps->torsoTimer = 600;   //TIMER_GESTURE;
    }
  }

  //hypov8 jump animations
  //include jumping and going faster then normal speeds(bunny hop) animations
  if (pml.groundPlane == qfalse &&  pml.walking == qfalse &&  pm->ps->groundEntityNum == ENTITYNUM_NONE)
  {
		  pm->xyspeed = sqrt(pm->ps->velocity[0] * pm->ps->velocity[0] + pm->ps->velocity[1] * pm->ps->velocity[1]);
	  if (pm->cmd.forwardmove >= 0)
	  {
		  if (pm->cmd.forwardmove == 0 && pm->xyspeed > 350) //bunny hop animation
		  {
			  //PM_ForceLegsAnim(LEGS_IDLE); //removed hypov8 because it allows torso YAW to work while idle
			  PM_ForceLegsAnim(LEGS_LAND);
			  pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		  }
		  else
		  {
			  PM_ForceLegsAnim(LEGS_JUMP);
			  pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		  }
	  }
	  else //if moving backward true
	  {
		  if (pm->xyspeed > 350) //bunny hop backwards
		  {
			  PM_ForceLegsAnim(LEGS_IDLE);
			  pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		  }
		  else
		  {
			  PM_ForceLegsAnim(LEGS_JUMPB);
			  pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
		  }
	  }
  }



}

/*
================
PM_DropTimers
================
*/
static void PM_DropTimers(void)
{
  // drop misc timing counter
  if (pm->ps->pm_time)
  {
    if (pml.msec >= pm->ps->pm_time)
    {
      pm->ps->pm_flags &= ~PMF_ALL_TIMES;
      pm->ps->pm_time   = 0;
    }
    else
      pm->ps->pm_time -= pml.msec;
  }

  // drop animation counter
  if (pm->ps->legsTimer > 0)
  {
    pm->ps->legsTimer -= pml.msec;
    if (pm->ps->legsTimer < 0)
      pm->ps->legsTimer = 0;
  }

  if (pm->ps->torsoTimer > 0)
  {
    pm->ps->torsoTimer -= pml.msec;
    if (pm->ps->torsoTimer < 0)
      pm->ps->torsoTimer = 0;
  }
}

/*
================
PM_UpdateViewAngles
This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void PM_UpdateViewAngles(playerState_t *ps, const usercmd_t *cmd)
{
	short temp[3];
  int i, hyp_angle;

  hyp_angle = 16000; // 15200; // hypov8 set the max angle faster. 16384 = 90deg

  if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION)
    return;     // no view changes at all

  if (ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0)
  {
	  ///////// temp[1] = cmd->angles[1] + ps->delta_angles[1];
	  // rain - always allow this.  viewlocking will take precedence
	  // if a medic is found
	  // rain - using a full short and converting on the client so that
	  // we get >1 degree resolution
	  /////////////////ps->stats[STAT_DEAD_YAW] = temp[1];
	  // tjw: don't let corpses bury their heads in the sand
	  ps->viewangles[2] = 0;
	  //cmd->angles[2] = 0;

	  ps->viewangles[0] = 0;
	  //cmd->angles[0] = 0;


    return;     // no view changes at all
  }

  // circularly clamp the angles with deltas
  for (i = 0; i < 3; i++)
  {
    temp[i] = cmd->angles[i] + ps->delta_angles[i];
    if (i == PITCH)
    {
      // don't let the player look up or down more than 90 degrees
		if (temp[i] > hyp_angle) // hypov8 was 1600 look up/down
      {
			ps->delta_angles[i] = hyp_angle - cmd->angles[i]; // hypov8 player look up and down 90 deg.. 
			temp[i] = hyp_angle;
      }
		else if (temp[i] < -hyp_angle)
      {
			ps->delta_angles[i] = -hyp_angle - cmd->angles[i];
			temp[i] = -hyp_angle;
      }
    }
    ps->viewangles[i] = SHORT2ANGLE(temp[i]);
  }
}

/*
================
PmoveSingle
================
*/
void trap_SnapVector(float *v);

void PmoveSingle(pmove_t *pmove)
{
  pm = pmove;

  if (pm->ps->origin[2] < -65500) //hypov8 add: stop player falling for ever. use entities fall off map?
	  pm->ps->origin[2] = 65500;
  
  // this counter lets us debug movement problems with a journal
  // by setting a conditional breakpoint fot the previous frame
  c_pmove++;

  // clear results
  pm->numtouch   = 0;
  pm->watertype  = 0;
  pm->waterlevel = 0;

  if (pm->ps->stats[STAT_HEALTH] <= 0)
  {
    pm->tracemask &= ~CONTENTS_BODY;    // corpses can fly through bodies
  }

  // make sure walking button is clear if they are running, to avoid
  // proxy no-footsteps cheats
  if (abs(pm->cmd.forwardmove) > 64 || abs(pm->cmd.rightmove) > 64)
    pm->cmd.buttons &= ~BUTTON_WALKING;

  // set the talk balloon flag
  if (pm->cmd.buttons & BUTTON_TALK)
    pm->ps->eFlags |= EF_TALK;
  else
    pm->ps->eFlags &= ~EF_TALK;

  // set the firing flag for continuous beam weapons
  if (!(pm->ps->pm_flags & PMF_RESPAWNED) && pm->ps->pm_type != PM_INTERMISSION && pm->ps->pm_type != PM_NOCLIP &&
       (pm->cmd.buttons & BUTTON_ATTACK) && pm->ps->ammo_mag[pm->ps->weapon])
    pm->ps->eFlags |= EF_FIRING;
  else
    pm->ps->eFlags &= ~EF_FIRING;

#if 0
  // clear the respawned flag if attack and use are cleared
  if (pm->ps->stats[STAT_HEALTH] > 0 &&
	  !(pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE))) //hypov8 allow shooting soon as respawned
    pm->ps->pm_flags &= ~PMF_RESPAWNED;
#else
 /// if (pm->ps->stats[STAT_HEALTH] > 0) //hypov8 when dead stop shoot working
//		pm->ps->pm_flags |= PMF_RESPAWNED;
#endif

  // if talk button is down, dissallow all other input
  // this is to prevent any possible intercept proxy from
  // adding fake talk balloons
  if (pmove->cmd.buttons & BUTTON_TALK)
  {
    // keep the talk button set tho for when the cmd.serverTime > 66 msec
    // and the same cmd is used multiple times in Pmove
    pmove->cmd.buttons     = BUTTON_TALK;
    pmove->cmd.forwardmove = 0;
    pmove->cmd.rightmove   = 0;
    pmove->cmd.upmove      = 0;
  }

  // clear all pmove local vars
  Com_Memset(&pml, 0, sizeof(pml));

  // determine the time
  pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
  if (pml.msec < 1)
    pml.msec = 1;
  else if (pml.msec > 200)
    pml.msec = 200;

  pm->ps->commandTime = pmove->cmd.serverTime;

  // save old org in case we get stuck
  VectorCopy(pm->ps->origin, pml.previous_origin);

  // save old velocity for crashlanding
  VectorCopy(pm->ps->velocity, pml.previous_velocity);

  pml.frametime = pml.msec * 0.001;

  // update the viewangles
  PM_UpdateViewAngles(pm->ps, &pm->cmd);

  // AngleVectors ist in kp genauso definiert wie in q3
  AngleVectors(pm->ps->viewangles, pml.forward, pml.right, pml.up);

  if (pm->cmd.upmove < 10)
    pm->ps->pm_flags &= ~PMF_JUMP_HELD;  /* not holding jump */

  // decide if backpedaling animations should be used
  if (pm->cmd.forwardmove < 0)
    pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
  else if (pm->cmd.forwardmove > 0 || (pm->cmd.forwardmove == 0 && pm->cmd.rightmove))
    pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;

  if (pm->ps->pm_type >= PM_DEAD)
  {
    pm->cmd.forwardmove = 0;
    pm->cmd.rightmove   = 0;
    pm->cmd.upmove      = 0;
  }

  if (pm->ps->pm_type == PM_SPECTATOR)
  {
    PM_CheckDuck();
    PM_FlyMove();
    PM_DropTimers();
    return;
  }

  if (pm->ps->pm_type == PM_NOCLIP)
  {
    PM_NoclipMove();
    PM_DropTimers();
    return;
  }

  if (pm->ps->pm_type == PM_FREEZE)
    return;     // no movement at all

  if (pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION)
    return;     // no movement at all

  // set watertype, and waterlevel
  PM_SetWaterLevel();
  pml.previous_waterlevel = pmove->waterlevel;

  // set mins, maxs, and viewheight
  PM_CheckDuck();

  // set groundentity
  PM_GroundTrace();

  if (pm->ps->pm_type == PM_DEAD)
    PM_DeadMove();

  PM_DropTimers();

  PM_CheckLadder();
#ifdef GT_USE_TA_TYPES
  if (pm->ps->powerups[PW_FLIGHT])
  {
    // flight powerup doesn't allow jump and has different friction
    PM_FlyMove();
  }
  else
#endif
  if (pm->ps->pm_flags & PMF_GRAPPLE_PULL)
  {
    PM_GrappleMove();
    // We can wiggle a bit
    PM_AirMove(); // NEW_AIRMOVE
  }
  else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
  {
    PM_WaterJumpMove();
  }
  else if (pm->waterlevel > 1)
  {
    // swimming
    PM_WaterMove();
  }
  else if (pml.ladder)   /* 0xA5EA */
  {
    PM_LadderMove();
  }
  else if (pml.walking)
  {
    // walking on ground
    PM_WalkMove();
  }
  else
  {
	// airborne
    PM_AirMove(); //NEW_AIRMOVE
  }

  PM_Animate();

  // set groundentity, watertype, and waterlevel
  PM_GroundTrace();

  /* TA: must update after every GroundTrace() - yet more clock cycles down the drain :( (14 vec rotations/frame) */
  // update the viewangles
  PM_UpdateViewAngles(pm->ps, &pm->cmd); //hypov8 add: xreal ok?

  PM_SetWaterLevel();

  // weapons
  PM_Weapon();

  // footstep events / legs animations
  PM_Footsteps();
 
  // torso animation
  PM_TorsoAnimation();

  // entering / leaving water splashes
  PM_WaterEvents();

  if ( !pmove->pmove_accurate )
  {
	// snap some parts of playerstate to save network bandwidth
	SnapVector( pm->ps->velocity );
  }
}

/*
================
Pmove
Can be called by either the server or the client
================
*/
void Pmove(pmove_t *pmove)
{
  int finalTime;

  finalTime = pmove->cmd.serverTime;

  if (finalTime < pmove->ps->commandTime)
    return; // should not happen

  if (finalTime > pmove->ps->commandTime + 1000)
    pmove->ps->commandTime = finalTime - 1000;

  pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount + 1) & ((1 << PS_PMOVEFRAMECOUNTBITS) - 1);

  // chop the move up if it is too long, to prevent framerate
  // dependent behavior
  while (pmove->ps->commandTime != finalTime)
  {
    int msec;

    msec = finalTime - pmove->ps->commandTime;

    if (pmove->pmove_fixed)
    {
      if (msec > pmove->pmove_msec)
        msec = pmove->pmove_msec;
    }
    else
    {
      if (msec > 66)
        msec = 66;
    }
    pmove->cmd.serverTime = pmove->ps->commandTime + msec;
    PmoveSingle(pmove);

    if (pmove->ps->pm_flags & PMF_JUMP_HELD)
      pmove->cmd.upmove = 20;
  }
  //PM_CheckStuck();
}
