/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2010 Lars '0xA5EA' Kandler
Copyright (C) 2011 Ari 'KRYPTYK' Mirles

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
#include "g_local.h"

#define MISSILE_PRESTEP_TIME 50

// Added for flamethrower -KRYPTYK


//=============================================================================
// DHM - Nerve :: Server side Flamethrower
//=============================================================================

// copied from cg_flamethrower.c
#define	FLAME_START_SIZE		1.0
#define	FLAME_START_MAX_SIZE	92.0	//radious	// when the flame is spawned, it should endevour to reach this size
#define	FLAME_START_SPEED		1200.0	// speed of flame as it leaves the nozzle
#define	FLAME_MIN_SPEED			60.0

// these are calculated (don't change)
#define	FLAME_LENGTH			(FLAMETHROWER_RANGE + 50.0)	// NOTE: only modify the range, since this should always reflect that range

#define	FLAME_LIFETIME			800	// life duration in milliseconds
#define	FLAME_FRICTION_PER_SEC	(2.0f*FLAME_START_SPEED)
#define	GET_FLAME_SIZE_SPEED(x)	(((float)x / FLAME_LIFETIME) / 0.3)	// x is the current sizeMax

#define	FLAME_THRESHOLD	50


void G_FlamerBurnTarget( gentity_t *self, gentity_t *body, qboolean directhit )
{
	int i;
  float radius, dist;
  vec3_t point, v;
  trace_t tr;

  if (!body->takedamage)
    return;

// JPW NERVE don't catch fire if same team in no FF
  if (body->client && body->s.number != self->r.ownerNum)
  {
    if (!(g_friendlyFire.integer) && OnSameTeam(body, self->parent))
      return;
  }
// jpw

// JPW NERVE don't catch fire if under water or invulnerable
  if (body->waterlevel >= 3)
  {
    body->flameQuota = 0;
    body->s.onFireEnd = level.time - 1;
    return;
  }
// jpw

  if (!body->r.bmodel)
  {
    VectorCopy( body->r.currentOrigin, point);
    if (body->client)
      point[2] += body->client->ps.viewheight;
    VectorSubtract( point, self->r.currentOrigin, v);
  }
  else
  {
    for (i = 0; i < 3; i++)
    {
      if (self->s.origin[i] < body->r.absmin[i])
        v[i] = body->r.absmin[i] - self->r.currentOrigin[i];
      else if (self->r.currentOrigin[i] > body->r.absmax[i])
        v[i] = self->r.currentOrigin[i] - body->r.absmax[i];
      else
        v[i] = 0;
    }
  }

  radius = self->speed;

  dist = VectorLength(v);

  // The person who shot the flame only burns when within 1/2 the radius
  /*if (body->s.number == self->r.ownerNum && dist >= (radius * 0.5))
    return;
  if (!directhit && dist >= radius)
    return;*/ //hypov8 todo: should be in trace

  // Non-clients that take damage get damaged here
  if (!body->client)
  {
    if (body->health > 0)
      G_Damage(body, self->parent, self->parent, vec3_origin, self->r.currentOrigin, self->damage, DAMAGE_NO_ARMOR, MOD_FLAMEGUN);
    return;
  }

  // JPW NERVE -- do a trace to see if there's a wall btwn. body & flame centroid -- prevents damage through walls
  trap_Trace(&tr, self->r.currentOrigin, NULL, NULL, point, body->s.number, MASK_SHOT);
  if (tr.fraction < 1.0)
    return;
  // jpw

  // now check the damageQuota to see if we should play a pain animation
  // first reduce the current damageQuota with time
  if (body->flameQuotaTime && body->flameQuota > 0)
  {
    body->flameQuota -= (int) (((float) (level.time - body->flameQuotaTime) / 1000) * 2.5f);
    if (body->flameQuota < 0)
      body->flameQuota = 0;
  }

  G_FlamerInflictPlayerDamage( self, body );//G_BurnMeGood
}

void G_FlameDamageAllPlayers( gentity_t *self, gentity_t *ignoreent) //G_FlameDamage
{
	gentity_t	*body;
	int			entityList[MAX_GENTITIES];
	int			i, e, numListedEntities;
	float		radius, boxradius;
	vec3_t		mins, maxs;

	radius = self->speed;
	boxradius = 1.41421356 * radius; // radius * sqrt(2) for bounding box enlargement

	for ( i = 0 ; i < 3 ; i++ )
	{
		mins[i] = self->r.currentOrigin[i] - boxradius;
		maxs[i] = self->r.currentOrigin[i] + boxradius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ )
	{
		body = &g_entities[entityList[ e ]];

		if( body == ignoreent )
			continue;

		G_FlamerBurnTarget( self, body, qfalse );
	}
}


