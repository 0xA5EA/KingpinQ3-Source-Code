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
// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"



//unvan .52
/*
======================
CG_DrawBoxFace

Draws a bounding box face
======================
*/
static void CG_DrawBoxFace( bool solid, vec3_t a, vec3_t b, vec3_t c, vec3_t d )
{
  polyVert_t verts[ 4 ];
  vec4_t     color = { 255.0f, 0.0f, 0.0f, 128.0f };

  VectorCopy( d, verts[ 0 ].xyz );
  verts[ 0 ].st[ 0 ] = 1;
  verts[ 0 ].st[ 1 ] = 1;
  Vector4Copy( color, verts[ 0 ].modulate );

  VectorCopy( c, verts[ 1 ].xyz );
  verts[ 1 ].st[ 0 ] = 1;
  verts[ 1 ].st[ 1 ] = 0;
  Vector4Copy( color, verts[ 1 ].modulate );

  VectorCopy( b, verts[ 2 ].xyz );
  verts[ 2 ].st[ 0 ] = 0;
  verts[ 2 ].st[ 1 ] = 0;
  Vector4Copy( color, verts[ 2 ].modulate );

  VectorCopy( a, verts[ 3 ].xyz );
  verts[ 3 ].st[ 0 ] = 0;
  verts[ 3 ].st[ 1 ] = 1;
  Vector4Copy( color, verts[ 3 ].modulate );

  trap_R_AddPolyToScene( solid ? cgs.media.whiteShader : cgs.media.outlineShader, 4, verts );
}

/*
======================
CG_DrawBoundingBox

Draws a bounding box
======================
*/
void CG_DrawBoundingBox( int type, vec3_t origin, vec3_t mins, vec3_t maxs )
{
  bool solid = (type > 1);

  vec3_t ppp, mpp, mmp, pmp;
  vec3_t mmm, pmm, ppm, mpm;

  ppp[ 0 ] = origin[ 0 ] + maxs[ 0 ];
  ppp[ 1 ] = origin[ 1 ] + maxs[ 1 ];
  ppp[ 2 ] = origin[ 2 ] + maxs[ 2 ];

  mpp[ 0 ] = origin[ 0 ] + mins[ 0 ];
  mpp[ 1 ] = origin[ 1 ] + maxs[ 1 ];
  mpp[ 2 ] = origin[ 2 ] + maxs[ 2 ];

  mmp[ 0 ] = origin[ 0 ] + mins[ 0 ];
  mmp[ 1 ] = origin[ 1 ] + mins[ 1 ];
  mmp[ 2 ] = origin[ 2 ] + maxs[ 2 ];

  pmp[ 0 ] = origin[ 0 ] + maxs[ 0 ];
  pmp[ 1 ] = origin[ 1 ] + mins[ 1 ];
  pmp[ 2 ] = origin[ 2 ] + maxs[ 2 ];

  ppm[ 0 ] = origin[ 0 ] + maxs[ 0 ];
  ppm[ 1 ] = origin[ 1 ] + maxs[ 1 ];
  ppm[ 2 ] = origin[ 2 ] + mins[ 2 ];

  mpm[ 0 ] = origin[ 0 ] + mins[ 0 ];
  mpm[ 1 ] = origin[ 1 ] + maxs[ 1 ];
  mpm[ 2 ] = origin[ 2 ] + mins[ 2 ];

  mmm[ 0 ] = origin[ 0 ] + mins[ 0 ];
  mmm[ 1 ] = origin[ 1 ] + mins[ 1 ];
  mmm[ 2 ] = origin[ 2 ] + mins[ 2 ];

  pmm[ 0 ] = origin[ 0 ] + maxs[ 0 ];
  pmm[ 1 ] = origin[ 1 ] + mins[ 1 ];
  pmm[ 2 ] = origin[ 2 ] + mins[ 2 ];

  //phew!

  CG_DrawBoxFace( solid, ppp, mpp, mmp, pmp );
  CG_DrawBoxFace( solid, ppp, pmp, pmm, ppm );
  CG_DrawBoxFace( solid, mpp, ppp, ppm, mpm );
  CG_DrawBoxFace( solid, mmp, mpp, mpm, mmm );
  CG_DrawBoxFace( solid, pmp, mmp, mmm, pmm );
  CG_DrawBoxFace( solid, mmm, mpm, ppm, pmm );
}

