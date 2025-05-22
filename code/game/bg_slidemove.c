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
// bg_slidemove.c -- part of bg_pmove functionality

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

/*
input: origin, velocity, bounds, groundPlane, trace function
output: origin, velocity, impacts, stairup boolean
*/

/*
==================
PM_SlideMove
Returns qtrue if the velocity was clipped in some way
==================
 */
#define MAX_CLIP_PLANES 5
qboolean PM_SlideMove(qboolean gravity)
{
  int bumpcount, numbumps;
  vec3_t dir;
  float d;
  int numplanes;
  vec3_t planes[MAX_CLIP_PLANES];
  vec3_t primal_velocity;
  //vec3_t clipVelocity;
  int i, j;
  //int k;
  trace_t trace;
  vec3_t end;
  float time_left;
  //float into;
  vec3_t endVelocity;
  //vec3_t endClipVelocity;

  numbumps = 4;

  VectorCopy(pm->ps->velocity, primal_velocity);
  VectorCopy(pm->ps->velocity, endVelocity);
#ifndef KINGPIN_PLAYERMOVE
  if (gravity)
  {
    endVelocity[2]     -= pm->ps->gravity * pml.frametime;
    pm->ps->velocity[2] = (pm->ps->velocity[2] + endVelocity[2]) * 0.5;
    primal_velocity[2]  = endVelocity[2];
    if (pml.groundPlane)
    {
      // slide along the ground plane
      PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal,  pm->ps->velocity, OVERCLIP);
    }
  }
#endif
  time_left = pml.frametime;

#ifndef KINGPIN_PLAYERMOVE
  // never turn against the ground plane
  if (pml.groundPlane)
  {
    numplanes = 1;
    VectorCopy(pml.groundTrace.plane.normal, planes[0]);
  }
  else
#endif
  {
    numplanes = 0;
  }

#ifndef KINGPIN_PLAYERMOVE
  // never turn against original velocity
  VectorNormalize2(pm->ps->velocity, planes[numplanes]); //hat ich mal auskomentiert, kein Unterschied gemerkt

  //writes the normalised pm->ps->velocity (norm 1) into planes[..
  numplanes++;
