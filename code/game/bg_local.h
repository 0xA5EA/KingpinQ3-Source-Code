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
// bg_local.h -- local definitions for the bg (both games) files
#ifndef BG_LOCAL_H_
#define BG_LOCAL_H_

#define KINGPIN_PLAYERMOVE          // 0xA5EA

#define MIN_WALK_NORMAL 0.7f        // can't walk on very steep slopes

#define STEPSIZE 18

#define JUMP_VELOCITY 270

#define TIMER_LAND 130
#define TIMER_GESTURE (34 * 66 + 50)

#define OVERCLIP 1.001f

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct
{
	vec3_t forward, right, up;
	float frametime;

	int msec;

	qboolean walking;
	qboolean ladder;        /* 0xA5EA, laddersupport */
	qboolean groundPlane;
	trace_t groundTrace;

	float impactSpeed;

	vec3_t previous_origin;
	vec3_t previous_velocity;
	int previous_waterlevel;
} pml_t;

extern pmove_t *pm;
extern pml_t pml;

// movement parameters
extern float pm_stopspeed;
extern float pm_duckScale;
extern float pm_swimScale;
extern float pm_wadeScale;
#define PM_LADDERSCALE 0.65f
#define PM_ACCELERATE 10.f

//float	pm_strafeaccelerate = 70;
#define PM_STRAFE_ACCELERATE 100.f      // 0xA5EA, war 70 (tr�ge)
//extern	float	pm_accelerate;
extern float pm_airaccelerate;
extern float pm_wateraccelerate;
extern float pm_flyaccelerate;

extern float pm_friction;
extern float pm_waterfriction;
extern float pm_flightfriction;

extern int c_pmove;

void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce);
void PM_AddTouchEnt(int entityNum);
void PM_AddEvent(int newEvent);
void PM_AddEvent2(int newEvent, int eventParms);

qboolean PM_SlideMove(qboolean gravity);
//void PM_StepSlideMove(qboolean gravity); //hypov8 merge: check this
qboolean        PM_StepSlideMove(qboolean gravity, qboolean predictive); //add hypov8 xreal
qboolean        PM_PredictStepMove(void);	//add hypov8 xreal
void            PM_StepEvent(vec3_t from, vec3_t to, vec3_t normal);	//add hypov8 xreal

#endif // BG_LOCAL_H_