/*
======================
CG_PositionEntityOnTag
Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionEntityOnTag(refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName)
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

/*
======================
CG_PositionRotatedEntityOnTag
Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionRotatedEntityOnTag(refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName)
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

/*
=================
CG_TransformSkeleton

transform relative bones to absolute ones required for vertex skinning
=================
*/
void CG_TransformSkeleton(refSkeleton_t *skel, const vec_t scale)

{
  int       i;
  refBone_t *bone;

    skel->scale = scale;

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

}

/*
==========================================================================
FUNCTIONS CALLED EACH FRAME
==========================================================================
*/
/*
======================
CG_SetEntitySoundPosition
Also called by event processing code
======================
*/
void CG_SetEntitySoundPosition(centity_t *cent)
{
  if (cent->currentState.solid == SOLID_BMODEL)
  {
    vec3_t origin;
    float *v;

    v = cgs.inlineModelMidpoints[cent->currentState.modelindex];
    VectorAdd(cent->lerpOrigin, v, origin);
    trap_S_UpdateEntityPosition(cent->currentState.number, origin);
  }
  else
    trap_S_UpdateEntityPosition(cent->currentState.number, cent->lerpOrigin);
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects(centity_t *cent)
{

  // update sound origins
  CG_SetEntitySoundPosition(cent);

  // add loop sound
  if (cent->currentState.loopSound)
  {
    if (cent->currentState.eType != ET_SPEAKER)
      trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.gameSounds[cent->currentState.loopSound]);
    else
      trap_S_AddRealLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.gameSounds[cent->currentState.loopSound]);
  }

  // constant light glow
  if (cent->currentState.constantLight)
  {
    int cl;
    float i, r, g, b;
    cl = cent->currentState.constantLight;
    r = (float) (cl & 0xFF) / 255.0;
    g = (float) ((cl >> 8) & 0xFF) / 255.0;
    b = (float) ((cl >> 16) & 0xFF) / 255.0;
    i = (float) ((cl >> 24) & 0xFF) * 4.0;
    trap_R_AddAdditiveLightToScene(cent->lerpOrigin, i, r, g, b);
  }
}

/*
==================
CG_General
==================
*/
static void CG_General(centity_t *cent)
{
  refEntity_t ent;
  entityState_t *s1;

  s1 = &cent->currentState;

  // if set to invisible, skip
  if (!s1->modelindex)
    return;

  Com_Memset(&ent, 0, sizeof(ent));

  // set frame
  ent.frame    = s1->frame;
  ent.oldframe = ent.frame;
  ent.backlerp = 0;

  VectorCopy(cent->lerpOrigin, ent.origin);
  VectorCopy(cent->lerpOrigin, ent.oldorigin);

  ent.hModel = cgs.gameModels[s1->modelindex];

  // player model
  if (s1->number == cg.snap->ps.clientNum)
    ent.renderfx |= RF_THIRD_PERSON;    // only draw from mirrors

  // convert angles to axis
  AnglesToAxis(cent->lerpAngles, ent.axis);

  // add to refresh list
  trap_R_AddRefEntityToScene(&ent);
}

/*
==================
CG_Speaker
Speaker entities can automatically play sounds
==================
*/
static void CG_Speaker(centity_t *cent)
{
  if (!cent->currentState.clientNum)      // FIXME: use something other than clientNum...
    return;                             // not auto triggering

  if (cg.time < cent->miscTime)
    return;

  trap_S_StartSound(NULL, cent->currentState.number, CHAN_ITEM, cgs.gameSounds[cent->currentState.eventParm]);

  //	ent->s.frame = ent->wait * 10;
  //	ent->s.clientNum = ent->random * 10;
  cent->miscTime = cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * crandom();
}