#endif
  for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
  {
    // calculate position we are trying to move to
    VectorMA(pm->ps->origin, time_left, pm->ps->velocity, end);
#if 0
    if (pm->debugLevel)
    {
      if (!(pm->ps->pmove_framecount % 250))
        Com_Printf("%i:wishdir vel[0] %d vel[1] %d vel[2] %d\n", c_pmove,
            pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
    } /* 0xA5EA */
#endif

    // see if we can make it there
    pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);

    if (trace.allsolid)
    {
      // entity is completely trapped in another solid
#ifndef KINGPIN_PLAYERMOVE
      VectorCopy(vec3_origin, pm->ps->velocity);
      // orignal kp: return 3;
      return qtrue;
#else
      pm->ps->velocity[2] = 0;    // don't build up falling damage, but allow sideways acceleration
      return qtrue;
#endif
    }

    if (trace.fraction > 0 /* && && !trace.ent->client)*/)
    {
#ifdef KINGPIN_PLAYERMOVE
      /* orginal KP code : Vec3_Copy (ent->velocity, original_velocity); */
      //VectorCopy(pm->ps->velocity, primal_velocity);
      numplanes = 0;
      VectorCopy(trace.endpos, pm->ps->origin);
#else
      //		if (trace.fraction > 0) {
      // actually covered some distance
      VectorCopy(trace.endpos, pm->ps->origin);
#endif
    }

    if (trace.fraction == 1)
      break;     // moved the entire distance

    // save entity for contact
    PM_AddTouchEnt(trace.entityNum);

    time_left -= time_left * trace.fraction;

    if (numplanes >= MAX_CLIP_PLANES)
    {
      // this shouldn't really happen
      VectorClear(pm->ps->velocity);
      return qtrue;
    }

    //
    // if this is the same plane we hit before, nudge velocity
    // out along it, which fixes some epsilon issues with
    // non-axial planes
    //
#ifndef KINGPIN_PLAYERMOVE
    for (i = 0; i < numplanes; i++)
    {
      if (DotProduct(trace.plane.normal, planes[i]) > 0.99)
      {
        VectorAdd(trace.plane.normal, pm->ps->velocity, pm->ps->velocity);
        break;
      }
    }
    if (i < numplanes)
      continue;
#endif
    VectorCopy(trace.plane.normal, planes[numplanes]);
    numplanes++;

#if 0
    /* 0xA5EA, it lookes like there is no need to change this to get kp movment back */
    //
    // modify original_velocity so it parallels all of the clip planes
    //
    for (i = 0; i < numplanes; i++)
    {
      ClipVelocity(original_velocity, planes[i], new_velocity, 1);

      for (j = 0; j < numplanes; j++)
        if ((j != i) && !VectorCompare(planes[i], planes[j]))
        {
          if (DotProduct(new_velocity, planes[j]) < 0)
            break;                                                                                                                              // not ok
        }
      if (j == numplanes)
        break;
    }

    if (i != numplanes)
    {                                                                                                                                                   // go along this plane
      //gi.dprintf( "Adjusting velocity for %s\n", ent->classname );
      VectorCopy(new_velocity, ent->velocity);
#endif
    //
    // modify velocity so it parallels all of the clip planes
    //


#ifdef KINGPIN_PLAYERMOVE
    for (i = 0; i < numplanes; i++)
    {
      PM_ClipVelocity(pm->ps->velocity, planes[i], pm->ps->velocity, OVERCLIP);
      for (j = 0; j < numplanes; j++)
      {
        if (j != i)
        {
          if (DotProduct(pm->ps->velocity, planes[j]) < 0)
            break;	// not ok
        }
      }
      if (j == numplanes)
        break;
    }

    if (i != numplanes)
    {	// go along this plane
    }
    else
    {	// go along the crease
      if (numplanes != 2)
      {
        //				Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
        VectorCopy(vec3_origin, pm->ps->velocity);
        break;
      }
      CrossProduct(planes[0], planes[1], dir);
      d = DotProduct(dir, pm->ps->velocity);
      VectorScale(dir, d, pm->ps->velocity);
    }
    //
    // if velocity is against the original velocity, stop dead
    // to avoid tiny occilations in sloping corners
    //
    if (DotProduct(pm->ps->velocity, primal_velocity) <= 0)
    {
      VectorCopy(vec3_origin, pm->ps->velocity);
      break;
    }
#else
    // find a plane that it enters
    for (i = 0; i < numplanes; i++)
    {
      into = DotProduct(pm->ps->velocity, planes[i]);
      if (into >= 0.1)
        continue;         // move doesn't interact with the plane

      // see how hard we are hitting things
      if (-into > pml.impactSpeed)
      {
        pml.impactSpeed = -into;
      }

      // slide along the plane
      PM_ClipVelocity(pm->ps->velocity, planes[i], clipVelocity, OVERCLIP);

      // slide along the plane
      PM_ClipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

      // see if there is a second plane that the new move enters
      for (j = 0; j < numplanes; j++)
      {
        if (j == i)
          continue;

        if (DotProduct(clipVelocity, planes[j]) >= 0.1)
          continue;       // move doesn't interact with the plane

        // try clipping the move to the plane
        PM_ClipVelocity(clipVelocity, planes[j], clipVelocity, OVERCLIP);
        PM_ClipVelocity(endClipVelocity, planes[j], endClipVelocity, OVERCLIP);
        // see if it goes back into the first clip plane
        if (DotProduct(clipVelocity, planes[i]) >= 0)
          continue;

        // slide the original velocity along the crease
        CrossProduct(planes[i], planes[j], dir);
        VectorNormalize(dir);
        d = DotProduct(dir, pm->ps->velocity);
        VectorScale(dir, d, clipVelocity);

        CrossProduct(planes[i], planes[j], dir);
        VectorNormalize(dir);
        d = DotProduct(dir, endVelocity);
        VectorScale(dir, d, endClipVelocity);

        // see if there is a third plane the the new move enters
        for (k = 0; k < numplanes; k++)
        {
          if (k == i || k == j)
            continue;

          if (DotProduct(clipVelocity, planes[k]) >= 0.1)
            continue;       // move doesn't interact with the plane

          // stop dead at a tripple plane interaction
          VectorClear(pm->ps->velocity);
          return qtrue;
        }
      }

      // if we have fixed all interactions, try another move
      VectorCopy(clipVelocity, pm->ps->velocity);
      VectorCopy(endClipVelocity, endVelocity);
      break;
    }
#endif
  }
#ifndef KINGPIN_PLAYERMOVE
  if (gravity)
    VectorCopy(endVelocity, pm->ps->velocity);
#endif
  // don't change velocity if in a timer (FIXME: is this correct?)
  if (pm->ps->pm_time)
  {
    if (pm->debugLevel)
      Com_Printf("in a timer\n");

    VectorCopy(primal_velocity, pm->ps->velocity);
  }

#ifdef KINGPIN_PLAYERMOVE
  //push down slope
  if (pml.groundPlane && pm->ps->velocity[2] <= 0)
  {
    VectorCopy(pm->ps->origin, end);
    end[2] -= pml.frametime * 400;
    pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);
    if (trace.fraction < 1 && !trace.startsolid)
      VectorCopy (trace.endpos, pm->ps->origin);
  }
