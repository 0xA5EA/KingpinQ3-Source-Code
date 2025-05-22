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
// g_weapon.c
// perform the server side effects of a weapon firing

#include "g_local.h"

static vec3_t forward, right, up;
static vec3_t muzzle;
//static vec3_t muzzleTrace;  // Added undeclared identifier -KRYPTYK

void G_ForceWeaponChange( gentity_t *ent, weapon_t weapon ) //unvan 0.52
{
  playerState_t *ps = &ent->client->ps;

  if ( weapon <= WP_NONE || !( ps->stats[ STAT_WEAPONS ] & ( 1 << weapon ) ) )
  {
    ps->persistant[ PERS_NEWWEAPON ] = WP_CROWBAR;
  }
  else
  {
    ps->persistant[ PERS_NEWWEAPON ] = weapon;
  }

  // force this here to prevent flamer effect from continuing
  //ps->generic1 = WPM_NOTFIRING;

  // The PMove will do an animated drop, raise, and set the new weapon
  ps->pm_flags |= PMF_WEAPON_SWITCH;
}

/*
===============
Bullet_Fire
===============
*/
void Bullet_Fire(gentity_t *ent, float spread, int damage, meansOfDeath_t mod, int bulletCount)
{
  trace_t tr;
  vec3_t end;
  float r;
  float u;
  gentity_t *tent;
  gentity_t *traceEnt;
  int passent, i;
  int showBlood = qtrue;
//unlagged - attack prediction #2
  // we have to use something now that the client knows in advance
  int seed = ent->client->attackTime % 256;//unlagged - attack prediction #2
//unlagged - attack prediction #2


//unlagged - backward reconciliation #2
  // backward-reconcile the other clients
  G_DoTimeShiftFor( ent );//unlagged - backward reconciliation #2
//unlagged - backward reconciliation #2

  passent = ent->s.number;

  for (i = 0; i < bulletCount; i++)
  {

#if 0 //std non predict
    r = random() * M_PI * 2.0f;
    u = sin(r) * crandom() * spread * 16;
    r = cos(r) * crandom() * spread * 16;
#else
//unlagged - attack prediction #2
    // this has to match what's on the client
    r = Q_random(&seed) * M_PI * 2.0f;
    u = sin(r) * Q_crandom(&seed) * spread * 16;
    r = cos(r) * Q_crandom(&seed) * spread * 16;
//unlagged - attack prediction #2
#endif

    VectorMA(muzzle, 8192 * 16, forward, end);
    VectorMA(end, r, right, end);
    VectorMA(end, u,    up, end);

    trap_Trace(&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT);

    if (tr.surfaceFlags & SURF_NOIMPACT)
      return;

    if (tr.entityNum == ENTITYNUM_NONE)
      return;

    traceEnt = &g_entities[tr.entityNum];

    // snap the endpos to integers, but nudged towards the line
    SnapVectorTowards(tr.endpos, muzzle);

    // send bullet impact
    if (traceEnt->takedamage && traceEnt->client)
    {
      if (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, traceEnt)) //hypov8 check for same team damage
        tent = G_TempEntity(tr.endpos, EV_NONE);
      else
      {
        if (showBlood)
          tent = G_TempEntity(tr.endpos, EV_BULLET_HIT_FLESH);
        else
          tent = G_TempEntity(tr.endpos, EV_NONE);
      }
      tent->s.eventParm = traceEnt->s.number;

      // we need the client number to determine whether or not to
      // suppress this event
      tent->s.clientNum = ent->s.clientNum;//unlagged - attack prediction #2

      if (LogAccuracyHit(traceEnt, ent))
        ent->client->accuracy_hits++;

      showBlood = qfalse; //only show blood once
    }
    else
    {
      tent = G_TempEntity(tr.endpos, BG_GetSufaceType(tr.surfaceFlags));
      tent->s.eventParm = DirToByte(tr.plane.normal);

      // we need the client number to determine whether or not to
      // suppress this event
      tent->s.clientNum = ent->s.clientNum;//unlagged - attack prediction #2
    }

    tent->s.otherEntityNum = ent->s.number;
    tent->s.weapon = ent->s.weapon;

    if (traceEnt->takedamage)
    {
      vec3_t tempvec;
      int distoEnt;
      VectorSubtract(tr.endpos, ent->client->ps.origin, tempvec);
      distoEnt = VectorNormalize(tempvec);
      if (distoEnt < 32 && mod == MOD_SHOTGUN) //hypov8 add close range to damage amount
        damage *= 2;
      else if (distoEnt < 64 && mod == MOD_SHOTGUN) //hypov8 add close range to damage amount
        damage = (int)ceil(1.5f*damage);

      G_Damage(traceEnt, ent, ent, forward, tr.endpos, damage, 0, mod);
    }
  }