//#define NO_ROTATING_ITEMS //disabled hypov8
/*
==================
CG_Item
==================
*/
static void CG_Item(centity_t *cent)
{
  refEntity_t ent;
  entityState_t *es;
  gitem_t *item;


  es = &cent->currentState;
  if (es->modelindex >= bg_numItems)
    CG_Error("Bad item index %i on entity", es->modelindex);

  // if set to invisible, skip
  if (!es->modelindex || (es->eFlags & EF_NODRAW))
    return;

  item = &bg_itemlist[es->modelindex];
  Com_Memset(&ent, 0, sizeof(ent));
  VectorCopy(cent->currentState.angles, cent->lerpAngles);
  AnglesToAxis(cent->lerpAngles, ent.axis);


  if (item->giType == IT_WEAPON)
  {
  // an extra height boost
    cent->lerpOrigin[2] += 1;
  }

  ent.hModel = cg_items[es->modelindex].models[0];        // WORLD_EWEAPONMODEL_POS das sind items, keine Waffen

  VectorCopy(cent->lerpOrigin, ent.origin);
  VectorCopy(cent->lerpOrigin, ent.oldorigin);

  ent.nonNormalizedAxes = qfalse;

  // items without glow textures need to keep a minimum light value
  // so they are always visible
  if ((item->giType == IT_WEAPON) || (item->giType == IT_ARMOR))
  {
    //ent.renderfx |= RF_MINLIGHT;
    //ent.renderfx |= RF_NOSHADOW;
    //FIXME(0xA5EA): does this brake reflection mapping ?
  }
  //FIXME: 0xA5EA IT_WEAPMOD and IT_CASH to add ?
#ifdef WITH_BAGMAN_MOD
  if (item->giType == IT_CASH || item->giType == IT_CASH_STOLEN)
  {
    //Com_Printf("IT_CASH\n");
    ent.renderfx |= RF_MINLIGHT;
    ent.renderfx |= RF_NOSHADOW;
    //FIXME(0xA5EA): cashdrop implement
  }
#endif // WITH_BAGMAN_MOD

#ifdef USE_KAMIKAZE
  if (item->giType == IT_HOLDABLE && item->giTag == HI_KAMIKAZE)
  {
    VectorScale(ent.axis[0], 2, ent.axis[0]);
    VectorScale(ent.axis[1], 2, ent.axis[1]);
    VectorScale(ent.axis[2], 2, ent.axis[2]);
    ent.nonNormalizedAxes = qtrue;
  }
#endif
  // add to refresh list
  trap_R_AddRefEntityToScene(&ent);
}
//============================================================================
/*
===============
CG_Missile
===============
*/
static void CG_Missile(centity_t *cent)
{
  refEntity_t ent;
  entityState_t *s1;
  const weaponInfo_t *weapon;
  int col = 0;
  s1 = &cent->currentState;
  if (s1->weapon >= WP_NUM_WEAPONS)
    s1->weapon = 0;
  weapon = &cg_weapons[s1->weapon];

  // calculate the axis
  VectorCopy(s1->angles, cent->lerpAngles);

  // add trails
  if (weapon->missileTrailFunc)
    weapon->missileTrailFunc(cent, weapon);

  if (cent->currentState.modelindex == TEAM_DRAGONS)
    col = 1;
  else if (cent->currentState.modelindex == TEAM_NIKKIS)
    col = 2;
  else
    col = 0;

  // add dynamic light
  if (weapon->missileDlight)
  {
    trap_R_AddAdditiveLightToScene(cent->lerpOrigin, weapon->missileDlight,
                           weapon->missileDlightColor[col][0],
                           weapon->missileDlightColor[col][1],
                           weapon->missileDlightColor[col][2]);
  }

  // add missile sound
  if (weapon->missileSound)
  {
    vec3_t velocity;
    BG_EvaluateTrajectoryDelta(&cent->currentState.pos, cg.time, velocity);
    trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound);
  }

  // create the render entity
  Com_Memset(&ent, 0, sizeof(ent));
  VectorCopy(cent->lerpOrigin, ent.origin);
  VectorCopy(cent->lerpOrigin, ent.oldorigin);

  // flicker between two skins
  ent.skinNum  = cg.clientFrame & 1;
  ent.hModel   = weapon->missileModel;
  ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

  // convert direction of travel into axis
  if (VectorNormalize2(s1->pos.trDelta, ent.axis[0]) == 0)
    ent.axis[0][2] = 1;

  // spin as it moves
  if (s1->pos.trType != TR_STATIONARY)
  {
    RotateAroundDirection(ent.axis, cg.time / 4);
  }
  else
  {
   //  #ifdef MISSIONPACK
    if ( s1->weapon == WP_GRENADE_LAUNCHER )
        AnglesToAxis( cent->lerpAngles, ent.axis );
    else
      RotateAroundDirection(ent.axis, s1->time);
  }
  // add to refresh list, possibly with quad glow
 // CG_AddRefEntityWithPowerups(&ent, s1, TEAM_FREE);//hypov8 no longer used
  trap_R_AddRefEntityToScene(&ent);
}