#endif

  return (bumpcount != 0);
}

/*
==================
PM_StepEvent
xreal
==================
*/
void PM_StepEvent(vec3_t from, vec3_t to, vec3_t normal)
{
  float           size;
  vec3_t          delta, dNormal;

  VectorSubtract(from, to, delta);
  VectorCopy(delta, dNormal);
  VectorNormalize(dNormal);

  size = DotProduct(normal, dNormal) * VectorLength(delta);

  if (size > 0.0f)
  {
    if (size > 2.0f)
    {
      if (size < 7.0f)
        PM_AddEvent(EV_STEPDN_4);
      else if (size < 11.0f)
        PM_AddEvent(EV_STEPDN_8);
      else if (size < 15.0f)
        PM_AddEvent(EV_STEPDN_12);
      else
        PM_AddEvent(EV_STEPDN_16);
    }
  }
  else
  {
    size = fabs(size);

    if (size > 2.0f)
    {
      if (size < 7.0f)
        PM_AddEvent(EV_STEP_4);
      else if (size < 11.0f)
        PM_AddEvent(EV_STEP_8);
      else if (size < 15.0f)
        PM_AddEvent(EV_STEP_12);
      else
        PM_AddEvent(EV_STEP_16);
    }
  }

  if (pm->debugLevel)
    Com_Printf("%i:stepped\n", c_pmove);
}
//xreal


#ifndef KINGPIN_PLAYERMOVE //xreal
qboolean PM_StepSlideMove(qboolean gravity, qboolean predictive)
{
  vec3_t          start_o, start_v;
  //	vec3_t          down_o, down_v;
  trace_t         trace;
  vec3_t          normal;
  vec3_t          step_v, step_vNormal;
  vec3_t          up, down;
  float           stepSize;
  qboolean        stepped = qfalse;

  VectorSet(normal, 0.0f, 0.0f, 1.0f);

  VectorCopy(pm->ps->origin, start_o);
  VectorCopy(pm->ps->velocity, start_v);

  if (PM_SlideMove(gravity) == 0)
  {
    VectorCopy(start_o, down);
    down[2] -= STEPSIZE;
    pm->trace(&trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);

    // we can step down
    if (trace.fraction > 0.01f && trace.fraction < 1.0f && !trace.allsolid && pml.groundPlane != qfalse)
    {
      if (pm->debugLevel)
        Com_Printf("%d: step down\n", c_pmove);

      stepped = qtrue;
    }
  }
  else // velocity not cliped
  {
    VectorCopy(start_o, down);
    //VectorMA(down, -STEPSIZE, normal, down);
    down[2] -= STEPSIZE;
    pm->trace(&trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);

    //never step up when you still have up velocity
    if (DotProduct(trace.plane.normal, pm->ps->velocity) > 0.0f &&
      (trace.fraction == 1.0f || DotProduct(trace.plane.normal, normal) < 0.7f))
    {
      if (pm->debugLevel)
        Com_Printf("up velocity, stopping movement\n");
      return stepped;
    }

    //VectorCopy(pm->ps->origin, down_o);
    //VectorCopy(pm->ps->velocity, down_v);

    VectorCopy(start_o, up);
    //VectorMA(up, STEPSIZE, normal, up);
    up[2] += STEPSIZE;

    // test the player position if they were a stepheight higher
    pm->trace(&trace, start_o, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);
    if (trace.allsolid)
    {
      if (pm->debugLevel)
        Com_Printf("%i:bend can't step\n", c_pmove);

      return stepped;		// can't step up
    }

    VectorSubtract(trace.endpos, start_o, step_v);
    VectorCopy(step_v, step_vNormal);
    VectorNormalize(step_vNormal);

    stepSize = DotProduct(normal, step_vNormal) * VectorLength(step_v);
    // try slidemove from this position
    VectorCopy(trace.endpos, pm->ps->origin);
    VectorCopy(start_v, pm->ps->velocity);

    if (PM_SlideMove(gravity) == 0)
    {
      if (pm->debugLevel)
        Com_Printf("%d: step up\n", c_pmove);

      stepped = qtrue;
    }

    // push down the final amount
    VectorCopy(pm->ps->origin, down);
    //VectorMA(down, -stepSize, normal, down);
    down[2] -= stepSize;// STEPSIZE;
    pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
    if (!trace.allsolid)
      VectorCopy(trace.endpos, pm->ps->origin);
#if 0
    if (trace.fraction < 1.0f
      && pm->ps->velocity[2] <0.0f) //add hypov8 stop falling but allow jumping up
      PM_ClipVelocity(pm->ps->velocity, trace.plane.normal, pm->ps->velocity, OVERCLIP);
#endif
  }

  if (!predictive && stepped)
    PM_StepEvent(start_o, pm->ps->origin, normal);

  return stepped;
}
#else //end xreal
#define	MIN_STEP_NORMAL	0.7		// can't step up onto very steep slopes
#define	MAX_CLIP_PLANES	5