//unlagged - backward reconciliation #2
  // put them back
  G_UndoTimeShiftFor( ent );//unlagged - backward reconciliation #2
//unlagged - backward reconciliation #2
}

/*
===============
Melee_Fire
===============
*/
void Melee_Fire(gentity_t *ent, float attackDist, int damage, meansOfDeath_t mod)
{
  trace_t tr;
  vec3_t end;
  gentity_t *tent;
  gentity_t *traceEnt;
  int passent;

  if (ent->client->noclip)
    return;

  VectorMA(muzzle, attackDist, forward, end);

  passent = ent->s.number;

  // backward-reconcile the other clients
  G_DoTimeShiftFor( ent );//unlagged - backward reconciliation #2

  trap_Trace(&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT);
  // put them back
  G_UndoTimeShiftFor( ent );//unlagged - backward reconciliation #2


  if (tr.surfaceFlags & SURF_NOIMPACT)
    return;

  if (tr.entityNum == ENTITYNUM_NONE)
    return;

  traceEnt = &g_entities[tr.entityNum];

  // snap the endpos to integers, but nudged towards the line
  SnapVectorTowards(tr.endpos, muzzle);

  // send bullet impact
  if (traceEnt->takedamage && traceEnt->client)
  {
    if (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, traceEnt)) //no SFX for same team
      tent = G_TempEntity(tr.endpos, EV_NONE);
    else
      tent = G_TempEntity(tr.endpos, EV_BULLET_HIT_FLESH);

    tent->s.eventParm = traceEnt->s.number;

    // we need the client number to determine whether or not to
    // suppress this event
    tent->s.clientNum = ent->s.clientNum;//unlagged - attack prediction #2

    if (LogAccuracyHit(traceEnt, ent))
      ent->client->accuracy_hits++;

    tent->s.otherEntityNum = ent->s.number;
    tent->s.weapon = ent->s.weapon;
  }
  else
  {
    tent = G_TempEntity(tr.endpos, BG_GetSufaceType(tr.surfaceFlags));
    tent->s.eventParm = DirToByte(tr.plane.normal);
    // we need the client number to determine whether or not to
    // suppress this event
    tent->s.clientNum = ent->s.clientNum;//unlagged - attack prediction #2
    tent->s.otherEntityNum = ent->s.number;
    tent->s.weapon = ent->s.weapon;
  }

  if (traceEnt->takedamage)
  {
    G_Damage(traceEnt, ent, ent, forward, tr.endpos, damage, 0, mod);
  }
}

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile(vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout)
{
  vec3_t v, newv;
  float dot;

  VectorSubtract(impact, start, v);
  dot = DotProduct(v, dir);
  VectorMA(v, -2 * dot, dir, newv);

  VectorNormalize(newv);
  VectorMA(impact, 8192, newv, endout);
}

/*
======================================================================
BlackJack
======================================================================
*/
void Weapon_BlackJack_Fire(gentity_t *ent)
{
}

/*
======================================================================
Crowbar
======================================================================
*/
static void Weapon_Crowbar_Fire(gentity_t *ent)
{
  int damage = CROWBAR_DAMAGE;

  if (ent->client->noclip)
    return;

  if (g_weaponmod.integer == WM_HITMEN)
  damage = CROWBAR_DAMAGE_HM;

  if (g_weaponmod.integer & WM_REALMODE)
  damage = CROWBAR_DAMAGE_RM;

  Melee_Fire(ent, CROWBAR_DIST, damage, MOD_CROWBAR);
}