void G_MissileImpact(gentity_t *ent, trace_t *trace);
/*
=================
G_RunFlamechunk

ET_FLAMETHROWER_CHUNK
use G_RunMissile code to bounce of walls
=================
*/
void G_RunFlamechunk( gentity_t *ent )
{
	trace_t	tr;
	vec3_t		origin, mins, maxs;
	int timeInUse = level.time - ent->timestamp;
	//float percent = ((float)timeInUse/ FLAME_LIFETIME) * 0.80f +0.20f;
	float radius, boxradius;
	int passent = ENTITYNUM_NONE;
	int timer = level.time - ent->timestamp;

	// ignore interactions with the missile owner for short time
	if (timer <  120 )
		passent = ent->r.ownerNum;

	// get current position after move
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	radius = ent->speed;
	boxradius = 1.41421356 * radius; // radius * sqrt(2) for bounding box enlargement
	//set bbox for player trace
	VectorSet(mins, -boxradius, -boxradius, -boxradius);
	VectorSet(maxs, boxradius, boxradius, boxradius);

//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	G_DoTimeShiftFor( &g_entities[ ent->r.ownerNum ] );
//unlagged - backward reconciliation #2

	// trace for humans only
	trap_Trace (&tr, ent->r.currentOrigin, mins, maxs, origin, passent, CONTENTS_BODY);
	if ( tr.startsolid || tr.allsolid )
	{	
		// make sure the tr.entityNum is set to the entity we're stuck in
		trap_Trace (&tr, ent->r.currentOrigin, mins, maxs, ent->r.currentOrigin, passent, CONTENTS_BODY);
		tr.fraction = 0;
	}
	if ( tr.fraction == 1 ) //no human hit
	{
		//trace for flame bounce
		trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, CONTENTS_SOLID | MASK_WATER);
		if ( tr.startsolid || tr.allsolid )
		{	
			// make sure the tr.entityNum is set to the entity we're stuck in
			trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask);
			tr.fraction = 0;
		}
		else
		{
			VectorCopy(tr.endpos, ent->r.currentOrigin);
		}
	}
	else
	{	//set origin to human impact position
		VectorCopy(tr.endpos, ent->r.currentOrigin);
	}
//unlagged - backward reconciliation #2
	// put them back
	G_UndoTimeShiftFor( &g_entities[ ent->r.ownerNum ]);
//unlagged - backward reconciliation #2


	trap_LinkEntity( ent );

	if ( tr.fraction != 1 ) //hit something
	{
		// never explode/bounce on sky/water
		if ( ( tr.surfaceFlags & SURF_NOIMPACT ) || (tr.contents & (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER ) ) )
		{
			G_FreeEntity( ent );
			return;
		}

		// we hit an entity
		if( tr.entityNum != ENTITYNUM_WORLD && tr.entityNum != ENTITYNUM_NONE )
		{
			gentity_t *ignoreEnt = NULL;
			if (timer <  120 )
				ignoreEnt = &g_entities[ ent->r.ownerNum ];
			G_FlameDamageAllPlayers(ent, ignoreEnt);

			ent->freeAfterEvent = qtrue;
			ent->s.eType = ET_GENERAL;
			SnapVectorTowards(tr.endpos, ent->s.pos.trBase); // save net bandwidth
			G_SetOrigin(ent, tr.endpos);
			trap_LinkEntity(ent);
			return;
		}

		G_MissileImpact( ent, &tr );
		if ( ent->s.eType != ET_FLAMETHROWER_CHUNK ) 
			return;	// exploded
	}


	// Adjust the size
	if ( ent->speed < FLAME_START_MAX_SIZE )
	{
		ent->speed += 8.0f;
		if ( ent->speed > FLAME_START_MAX_SIZE )
			ent->speed = FLAME_START_MAX_SIZE;
	}

	// Remove after timeout
	if ( level.time - ent->timestamp > (FLAME_LIFETIME) )
	{
		G_FreeEntity( ent );
		return;
	}

	G_RunThink( ent );
}

