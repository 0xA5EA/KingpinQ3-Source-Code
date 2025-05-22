/*
===========================================================================
Copyright (C) 2006 Neil Toronto.

This file is part of the Unlagged source code.

Unlagged source code is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Unlagged source code is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Unlagged source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "cg_local.h"

// we'll need these prototypes
void CG_Bullet(int weapon, vec3_t origin, int sourceEntityNum, vec3_t normal, int ImpactType, int fleshEntityNum);
int BG_GetSufaceType(int flags);


int CG_GetSufaceType(int flags)
{
  switch (BG_GetSufaceType(flags))
  {
    case EV_BULLET_HIT_METAL:
      return IMPACTSOUND_METAL;
    case EV_BULLET_HIT_WOOD:
      return IMPACTSOUND_WOOD;
    case EV_BULLET_HIT_EARTH:
      return IMPACTSOUND_EARTH;
    case EV_BULLET_HIT_GLASS:
      return IMPACTSOUND_GLASS;
    case EV_BULLET_HIT_SNOW:
      return IMPACTSOUND_SNOW;
    default:
    case EV_BULLET_HIT_WALL:
      return IMPACTSOUND_DEFAULT;
  }
}




/*
=======================
CG_PredictWeaponEffects

Draws predicted effects for the railgun, shotgun, and machinegun.  The
lightning gun is done in CG_LightningBolt, since it was just a matter
of setting the right origin and angles.
=======================
*/
void CG_PredictWeaponEffects( centity_t *cent ) {
	vec3_t		muzzlePoint, endPoint, forward, right, up;
	entityState_t *ent = &cent->currentState;
	int seed, impactType, i, fleshEntityNum, spread; //x+y spread?
	trace_t tr;

	// if the client isn't us, forget it
	if ( cent->currentState.number != cg.predictedPlayerState.clientNum ) {
		return;
	}

	// if it's not switched on server-side, forget it
	if ( !cgs.delagHitscan || !cg_delag.integer) {
		return;
	}

	// get the muzzle point
	VectorCopy( cg.predictedPlayerState.origin, muzzlePoint );
	muzzlePoint[2] += cg.predictedPlayerState.viewheight;

	// get forward, right, and up
	AngleVectors( cg.predictedPlayerState.viewangles, forward, right, up );
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
	SnapVector(muzzlePoint); //hypov8 match server

	// the server will use this exact time (it'll be serverTime on that end)
	seed = cg.oldTime % 256;

	// was it a shotgun attack?
	if ( ent->weapon == WP_SHOTGUN ) 
	{
		float r, u;
		int showBlood = qtrue;
		int drewBlood = qfalse;

		// do everything like the server does
		SnapVector( muzzlePoint );
		VectorScale( forward, 4096, endPoint );

		SnapVector( endPoint );
		/*VectorSubtract(endPoint, muzzlePoint, v);
		VectorNormalize( v );
		VectorScale( v, 32, v );
		VectorAdd( muzzlePoint, v, v );*/

		if (cgs.weaponmod & WM_REALMODE) //g_weaponmod.integer
			spread = DEFAULT_SHOTGUN_SPREAD_RM;
		else
			spread = DEFAULT_SHOTGUN_SPREAD;

		// generate the "random" spread pattern
		for (i = 0; i < DEFAULT_SHOTGUN_COUNT; i++)
		{
			r = Q_random(&seed) * M_PI * 2.0f;
			u = sin(r) * Q_crandom(&seed) * spread * 16;
			r = cos(r) * Q_crandom(&seed) * spread * 16;

			//r = Q_crandom(&seed) * spread * 16;
			//u = Q_crandom(&seed) * spread * 16;
			VectorMA(muzzlePoint, 8192 * 16, forward, endPoint);
			VectorMA(endPoint, r, right, endPoint);
			VectorMA(endPoint, u, up, endPoint);
			CG_Trace(&tr, muzzlePoint, NULL, NULL, endPoint, cg.predictedPlayerState.clientNum, MASK_SHOT);

			if (tr.surfaceFlags & SURF_NOIMPACT)
				continue;

			//SnapVectorTowards(tr.endpos, muzzlePoint);

			// do bullet impact
			if (!drewBlood && tr.entityNum < MAX_CLIENTS)
			{
				impactType = IMPACTSOUND_FLESH;
				fleshEntityNum = tr.entityNum;
				drewBlood = qtrue;
			}
			else 
			{
				impactType = CG_GetSufaceType(tr.surfaceFlags);
				fleshEntityNum = 0;
			}

			// do the bullet impact
			CG_Bullet(ent->weapon, tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, impactType, fleshEntityNum);
		}
	}
	//hypo client predicted bullet effects
	else if (ent->weapon == WP_PISTOL || ent->weapon == WP_MACHINEGUN || ent->weapon == WP_HMG)
	{
		float r, u;

		switch (ent->weapon)
		{
			case WP_PISTOL:
				spread = PISTOL_SPREAD;
				break;
			case WP_MACHINEGUN:
				spread = MACHINEGUN_SPREAD ; // (cgs.weaponmod & WM_REALMODE)?
				break;
			default:
			case WP_HMG:
				spread = HMG_SPREAD;
				break;
		}

		// do everything exactly like the server does
		r = Q_random(&seed) * M_PI * 2.0f;
		u = sin(r) * Q_crandom(&seed) * spread * 16;
		r = cos(r) * Q_crandom(&seed) * spread * 16;

		VectorMA(muzzlePoint, 8192 * 16, forward, endPoint);
		VectorMA(endPoint, r, right, endPoint);
		VectorMA(endPoint, u, up, endPoint);

		CG_Trace(&tr, muzzlePoint, NULL, NULL, endPoint, cg.predictedPlayerState.clientNum, MASK_SHOT);

		if (tr.surfaceFlags & SURF_NOIMPACT)
		{
			return;
		}

		// snap the endpos to integers, but nudged towards the line
		//SnapVectorTowards(tr.endpos, muzzlePoint);

		// do bullet impact
		if (tr.entityNum < MAX_CLIENTS)
		{
			impactType = IMPACTSOUND_FLESH;
			fleshEntityNum = tr.entityNum;
		}
		else {
			impactType = CG_GetSufaceType(tr.surfaceFlags);
			fleshEntityNum = 0;
		}

		// do the bullet impact
		CG_Bullet(ent->weapon, tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, impactType, fleshEntityNum);
	}
	//crowbar effects prediction
	else if (ent->weapon == WP_CROWBAR)
	{
		VectorMA(muzzlePoint, CROWBAR_DIST, forward, endPoint);
		CG_Trace(&tr, muzzlePoint, NULL, NULL, endPoint, cg.predictedPlayerState.clientNum, MASK_SHOT);

		if (tr.surfaceFlags & SURF_NOIMPACT)
			return;

		if (tr.entityNum == ENTITYNUM_NONE)
			return;

		// snap the endpos to integers, but nudged towards the line
		//SnapVectorTowards(tr.endpos, muzzlePoint);

		// do bullet impact
		if (tr.entityNum < MAX_CLIENTS)
		{
			impactType = IMPACTSOUND_FLESH;
			fleshEntityNum = tr.entityNum;
		}
		else {
			impactType = CG_GetSufaceType(tr.surfaceFlags);
			fleshEntityNum = 0;
		}

		// do the bullet impact
		CG_Bullet(ent->weapon, tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, impactType, fleshEntityNum);
	}
}



/*
================
CG_Cvar_ClampInt

Clamps a cvar between two integer values, returns qtrue if it had to.
================
*/
qboolean CG_Cvar_ClampInt( const char *name, vmCvar_t *vmCvar, int min, int max ) {
	if ( vmCvar->integer > max ) {
		CG_Printf( "Allowed values are %d to %d.\n", min, max );

		Com_sprintf( vmCvar->string, MAX_CVAR_VALUE_STRING, "%d", max );
		vmCvar->value = max;
		vmCvar->integer = max;

		trap_Cvar_Set( name, vmCvar->string );
		return qtrue;
	}

	if ( vmCvar->integer < min ) {
		CG_Printf( "Allowed values are %d to %d.\n", min, max );

		Com_sprintf( vmCvar->string, MAX_CVAR_VALUE_STRING, "%d", min );
		vmCvar->value = min;
		vmCvar->integer = min;

		trap_Cvar_Set( name, vmCvar->string );
		return qtrue;
	}

	return qfalse;
}