/*
======================================================================
PISTOL
======================================================================
*/
static void Weapon_Pistol_Fire(gentity_t *ent)
{
  int damage = PISTOL_DAMAGE;
  int spread = PISTOL_SPREAD;
  //int nulNum = qfalse;
  //int seed = ent->client->attackTime % 256;//unlagged - attack prediction #2

  if (g_weaponmod.integer == WM_HITMEN)
    damage = PISTOL_DAMAGE_HM;

  if (ent->client->ps.stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_PISTOLMAGNUM))
  {
    damage *= 2; //hypov8 note 100hp hitmen?
    spread = PISTOL_SPREAD / 3; //add accuracy to modded pistol?
  }

  Bullet_Fire(ent, spread, damage, MOD_PISTOL, 1);
}


/*
======================================================================
MACHINEGUN
======================================================================
*/
static void Weapon_Tomgun_Fire(gentity_t *ent)
{
  int damage = MACHINEGUN_DAMAGE;
  //int nulNum = qfalse;
  //int seed = ent->client->attackTime % 256;//unlagged - attack prediction #2

  if (g_gametype.integer == GT_TEAM)
    damage = MACHINEGUN_TEAM_DAMAGE;

  if (g_weaponmod.integer & WM_REALMODE)
    damage /= 2;

  Bullet_Fire(ent, MACHINEGUN_SPREAD, damage, MOD_MACHINEGUN, 1);
}


/*
======================
SnapVectorTowards
Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating
into a wall.
======================
*/
//unlagged - attack prediction #3
// moved to q_shared.c
/*
void SnapVectorTowards(vec3_t v, vec3_t to)
{
  int i;
  for (i = 0; i < 3; i++)
  {
    if (to[i] <= v[i])
    {
      //v[i] = (int)v[i];
      v[i] = floor(v[i]); //hypov8 et
    }
    else
    {
      //v[i] = (int)v[i] + 1;
      v[i] = ceil(v[i]); //hypov8 et

  }
}
}
*/
//unlagged - attack prediction #3


/*
======================================================================
BFG
======================================================================
*/
#ifdef ALLQ3ITEMS
void BFG_Fire(gentity_t *ent)
{
  gentity_t *m;

  m = fire_bfg(ent, muzzle, forward);

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}
#endif

/*
======================================================================
SHOTGUN
======================================================================
*/
// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads

void Weapon_Shotgun_Fire(gentity_t *ent)
{
  gentity_t		*tent;
  int spread;
  int damage = DEFAULT_SHOTGUN_DAMAGE;
  

  // send shotgun blast
  tent = G_TempEntity( muzzle, EV_FIRE_SHOTGUN);
  VectorScale( forward, 4096, tent->s.origin2 );
  SnapVector( tent->s.origin2 );

  tent->s.otherEntityNum = ent->s.number;

  if (g_weaponmod.integer & WM_REALMODE)
    spread = DEFAULT_SHOTGUN_SPREAD_RM;
  else
    spread = DEFAULT_SHOTGUN_SPREAD;

  Bullet_Fire(ent, spread, damage, MOD_SHOTGUN, DEFAULT_SHOTGUN_COUNT);
}


/*
======================================================================
GRENADE LAUNCHER
======================================================================
*/
void Weapon_Grenadelauncher_Fire(gentity_t *ent)
{
  //gentity_t *m;
  // extra vertical velocity
  //forward[2] += 0.2f;
  //VectorNormalize(forward);
  /*m =*/ fire_grenade(ent, muzzle, forward);

  /* hypov8 disabled. makes grenade follow user, hard to aim. FIXED in fire_grenade */
  //VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
  //FIXME(0xA5EA): enabled this for testing

  /* hypov8 need to add it to grenade distance. not position */
}