/*
=================
fire_flamechunk
=================
*/
gentity_t *fire_flamechunk(gentity_t *self, vec3_t start, vec3_t dir)
{
	gentity_t	*bolt;
	// Only spawn every other frame
	if (self->countLastShootFrame > level.time)
	{	//self->count2--;
		return NULL;
	}

	self->countLastShootFrame = level.time+100; //flamer damage time
	VectorNormalize(dir);

	// Added think -KRYPTYK
	bolt = G_Spawn();
	bolt->classname		= "flamechunk";
	bolt->nextthink		= level.time + 2000; // should be removed by this time
	bolt->think			= G_FreeEntity;
	bolt->timestamp			= level.time;
	bolt->flameQuotaTime	= level.time + 50;
	bolt->s.eType			= ET_FLAMETHROWER_CHUNK;
	bolt->r.svFlags		= SVF_USE_CURRENT_ORIGIN;
#ifndef HYPODEBUG						//flamer sfx uses an event msg and done localy to save bandwidth
	bolt->r.svFlags		|= SVF_NOCLIENT; //dont send entity to client
#endif
	bolt->s.weapon			= WP_FLAMER;
	bolt->r.ownerNum		= self->s.number;
	bolt->parent			= self;
	bolt->damage			= 2;//(rand()%2) ? 2 : 1; //rand damage value between 1-2
	bolt->methodOfDeath		= MOD_FLAMEGUN;
	bolt->clipmask			= MASK_SHOT;
	bolt->countLastShootFrame = 0;	// how often it bounced off of something
	bolt->target_ent		= NULL;
	bolt->s.eFlags			= EF_BOUNCE_HALF | EF_BOUNCE;
	bolt->s.pos.trType		= TR_DECCELERATE;
	bolt->s.pos.trTime		= level.time; // -MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	bolt->s.pos.trDuration	= FLAME_LIFETIME;

	// 'speed' will be the current size radius of the chunk. bbox upsized later
	bolt->speed = FLAME_START_SIZE;
	VectorSet (bolt->r.mins, -4, -4, -4);
	VectorSet (bolt->r.maxs,  4,  4,  4);
	VectorCopy(start, bolt->s.pos.trBase);

	//set direction
	VectorScale(dir, FLAME_START_SPEED, bolt->s.pos.trDelta);
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

/*
================
G_BounceMissile
================
*/
void G_BounceMissile(gentity_t *ent, trace_t *trace)
{
	vec3_t velocity;
	float dot;
	int hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + (level.time - level.previousTime) * trace->fraction;

	BG_EvaluateTrajectoryDelta(&ent->s.pos, hitTime, velocity);

	dot = DotProduct(velocity, trace->plane.normal);

	VectorMA(velocity, -2 * dot, trace->plane.normal, ent->s.pos.trDelta);

	if (ent->s.eFlags & EF_BOUNCE_HALF)
	{
		VectorScale(ent->s.pos.trDelta, 0.65f, ent->s.pos.trDelta);

		// check for stop
		if (trace->plane.normal[2] > 0.2 && VectorLength(ent->s.pos.trDelta) < 40)
		{
			G_SetOrigin(ent, trace->endpos);
			ent->s.time = level.time / 4;
			return;
		}
	}

	VectorAdd(ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
	ent->s.pos.trTime = level.time;
}

/*
================
G_ExplodeMissile
Explode a missile without an impact
================
*/
void G_ExplodeMissile(gentity_t *ent)
{
	vec3_t dir;
	vec3_t origin;

	BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);
	SnapVector(origin);
	G_SetOrigin(ent, origin);

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent(ent, EV_MISSILE_MISS, DirToByte(dir));

	ent->freeAfterEvent = qtrue;

	// splash damage
	if (ent->splashDamage)
	{
		if (G_RadiusDamage(ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent, ent->splashMethodOfDeath))
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
	}
	trap_LinkEntity(ent);
}

#if 0  // FIXME: 0xA5EA
//#ifdef MISSIONPACK
/*
================
ProximityMine_Explode
================
*/
static void ProximityMine_Explode(gentity_t *mine)
{
	G_ExplodeMissile(mine);
	// if the prox mine has a trigger free it
	if (mine->activator)
	{
		G_FreeEntity(mine->activator);
		mine->activator = NULL;
	}
}

/*
================
ProximityMine_Die
================
*/
static void ProximityMine_Die(gentity_t *ent, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	ent->think     = ProximityMine_Explode;
	ent->nextthink = level.time + 1;
}

/*
================
ProximityMine_Trigger
================
*/
void ProximityMine_Trigger(gentity_t *trigger, gentity_t *other, trace_t *trace)
{
	vec3_t v;
	gentity_t *mine;

	if (!other->client)
		return;

	// trigger is a cube, do a distance test now to act as if it's a sphere
	VectorSubtract(trigger->s.pos.trBase, other->s.pos.trBase, v);
	if (VectorLength(v) > trigger->parent->splashRadius)
		return;

	if (g_gametype.integer >= GT_TEAM)
	{
		// don't trigger same team mines
		if (trigger->parent->s.generic1 == other->client->sess.sessionTeam)
			return;
	}

	// ok, now check for ability to damage so we don't get triggered thru walls, closed doors, etc...
	if (!CanDamage(other, trigger->s.pos.trBase))
		return;

	// trigger the mine!
	mine              = trigger->parent;
	mine->s.loopSound = 0;
	G_AddEvent(mine, EV_PROXIMITY_MINE_TRIGGER, 0);
	mine->nextthink = level.time + 500;

	G_FreeEntity(trigger);
}

/*
================
ProximityMine_Activate
================
*/
static void ProximityMine_Activate(gentity_t *ent)
{
	gentity_t *trigger;
	float r;

	ent->think     = ProximityMine_Explode;
	ent->nextthink = level.time + g_proxMineTimeout.integer;

	ent->takedamage = qtrue;
	ent->health     = 1;
	ent->die        = ProximityMine_Die;

	ent->s.loopSound = G_SoundIndex("sound/weapons/proxmine/wstbtick.ogg");

	// build the proximity trigger
	trigger = G_Spawn();

	trigger->classname = "proxmine_trigger";

	r = ent->splashRadius;
	VectorSet(trigger->r.mins, -r, -r, -r);
	VectorSet(trigger->r.maxs, r, r, r);

	G_SetOrigin(trigger, ent->s.pos.trBase);

	trigger->parent     = ent;
	trigger->r.contents = CONTENTS_TRIGGER;
	trigger->touch      = ProximityMine_Trigger;

	trap_LinkEntity(trigger);

	// set pointer to trigger so the entity can be freed when the mine explodes
	ent->activator = trigger;
}

/*
================
ProximityMine_ExplodeOnPlayer
================
*/
static void ProximityMine_ExplodeOnPlayer(gentity_t *mine)
{
	gentity_t *player;

	player                     = mine->enemy;
	player->client->ps.eFlags &= ~EF_TICKING;

	if (player->client->invulnerabilityTime > level.time)
	{
		G_Damage(player, mine->parent, mine->parent, vec3_origin, mine->s.origin, 1000, DAMAGE_NO_KNOCKBACK, MOD_JUICED);
		player->client->invulnerabilityTime = 0;
		G_TempEntity(player->client->ps.origin, EV_JUICED);
	}
	else
	{
		G_SetOrigin(mine, player->s.pos.trBase);
		// make sure the explosion gets to the client
		mine->r.svFlags          &= ~SVF_NOCLIENT;
		mine->splashMethodOfDeath = MOD_PROXIMITY_MINE;
		G_ExplodeMissile(mine);
	}
}

/*
================
ProximityMine_Player
================
*/
static void ProximityMine_Player(gentity_t *mine, gentity_t *player)
{
	if (mine->s.eFlags & EF_NODRAW)
		return;

	G_AddEvent(mine, EV_PROXIMITY_MINE_STICK, 0);

	if (player->s.eFlags & EF_TICKING)
	{
		player->activator->splashDamage += mine->splashDamage;
		player->activator->splashRadius *= 1.50;
		mine->think                      = G_FreeEntity;
		mine->nextthink                  = level.time;
		return;
	}

	player->client->ps.eFlags |= EF_TICKING;
	player->activator          = mine;

	mine->s.eFlags    |= EF_NODRAW;
	mine->r.svFlags   |= SVF_NOCLIENT;
	mine->s.pos.trType = TR_LINEAR;
	VectorClear(mine->s.pos.trDelta);

	mine->enemy = player;
	mine->think = ProximityMine_ExplodeOnPlayer;
	if (player->client->invulnerabilityTime > level.time)
	{
		mine->nextthink = level.time + 2 * 1000;
	}
	else
	{
		mine->nextthink = level.time + 10 * 1000;
	}
}
//#endif
#endif // 0 FIXME: 0xA5EA

/*
================
G_MissileImpact

also used for flame chunks to bonce
================
*/
void G_MissileImpact(gentity_t *ent, trace_t *trace)
{
	gentity_t *other;
	qboolean hitClient = qfalse;
	qboolean expOnPlayer = qfalse;



#if 0 //def MISSIONPACK
	vec3_t forward, impactpoint, bouncedir;
	int eFlags;
#endif
	other = &g_entities[trace->entityNum];

	if(ent->s.weapon == WP_GRENADE_LAUNCHER 
		&& (g_weaponmod.integer & WM_HITMEN) 
		&& other->takedamage)
		expOnPlayer = qtrue;

	// check for bounce
	if ((!other->takedamage || !expOnPlayer) && (ent->s.eFlags & (EF_BOUNCE | EF_BOUNCE_HALF)))
	{
		G_BounceMissile(ent, trace);
		if (ent->s.weapon != WP_FLAMER)
		{
			// 0xA5EA, two new bounce sounds
			if ( trace->surfaceFlags & SURF_METALLIGHT || trace->surfaceFlags & SURF_METALSTEPS || trace->surfaceFlags & SURF_TIN )
				G_AddEvent(ent, EV_GRENADE_BOUNCE_METAL, 0);
			else if ( trace->surfaceFlags & SURF_WOOD )
				G_AddEvent(ent, EV_GRENADE_BOUNCE_WOOD, 0);
			else
				G_AddEvent(ent, EV_GRENADE_BOUNCE, 0);
		}
		else
		{
			gentity_t *newEnt;
			newEnt = G_Spawn();// spawn temp entity for impact mark
			newEnt->freeAfterEvent = qtrue;	
			newEnt->s.eType = ET_GENERAL;
			G_SetOrigin(newEnt, trace->endpos);
			G_AddEvent(ent, EV_MISSILE_MISS, DirToByte(trace->plane.normal));//hypov8 todo: burn walls
			trap_LinkEntity(newEnt);
		}

		return;
	}

	// impact damage
	if (other->takedamage)
	{
		// FIXME: wrong damage direction?
		if (ent->damage)
		{
			vec3_t velocity;

			if (LogAccuracyHit(other, &g_entities[ent->r.ownerNum]))
			{
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}

			BG_EvaluateTrajectoryDelta(&ent->s.pos, level.time, velocity);

			if ( VectorLength(velocity) == 0 )
				velocity[ 2 ] = 1;    // stepped on a grenade

			if ( ent->s.weapon == WP_FLAMER )
			{
				//shouldent happen
				Com_Printf("ERROR: flamer\n");
				/*int doDamage = qtrue;
				int timeInUse = level.time - ent->timestamp;
				int percent = ((float)timeInUse/ FLAME_LIFETIME)*5;
				if ( other->flameQuotaTime > level.time )
				{
					//entity will stay and cause damage
					//do less gamage the older the flame
					int rnd = rand()%5;
					if ((rnd) < percent)
						doDamage =qfalse;
				}
				else
				if (doDamage)
				*/	
				
				//set player on fire
				//G_FlamerBurnTarget(ent, other, 0);

				/*gentity_t *ignoreEnt = NULL;
				int timer = level.time - ent->timestamp;
				// ignore interactions with the missile owner for short time
				if ( timer < 120 )
					ignoreEnt = &g_entities[ ent->r.ownerNum ];
		
				G_FlameDamageAllPlayers(ent, ignoreEnt);
				//free entity
				ent->freeAfterEvent = qtrue;
				ent->s.eType = ET_GENERAL;
				SnapVectorTowards(trace->endpos, ent->s.pos.trBase); // save net bandwidth
				G_SetOrigin(ent, trace->endpos);
				trap_LinkEntity(ent);*/
				return;
				//do flamer damage
				//G_FlamerInflictPlayerDamage(ent, other);//G_BurnMeGood
				//other->s.onFireStart;
				//other->s.onFireEnd;
				//other->flameQuotaTime = level.time +  FLAME_LIFETIME ;
				//G_Damage(other, ent, &g_entities[ ent->r.ownerNum ], velocity, ent->s.origin, ent->damage, DAMAGE_NO_KNOCKBACK, ent->methodOfDeath);
			}
			else
				G_Damage(other, ent, &g_entities[ ent->r.ownerNum ], velocity, ent->s.origin, ent->damage, 0, ent->methodOfDeath);
		}
	}

#if 1 //hitmen
	if (!qstrcmp(ent->classname, "hook"))
	{
		gentity_t *nent;
		vec3_t v;

		nent = G_Spawn();
		if (other->takedamage && other->client)
		{

			G_AddEvent(nent, EV_MISSILE_HIT, DirToByte(trace->plane.normal));
			nent->s.otherEntityNum = other->s.number;

			ent->enemy = other;

			v[0] = other->r.currentOrigin[0] + (other->r.mins[0] + other->r.maxs[0]) * 0.5;
			v[1] = other->r.currentOrigin[1] + (other->r.mins[1] + other->r.maxs[1]) * 0.5;
			v[2] = other->r.currentOrigin[2] + (other->r.mins[2] + other->r.maxs[2]) * 0.5;

			SnapVectorTowards(v, ent->s.pos.trBase); // save net bandwidth
		}
		else
		{
			VectorCopy(trace->endpos, v);
			G_AddEvent(nent, EV_MISSILE_MISS, DirToByte(trace->plane.normal));
			ent->enemy = NULL;
		}

		SnapVectorTowards(v, ent->s.pos.trBase);     // save net bandwidth

		nent->freeAfterEvent = qtrue;
		// change over to a normal entity right at the point of impact
		nent->s.eType = ET_GENERAL;
		ent->s.eType  = ET_GRAPPLE;

		G_SetOrigin(ent, v);
		G_SetOrigin(nent, v);

		ent->think     = Weapon_HookThink;
		ent->nextthink = level.time + FRAMETIME;

		ent->parent->client->ps.pm_flags |= PMF_GRAPPLE_PULL;
		VectorCopy(ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);

		trap_LinkEntity(ent);
		trap_LinkEntity(nent);
		return;
	}
#endif
	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?
	if (other->takedamage && other->client)
	{
		G_AddEvent(ent, EV_MISSILE_HIT, DirToByte(trace->plane.normal));
		ent->s.otherEntityNum = other->s.number;
	}
	else if (trace->surfaceFlags & SURF_METALSTEPS)
	{
		G_AddEvent(ent, EV_MISSILE_MISS_METAL, DirToByte(trace->plane.normal));
	}
	else
	{
		G_AddEvent(ent, EV_MISSILE_MISS, DirToByte(trace->plane.normal));
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards(trace->endpos, ent->s.pos.trBase); // save net bandwidth

	G_SetOrigin(ent, trace->endpos);

	// splash damage (doesn't apply to person directly hit)
	if (ent->splashDamage)
	{
		if (G_RadiusDamage(trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius, other, ent->splashMethodOfDeath))
		{
			if (!hitClient)
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}
	trap_LinkEntity(ent);
}

/*
================
G_RunMissile
================
*/
void G_RunMissile(gentity_t *ent)
{
	vec3_t origin;
	trace_t tr;
	int passent;

	// get current position
	BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);

	// if this missile bounced off an invulnerability sphere
	if (ent->target_ent)
	{
		passent = ent->target_ent->s.number;
	}
	else
	{
		// ignore interactions with the missile owner
		passent = ent->r.ownerNum;
	}

//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	G_DoTimeShiftFor( &g_entities[ ent->r.ownerNum ] );
//unlagged - backward reconciliation #2

	// trace a line from the previous position to the current position
	trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask);

	if (tr.startsolid || tr.allsolid)
	{
		// make sure the tr.entityNum is set to the entity we're stuck in
		trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask);
		tr.fraction = 0;
	}
	else
	{
		VectorCopy(tr.endpos, ent->r.currentOrigin);
	}

//unlagged - backward reconciliation #2
	// put them back
	G_UndoTimeShiftFor( &g_entities[ ent->r.ownerNum ]);
//unlagged - backward reconciliation #2

	trap_LinkEntity(ent);

	if (tr.fraction != 1)
	{
		// never explode or bounce on sky
		if (tr.surfaceFlags & SURF_NOIMPACT)
		{
			// If grapple, reset owner
			if (ent->parent && ent->parent->client && ent->parent->client->hook == ent)
				ent->parent->client->hook = NULL;

			G_FreeEntity(ent);
			return;
		}

		G_MissileImpact(ent, &tr);

		if (ent->s.eType != ET_MISSILE)
			return;     // exploded

	}

	// check think function after bouncing
	G_RunThink(ent);
}