#ifdef HYPODEBUG
/*
===============
CG_Flamer
===============
*/
static void CG_FlamerDebug(centity_t *cent)
{
  refEntity_t ent;
  entityState_t *s1;
  const weaponInfo_t *weapon;
  int col = 0;
  s1 = &cent->currentState;
  if (s1->weapon >= WP_NUM_WEAPONS)
    s1->weapon = 0;
  weapon = &cg_weapons[s1->weapon];

  // calculate the axis
  VectorCopy(s1->angles, cent->lerpAngles);

  // create the render entity
  Com_Memset(&ent, 0, sizeof(ent));
  VectorCopy(cent->lerpOrigin, ent.origin);
  VectorCopy(cent->lerpOrigin, ent.oldorigin);

  // flicker between two skins
  ent.skinNum  = cg.clientFrame & 1;
  ent.hModel   = weapon->missileModel;
  ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

  // convert direction of travel into axis
  if (VectorNormalize2(s1->pos.trDelta, ent.axis[0]) == 0)
    ent.axis[0][2] = 1;

  // spin as it moves
  RotateAroundDirection(ent.axis, s1->time);

  // add to refresh list
  trap_R_AddRefEntityToScene(&ent);
}
#endif

/*
===============
CG_Grapple
This is called when the grapple is sitting up against the wall
===============
*/
static void CG_Grapple(centity_t *cent)
{
  refEntity_t ent;
  entityState_t *s1;
  const weaponInfo_t *weapon;

  s1 = &cent->currentState;
  if (s1->weapon >= WP_NUM_WEAPONS)
  {
    s1->weapon = 0;
  }
  weapon = &cg_weapons[s1->weapon];

  // calculate the axis
  VectorCopy(s1->angles, cent->lerpAngles);

#if 0 // FIXME add grapple pull sound here..?
  // add missile sound
  if (weapon->missileSound)
  {
    trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->missileSound);
  }
#endif

  // Will draw cable if needed
  CG_GrappleTrail(cent, weapon);

  // create the render entity
  Com_Memset(&ent, 0, sizeof(ent));
  VectorCopy(cent->lerpOrigin, ent.origin);
  VectorCopy(cent->lerpOrigin, ent.oldorigin);

  // flicker between two skins
  ent.skinNum  = cg.clientFrame & 1;
  ent.hModel   = weapon->missileModel;
  ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

  // convert direction of travel into axis
  if (VectorNormalize2(s1->pos.trDelta, ent.axis[0]) == 0)
    ent.axis[0][2] = 1;

  trap_R_AddRefEntityToScene(&ent);
}

/*
===============
CG_Mover
===============
*/
static void CG_Mover(centity_t *cent)
{
  refEntity_t ent;
  entityState_t *s1;

  s1 = &cent->currentState;

  // create the render entity
  Com_Memset(&ent, 0, sizeof(ent));
  VectorCopy(cent->lerpOrigin, ent.origin);
  VectorCopy(cent->lerpOrigin, ent.oldorigin);
  AnglesToAxis(cent->lerpAngles, ent.axis);

  ent.renderfx = RF_NOSHADOW;

  // flicker between two skins (FIXME?)
  ent.skinNum = (cg.time >> 6) & 1;

  // get the model, either as a bmodel or a modelindex
  if (s1->solid == SOLID_BMODEL)
    ent.hModel = cgs.inlineDrawModel[s1->modelindex];
  else
    ent.hModel = cgs.gameModels[s1->modelindex];

  // add to refresh list
  trap_R_AddRefEntityToScene(&ent);

  // add the secondary model
  if (s1->modelindex2)
  {
    ent.skinNum = 0;
    ent.hModel  = cgs.gameModels[s1->modelindex2];
    trap_R_AddRefEntityToScene(&ent);
  }
}

/*
===============
CG_Beam
Also called as an event
===============
*/
void CG_Beam(centity_t *cent)
{
  refEntity_t ent;
  entityState_t *s1;

  s1 = &cent->currentState;

  // create the render entity
  Com_Memset(&ent, 0, sizeof(ent));
  VectorCopy(s1->pos.trBase, ent.origin);
  VectorCopy(s1->origin2, ent.oldorigin);
  AxisClear(ent.axis);
  ent.reType = RT_BEAM;

  ent.renderfx = RF_NOSHADOW;

  // add to refresh list
  trap_R_AddRefEntityToScene(&ent);
}


