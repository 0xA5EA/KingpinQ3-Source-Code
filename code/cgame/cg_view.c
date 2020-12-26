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
along with KingpinQ3 source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering

#include "cg_local.h"

/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.
=============================================================================
*/

/*
=================
CG_TestModel_f
Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f(void)
{
  vec3_t angles;

  Com_Memset(&cg.testModelEntity, 0, sizeof(cg.testModelEntity));
  if (trap_Argc() < 2)
  {
    CG_Printf("usage: testModel <modelname> [lerp]\n");
    return;
  }

  Q_strncpyz(cg.testModelName, CG_Argv(1), MAX_QPATH);
  cg.testModelEntity.hModel = trap_R_RegisterModel(cg.testModelName);

  if (trap_Argc() == 3)
  {
    cg.testModelEntity.backlerp = 1.0f - atof(CG_Argv(2));
    cg.testModelEntity.frame = 1;
    cg.testModelEntity.oldframe = 0;
  }
  if (!cg.testModelEntity.hModel)
  {
    CG_Printf("Can't register model\n");
    return;
  }

  VectorMA(cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin);

  angles[PITCH] = 0;
  angles[YAW] = 180 + cg.refdefViewAngles[1];
  angles[ROLL] = 0;

  AnglesToAxis(angles, cg.testModelEntity.axis);
  cg.testGun = qfalse;
}

/*
=================
CG_TestGun_f
Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f(void)
{
  CG_TestModel_f();
  cg.testGun = qtrue;
  cg.testModelEntity.renderfx |= RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}

/*
=================
CG_TestAnimation_f
=================
*/
void CG_TestAnimation_f(void)
{
  if (trap_Argc() < 2)
  {
    CG_Printf("usage: testAnimation <animationname>\n");
    return;
  }

  Q_strncpyz(cg.testAnimationName, CG_Argv(1), MAX_QPATH);
  cg.testAnimation = trap_R_RegisterAnimation(cg.testAnimationName);
  //CG_RegisterAnimation

  if (!cg.testAnimation)
  {
    CG_Printf("Can't register animation\n");
    return;
  }

  // check the bones if they match
  if (!trap_R_CheckSkeleton(&cg.testModelEntity.skeleton, cg.testModelEntity.hModel, cg.testAnimation))
  {
    CG_Printf("Can't verify animation\n");
    return;
  }

  // modify bones and set proper local bounds for culling
  if (!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
                            cg.testAnimation,
                            cg.testModelEntity.oldframe, cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
  {
    CG_Printf("Can't build animation\n");
    return;
  }
}

/*
=================
CG_TestBlend_f
=================
*/
void CG_TestBlend_f(void)
{
  if (trap_Argc() < 2)
  {
    CG_Printf("usage: testBlend <animationname> [lerp factor 0.0 - 1.0]\n");
    return;
  }

  if (!cg.testAnimation)
  {
    CG_Printf("Use testAnimation first to set a valid animation\n");
    return;
  }

  Q_strncpyz(cg.testAnimation2Name, CG_Argv(1), MAX_QPATH);
  cg.testAnimation2 = trap_R_RegisterAnimation(cg.testAnimation2Name);

  if (!cg.testAnimation2)
  {
    CG_Printf("Can't register animation2 for blending\n");
    return;
  }

  if (trap_Argc() == 3)
    cg.testModelEntity.backlerp = 1.0f - atof(CG_Argv(2));

  // modify bones and set proper local bounds for culling
  if (!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
                            cg.testAnimation,
                            cg.testModelEntity.oldframe, cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
  {
    CG_Printf("Can't build animation\n");
    return;
  }

  if (!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
                            cg.testAnimation2,
                            cg.testModelEntity.oldframe, cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
  {
    CG_Printf("Can't build animation2\n");
    return;
  }

  // lerp between first and second animation
  if (!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 1.0 - cg.testModelEntity.backlerp))
  {
    CG_Printf("Can't blend animation2\n");
    return;
  }
}

/*
=================
CG_TestModelNextFrame_f
=================
*/
void CG_TestModelNextFrame_f(void)
{
  cg.testModelEntity.frame++;
  CG_Printf("frame %i\n", cg.testModelEntity.frame);
  CG_Printf("cg.testanimation %i\n", cg.testAnimation);

  if (cg.testAnimation)
  {
    if (!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
                              cg.testAnimation,
                              cg.testModelEntity.oldframe,
                              cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
    {
      CG_Printf("Can't build animation\n");
    }
  }

  if (cg.testAnimation2)
  {
    if (!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
                              cg.testAnimation2,
                              cg.testModelEntity.oldframe,
                              cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
    {
      CG_Printf("Can't build animation2\n");
    }

    if (!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 0.5))
    {
      CG_Printf("Can't blend animation2\n");
    }
  }
}

/*
=================
CG_TestModelPrevFrame_f
=================
*/
void CG_TestModelPrevFrame_f(void)
{
  cg.testModelEntity.frame--;
  if (cg.testModelEntity.frame < 0)
    cg.testModelEntity.frame = 0;

  CG_Printf("frame %i\n", cg.testModelEntity.frame);

  if (cg.testAnimation)
  {
    if (!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
                              cg.testAnimation,
                              cg.testModelEntity.oldframe,
                              cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
    {
      CG_Printf("Can't build animation\n");
    }
  }

  if (cg.testAnimation2)
  {
    if (!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
                              cg.testAnimation2,
                              cg.testModelEntity.oldframe,
                              cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
    {
      CG_Printf("Can't build animation2\n");
    }

    if (!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 0.5))
      CG_Printf("Can't blend animation2\n");
  }
}

/*
=================
CG_TestModelIncreaseLerp_f
=================
*/
void CG_TestModelIncreaseLerp_f(void)
{
  cg.testModelEntity.backlerp -= 0.1f;
  if (cg.testModelEntity.backlerp < 0)
    cg.testModelEntity.backlerp = 0;

  CG_Printf("lerp %f\n", 1.0f - cg.testModelEntity.backlerp);

  if (cg.testAnimation)
  {
    if (!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
                              cg.testAnimation,
                              cg.testModelEntity.oldframe,
                              cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
    {
      CG_Printf("Can't build animation\n");
    }
  }

  if (cg.testAnimation2)
  {
    if (!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
                              cg.testAnimation2,
                              cg.testModelEntity.oldframe,
                              cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
    {
      CG_Printf("Can't build animation2\n");
    }

    if (!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 1.0 - cg.testModelEntity.backlerp))
      CG_Printf("Can't blend animation2\n");
  }
}

/*
=================
CG_TestModelDecreaseLerp_f
=================
*/
void CG_TestModelDecreaseLerp_f(void)
{
  cg.testModelEntity.backlerp += 0.1f;
  if (cg.testModelEntity.backlerp > 1.0)
  {
    cg.testModelEntity.backlerp = 1;
  }
  CG_Printf("lerp %f\n", 1.0f - cg.testModelEntity.backlerp);

  if (cg.testAnimation)
  {
    if (!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
                              cg.testAnimation,
                              cg.testModelEntity.oldframe,
                              cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
    {
      CG_Printf("Can't build animation\n");
    }
  }

  if (cg.testAnimation2)
  {
    if (!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
                              cg.testAnimation2,
                              cg.testModelEntity.oldframe,
                              cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
    {
      CG_Printf("Can't build animation2\n");
    }

    if (!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 1.0 - cg.testModelEntity.backlerp))
    {
      CG_Printf("Can't blend animation2\n");
    }
  }
}

/*
=================
CG_TestModelNextSkin_f
=================
*/
void CG_TestModelNextSkin_f(void)
{
  cg.testModelEntity.skinNum++;
  CG_Printf("skin %i\n", cg.testModelEntity.skinNum);
}

/*
=================
CG_TestModelPrevSkin_f
=================
*/
void CG_TestModelPrevSkin_f(void)
{
  cg.testModelEntity.skinNum--;

  if (cg.testModelEntity.skinNum < 0)
    cg.testModelEntity.skinNum = 0;

  CG_Printf("skin %i\n", cg.testModelEntity.skinNum);
}

/*
=================
CG_AddTestModel
=================
*/
static void CG_AddTestModel(void)
{
  int i;

  // re-register the model, because the level may have changed
  cg.testModelEntity.hModel = trap_R_RegisterModel(cg.testModelName);
  if (!cg.testModelEntity.hModel)
  {
    CG_Printf("Can't register model\n");
    return;
  }

  // if testing a gun, set the origin reletive to the view origin
  if (cg.testGun)
  {
    VectorCopy(cg.refdef.vieworg, cg.testModelEntity.origin);
    VectorCopy(cg.refdef.viewaxis[0], cg.testModelEntity.axis[0]);
    VectorCopy(cg.refdef.viewaxis[1], cg.testModelEntity.axis[1]);
    VectorCopy(cg.refdef.viewaxis[2], cg.testModelEntity.axis[2]);

    // allow the position to be adjusted
    for (i = 0; i < 3; i++)
    {
      cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gun_x.value;
      cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gun_y.value;
      cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gun_z.value;
    }
  }

#if 0
  if (cg.testAnimation)
  {
    int             max = trap_R_AnimNumFrames(cg.testAnimation);

    cg.testModelEntity.oldframe = cg.testModelEntity.frame;

    cg.testModelEntity.frame++;
    if (cg.testModelEntity.frame >= max)
    {
      cg.testModelEntity.frame = 0;
    }

    if (!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
                              cg.testAnimation,
                              cg.testModelEntity.oldframe,
                              cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
    {
      CG_Printf("Can't build animation\n");
    }
  }
#endif

  if (cg.testModelEntity.skeleton.type == SK_RELATIVE)
  {
    // transform relative bones to absolute ones required for vertex skinning
    CG_TransformSkeleton(&cg.testModelEntity.skeleton, 1);
  }

  trap_R_AddRefEntityToScene(&cg.testModelEntity);
}

/*
=================
CG_TestOmniLight_f
Creates a omni-directional light in front of the current position, which can then be moved around
=================
*/
void CG_TestOmniLight_f(void)
{
  vec3_t angles;

  Com_Memset(&cg.testLight, 0, sizeof(cg.testLight));
  if (trap_Argc() < 2)
  {
    CG_Printf("usage: testOmniLight <lightShaderName>\n");
    return;
  }

  Q_strncpyz(cg.testLightName, CG_Argv(1), sizeof(cg.testLightName));
  cg.testLight.attenuationShader = trap_R_RegisterShaderLightAttenuation(cg.testLightName);

  if (!cg.testLight.attenuationShader)
  {
    CG_Printf("Can't register attenuation shader\n");
    return;
  }

  cg.testLight.rlType = RL_OMNI;
  //cg.testLight.lightfx = LF_ROTATION;

  VectorMA(cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testLight.origin);

  cg.testLight.color[0] = 1.0;
  cg.testLight.color[1] = 1.0;
  cg.testLight.color[2] = 1.0;

  cg.testLight.radius[0] = 300;
  cg.testLight.radius[1] = 300;
  cg.testLight.radius[2] = 300;

  angles[PITCH] = cg.refdefViewAngles[PITCH];
  angles[YAW] = cg.refdefViewAngles[YAW];	// + 180;
  angles[ROLL] = 0;

  AnglesToQuat(angles, cg.testLight.rotation);

  cg.testFlashLight = qfalse;
}

/*
=================
CG_TestProjLight_f
Creates a projective light in front of the current position, which can then be moved around
=================
*/
void CG_TestProjLight_f(void)
{
  float fov_x;
  Com_Memset(&cg.testLight, 0, sizeof(cg.testLight));
  if (trap_Argc() < 2)
  {
    CG_Printf("usage: testProjLight <lightShaderName>\n");
    return;
  }

  Q_strncpyz(cg.testLightName, CG_Argv(1), sizeof(cg.testLightName));
  cg.testLight.attenuationShader = trap_R_RegisterShaderLightAttenuation(cg.testLightName);

  if (!cg.testLight.attenuationShader)
  {
    CG_Printf("Can't register attenuation shader\n");
    return;
  }

  cg.testLight.rlType = RL_PROJ;
  //  cg.testLight.lightfx = LF_ROTATION;

  // VectorMA(cg.refdef.vieworg, 10, cg.refdef.viewaxis[0], cg.testLight.origin);
  VectorCopy(cg.refdef.vieworg, cg.testLight.origin);

  cg.testLight.color[0] = 1.0;
  cg.testLight.color[1] = 1.0;
  cg.testLight.color[2] = 1.0;

  //AnglesToQuat(cg.refdefViewAngles, cg.testLight.rotation);
  QuatClear(cg.testLight.rotation);
#if 0 // FIXME: 0xA5EA
  cg.testLight.fovX = 45;
  cg.testLight.fovY = 45;
  cg.testLight.distNear = 1;
  cg.testLight.distFar = 800;
#endif

  //VectorSet(cg.testLight.projTarget, 800, 0, 0);
  //VectorSet(cg.testLight.projRight, 50, 0, 0);
  //VectorSet(cg.testLight.projUp, 50, 0, 0);
#ifndef Q3_VM
  fov_x = tanf((cg.refdef.fov_x * 0.5f)*M_DEG2RAD);
#else
  fov_x = tan((cg.refdef.fov_x * 0.5f)*DEG2RAD);
#endif
  VectorCopy(cg.refdef.viewaxis[0], cg.testLight.projTarget);
  VectorScale(cg.refdef.viewaxis[1], -fov_x, cg.testLight.projRight);
  VectorScale(cg.refdef.viewaxis[2], fov_x, cg.testLight.projUp);
  VectorScale(cg.refdef.viewaxis[0], 10, cg.testLight.projStart);
  VectorScale(cg.refdef.viewaxis[0], 1000, cg.testLight.projEnd);
  cg.testFlashLight = qfalse;
}

/*
=================
CG_TestFlashLight_f
=================
*/
void CG_TestFlashLight_f(void)
{
  if (trap_Argc() < 2)
  {
    CG_Printf("usage: testFlashLight <lightShaderName>\n");
    return;
  }

  CG_TestProjLight_f();
  cg.testFlashLight = qtrue;
}

static void CG_AddTestLight(void)
{
  float fov_x;

  // re-register the model, because the level may have changed
  cg.testLight.attenuationShader = trap_R_RegisterShaderLightAttenuation(cg.testLightName);
  if (!cg.testLight.attenuationShader)
  {
    CG_Printf("Can't register attenuation shader\n");
    return;
  }
  // if testing a flashlight, set the projection direction reletive to the view direction
  if (cg.testFlashLight)
  {
    VectorCopy(cg.refdef.vieworg, cg.testLight.origin);
#ifndef Q3_VM
    fov_x = tanf(M_DEG2RAD*(cg.refdef.fov_x * 0.5f));
#else
    fov_x = tan(DEG2RAD*(cg.refdef.fov_x * 0.5f));
#endif
    VectorCopy(cg.refdef.viewaxis[0], cg.testLight.projTarget);
    VectorScale(cg.refdef.viewaxis[1], -fov_x, cg.testLight.projRight);
    VectorScale(cg.refdef.viewaxis[2], fov_x, cg.testLight.projUp);
    VectorScale(cg.refdef.viewaxis[0], 10, cg.testLight.projStart);
    VectorScale(cg.refdef.viewaxis[0], 1000, cg.testLight.projEnd);
  }
  trap_R_AddRefLightToScene(&cg.testLight);
}

/*
=================
CG_TestGib_f
=================
*/
void CG_TestGib_f(void)
{
  vec3_t origin;
  VectorMA(cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], origin);
  CG_GibPlayer(origin);
}

//============================================================================
/*
=================
CG_CalcVrect
Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect(void)
{
  int size;

  // the intermission should allways be full screen
  if (cg.snap->ps.pm_type == PM_INTERMISSION)
  {
    size = 100;
  }
  else
  {
    // bound normal viewsize
    if (cg_viewsize.integer < 30)
    {
      trap_Cvar_Set("cg_viewsize", "30");
      size = 30;
    }
    else if (cg_viewsize.integer > 100)
    {
      trap_Cvar_Set("cg_viewsize", "100");
      size = 100;
    }
    else
      size = cg_viewsize.integer;
  }

  cg.refdef.width = cgs.glconfig.vidWidth * size / 100;
  cg.refdef.width &= ~1;

  cg.refdef.height = cgs.glconfig.vidHeight * size / 100;
  cg.refdef.height &= ~1;

  cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width) / 2;
  cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height) / 2;
}

/*
===============
CG_OffsetThirdPersonView
===============
*/
#define	FOCUS_DISTANCE	512
static void CG_OffsetThirdPersonView(void)
{
  vec3_t forward, right, up;
  vec3_t view;
  vec3_t focusAngles;
  trace_t trace;
  static const vec3_t mins = { -8, -8, -8 };
  static const vec3_t maxs = { 8, 8, 8 };
  vec3_t focusPoint;
  float focusDist;
  float forwardScale, sideScale;
  vec3_t surfNormal;

  VectorSet(surfNormal, 0.0f, 0.0f, 1.0f);
  VectorMA(cg.refdef.vieworg, cg.predictedPlayerState.viewheight, surfNormal, cg.refdef.vieworg);
  VectorCopy(cg.refdefViewAngles, focusAngles);

  // if dead, look at killer
  if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
  {
    focusAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
    cg.refdefViewAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
  }

  AngleVectors(focusAngles, forward, NULL, NULL);

  VectorMA(cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint);
  VectorCopy(cg.refdef.vieworg, view);
  VectorMA(view, 12, surfNormal, view);

  //cg.refdefViewAngles[PITCH] *= 0.5;
  AngleVectors(cg.refdefViewAngles, forward, right, up);

  forwardScale = cos(cg_thirdPersonAngle.value / 180 * M_PI);
  sideScale = sin(cg_thirdPersonAngle.value / 180 * M_PI);

  VectorMA(view, -cg_thirdPersonRange.value * forwardScale, forward, view);
  VectorMA(view, -cg_thirdPersonRange.value * sideScale, right, view);

  // trace a ray from the origin to the viewpoint to make sure the view isn't
  // in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything
  if (!cg_cameraMode.integer)
  {
    CG_Trace(&trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID);

    if (trace.fraction != 1.0)
    {
      VectorCopy(trace.endpos, view);
      view[2] += (1.0 - trace.fraction) * 32;

      // try another trace to this position, because a tunnel may have the ceiling
      // close enogh that this is poking out
      CG_Trace(&trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID);
      VectorCopy(trace.endpos, view);
    }
  }

  VectorCopy(view, cg.refdef.vieworg);

  // select pitch to look at focus point from vieword
  VectorSubtract(focusPoint, cg.refdef.vieworg, focusPoint);
  focusDist = sqrt(focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1]);

  if (focusDist < 1)
    focusDist = 1;			// should never happen

  cg.refdefViewAngles[PITCH] = -180 / M_PI * atan2(focusPoint[2], focusDist);
  cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle.value;
}

// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset(void)
{
  float steptime;
  int timeDelta;
  vec3_t normal;
  //playerState_t  *ps = &cg.predictedPlayerState;
  VectorSet(normal, 0.0f, 0.0f, 1.0f);

  steptime = STEP_TIME;

  // smooth out stair climbing
  timeDelta = cg.time - cg.stepTime;
  if (timeDelta < steptime)
  {
    float stepChange = cg.stepChange * (steptime - timeDelta) / steptime;
    cg.refdef.vieworg[2] -= stepChange;
  }
}

/*
===============
CG_OffsetFirstPersonView
===============
*/
static void CG_OffsetFirstPersonView(void)
{
  float *origin;
  float *angles;
  float  bob;
  float  ratio;
  float  delta;
  float  speed;
  float  f;
  vec3_t predictedVelocity;
  int timeDelta;
  vec3_t normal;
//	playerState_t  *ps = &cg.predictedPlayerState;
  VectorSet(normal, 0.0f, 0.0f, 1.0f);

  if (cg.snap->ps.pm_type == PM_INTERMISSION)
    return;

  origin = cg.refdef.vieworg;
  angles = cg.refdefViewAngles;

  // if dead, fix the angle and don't add any kick
  if (cg.snap->ps.stats[STAT_HEALTH] <= 0)
  {
    angles[ROLL] = 40;
    angles[PITCH] = -15;
    angles[YAW] = cg.snap->ps.stats[STAT_DEAD_YAW];
    origin[2] += cg.predictedPlayerState.viewheight;
    return;
  }

  // add angles based on weapon kick
  VectorAdd(angles, cg.kick_angles, angles);

  // add angles based on damage kick
  if (cg.damageTime)
  {
    ratio = cg.time - cg.damageTime;
    if (ratio < DAMAGE_DEFLECT_TIME)
    {
      ratio /= DAMAGE_DEFLECT_TIME;
      angles[PITCH] += ratio * cg.v_dmg_pitch;
      angles[ROLL] += ratio * cg.v_dmg_roll;
    }
    else
    {
      ratio = 1.0 - (ratio - DAMAGE_DEFLECT_TIME) / DAMAGE_RETURN_TIME;
      if (ratio > 0)
      {
        angles[PITCH] += ratio * cg.v_dmg_pitch;
        angles[ROLL] += ratio * cg.v_dmg_roll;
      }
    }
  }

  // add pitch based on fall kick
#if 0
  ratio = (cg.time - cg.landTime) / FALL_TIME;
  if (ratio < 0)
    ratio = 0;
  angles[PITCH] += ratio * cg.fall_value;
#endif

  // add angles based on velocity
  VectorCopy(cg.predictedPlayerState.velocity, predictedVelocity);

  delta = DotProduct(predictedVelocity, cg.refdef.viewaxis[0]);
  angles[PITCH] += delta * cg_runpitch.value;

  delta = DotProduct(predictedVelocity, cg.refdef.viewaxis[1]);
  angles[ROLL] -= delta * cg_runroll.value;

  // add angles based on bob
  // make sure the bob is visible even at low speeds
  speed = cg.xyspeed > 200 ? cg.xyspeed : 200;

  delta = cg.bobfracsin * cg_bobpitch.value * speed;

  if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
    delta *= 3;				// crouching

  angles[PITCH] += delta;
  delta = cg.bobfracsin * cg_bobroll.value * speed;

  if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
    delta *= 3;				// crouching accentuates roll

  if (cg.bobcycle & 1)
    delta = -delta;

  angles[ROLL] += delta;

  // add view height
  origin[2] += cg.predictedPlayerState.viewheight;

  // smooth out duck height changes
  timeDelta = cg.time - cg.duckTime;

  if (timeDelta < DUCK_TIME)
    cg.refdef.vieworg[2] -= cg.duckChange * (DUCK_TIME - timeDelta) / DUCK_TIME;

  // add bob height
  bob = cg.bobfracsin * cg.xyspeed * cg_bobup.value;

  if (bob > 6)
    bob = 6;

  // TA: likewise for bob
  origin[2] += bob;

  // add fall height
  delta = cg.time - cg.landTime;

  if (delta < LAND_DEFLECT_TIME)
  {
    f = delta / LAND_DEFLECT_TIME;
    cg.refdef.vieworg[2] += cg.landChange * f;
  }
  else if (delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME)
  {
    delta -= LAND_DEFLECT_TIME;
    f = 1.0 - (delta / LAND_RETURN_TIME);
    cg.refdef.vieworg[2] += cg.landChange * f;
  }

  // add step offset
  CG_StepOffset();

  // add kick offset
  VectorAdd(origin, cg.kick_origin, origin);

  // pivot the eye based on a neck length
#if 0
  {
#define	NECK_LENGTH		8
    vec3_t forward, up;

    cg.refdef.vieworg[2] -= NECK_LENGTH;
    AngleVectors(cg.refdefViewAngles, forward, NULL, up);
    VectorMA(cg.refdef.vieworg, 3, forward, cg.refdef.vieworg);
    VectorMA(cg.refdef.vieworg, NECK_LENGTH, up, cg.refdef.vieworg);
  }
#endif
}

//======================================================================
void CG_ZoomDown_f(void)
{
  if (cg.zoomed)
    return;

  cg.zoomed = qtrue;
  cg.zoomTime = cg.time;
}

void CG_ZoomUp_f(void)
{
  if (!cg.zoomed)
    return;

  cg.zoomed = qfalse;
  cg.zoomTime = cg.time;
}

/*
====================
CG_CalcFov
Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4
static int CG_CalcFov(void)
{
  float  x;
  float  phase;
  float  v;
  int contents;
  float  fov_x, fov_y;
  float  zoomFov;
  float  f;
  int inwater;

  if (cg.predictedPlayerState.pm_type == PM_INTERMISSION)
    fov_x = 90;
  else if (cg.predictedPlayerState.pm_type == PM_SPECTATOR)
    fov_x = 90;
  else
  {
    // user selectable
    if (cgs.dmflags & DF_FIXED_FOV)
    {
      fov_x = 90;
    }
    else
    {
      fov_x = cg_fov.value; //hypov8 cap max fov. cheat? and gun looks silly
      if (fov_x < 30) //hypov8 was 1
        fov_x = 30;
      else if (fov_x > 120) //hypov8 was 160
        fov_x = 120;
    }

    // account for zooms
    zoomFov = cg_zoomFov.value;
    if (zoomFov < 30)  //hypov8 was 1
      zoomFov = 30;
    else if (zoomFov > 120) //hypov8 was 160
      zoomFov = 120;

    if (cg.zoomed)
    {
      f = (cg.time - cg.zoomTime) / (float)ZOOM_TIME;
      if (f > 1.0)
        fov_x = zoomFov;
      else
        fov_x = fov_x + f * (zoomFov - fov_x);
    }
    else
    {
      f = (cg.time - cg.zoomTime) / (float)ZOOM_TIME;

      if (f > 1.0)
        fov_x = fov_x;
      else
        fov_x = zoomFov + f * (fov_x - zoomFov);
    }
  }

  x = cg.refdef.width / tan(fov_x / 360 * M_PI);
  fov_y = atan2(cg.refdef.height, x);
  fov_y = fov_y * 360 / M_PI;

  // warp if underwater
  contents = CG_PointContents(cg.refdef.vieworg, -1);

  if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
  {
    phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
    v = WAVE_AMPLITUDE * sin(phase);
    fov_x += v;
    fov_y -= v;
    inwater = qtrue;
  }
  else
    inwater = qfalse;

  // set it
  cg.refdef.fov_x = fov_x;
  cg.refdef.fov_y = fov_y;

  if (!cg.zoomed)
    cg.zoomSensitivity = 1;
  else
    cg.zoomSensitivity = cg.refdef.fov_y / 75.0;

  return inwater;
}

/*
===============
CG_DamageBlendBlob
===============
*/
static void CG_DamageBlendBlob(void)
{
  int t;
  int maxTime;
  refEntity_t     ent;
  float fade;

  if (!cg.damageValueBlood)
    return;


  //if (cg.cameraMode) {
  //  return;
  //}

  maxTime = DAMAGE_TIME;
  t = cg.time - cg.damageTimeBlood;
  if (t <= 0 || t >= maxTime)
    return;

  Com_Memset(&ent, 0, sizeof(ent));
  ent.reType = RT_SPRITE;
  ent.renderfx = RF_FIRST_PERSON;

  VectorMA(cg.refdef.vieworg, 8, cg.refdef.viewaxis[0], ent.origin);
  VectorMA(ent.origin, cg.damageX * -8, cg.refdef.viewaxis[1], ent.origin);
  VectorMA(ent.origin, cg.damageY * 8, cg.refdef.viewaxis[2], ent.origin);

  ent.radius = cg.damageValueBlood * 3;
  ent.customShader = cgs.media.viewBloodShader;

  fade = ( 1.0 - ( ( float ) t / maxTime ) );
  ent.shaderRGBA[0] = (byte)(127 * fade);
  ent.shaderRGBA[1] = (byte)(127 * fade);
  ent.shaderRGBA[2] = (byte)(127 * fade);
  ent.shaderRGBA[3] = 255; //not used(shader color blended)
  trap_R_AddRefEntityToScene(&ent);
}

/*
===============
CG_DamageBlendFlames
===============
*/
static void CG_DamageBlendFlames(void)
{
  int t;
  int maxTime;
  refEntity_t     ent;
  float fade;

  if ( !cg.damageTimeFlamer )
	  return;

  maxTime = DAMAGE_TIME * 4;
  t = cg.time - cg.damageTimeFlamer;
  if (t <= 0 || t >= maxTime)
    return;

  Com_Memset(&ent, 0, sizeof(ent));
  ent.reType = RT_SPRITE;
  ent.renderfx = RF_FIRST_PERSON;

  VectorMA(cg.refdef.vieworg, 8, cg.refdef.viewaxis[0], ent.origin);
  VectorMA(ent.origin, 0, cg.refdef.viewaxis[1], ent.origin);
  VectorMA(ent.origin, 8, cg.refdef.viewaxis[2], ent.origin);

  ent.radius = 180;
  ent.customShader = cgs.media.onFireShader2;

  fade = ( 1.0 - ( ( float ) t / maxTime ) );
  ent.shaderRGBA[0] = (byte)(32 * fade);
  ent.shaderRGBA[1] = (byte)(32 * fade);
  ent.shaderRGBA[2] = (byte)(32 * fade);
  ent.shaderRGBA[3] = 255; //not used(shader color blended)
  trap_R_AddRefEntityToScene(&ent);
}

#ifdef HYPODEBUG
void cg_TempItemLocation(vec3_t origin)
{
	localEntity_t *le;
	refEntity_t *re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + 50;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.shotgunBrassModel;
	AxisClear(re->axis);

	VectorCopy(origin, re->origin);

}
#endif

/*
===============
CG_StartShadowCaster

Helper function to add a inverse dynamic light to create shadows for the
following models.
===============
*/
void CG_StartShadowCaster( vec3_t origin, const vec3_t mins, const vec3_t maxs )
{
  vec3_t ambientLight, directedLight, lightDir;
  vec3_t lightPos;
  trace_t tr;
  vec3_t traceMins = { -3.0f, -3.0f, -3.0f };
  vec3_t traceMaxs = { 3.0f, 3.0f, 3.0f };
  float maxLightDist = Distance(maxs, mins);

  // find a point to place the light source by tracing in the
  // average light direction
  trap_R_LightForPoint(origin, ambientLight, directedLight, lightDir);
  VectorMA(origin, 3.0f * maxLightDist, lightDir, lightPos);

  CG_Trace(&tr, origin, traceMins, traceMaxs, lightPos, 0, MASK_OPAQUE);

  if (!tr.startsolid)
  {
    VectorCopy(tr.endpos, lightPos);
  }

	//hypo find origin with a model
#if 0 //def HYPODEBUG
	cg_TempItemLocation(lightPos);
#endif
  trap_R_AddLightToScene(lightPos, 2.0f * Distance(lightPos, origin), 3.0f,
      directedLight[0], directedLight[1], directedLight[2], 0,
      REF_RESTRICT_DLIGHT | REF_INVERSE_DLIGHT);
}
/*
===============
CG_EndShadowCaster

Helper function to terminate the list of models for the last shadow caster.
following models.
===============
*/
void CG_EndShadowCaster( void ) {
	trap_R_AddLightToScene( vec3_origin, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f,
				0, 0 ); //hypov8 merge: todo? REF_RESTRICT_DLIGHT
}

/*
===============
CG_CalcViewValues
Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues(void)
{
  playerState_t *ps = 0;

  Com_Memset(&cg.refdef, 0, sizeof(cg.refdef));

  // calculate size of 3D view
  CG_CalcVrect();

  ps = &cg.predictedPlayerState;
  /*
  	if (cg.cameraMode) {
  		vec3_t origin, angles;
  		if (trap_getCameraInfo(cg.time, &origin, &angles)) {
  			VectorCopy(origin, cg.refdef.vieworg);
  			angles[ROLL] = 0;
  			VectorCopy(angles, cg.refdefViewAngles);
  			AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
  			return CG_CalcFov();
  		} else {
  			cg.cameraMode = qfalse;
  		}
  	}
  */
  // intermission view
  if (ps->pm_type == PM_INTERMISSION)
  {
    VectorCopy(ps->origin, cg.refdef.vieworg);
    VectorCopy(ps->viewangles, cg.refdefViewAngles);
    AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);

    return CG_CalcFov();
  }

  cg.bobcycle = (ps->bobCycle & 128) >> 7;
  cg.bobfracsin = fabs(sin((ps->bobCycle & 127) / 127.0 * M_PI));
  cg.xyspeed = sqrt(ps->velocity[0] * ps->velocity[0] + ps->velocity[1] * ps->velocity[1]);

  VectorCopy(ps->origin, cg.refdef.vieworg);
  VectorCopy( ps->viewangles, cg.refdefViewAngles );

  if (cg_cameraOrbit.integer)
  {
    if (cg.time > cg.nextOrbitTime)
    {
      cg.nextOrbitTime = cg.time + cg_cameraOrbitDelay.integer;
      cg_thirdPersonAngle.value += cg_cameraOrbit.value;
    }
  }
  // add error decay
  if (cg_errorDecay.value > 0)
  {
    int t;
    float f;

    t = cg.time - cg.predictedErrorTime;
    f = (cg_errorDecay.value - t) / cg_errorDecay.value;

    if (f > 0 && f < 1)
      VectorMA(cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg);
    else
      cg.predictedErrorTime = 0;
  }

  if (cg.renderingThirdPerson)
  {
    // back away from character
    CG_OffsetThirdPersonView();
  }
  else
  {
    // offset for local bobbing and kicks
    CG_OffsetFirstPersonView();
  }

  // position eye reletive to origin
  AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);

  if (cg.hyperspace)
    cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;

  // field of view
  return CG_CalcFov();
}

/*
=====================
CG_PowerupTimerSounds
=====================
*/
static void CG_PowerupTimerSounds(void)
{
  int i, t;

  // powerup timers going away
  for (i = PW_DEGRADE_START; i < MAX_POWERUPS; i++) //hypov8 add: powerup countdown sounds
  {
    t = cg.snap->ps.powerups[i];

    if (t <= cg.time)
      continue;

    if (t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME)
      continue;

    if ((t - cg.time) / POWERUP_BLINK_TIME != (t - cg.oldTime) / POWERUP_BLINK_TIME)
      trap_S_StartSound(NULL, cg.snap->ps.clientNum, CHAN_ITEM, cgs.media.wearOffSound);
  }
}

/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound(sfxHandle_t sfx)
{
  if (!sfx)
    return;

  cg.soundBuffer[cg.soundBufferIn] = sfx;
  cg.soundBufferIn = (cg.soundBufferIn + 1) % MAX_SOUNDBUFFER;

  if (cg.soundBufferIn == cg.soundBufferOut)
    cg.soundBufferOut++;
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds(void)
{
  if (cg.soundTime < cg.time)
  {
    if (cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[cg.soundBufferOut])
    {
      trap_S_StartLocalSound(cg.soundBuffer[cg.soundBufferOut], CHAN_ANNOUNCER);
      cg.soundBuffer[cg.soundBufferOut] = 0;
      cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
      cg.soundTime = cg.time + 750;
    }
  }
}

//=========================================================================
/*
=================
CG_DrawActiveFrame
Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame(int serverTime, stereoFrame_t stereoView, qboolean demoPlayback)
{
  int inwater;

  cg.time = serverTime;

  cg.demoPlayback = demoPlayback;

  // update cvars
  CG_UpdateCvars();

  // if we are only updating the screen as a loading
  // pacifier, don't even try to read snapshots
  if (cg.progress > 0)
  {
    CG_DrawInformation();
    return;
  }

  CG_PB_ClearPolyBuffers();

  // any looped sounds will be respecified as entities
  // are added to the render list
  trap_S_ClearLoopingSounds(qfalse);

  // clear all the render lists
  trap_R_ClearScene();

  // set up cg.snap and possibly cg.nextSnap
  CG_ProcessSnapshots();

  // if we haven't received any snapshots yet, all
  // we can draw is the information screen
  if (!cg.snap || (cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE))
  {
    CG_DrawInformation();
    return;
  }

  // let the client system know what our weapon and zoom settings are
  trap_SetUserCmdValue(cg.weaponSelect, cg.zoomSensitivity);

  // this counter will be bumped for every valid scene we generate
  cg.clientFrame++;

  // update cg.predictedPlayerState
  CG_PredictPlayerState();

  // decide on third person view
  cg.renderingThirdPerson = cg_thirdPerson.integer || (cg.snap->ps.stats[STAT_HEALTH] <= 0) || cg.snap->entities->chaseCamMode; //hypov8 ToDo: add cam mode for specs

  // build cg.refdef
  inwater = CG_CalcViewValues();
  if (inwater)
    cg.refdef.rdflags |= RDF_UNDERWATER;

  // first person blend blobs, done after AnglesToAxis
  //if (!cg.renderingThirdPerson) //always render
    CG_DamageBlendBlob();

	CG_DamageBlendFlames();

  // build the render lists
  if (!cg.hyperspace)
  {
    CG_AddPacketEntities();	// adter calcViewValues, so predicted player state is correct
    CG_AddMarks();
    CG_AddParticles();
    CG_AddLocalEntities();

    CG_AddFlameChunks(); //FIXME(0xA5EA): check trails of etxreal
	// Added -KRYPTYK
    CG_AddTrails();		// this must come last, so the trails dropped this frame get drawn
  }

  CG_AddViewWeapon(&cg.predictedPlayerState);

  // add buffered sounds
  CG_PlayBufferedSounds();

  // play buffered voice chats
  CG_PlayBufferedVoiceChats();

  // finish up the rest of the refdef
  if (cg.testModelEntity.hModel)
    CG_AddTestModel();

  // Tr3B - test light to preview Doom3 style light attenuation shaders
  if (cg.testLight.attenuationShader)
    CG_AddTestLight();

  cg.refdef.time = cg.time;
  Com_Memcpy(cg.refdef.areamask, cg.snap->areamask, sizeof(cg.refdef.areamask));

  // warning sounds when powerup is wearing off
  CG_PowerupTimerSounds(); //hypov8 do not need anymore???

  // update audio positions
  //	if (cg_thirdPerson.value) { //daemon .50 hypov8 todo:
  //		trap_S_Respatialize( -1, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );
  //else
  trap_S_Respatialize(cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater);

  // make sure the lagometerSample and frame timing isn't done twice when in stereo
  if (stereoView != STEREO_RIGHT)
  {
    cg.frametime = cg.time - cg.oldTime;

    if (cg.frametime < 0)
      cg.frametime = 0;

    cg.oldTime = cg.time;
    CG_AddLagometerFrameInfo();
  }

  if (cg_timescale.value != cg_timescaleFadeEnd.value)
  {
    if (cg_timescale.value < cg_timescaleFadeEnd.value)
    {
      cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
      if (cg_timescale.value > cg_timescaleFadeEnd.value)
        cg_timescale.value = cg_timescaleFadeEnd.value;
    }
    else
    {
      cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
      if (cg_timescale.value < cg_timescaleFadeEnd.value)
        cg_timescale.value = cg_timescaleFadeEnd.value;
    }

    if (cg_timescaleFadeSpeed.value)
      trap_Cvar_Set("timescale", va("%f", cg_timescale.value));
  }

  // actually issue the rendering calls
  CG_DrawActive(stereoView);

  if (cg_stats.integer)
    CG_Printf("cg.clientFrame:%i\n", cg.clientFrame);
}

// some culling bits
typedef struct plane_s
{
  vec3_t normal;
  float dist;
} plane_t;

static plane_t frustum[4];

qboolean CG_CullPointAndRadius(const vec3_t pt, vec_t radius)
{
  int i;
  plane_t *frust;

  // check against frustum planes
  for (i = 0; i < 4; i++)
  {
    frust = &frustum[i];

    if ((DotProduct( pt, frust->normal ) - frust->dist) < -radius)
      return (qtrue);
  }

  return (qfalse);
}