//=============================================================================
/*
=================
fire_plasma
=================
*/
#ifdef ALLQ3ITEMS
gentity_t *fire_plasma(gentity_t *self, vec3_t start, vec3_t dir)
{
	gentity_t *bolt;

	VectorNormalize(dir);

	bolt                      = G_Spawn();
	bolt->classname           = "plasma";
	bolt->nextthink           = level.time + 10000;
	bolt->think               = G_ExplodeMissile;
	bolt->s.eType             = ET_MISSILE;
	bolt->r.svFlags           = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon            = WP_PLASMAGUN;
	bolt->r.ownerNum          = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->parent              = self;
	bolt->damage              = 20;
	bolt->splashDamage        = 15;
	bolt->splashRadius        = 20;
	bolt->methodOfDeath       = MOD_PLASMA;
	bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
	bolt->clipmask            = MASK_SHOT;
	bolt->target_ent          = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;     // move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);
	VectorScale(dir, 2000, bolt->s.pos.trDelta);
	SnapVector(bolt->s.pos.trDelta);                            // save net bandwidth

	VectorCopy(start, bolt->r.currentOrigin);

	return bolt;
}
#endif
//=============================================================================


/*
=================
fire_grenade
=================
*/
gentity_t *fire_grenade(gentity_t *self, vec3_t start, vec3_t dir)
{
	gentity_t *bolt;
	int damage = 240;      // 0xA5EA: kp value //hypov8 todo: weak?
	int radius = damage*2; // 0xA5EA: kp value

	//add hypov8
	vec3_t min = { -4, -4, -4 };
	vec3_t max = { 4, 4, 4 };
	vec_t speed;

	//hypov8 add velocity if inline with view angle. including minus velocity
	speed = DotProduct(dir, self->client->ps.velocity)/2;

	radius *= 0.75;        // 0xA5EA: kp value
	VectorNormalize(dir);

	bolt             = G_Spawn();
	bolt->classname  = "grenade";
	bolt->nextthink  = level.time + 2500;
	bolt->think      = G_ExplodeMissile;
	bolt->s.eType    = ET_MISSILE;
	bolt->r.svFlags  = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon   = WP_GRENADE_LAUNCHER;
	bolt->s.eFlags   = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->parent     = self;

	bolt->damage              = damage;                       
	bolt->splashDamage        = damage;                        
	bolt->splashRadius        = radius; 
	bolt->methodOfDeath       = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask            = MASK_SHOT | MASK_ROCKET; //hypov8 add: rocket clip_brush
	bolt->target_ent          = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;     // move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);
	VectorScale(dir, 750 +speed, bolt->s.pos.trDelta); //hypov8 add: +speed
	SnapVector(bolt->s.pos.trDelta);                            // save net bandwidth

	VectorCopy(start, bolt->r.currentOrigin);

	VectorCopy(min, bolt->r.mins);
	VectorCopy(max, bolt->r.maxs);

	return bolt;
}