/*
======================================================================
ROCKET
======================================================================
*/
void Weapon_RocketLauncher_Fire(gentity_t *ent)
{
  //gentity_t *m;
  fire_rocket(ent, muzzle, forward);
  //VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

/*
======================================================================
PLASMA GUN
======================================================================
*/
#ifdef ALLQ3ITEMS
void Weapon_Plasmagun_Fire(gentity_t *ent)
{
  gentity_t *m;

  m                = fire_plasma(ent, muzzle, forward);
  m->damage       *= s_quadFactor;
  m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}
#endif

/*
======================================================================
RAILGUN
======================================================================
*/
/*
=================
weapon_railgun_fire
=================
*/
#if 0
#define MAX_RAIL_HITS 4
void weapon_railgun_fire(gentity_t *ent)
{
  vec3_t end;

//#ifdef MISSIONPACK
//	vec3_t impactpoint, bouncedir;
//#endif
  trace_t trace;
  gentity_t *tent;
  gentity_t *traceEnt;
  int damage;
  int i;
  int hits;
  int unlinked;
  int passent;
  gentity_t *unlinkedEntities[MAX_RAIL_HITS];

  damage = 100 ;

  VectorMA(muzzle, 8192, forward, end);
//unlagged - backward reconciliation #2
  // backward-reconcile the other clients
  G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

  // trace only against the solids, so the railgun will go through people
  unlinked = 0;
  hits     = 0;
  passent  = ent->s.number;
  do
  {
    trap_Trace(&trace, muzzle, NULL, NULL, end, passent, MASK_SHOT);

    if (trace.entityNum >= ENTITYNUM_MAX_NORMAL)
      break;

    traceEnt = &g_entities[trace.entityNum];
    if (traceEnt->takedamage)
    {
#if 0 //ef MISSIONPACK
      if (traceEnt->client && traceEnt->client->invulnerabilityTime > level.time)
      {
        if (G_InvulnerabilityEffect(traceEnt, forward, trace.endpos, impactpoint, bouncedir))
        {
          G_BounceProjectile(muzzle, impactpoint, bouncedir, end);
          // snap the endpos to integers to save net bandwidth, but nudged towards the line
          SnapVectorTowards(trace.endpos, muzzle);
          // send railgun beam effect
          tent = G_TempEntity(trace.endpos, EV_RAILTRAIL);
          // set player number for custom colors on the railtrail
          tent->s.clientNum = ent->s.clientNum;
          VectorCopy(muzzle, tent->s.origin2);
          // move origin a bit to come closer to the drawn gun muzzle
          VectorMA(tent->s.origin2, 4, right, tent->s.origin2);
          VectorMA(tent->s.origin2, -1, up, tent->s.origin2);
          tent->s.eventParm = 255;    // don't make the explosion at the end
          //
          VectorCopy(impactpoint, muzzle);
          // the player can hit him/herself with the bounced rail
          passent = ENTITYNUM_NONE;
        }
      }
      else
      {
        if (LogAccuracyHit(traceEnt, ent))
        {
          hits++;
        }
        G_Damage(traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_HMG);
      }
#else
      if (LogAccuracyHit(traceEnt, ent))
      {				hits++;
    }
    G_Damage(traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_HMG);
#endif
    }
    if (trace.contents & CONTENTS_SOLID)
      break;      // we hit something solid enough to stop the beam

    // unlink this entity, so the next trace will go past it
    trap_UnlinkEntity(traceEnt);
    unlinkedEntities[unlinked] = traceEnt;
    unlinked++;
  }
  while(unlinked < MAX_RAIL_HITS);
//unlagged - backward reconciliation #2
  // put them back
  G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

  // link back in any entities we unlinked
  for (i = 0; i < unlinked; i++)
  {
    trap_LinkEntity(unlinkedEntities[i]);
  }

  // the final trace endpos will be the terminal point of the rail trail
  // snap the endpos to integers to save net bandwidth, but nudged towards the line
  SnapVectorTowards(trace.endpos, muzzle);

  // send railgun beam effect
  tent = G_TempEntity(trace.endpos, EV_RAILTRAIL);

  // set player number for custom colors on the railtrail
  tent->s.clientNum = ent->s.clientNum;

  VectorCopy(muzzle, tent->s.origin2);
  // move origin a bit to come closer to the drawn gun muzzle
  VectorMA(tent->s.origin2, 4, right, tent->s.origin2);
  VectorMA(tent->s.origin2, -1, up, tent->s.origin2);

  // no explosion at end if SURF_NOIMPACT, but still make the trail
  if (trace.surfaceFlags & SURF_NOIMPACT)
    tent->s.eventParm = 255;    // don't make the explosion at the end
  else
    tent->s.eventParm = DirToByte(trace.plane.normal);

  tent->s.clientNum = ent->s.clientNum;

  // give the shooter a reward sound if they have made two railgun hits in a row
  if (hits == 0)
  {
    // complete miss
    ent->client->accurateCount = 0;
  }
  else
  {
    // check for "impressive" reward sound
    ent->client->accurateCount += hits;
    #if 0 // 0xA5EA, removed impressive
    if (ent->client->accurateCount >= 2)
    {
      ent->client->accurateCount -= 2;
      ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
      // add the sprite over the player's head
      ent->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);
      ent->client->ps.eFlags |= EF_AWARD_IMPRESSIVE;
      ent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
    }
    #endif
    ent->client->accuracy_hits++;
  }
}
#endif