/*
===============
CG_Portal
===============
*/
static void CG_Portal(centity_t *cent)
{
  refEntity_t ent;
  entityState_t *s1;

  s1 = &cent->currentState;

  // create the render entity
  Com_Memset(&ent, 0, sizeof(ent));
  VectorCopy(cent->lerpOrigin, ent.origin);
  VectorCopy(s1->origin2, ent.oldorigin);
  ByteToDir(s1->eventParm, ent.axis[0]);
  PerpendicularVector(ent.axis[1], ent.axis[0]);

  // negating this tends to get the directions like they want
  // we really should have a camera roll value
  VectorSubtract(vec3_origin, ent.axis[1], ent.axis[1]);

  CrossProduct(ent.axis[0], ent.axis[1], ent.axis[2]);
  ent.reType   = RT_PORTALSURFACE;
  ent.oldframe = s1->powerups;
  ent.frame    = s1->frame;                      // rotation speed
  ent.skinNum  = s1->clientNum / 256.0 * 360;    // roll offset

  // add to refresh list
  trap_R_AddRefEntityToScene(&ent);
}


/*
=========================
CG_AdjustPositionForMover
Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out)
{
  centity_t *cent;
  vec3_t oldOrigin, origin, deltaOrigin;
  vec3_t oldAngles, angles, deltaAngles;

  if (moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL)
  {
    VectorCopy(in, out);
    return;
  }

  cent = &cg_entities[moverNum];
  if (cent->currentState.eType != ET_MOVER)
  {
    VectorCopy(in, out);
    return;
  }

  BG_EvaluateTrajectory(&cent->currentState.pos, fromTime, oldOrigin);
  BG_EvaluateTrajectory(&cent->currentState.apos, fromTime, oldAngles);

  BG_EvaluateTrajectory(&cent->currentState.pos, toTime, origin);
  BG_EvaluateTrajectory(&cent->currentState.apos, toTime, angles);

  VectorSubtract(origin, oldOrigin, deltaOrigin);
  VectorSubtract(angles, oldAngles, deltaAngles);

  VectorAdd(in, deltaOrigin, out);
  // FIXME: origin change when on a rotating object
}

/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition(centity_t *cent)
{
  vec3_t current, next;
  float f;

  // it would be an internal error to find an entity that interpolates without
  // a snapshot ahead of the current one
  if (cg.nextSnap == NULL) /*hypov8 return??, compiler nag about cg.nextSnap missing*/
  {
    CG_Error("CG_InterpoateEntityPosition: cg.nextSnap == NULL");
    return; //add hypov8 'Dereferencing null pointer'
  }

  f = cg.frameInterpolation;

  // this will linearize a sine or parabolic curve, but it is important
  // to not extrapolate player positions if more recent data is available
  BG_EvaluateTrajectory(&cent->currentState.pos, cg.snap->serverTime, current);
  BG_EvaluateTrajectory(&cent->nextState.pos, cg.nextSnap->serverTime, next);

  cent->lerpOrigin[0] = current[0] + f * (next[0] - current[0]);
  cent->lerpOrigin[1] = current[1] + f * (next[1] - current[1]);
  cent->lerpOrigin[2] = current[2] + f * (next[2] - current[2]);

  BG_EvaluateTrajectory(&cent->currentState.apos, cg.snap->serverTime, current);
  BG_EvaluateTrajectory(&cent->nextState.apos, cg.nextSnap->serverTime, next);

  cent->lerpAngles[0] = LerpAngle(current[0], next[0], f);
  cent->lerpAngles[1] = LerpAngle(current[1], next[1], f);
  cent->lerpAngles[2] = LerpAngle(current[2], next[2], f);
}