//=============================================================================
/*
=================
fire_rocket
=================
*/
gentity_t *fire_rocket(gentity_t *self, vec3_t start, vec3_t dir)
{
	gentity_t *bolt;

	VectorNormalize(dir);

	bolt             = G_Spawn();
	bolt->classname  = "rocket";
	bolt->nextthink  = level.time + 15000;
	bolt->think      = G_ExplodeMissile;
	bolt->s.eType    = ET_MISSILE;
	bolt->r.svFlags  = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon   = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->parent     = self;
	bolt->damage     = 100 + (int)(random() * 20.0);            // 0xA5EA, kp-damage, orignal 100; was 110 + (int)(random() * 20.0) -KRYPTYK

	bolt->splashDamage = 120;                                   // 0xA5EA kp-value, orginal 100; was 80 -KRYPTYK
	bolt->splashRadius = 180;                                   // 0xA5EA kp-value 180; was 150 -KRYPTYK

	bolt->methodOfDeath       = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask            = MASK_SHOT | MASK_ROCKET; //add hypov8 rocket clip
	bolt->target_ent          = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;     // move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);
	VectorScale(dir, ROCKET_SPEED, bolt->s.pos.trDelta);
	SnapVector(bolt->s.pos.trDelta);                            // save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	return bolt;
}