//step event
// check if we moved up against a verticall wall
// dont step on slopes
void PM_checkStepEvent(vec3_t start_pos, vec3_t end_pos)
{
  trace_t trace;
  vec3_t start, end;
  float slope;
  float dist = end_pos[2] - start_pos[2];

  if (dist == 0.0f || Q_fabs(dist) < 2.0f)
    return;

  //steped up
  if (dist > 0.0f)
  {
    //trace flat_forward from start_pos
    VectorCopy(start_pos, start);
    VectorCopy(end_pos, end);
  }
  else //step down.
  {
    //trace flat_forward from end_pos to start_pos
    VectorCopy(end_pos, start);
    VectorCopy(start_pos, end);
  }

  start[2] = end[2] = start[2] + 0.25f; //move slight up
  pm->trace(&trace, start, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);
  //should not happen
  if (trace.startsolid)
    return;
  if (trace.fraction < 1.0f)
  {
    slope = DotProduct(trace.plane.normal, vec3_up);
    if (slope < MIN_STEP_NORMAL)
    {
      PM_StepEvent(start_pos, end_pos, vec3_up); //flat?
      return;
    }
  }
}

//quake2 move
qboolean PM_StepSlideMove(qboolean gravity, qboolean predictive)
{
  vec3_t		start_o, start_v;
  vec3_t		down_o, down_v;
  trace_t		trace;
  float		down_dist, up_dist;
  //	vec3_t		delta;
  vec3_t		up, down;
  qboolean stepped = qfalse; //q3. used for view smooth
  qboolean blocked_fwd = qfalse; //q3. used for view smooth

  VectorCopy(pm->ps->origin, start_o);
  VectorCopy(pm->ps->velocity, start_v);

  //move forward
  if (PM_SlideMove(gravity))
    blocked_fwd = qtrue;
  //copy flat move end pos
  VectorCopy(pm->ps->origin, down_o);
  VectorCopy(pm->ps->velocity, down_v);

  VectorCopy(start_o, up);
  up[2] += STEPSIZE;
  //trace up, check blocked
  pm->trace(&trace, up, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);
  if (trace.allsolid)
    return qfalse;		// can't step up

  // try sliding from step up
  VectorCopy(up, pm->ps->origin);
  VectorCopy(start_v, pm->ps->velocity);

  PM_SlideMove(gravity);

  // push down the final amount
  VectorCopy(pm->ps->origin, down);
  down[2] -= STEPSIZE;
  pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
  if (!trace.allsolid)
  {
    VectorCopy(trace.endpos, pm->ps->origin);
  }

  VectorCopy(pm->ps->origin, up);

  // decide which one went farther
  down_dist = (down_o[0] - start_o[0])*(down_o[0] - start_o[0]) + (down_o[1] - start_o[1])*(down_o[1] - start_o[1]);
  up_dist = (up[0] - start_o[0])*(up[0] - start_o[0]) + (up[1] - start_o[1])*(up[1] - start_o[1]);

  //step down
  if (down_dist > up_dist || trace.plane.normal[2] < MIN_STEP_NORMAL)
  {
    //q3 smooth view going down stairs
    if (pml.groundPlane)
      PM_checkStepEvent(start_o, down_o);

    VectorCopy(down_o, pm->ps->origin);
    VectorCopy(down_v, pm->ps->velocity);
    return qtrue;
  }

  //q3 smooth view going up stairs
  if (pml.groundPlane)
    PM_checkStepEvent(start_o, up);

  //!! Special case
  // if we were walking along a plane, then we need to copy the Z over
  pm->ps->velocity[2] = down_v[2];

  return qfalse;
}
#endif




/*
==================
PM_PredictStepMove
//hypov8 add
//xreal copy, stops player going into jump motion on small steps
==================
*/
qboolean PM_PredictStepMove(void)
{
	vec3_t          velocity, origin;
	float           impactSpeed;
	qboolean        stepped = qfalse;

	VectorCopy(pm->ps->velocity, velocity);
	VectorCopy(pm->ps->origin, origin);
	impactSpeed = pml.impactSpeed;

	if (PM_StepSlideMove(qfalse, qtrue)) //hypov8 prediction??
		stepped = qtrue;

	VectorCopy(velocity, pm->ps->velocity);
	VectorCopy(origin, pm->ps->origin);
	pml.impactSpeed = impactSpeed;

	return stepped;
}