/*
===============
CG_CalcEntityLerpPositions
===============
*/
static void CG_CalcEntityLerpPositions(centity_t *cent)
{
//unlagged - projectile nudge
  // this will be set to how far forward projectiles will be extrapolated
  int timeshift = 0;
//unlagged - projectile nudge

//unlagged - smooth clients #2
  // this is done server-side now - cg_smoothClients is undefined
  // players will always be TR_INTERPOLATE

  // if this player does not want to see extrapolated players
  if (!cg_smoothClients.integer)
  {
    // make sure the clients use TR_INTERPOLATE
    if (cent->currentState.number < MAX_CLIENTS)
    {
      cent->currentState.pos.trType = TR_INTERPOLATE;
      cent->nextState.pos.trType    = TR_INTERPOLATE;
    }
  }

//unlagged - smooth clients #2

  if (cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE)
  {
    CG_InterpolateEntityPosition(cent);
    return;
  }

  // first see if we can interpolate between two snaps for
  // linear extrapolated clients
  if (cent->interpolate && cent->currentState.pos.trType == TR_LINEAR_STOP &&
      cent->currentState.number < MAX_CLIENTS)
  {
    CG_InterpolateEntityPosition(cent);
    return;
  }

  if ( cg_projectileNudge.integer &&
       !cg.demoPlayback &&
       cent->currentState.eType == ET_MISSILE&&
       !( cg.snap->ps.pm_flags & PMF_FOLLOW ) )
  {
    timeshift = cg.ping; //hypov8 note: antilag on rocket
  }

  // just use the current frame and evaluate as best we can
  BG_EvaluateTrajectory( &cent->currentState.pos,
    (cg.time + timeshift), cent->lerpOrigin );
  BG_EvaluateTrajectory( &cent->currentState.apos,
    (cg.time + timeshift), cent->lerpAngles );

  // if there's a time shift
  if ( timeshift != 0 )
  {
    trace_t tr;
    vec3_t lastOrigin;

    BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, lastOrigin );

    CG_Trace( &tr, lastOrigin, vec3_origin, vec3_origin, cent->lerpOrigin,
      cent->currentState.number, MASK_SHOT );

    // don't let the projectile go through the floor
    if ( tr.fraction < 1.0f ) {
      cent->lerpOrigin[0] = lastOrigin[0] + tr.fraction * ( cent->lerpOrigin[0] - lastOrigin[0] );
      cent->lerpOrigin[1] = lastOrigin[1] + tr.fraction * ( cent->lerpOrigin[1] - lastOrigin[1] );
      cent->lerpOrigin[2] = lastOrigin[2] + tr.fraction * ( cent->lerpOrigin[2] - lastOrigin[2] );
    }
  }
//unlagged - projectile nudge

  // adjust for riding a mover if it wasn't rolled into the predicted
  // player state
  if (cent != &cg.predictedPlayerEntity)
  {
    CG_AdjustPositionForMover(cent->lerpOrigin, cent->currentState.groundEntityNum,
    cg.snap->serverTime, cg.time, cent->lerpOrigin);
  }
}