/*
=================
fire_grapple
=================
*/
gentity_t *fire_grapple(gentity_t *self, vec3_t start, vec3_t dir)
{
	gentity_t *hook;
//unlagged - grapple
	int hooktime;
//unlagged - grapple

	VectorNormalize(dir);

	hook                = G_Spawn();
	hook->classname     = "hook";
	hook->nextthink     = level.time + 10000;
	hook->think         = Weapon_HookFree;
	hook->s.eType       = ET_MISSILE;
	hook->r.svFlags     = SVF_USE_CURRENT_ORIGIN;
	hook->s.weapon      = WP_GRAPPLING_HOOK;
	hook->r.ownerNum    = self->s.number;
	hook->methodOfDeath = MOD_GRAPPLE;
	hook->clipmask      = MASK_SHOT | MASK_ROCKET;
	hook->parent        = self;
	hook->target_ent    = NULL;

//unlagged - grapple
	// we might want this later
	hook->s.otherEntityNum = self->s.number;

	// setting the projectile base time back makes the hook's first
	// step larger

	if ( self->client ) {
		hooktime = self->client->pers.cmd.serverTime + 50;
	}
	else {
		hooktime = level.time - MISSILE_PRESTEP_TIME;
	}

	hook->s.pos.trTime = hooktime;
//unlagged - grapple
	hook->s.pos.trType     = TR_LINEAR;
//unlagged - grapple
	//hook->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
//unlagged - grapple
	hook->s.otherEntityNum = self->s.number;                        // use to match beam in client
	VectorCopy(start, hook->s.pos.trBase);
	VectorScale(dir, 800, hook->s.pos.trDelta);
	SnapVector(hook->s.pos.trDelta);                                // save net bandwidth
	VectorCopy(start, hook->r.currentOrigin);

	self->client->hook = hook;
	return hook;
}