static void Weapon_Hmg_Fire(gentity_t *ent)
{
  //int nulNum = qfalse;
  //int seed = ent->client->attackTime % 256;//unlagged - attack prediction #2
  if ((ent->client->ps.stats[STAT_WEAP_MODS] & (1 << PW_WPMOD_COOLING)))
    ent->client->hmgShotsWithCooling--;

  if (ent->client->hmgShotsWithCooling <= 0)
    ent->client->ps.stats[STAT_WEAP_MODS] &= ~(1 << PW_WPMOD_COOLING);

  Bullet_Fire(ent, HMG_SPREAD, HMG_DAMAGE, MOD_HMG, 1);
}
/*
======================================================================
GRAPPLING HOOK
======================================================================
*/
void Weapon_GrapplingHook_Fire(gentity_t *ent)
{
  if (!ent->client->fireHeld && !ent->client->hook)
    fire_grapple(ent, muzzle, forward);

  ent->client->fireHeld = qtrue;
}

void Weapon_HookFree(gentity_t *ent)
{
  ent->parent->client->hook         = NULL;
  ent->parent->client->ps.pm_flags &= ~PMF_GRAPPLE_PULL;
  G_FreeEntity(ent);
}

void Weapon_HookThink(gentity_t *ent)
{
  if (ent->enemy)
  {
    vec3_t v, oldorigin;

    VectorCopy(ent->r.currentOrigin, oldorigin);

    v[0] = ent->enemy->r.currentOrigin[0] + (ent->enemy->r.mins[0] + ent->enemy->r.maxs[0]) * 0.5;
    v[1] = ent->enemy->r.currentOrigin[1] + (ent->enemy->r.mins[1] + ent->enemy->r.maxs[1]) * 0.5;
    v[2] = ent->enemy->r.currentOrigin[2] + (ent->enemy->r.mins[2] + ent->enemy->r.maxs[2]) * 0.5;

    SnapVectorTowards(v, oldorigin); // save net bandwidth

    G_SetOrigin(ent, v);
  }

  VectorCopy(ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);
}

/*
======================================================================
LIGHTNING GUN
======================================================================
*/


#ifdef USE_FLAMEGUN
// Added -KRYPTYK
void G_FlamerInflictPlayerDamage( gentity_t *self, gentity_t *body )//G_BurnMeGood
{
  // add the new damage
  body->flameQuota += 5;
  body->flameQuotaTime = level.time;

  G_Damage( body, self->parent, self->parent, vec3_origin, self->r.currentOrigin, self->damage, DAMAGE_NO_ARMOR, MOD_FLAMEGUN );
  body->lastBurnedFrameNumber = level.framenum;

  // make em burn for extended time
  if( body->client && (body->health <= 0 || body->flameQuota > 0) )  // JPW NERVE was > FLAME_THRESHOLD
  {
    if (body->s.onFireEnd < level.time)
      body->s.onFireStart = level.time;

    body->s.onFireEnd = level.time + FIRE_FLASH_TIME;
    body->flameBurnEnt = self->r.ownerNum;
    // add to playerState for client-side effect
    //body->client->ps.onFireStart = level.time; //hypov8 todo: not used??
  }
}