/*
===============
CG_TeamBase
===============
*/
static void CG_TeamBase(centity_t *cent)
{
  refEntity_t model;

  //FIXME(0xA5EA): bagman !!!
  if (cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF)
  {
    // show the flag base
    Com_Memset(&model, 0, sizeof(model));

    model.reType = RT_MODEL;

    VectorCopy(cent->lerpOrigin, model.lightingOrigin);
    VectorCopy(cent->lerpOrigin, model.origin);

    AnglesToAxis(cent->currentState.angles, model.axis);

    if (cent->currentState.modelindex == TEAM_DRAGONS)
      model.hModel = cgs.media.dragonFlagBaseModel;
    else if (cent->currentState.modelindex == TEAM_NIKKIS)
      model.hModel = cgs.media.nikkiFlagBaseModel;
    else
      model.hModel = cgs.media.neutralFlagBaseModel;

    trap_R_AddRefEntityToScene(&model);
  }
#ifdef WITH_BAGMAN_MOD
  else if (cgs.gametype == GT_BAGMAN)
  {
    // show the flag base
    Com_Memset(&model, 0, sizeof(model));

    model.reType = RT_MODEL;

    VectorCopy(cent->lerpOrigin, model.lightingOrigin);
    VectorCopy(cent->lerpOrigin, model.origin);

    AnglesToAxis(cent->currentState.angles, model.axis);

    if (cent->currentState.modelindex == TEAM_DRAGONS)
      model.hModel = cgs.media.dragonsSafeBaseModel;
    else if (cent->currentState.modelindex == TEAM_NIKKIS)
      model.hModel = cgs.media.nikkisSafeBaseModel;

    trap_R_AddRefEntityToScene(&model);
  }
#endif // WITH_BAGMAN_MOD
#ifdef GT_USE_TA_TYPES
  else if (cgs.gametype == GT_OBELISK)
  {
    // show the obelisk
    Com_Memset(&model, 0, sizeof(model));
    model.reType = RT_MODEL;
    VectorCopy(cent->lerpOrigin, model.lightingOrigin);
    VectorCopy(cent->lerpOrigin, model.origin);
    AnglesToAxis(cent->currentState.angles, model.axis);

    model.hModel = cgs.media.overloadBaseModel;
    trap_R_AddRefEntityToScene(&model);
    // if hit
    if (cent->currentState.frame == 1)
    {
      // show hit model
      // modelindex2 is the health value of the obelisk
      c                   = cent->currentState.modelindex2;
      model.shaderRGBA[0] = 0xff;
      model.shaderRGBA[1] = c;
      model.shaderRGBA[2] = c;
      model.shaderRGBA[3] = 0xff;
      //
      model.hModel = cgs.media.overloadEnergyModel;
      trap_R_AddRefEntityToScene(&model);
    }
    // if respawning
    if (cent->currentState.frame == 2)
    {
      if (!cent->miscTime)
      {
        cent->miscTime = cg.time;
      }
      t = cg.time - cent->miscTime;
      //h = (cg_obeliskRespawnDelay.integer - 5) * 1000;		// 0xA5EA
      //
      if (t > h)
      {
        c = (float)(t - h) / h;
        if (c > 1)
          c = 1;
      }
      else
      {
        c = 0;
      }
      // show the lights
      AnglesToAxis(cent->currentState.angles, model.axis);
      //
      model.shaderRGBA[0] = c * 0xff;
      model.shaderRGBA[1] = c * 0xff;
      model.shaderRGBA[2] = c * 0xff;
      model.shaderRGBA[3] = c * 0xff;

      model.hModel = cgs.media.overloadLightsModel;
      trap_R_AddRefEntityToScene(&model);
      // show the target
      if (t > h)
      {
        if (!cent->muzzleFlashTime)
        {
          trap_S_StartSound(cent->lerpOrigin, ENTITYNUM_NONE, CHAN_BODY,  cgs.media.obeliskRespawnSound);
          cent->muzzleFlashTime = 1;
        }
        VectorCopy(cent->currentState.angles, angles);
        angles[YAW] += (float)16 * acos(1 - c) * 180 / M_PI;
        AnglesToAxis(angles, model.axis);

        VectorScale(model.axis[0], c, model.axis[0]);
        VectorScale(model.axis[1], c, model.axis[1]);
        VectorScale(model.axis[2], c, model.axis[2]);

        model.shaderRGBA[0] = 0xff;
        model.shaderRGBA[1] = 0xff;
        model.shaderRGBA[2] = 0xff;
        model.shaderRGBA[3] = 0xff;
        //
        model.origin[2] += 56;
        model.hModel     = cgs.media.overloadTargetModel;
        trap_R_AddRefEntityToScene(&model);
      }
      else
      {
        //FIXME: show animated smoke
      }
    }
    else
    {
      cent->miscTime        = 0;
      cent->muzzleFlashTime = 0;
      // modelindex2 is the health value of the obelisk
      c                   = cent->currentState.modelindex2;
      model.shaderRGBA[0] = 0xff;
      model.shaderRGBA[1] = c;
      model.shaderRGBA[2] = c;
      model.shaderRGBA[3] = 0xff;
      // show the lights
      model.hModel = cgs.media.overloadLightsModel;
      trap_R_AddRefEntityToScene(&model);
      // show the target
      model.origin[2] += 56;
      model.hModel     = cgs.media.overloadTargetModel;
      trap_R_AddRefEntityToScene(&model);
    }
  }
  else if (cgs.gametype == GT_HARVESTER)
  {
    // show harvester model
    Com_Memset(&model, 0, sizeof(model));
    model.reType = RT_MODEL;
    VectorCopy(cent->lerpOrigin, model.lightingOrigin);
    VectorCopy(cent->lerpOrigin, model.origin);
    AnglesToAxis(cent->currentState.angles, model.axis);

    if (cent->currentState.modelindex == TEAM_DRAGONS)
    {
      model.hModel     = cgs.media.harvesterModel;
      model.customSkin = cgs.media.harvesterRedSkin;
    }
    else if (cent->currentState.modelindex == TEAM_NIKKIS)
    {
      model.hModel     = cgs.media.harvesterModel;
      model.customSkin = cgs.media.harvesterBlueSkin;
    }
    else
    {
      model.hModel     = cgs.media.harvesterNeutralModel;
      model.customSkin = 0;
    }
    trap_R_AddRefEntityToScene(&model);
  }
#endif
}