/*
=================
fire_nail
=================
*/
#define NAILGUN_SPREAD 500

gentity_t *fire_nail(gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up)
{
	gentity_t *bolt;
	vec3_t dir;
	vec3_t end;
	float r, u, scale;

	bolt                = G_Spawn();
	bolt->classname     = "nail";
	bolt->nextthink     = level.time + 10000;
	bolt->think         = G_ExplodeMissile;
	bolt->s.eType       = ET_MISSILE;
	bolt->r.svFlags     = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon      = WP_PISTOL;
	bolt->r.ownerNum    = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->parent        = self;
	bolt->damage        = 20;
	bolt->methodOfDeath = MOD_HMG;
	bolt->clipmask      = MASK_SHOT;
	bolt->target_ent    = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;
	VectorCopy(start, bolt->s.pos.trBase);

	r = random() * M_PI * 2.0f;
	u = sin(r) * crandom() * NAILGUN_SPREAD * 16;
	r = cos(r) * crandom() * NAILGUN_SPREAD * 16;
	VectorMA(start, 8192 * 16, forward, end);
	VectorMA(end, r, right, end);
	VectorMA(end, u, up, end);
	VectorSubtract(end, start, dir);
	VectorNormalize(dir);

	scale = 555 + random() * 1800;
	VectorScale(dir, scale, bolt->s.pos.trDelta);
	SnapVector(bolt->s.pos.trDelta);

	VectorCopy(start, bolt->r.currentOrigin);

	return bolt;
}
#if 0                                                               //FIXME: 0xA5EA
/*
=================
fire_prox
=================
*/
gentity_t *fire_prox(gentity_t *self, vec3_t start, vec3_t dir)
{
	gentity_t *bolt;

	VectorNormalize(dir);

	bolt                      = G_Spawn();
	bolt->classname           = "prox mine";
	bolt->nextthink           = level.time + 3000;
	bolt->think               = G_ExplodeMissile;
	bolt->s.eType             = ET_MISSILE;
	bolt->r.svFlags           = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon            = WP_PROX_LAUNCHER;
	bolt->s.eFlags            = 0;
	bolt->r.ownerNum          = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->parent              = self;
	bolt->damage              = 0;
	bolt->splashDamage        = 100;
	bolt->splashRadius        = 150;
	bolt->methodOfDeath       = MOD_PROXIMITY_MINE;
	bolt->splashMethodOfDeath = MOD_PROXIMITY_MINE;
	bolt->clipmask            = MASK_SHOT;
	bolt->target_ent          = NULL;
	// count is used to check if the prox mine left the player bbox
	// if count == 1 then the prox mine left the player bbox and can attack to it
	bolt->count = 0;

	//FIXME: we prolly wanna abuse another field
	bolt->s.generic1 = self->client->sess.sessionTeam;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;     // move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);
	VectorScale(dir, 700, bolt->s.pos.trDelta);
	SnapVector(bolt->s.pos.trDelta);                            // save net bandwidth

	VectorCopy(start, bolt->r.currentOrigin);

	return bolt;
}
//#endif
#endif // 0 0xA5EA
