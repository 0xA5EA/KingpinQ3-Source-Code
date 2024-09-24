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

#if 1 	//hypov8 todo: cleanup. disabled predicting bullets/fx localy..
// we'll need these prototypes
void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, int otherEntNum );
//void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum ); //orig unlag
void CG_Bullet(int weapon, vec3_t origin, int sourceEntityNum, vec3_t normal, int ImpactType, int fleshEntityNum);
// and this as well

//hypov8 needs to match g_xxx settings
//moved to bg_public
//#define MACHINEGUN_SPREAD	200

/*
=======================
CG_PredictWeaponEffects

Draws predicted effects for the railgun, shotgun, and machinegun.  The
lightning gun is done in CG_LightningBolt, since it was just a matter
of setting the right origin and angles.
=======================
*/
void CG_PredictWeaponEffects( centity_t *cent ) {
	vec3_t		muzzlePoint, forward, right, up;
	entityState_t *ent = &cent->currentState;

	// if the client isn't us, forget it
	if ( cent->currentState.number != cg.predictedPlayerState.clientNum ) {
		return;
	}

	// if it's not switched on server-side, forget it
	if ( !cgs.delagHitscan ) {
		return;
	}

	// get the muzzle point
	VectorCopy( cg.predictedPlayerState.origin, muzzlePoint );
	muzzlePoint[2] += cg.predictedPlayerState.viewheight;

	// get forward, right, and up
	AngleVectors( cg.predictedPlayerState.viewangles, forward, right, up );
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );


	// was it a shotgun attack?
	if ( ent->weapon == WP_SHOTGUN ) 
	{
		// do we have it on for the shotgun?
		if ( cg_delag.integer & 1 || cg_delag.integer & 4 ) 
		{
			vec3_t endPoint, v;
			int seed = cg.oldTime % 256;
			float r, u;
			trace_t tr;
			int flesh;
			int fleshEntityNum = 0;

			int i, spread;
			int showBlood = qtrue;
			int drewBlood = qfalse;
			int damage = DEFAULT_SHOTGUN_DAMAGE;

			// do everything like the server does
			SnapVector( muzzlePoint );
			VectorScale( forward, 4096, endPoint );

			SnapVector( endPoint );
			VectorSubtract(endPoint, muzzlePoint, v);
			VectorNormalize( v );
			VectorScale( v, 32, v );
			VectorAdd( muzzlePoint, v, v );

			if (/*g_weaponmod.integer*/ cgs.weaponmod & WM_REALMODE)
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

				SnapVectorTowards(tr.endpos, muzzlePoint);

				// do bullet impact
				if (!drewBlood && tr.entityNum < MAX_CLIENTS)
				{
					flesh = IMPACTSOUND_FLESH;
					fleshEntityNum = tr.entityNum;
					drewBlood = qtrue;
				}
				else 
				{
					flesh = qfalse;
				}

				CG_Bullet(ent->weapon, tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, flesh, fleshEntityNum);
			}
		}
	}

#if 1 //hypo client predicted wep effects
	else if (ent->weapon == WP_PISTOL || ent->weapon == WP_MACHINEGUN || ent->weapon == WP_HMG)
	{
		// do we have it on for the machinegun?
		if (cg_delag.integer & 1 || cg_delag.integer & 2)
		{
			// the server will use this exact time (it'll be serverTime on that end)
			int seed = cg.oldTime % 256;
			float r, u;
			trace_t tr;
			qboolean flesh;
			int fleshEntityNum = 0;
			vec3_t endPoint;
			int spread; //x+y?

			switch (ent->weapon)
			{
				case WP_PISTOL:
					spread = PISTOL_SPREAD;
					break;
				case WP_MACHINEGUN:
					spread = MACHINEGUN_SPREAD ; // (cgs.weaponmod & WM_REALMODE)?
					break;
				case WP_HMG:
					spread = HMG_SPREAD;
					break;
			}

			fleshEntityNum = 0; //hypov8 initilized 0??

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
			SnapVectorTowards(tr.endpos, muzzlePoint);

			// do bullet impact
			if (tr.entityNum < MAX_CLIENTS)
			{
				flesh = IMPACTSOUND_FLESH;
				fleshEntityNum = tr.entityNum;
			}
			else {
				flesh = qfalse;
			}

			// do the bullet impact
			//CG_Bullet( tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, flesh, fleshEntityNum ); //orig unlag
			CG_Bullet(ent->weapon, tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, flesh, fleshEntityNum);
		}
	}
#endif

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
#endif