void Weapon_Flamethrower_Fire(gentity_t *ent)
{
  fire_flamechunk ( ent, muzzle, forward );
}
#endif



//======================================================================
/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit(gentity_t *target, gentity_t *attacker)
{
  if (!target->takedamage)
    return qfalse;

  if (target == attacker)
    return qfalse;

  if (!target->client)
    return qfalse;

  if (!attacker->client)
    return qfalse;

  if (target->client->ps.stats[STAT_HEALTH] <= 0)
    return qfalse;

  if (OnSameTeam(target, attacker))
    return qfalse;

  return qtrue;
}

/*
===============
CalcMuzzlePoint
set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint(gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint)
{
  VectorCopy(ent->s.pos.trBase, muzzlePoint);
  muzzlePoint[2] += ent->client->ps.viewheight;
  VectorMA(muzzlePoint, 14, forward, muzzlePoint);
  // snap to integer coordinates for more efficient network bandwidth usage
  SnapVector(muzzlePoint);
}

/*
===============
CalcMuzzlePointOrigin
set muzzle location relative to pivoting eye
===============
*/
static void CalcMuzzlePointOrigin(gentity_t *ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint)
{
  VectorCopy(ent->s.pos.trBase, muzzlePoint);
  muzzlePoint[2] += ent->client->ps.viewheight;
  VectorMA(muzzlePoint, 14, forward, muzzlePoint);
  // snap to integer coordinates for more efficient network bandwidth usage
  SnapVector(muzzlePoint);
}

/*
===============
FireWeapon
===============
*/
void FireWeapon(gentity_t *ent)
{
  // track shots taken for accuracy tracking.  Grapple is not a weapon and gauntet is just not tracked
  if (ent->s.weapon != WP_GRAPPLING_HOOK && ent->s.weapon != WP_CROWBAR)
  {
    //FIXME(0xA5EA): how about flamegun?
    if (ent->s.weapon == WP_SHOTGUN)
      ent->client->accuracy_shots += DEFAULT_SHOTGUN_COUNT;
    else
      ent->client->accuracy_shots++;
  }

  // set aiming directions
  AngleVectors(ent->client->ps.viewangles, forward, right, up);

  CalcMuzzlePointOrigin(ent, ent->client->oldOrigin, forward, right, up, muzzle);

  // fire the specific weapon
  switch(ent->s.weapon)
  {
    case WP_CROWBAR:
      Weapon_Crowbar_Fire(ent);
      break;

    case WP_PISTOL:
      Weapon_Pistol_Fire(ent);
      break;

    case WP_SHOTGUN:
      Weapon_Shotgun_Fire(ent);
      break;

    case WP_MACHINEGUN:
      Weapon_Tomgun_Fire(ent);
      break;

    case WP_GRENADE_LAUNCHER:
      Weapon_Grenadelauncher_Fire(ent);
      break;

    case WP_ROCKET_LAUNCHER:
      Weapon_RocketLauncher_Fire(ent);
      break;

    case WP_HMG:        //FIXME: 0xA5EA, ok for hmg ?
      Weapon_Hmg_Fire(ent);
      break;

    case WP_GRAPPLING_HOOK:
      Weapon_GrapplingHook_Fire(ent);
      break;
#ifdef USE_FLAMEGUN
    case WP_FLAMER:
      Weapon_Flamethrower_Fire(ent);
      break;
#endif
    default:
      //G_Error("Bad ent->s.weapon %d", ent->s.weapon);
      break;
  }
}

/*
======================================================================
NAILGUN
======================================================================
*/
#if 0
void Weapon_Nailgun_Fire(gentity_t *ent)
{
  //gentity_t *m;
  int count;

  for (count = 0; count < NUM_NAILSHOTS; count++)
  {
    fire_nail(ent, muzzle, forward, right, up);
  }
//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta ); // "real" physics
}
#endif