/*
===============
CG_AddCEntity
===============
*/
static void CG_AddCEntity(centity_t *cent)
{
  // event-only entities will have been dealt with already
  if (cent->currentState.eType >= ET_EVENTS)
    return;

  // calculate the current origin
  CG_CalcEntityLerpPositions(cent);

  // add automatic effects
  CG_EntityEffects(cent);

  switch (cent->currentState.eType)
  {
  default:
    CG_Error("Bad entity type: %i\n", cent->currentState.eType);
    break;
  case ET_INVISIBLE:
  case ET_PUSH_TRIGGER:
  case ET_TELEPORT_TRIGGER:
    break;
  case ET_GENERAL:
    CG_General(cent);
    break;
  //hypov8 merge: todo
  /*case ET_CORPSE:
    CG_Corpse( cent );
  break;*/
  case ET_PLAYER:
    CG_Player(cent);
    break;
  case ET_ITEM:
    CG_Item(cent);
    break;
  case ET_MISSILE:
    CG_Missile(cent);
    break;
  case ET_MOVER:
    CG_Mover(cent);
    break;
  case ET_BEAM:
    //FIXME(0xA5EA): remove this again
    CG_Printf("ET_BEAM\n");
    CG_Beam(cent);
    break;
  case ET_PORTAL:
    CG_Portal(cent);
    break;
  case ET_SPEAKER:
    CG_Speaker(cent);
    break;
  case ET_GRAPPLE:
    CG_Grapple(cent);
    break;
  case ET_TEAM:
    CG_TeamBase(cent);
    break;
  case ET_FLAMETHROWER_CHUNK:
#ifdef HYPODEBUG
  CG_FlamerDebug(cent); //debug
#endif
    break; //hypov8 note: null. client effects generated localy
    //FIXME(0xA5EA): flame gun ET_FLAMETHROWER_CHUNK
  }
}

/*
===============
CG_AddPacketEntities
===============
*/
void CG_AddPacketEntities(void)
{
  int num;
  centity_t *cent;
  playerState_t *ps;

  // set cg.frameInterpolation
  if (cg.nextSnap)
  {
    int delta;
    delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
  if ( delta == 0 )
    cg.frameInterpolation = 0;
  else
    cg.frameInterpolation = ( float ) ( cg.time - cg.snap->serverTime ) / delta; //hypov8 note: should this be 1?
  }
  else
  {
    cg.frameInterpolation = 0;  // actually, it should never be used, because
    // no entities should be marked as interpolating
  }


  // the auto-rotating items will all have the same axis
  cg.autoAngles[0] = 0;
  cg.autoAngles[1] = 0;
  //cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
  cg.autoAngles[2] = 0;

  cg.autoAnglesFast[0] = 0;
  cg.autoAnglesFast[1] = 0;
  //cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
  cg.autoAnglesFast[2] = 0;

  AnglesToAxis(cg.autoAngles, cg.autoAxis);
  AnglesToAxis(cg.autoAnglesFast, cg.autoAxisFast);

  // generate and add the entity from the playerstate
  ps = &cg.predictedPlayerState;
  BG_PlayerStateToEntityState(ps, &cg.predictedPlayerEntity.currentState, qfalse);
  CG_AddCEntity(&cg.predictedPlayerEntity);

  // lerp the non-predicted value for lightning gun origins
  CG_CalcEntityLerpPositions(&cg_entities[cg.snap->ps.clientNum]);

  // add each entity sent over by the server
  for (num = 0; num < cg.snap->numEntities; num++)
  {
  cent = &cg_entities[cg.snap->entities[num].number];
    CG_AddCEntity(cent);
  }

  //unvan .52
    //make an attempt at drawing bounding boxes of selected entity types
  if ( cg_drawBBox.integer )
  {
    for ( unsigned int num = 0; num < (unsigned int)cg.snap->numEntities; num++ )
    {
      float         x, zd, zu;
      vec3_t        mins, maxs;
      entityState_t *es;

      cent = &cg_entities[ cg.snap->entities[ num ].number ];
      es = &cent->currentState;

      switch ( es->eType )
      {
        case entityType_t::ET_MISSILE:
        case entityType_t::ET_FLAMETHROWER_CHUNK:
          x = ( es->solid & 255 );
          zd = ( ( es->solid >> 8 ) & 255 );
          zu = ( ( es->solid >> 16 ) & 255 ) - 32;

          mins[ 0 ] = mins[ 1 ] = -x;
          maxs[ 0 ] = maxs[ 1 ] = x;
          mins[ 2 ] = -zd;
          maxs[ 2 ] = zu;

          CG_DrawBoundingBox( cg_drawBBox.integer, cent->lerpOrigin, mins, maxs );
          break;

        default:
          break;
      }
    }
  }




}