#if 0 // FIXME: 0xA5EA
/*
======================================================================
PROXIMITY MINE LAUNCHER
======================================================================
*/
void weapon_proxlauncher_fire(gentity_t *ent)
{
  gentity_t *m;

  // extra vertical velocity
  forward[2] += 0.2f;
  VectorNormalize(forward);

  m                = fire_prox(ent, muzzle, forward);
  m->damage       *= s_quadFactor;
  m->splashDamage *= s_quadFactor;

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta ); // "real" physics
}

//#endif
#endif // 0 0xA5EA

#ifdef USE_KAMIKAZE
/*
===============
KamikazeRadiusDamage
===============
*/
static void KamikazeRadiusDamage(vec3_t origin, gentity_t *attacker, float damage, float radius)
{
  float dist;
  gentity_t *ent;
  int entityList[MAX_GENTITIES];
  int numListedEntities;
  vec3_t mins, maxs;
  vec3_t v;
  vec3_t dir;
  int i, e;

  if (radius < 1)
    radius = 1;

  for (i = 0; i < 3; i++)
  {
    mins[i] = origin[i] - radius;
    maxs[i] = origin[i] + radius;
  }

  numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

  for (e = 0; e < numListedEntities; e++)
  {
    ent = &g_entities[entityList[e]];

    if (!ent->takedamage)
      continue;

    // dont hit things we have already hit
    if (ent->kamikazeTime > level.time)
      continue;

    // find the distance from the edge of the bounding box
    for (i = 0; i < 3; i++)
    {
      if (origin[i] < ent->r.absmin[i])
        v[i] = ent->r.absmin[i] - origin[i];
      else if (origin[i] > ent->r.absmax[i])
        v[i] = origin[i] - ent->r.absmax[i];
      else
        v[i] = 0;
    }

    dist = VectorLength(v);

    if (dist >= radius)
      continue;

//		if ( CanDamage (ent, origin) ) {
    VectorSubtract(ent->r.currentOrigin, origin, dir);
    // push the center of mass higher than the origin so players
    // get knocked into the air more
    dir[2] += 24;
    G_Damage(ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS | DAMAGE_NO_TEAM_PROTECTION, MOD_KAMIKAZE);
    ent->kamikazeTime = level.time + 3000;
//		}
  }
}

/*
===============
KamikazeShockWave
===============
*/
static void KamikazeShockWave(vec3_t origin, gentity_t *attacker, float damage, float push, float radius)
{
  float dist;
  gentity_t *ent;
  int entityList[MAX_GENTITIES];
  int numListedEntities;
  vec3_t mins, maxs;
  vec3_t v;
  vec3_t dir;
  int i, e;

  if (radius < 1)
    radius = 1;

  for (i = 0; i < 3; i++)
  {
    mins[i] = origin[i] - radius;
    maxs[i] = origin[i] + radius;
  }

  numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

  for (e = 0; e < numListedEntities; e++)
  {
    ent = &g_entities[entityList[e]];

    // dont hit things we have already hit
    if (ent->kamikazeShockTime > level.time)
      continue;

    // find the distance from the edge of the bounding box
    for (i = 0; i < 3; i++)
    {
      if (origin[i] < ent->r.absmin[i])
        v[i] = ent->r.absmin[i] - origin[i];
      else if (origin[i] > ent->r.absmax[i])
        v[i] = origin[i] - ent->r.absmax[i];
      else
        v[i] = 0;
    }

    dist = VectorLength(v);
    if (dist >= radius)
      continue;

//		if ( CanDamage (ent, origin) ) {
    VectorSubtract(ent->r.currentOrigin, origin, dir);
    dir[2] += 24;
    G_Damage(ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS | DAMAGE_NO_TEAM_PROTECTION, MOD_KAMIKAZE);
    //
    dir[2] = 0;
    VectorNormalize(dir);
    if (ent->client)
    {
      ent->client->ps.velocity[0] = dir[0] * push;
      ent->client->ps.velocity[1] = dir[1] * push;
      ent->client->ps.velocity[2] = 100;
    }
    ent->kamikazeShockTime = level.time + 3000;
//		}
  }
}

/*
===============
KamikazeDamage
===============
*/
static void KamikazeDamage(gentity_t *self)
{
  int i;
  float t;
  gentity_t *ent;
  vec3_t newangles;

  self->count += 100;

  if (self->count >= KAMI_SHOCKWAVE_STARTTIME)
  {
    // shockwave push back
    t = self->count - KAMI_SHOCKWAVE_STARTTIME;
    KamikazeShockWave(self->s.pos.trBase, self->activator, 25, 400, (int)(float) t * KAMI_SHOCKWAVE_MAXRADIUS / (KAMI_SHOCKWAVE_ENDTIME - KAMI_SHOCKWAVE_STARTTIME));
  }
  //
  if (self->count >= KAMI_EXPLODE_STARTTIME)
  {
    // do our damage
    t = self->count - KAMI_EXPLODE_STARTTIME;
    KamikazeRadiusDamage(self->s.pos.trBase, self->activator, 400, (int)(float) t * KAMI_BOOMSPHERE_MAXRADIUS / (KAMI_IMPLODE_STARTTIME - KAMI_EXPLODE_STARTTIME));
  }

  // either cycle or kill self
  if (self->count >= KAMI_SHOCKWAVE_ENDTIME)
  {
    G_FreeEntity(self);
    return;
  }
  self->nextthink = level.time + 100;

  // add earth quake effect
  newangles[0] = crandom() * 2;
  newangles[1] = crandom() * 2;
  newangles[2] = 0;
  for (i = 0; i < MAX_CLIENTS; i++)
  {
    ent = &g_entities[i];

    if (!ent->inuse)
      continue;

    if (!ent->client)
      continue;

    if (ent->client->ps.groundEntityNum != ENTITYNUM_NONE)
    {
      ent->client->ps.velocity[0] += crandom() * 120;
      ent->client->ps.velocity[1] += crandom() * 120;
      ent->client->ps.velocity[2]  = 30 + random() * 25;
    }

    ent->client->ps.delta_angles[0] += ANGLE2SHORT(newangles[0] - self->movedir[0]);
    ent->client->ps.delta_angles[1] += ANGLE2SHORT(newangles[1] - self->movedir[1]);
    ent->client->ps.delta_angles[2] += ANGLE2SHORT(newangles[2] - self->movedir[2]);
  }
  VectorCopy(newangles, self->movedir);
}

/*
===============
G_StartKamikaze
===============
*/
void G_StartKamikaze(gentity_t *ent)
{
  gentity_t *explosion;
  gentity_t *te;
  vec3_t snapped;

  // start up the explosion logic
  explosion = G_Spawn();

  explosion->s.eType   = ET_EVENTS + EV_KAMIKAZE;
  explosion->eventTime = level.time;

  if (ent->client)
    VectorCopy(ent->s.pos.trBase, snapped);
  else
    VectorCopy(ent->activator->s.pos.trBase, snapped);

  SnapVector(snapped);     // save network bandwidth
  G_SetOrigin(explosion, snapped);

  explosion->classname    = "kamikaze";
  explosion->s.pos.trType = TR_STATIONARY;

  explosion->kamikazeTime = level.time;

  explosion->think     = KamikazeDamage;
  explosion->nextthink = level.time + 100;
  explosion->count     = 0;
  VectorClear(explosion->movedir);

  trap_LinkEntity(explosion);

  if (ent->client)
  {
    explosion->activator = ent;
    ent->s.eFlags &= ~EF_KAMIKAZE;
    // nuke the guy that used it
    G_Damage(ent, ent, ent, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_KAMIKAZE);
  }
  else
  {
    if (!qstrcmp(ent->activator->classname, "bodyque"))
      explosion->activator = &g_entities[ent->activator->r.ownerNum];
    else
      explosion->activator = ent->activator;
  }

  // play global sound at all clients
  te              = G_TempEntity(snapped, EV_GLOBAL_TEAM_SOUND);
  te->r.svFlags  |= SVF_BROADCAST;
  te->s.eventParm = GTS_KAMIKAZE;
}
#endif
