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
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"

// for the voice chats
#include "../../ui/menudef.h"
//==========================================================================

/*
===================
CG_PlaceString
Also called by scoreboard drawing
===================
*/
const char *CG_PlaceString(int rank)
{
  static char str[64];
  char *s, *t;

  if (rank & RANK_TIED_FLAG)
  {
    rank &= ~RANK_TIED_FLAG;
    t = "Tied for ";
  }
  else
    t = "";

  if (rank == 1)
    s = S_COLOR_BLUE "1st" S_COLOR_WHITE;        // draw in blue
  else if (rank == 2)
    s = S_COLOR_RED "2nd" S_COLOR_WHITE;         // draw in red
  else if (rank == 3)
    s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;      // draw in yellow
  else if (rank == 11)
    s = "11th";
  else if (rank == 12)
    s = "12th";
  else if (rank == 13)
    s = "13th";
  else if (rank % 10 == 1)
    s = va("%ist", rank);
  else if (rank % 10 == 2)
    s = va("%ind", rank);
  else if (rank % 10 == 3)
    s = va("%ird", rank);
  else
    s = va("%ith", rank);

  Com_sprintf(str, sizeof(str), "%s%s", t, s);
  return str;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary(entityState_t *ent)
{
  int mod;
  int target, attacker;
  char *message;
  char *message2;
  const char *targetInfo;
  const char *attackerInfo;
  char targetName[32];
  char attackerName[32];
  gender_t gender;
  clientInfo_t *ci;

  target   = ent->otherEntityNum;
  attacker = ent->otherEntityNum2;
  mod      = ent->eventParm;

  if (target < 0 || target >= MAX_CLIENTS)
    CG_Error("CG_Obituary: target out of range");

  ci = &cgs.clientinfo[target];

  if (attacker < 0 || attacker >= MAX_CLIENTS)
  {
    attacker = ENTITYNUM_WORLD;
    attackerInfo = NULL;
  }
  else
    attackerInfo = CG_ConfigString(CS_PLAYERS + attacker);

  targetInfo = CG_ConfigString(CS_PLAYERS + target);
  if (!targetInfo)
    return;

  Q_strncpyz(targetName, Info_ValueForKey(targetInfo, "n"), sizeof(targetName) - 2);
  qstrcat(targetName, S_COLOR_WHITE);

  message2 = "";

  // check for single client messages
  switch (mod)
  {
  case MOD_SUICIDE:      message = "suicides";                       break;
  case MOD_FALLING:      message = "cratered";                       break;
  case MOD_CRUSH:        message = "was squished";                   break;
  case MOD_WATER:        message = "sank like a rock";               break;
  case MOD_SLIME:        message = "melted";                         break;
  case MOD_LAVA:         message = "does a back flip into the lava"; break;
  case MOD_TARGET_LASER: message = "saw the light";                  break;
  case MOD_TRIGGER_HURT: message = "was in the wrong place";         break;
  default:               message = NULL;                             break;
  }

  if (attacker == target)
  {
    gender = ci->gender;
    switch (mod)
    {
#ifdef USE_KAMIKAZE
    case MOD_KAMIKAZE:
      message = "goes out with a bang";
      break;
#endif
    case MOD_GRENADE_SPLASH:
      if (gender == GENDER_FEMALE)
        message = "tripped on her own grenade";
      else if (gender == GENDER_NEUTER)
        message = "tripped on its own grenade";
      else
        message = "tripped on his own grenade";
      break;
    case MOD_ROCKET_SPLASH:
      if (gender == GENDER_FEMALE)
        message = "blew herself up";
      else if (gender == GENDER_NEUTER)
        message = "blew itself up";
      else
        message = "blew himself up";
      break;
    default:
      if (gender == GENDER_FEMALE)
        message = "killed herself";
      else if (gender == GENDER_NEUTER)
        message = "killed itself";
      else
        message = "killed himself";
      break;
    }
  }

  if (message)
  {
    CG_Printf("%s %s.\n", targetName, message);
    return;
  }

  // check for kill messages from the current clientNum
  if (attacker == cg.snap->ps.clientNum)
  {
    char *s;
    if (cgs.gametype < GT_TEAM)
      s = va("You fragged %s\n%s place with %i", targetName, CG_PlaceString(cg.snap->ps.persistant[PERS_RANK] + 1), cg.snap->ps.persistant[PERS_SCORE]);
    else
      s = va("You fragged %s", targetName);

    (void)s; //shutup compiler

#ifdef USE_GT_SINGLEPLAYER
    if (!(cg_singlePlayerActive.integer && cg_cameraOrbit.integer))
      CG_CenterPrint(s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH);
#endif
    // print the text message as well
  }

  // check for double client messages
  if (!attackerInfo)
  {
    attacker = ENTITYNUM_WORLD;
    qstrcpy(attackerName, "noname");
  }
  else
  {
    Q_strncpyz(attackerName, Info_ValueForKey(attackerInfo, "n"), sizeof(attackerName) - 2);
    qstrcat(attackerName, S_COLOR_WHITE);
    // check for kill messages about the current clientNum
    if (target == cg.snap->ps.clientNum)
      Q_strncpyz(cg.killerName, attackerName, sizeof(cg.killerName));
  }

  if (attacker != ENTITYNUM_WORLD)
  {
    switch (mod)
    {
    case MOD_GRAPPLE:        message = "was caught by";                                   break;
    case MOD_CROWBAR:        message = "was crowbared by";                                break;
    case MOD_PISTOL:         message = "catched";                                         break;
    case MOD_MACHINEGUN:     message = "was machinegunned by";                            break;
    case MOD_SHOTGUN:        message = "was gunned down by";                              break;
    case MOD_GRENADE:        message  = "ate";             message2 = "'s grenade";       break;
    case MOD_GRENADE_SPLASH: message  = "was shredded by"; message2 = "'s shrapnel";      break;
    case MOD_ROCKET:         message  = "ate";             message2 = "'s rocket";        break;
    case MOD_ROCKET_SPLASH:  message  = "almost dodged";   message2 = "'s rocket";        break;
    case MOD_HMG:            message = "was hmggunned by";                                break;
    case MOD_FLAMEGUN:       message = "was flamed by";                                   break;
    case MOD_TELEFRAG:       message  = "tried to invade"; message2 = "'s personal space";break;
    default:                 message = "was killed by";                                   break;
    }

    if (message)
    {
      CG_Printf("%s %s %s%s\n", targetName, message, attackerName, message2);
      return;
    }
  }

  // we don't know what it was
  CG_Printf("%s died.\n", targetName);
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem(centity_t *cent)
{
  clientInfo_t *ci;
  int itemNum, clientNum;
  gitem_t *item;
  entityState_t *es;

  es = &cent->currentState;

  itemNum = (es->event & ~EV_EVENT_BITS) - EV_USE_ITEM0;
  if (itemNum < 0 || itemNum > HI_NUM_HOLDABLE)
    itemNum = 0;

  // print a message if the local player
  if (es->number == cg.snap->ps.clientNum)
  {
    if (!itemNum)
      CG_CenterPrint("No item to use", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH);
    else
    {
      item = BG_FindItemForHoldable((holdable_t)itemNum);
      CG_CenterPrint(va("Use %s", item->pickup_name), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH);
    }
  }

  switch (itemNum)
  {
  default:
  case HI_NONE:
    trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.useNothingSound);
    break;

  case HI_TELEPORTER:
    break;

  case HI_MEDKIT:
    clientNum = cent->currentState.clientNum;
    if (clientNum >= 0 && clientNum < MAX_CLIENTS)
    {
      ci = &cgs.clientinfo[clientNum];
      ci->medkitUsageTime = cg.time;
    }
    trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.medkitSound);
    break;

#ifdef USE_KAMIKAZE
  case HI_KAMIKAZE:
    break;
#endif
#if 0 //def MISSIONPACK
  case HI_PORTAL:
    break;
  case HI_INVULNERABILITY:
    trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.useInvulnerabilitySound);
    break;
#endif
  }
}



/*
================
CG_ItemPickup
A new item was picked up this frame
================
*/
static void CG_ItemPickup(int itemNum)
{
  cg.itemPickup          = itemNum;
  cg.itemPickupTime      = cg.time;
  cg.itemPickupBlendTime = cg.time;
}


/*
================
CG_AutoSwitch
Switch to new weapon
================
*/
static void CG_AutoSwitch(int itemNum)
{
  // see if it should be the grabbed weapon
  if (cg_autoswitch.integer)
  {
    int curWep = cg.predictedPlayerState.weapon;
    //( bg_itemlist[ itemNum ].giTag == WP_PISTOL )

    if ( cg_autoswitch.integer == 2 )
    {
      if ( curWep > WP_PISTOL )
        return;
    }
    else if ( cg_autoswitch.integer == 3 )
    {	//check if we have major weps. if we are holding a minor wep, we must have selected it.
      if ( curWep <= WP_PISTOL || curWep > WP_LAST )
      {
        int i;
        for ( i = ( WP_FIRST + 1 ); i <= WP_LAST; i++ )
        {
          if ( cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) )
            return;
        }
      }
      else
        return;
    }

    cg.weaponSelectTime = cg.time;
    cg.weaponSelect = bg_itemlist[ itemNum ].giTag;
  }
}

static void CG_HMInfoMsg(centity_t *cent, int eventParm)
{
  if ( eventParm >= 0 && eventParm <= 3 )
  {
    switch ( eventParm )
    {
    case 2:
      CG_CenterPrint("Disabling weapons", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH);
      break;
    case 1:
      CG_CenterPrint("Changing weapons", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH);
      break;
    case 0:
      CG_CenterPrint("Fight!!!", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH);
      break;
    }
  }

  cg.weapontimer_hm = eventParm;
  //trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );  /* 0xA5EA */
  //FIXME(0xA5EA): enable this ?
}

/*
================
CG_PainEvent
Also called by playerstate transition
================
*/
void CG_PainEvent(centity_t *cent, int health)
{
  char *snd;

  // don't do more than two pain sounds a second
  if (cg.time - cent->pe.painTime < 500)
    return;
#if 0 // hypov8 test lower health warnings sounds
  if (health < 25)
    snd = "*pain25_1.ogg"; // hypov8 this should be changed, so only low health makes different noise 10, 20, 30?
  else if (health < 50)
    snd = "*pain50_1.ogg";
  else if (health < 75)
    snd = "*pain75_1.ogg";
  else
    snd = "*pain100_1.ogg";
#else
  if (health < 10)
    snd = "*pain25_1.ogg";
  else if (health < 20)
    snd = "*pain50_1.ogg";
  else if (health < 30)
    snd = "*pain75_1.ogg";
  else
    snd = "*pain100_1.ogg";
#endif

  trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE,  CG_CustomSound(cent->currentState.number, snd)); //"*pain25_1.ogg" etc...

  // save pain time for programitic twitch animation
  cent->pe.painTime = cg.time;
  cent->pe.painDirection ^= 1;
}

/*
================
CG_PainEventSelf
================
*/
void CG_PainEventSelf()
{
  // don't do more than two pain sounds a second
  if (cg.time - cg.predictedPlayerEntity.pe.painTime < 500)
    return;

  trap_S_StartSound(NULL,cg.snap->ps.clientNum, CHAN_LOCAL,cgs.media.flameScreenSound );

  // save pain time for programitic twitch animation
  if ( cg.renderingThirdPerson )
  {
    cg.predictedPlayerEntity.pe.painTime =  cg.time;
    cg.predictedPlayerEntity.pe.painDirection ^= 1;
  }
}



#if 1 //daemon .50

/*
=========================
CG_OnPlayerWeaponChange

Called on weapon change
=========================
*/
void CG_OnPlayerWeaponChange()
{
  /*
  playerState_t *ps = &cg.snap->ps;

  // Change the HUD to match the weapon. Close the old hud first
  Rocket_ShowHud( ps->weapon );

  // Rebuild weapon lists if UI is in focus.
  if ( trap_Key_GetCatcher() == KEYCATCH_UI && ps->persistant[ PERS_TEAM ] == TEAM_HUMANS )
  {
    CG_Rocket_BuildArmourySellList( "default" );
    CG_Rocket_BuildArmouryBuyList( "default" );
  }

  cg.weaponOffsetsFilter.Reset( );*/
	cg.weaponSelect = cg.snap->ps.weapon;

  //cg.predictedPlayerEntity.pe.weapon.animationNumber = -1; //force weapon lerpframe recalculation
}
#endif

/*
==============
CG_FootstepForEventParm
==============
*/
#if 0
static int CG_FootstepForEventParm(int eventparm)
{
  if (eventparm & SURF_NOSTEPS)
    return FOOTSTEP_NORMAL;
  //FIXME(0xA5EA): FOOTSTEP_NORMAL ??
  if (eventparm & SURF_METALSTEPS)
    return FOOTSTEP_METAL;
  else if (eventparm & SURF_GRAVEL)
    return FOOTSTEP_GRAVEL;
  else if (eventparm & SURF_METALLIGHT)
    return FOOTSTEP_METALLIGHT;
  else if (eventparm & SURF_RUG)
    return FOOTSTEP_RUG;
  else if (eventparm & SURF_SNOW)
    return FOOTSTEP_SNOW;
  else if (eventparm & SURF_WOOD)
    return FOOTSTEP_WOOD;
  else if (eventparm & SURF_MARBLE)
    return FOOTSTEP_MARBLE;
  else if (eventparm & SURF_TIN)
    return FOOTSTEP_TIN;
  else if (eventparm & SURF_GRASS)
    return FOOTSTEP_CRASS;
  else if (eventparm & SURF_GLASS)
    return FOOTSTEP_GLASS;
  else if (eventparm & SURF_LADDER)
    return FOOTSTEP_LADDER;
  else
    return FOOTSTEP_NORMAL;
}
#endif

#define DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x "\n");}

//unlagged - attack prediction #2
#define ANTILAG_PREDICTED(es) (es->otherEntityNum == cg.predictedPlayerState.clientNum && \
              cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 4))
//unlagged - attack prediction #2


/*
==============
CG_EntityEvent
An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
void CG_EntityEvent(centity_t *cent, vec3_t position)
{
  entityState_t *es;
  int event;
  //int land_type = FOOTSTEP_NORMAL;         /* 0xA5EA */
  //	int random;
  vec3_t dir;
  const char *s;
  int clientNum;
  //clientInfo_t *ci;

  //cent->currentState.eventParm
  es = &cent->currentState;
  event = es->event & ~EV_EVENT_BITS;

  if (cg_debugEvents.integer)
    CG_Printf("ent:%3i  event:%3i ", es->number, event);

  if (!event)
  {
    DEBUGNAME("ZEROEVENT");
    return;
  }

  clientNum = es->clientNum;

  if (clientNum < 0 || clientNum >= MAX_CLIENTS)
    clientNum = 0;

  //ci = &cgs.clientinfo[clientNum];

  switch (event)
  {
  // movement generated events
  case EV_FOOTSTEP:
    DEBUGNAME("EV_FOOTSTEP");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_NORMAL][rand() & 3]);       /* 0xA5EA ci->footsteps */
    break;
  case EV_FOOTSTEP_METAL:
    DEBUGNAME("EV_FOOTSTEP_METAL");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_METAL][rand() & 3]);
    break;
  case EV_FOOTSPLASH:
    DEBUGNAME("EV_FOOTSPLASH");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_SPLASH][rand() & 3]);
    break;
  case EV_FOOTWADE:
    DEBUGNAME("EV_FOOTWADE");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_SPLASH][rand() & 3]);
    break;
  case EV_SWIM:
    DEBUGNAME("EV_SWIM");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY,  cgs.media.footsteps[FOOTSTEP_SPLASH][rand() & 3]);
    break;

  case EV_FOOTSTEP_GRAVEL:
    DEBUGNAME("EV_FOOTSTEP_GRAVEL");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_GRAVEL][rand() & 3]);
    break;

  case EV_FOOTSTEP_METALLIGHT:
    DEBUGNAME("EV_FOOTSTEP_METALLIGHT");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_METALLIGHT][rand() & 3]);
    break;

  case EV_FOOTSTEP_RUG:
    DEBUGNAME("EV_FOOTSTEP_RUG");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_RUG][rand() & 3]);
    break;

  case EV_FOOTSTEP_SNOW:
    DEBUGNAME("EV_FOOTSTEP_SNOW");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_SNOW][rand() & 3]);
    break;

  case EV_FOOTSTEP_WOOD:
    DEBUGNAME("EV_FOOTSTEP_WOOD");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_WOOD][rand() & 3]);
    break;

  case EV_FOOTSTEP_CRASS:
    DEBUGNAME("EV_FOOTSTEP_CRASS");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_CRASS][rand() & 3]);
    break;

  case EV_FOOTSTEP_MARBLE:
    DEBUGNAME("EV_FOOTSTEP_MARBLE");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_MARBLE][rand() & 3]);
    break;

  case EV_FOOTSTEP_TIN:
    DEBUGNAME("EV_FOOTSTEP_TIN");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_TIN][rand() & 3]);
    break;

  case EV_FOOTSTEP_LADDER:
    DEBUGNAME("EV_FOOTSTEP_LADDER");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_LADDER][rand() & 3]);
    break;

  case EV_FOOTSTEP_GLASS:
    DEBUGNAME("EV_FOOTSTEP_GLASS");
    if (cg_footsteps.integer)
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_GLASS][rand() & 3]);
    break;

  case EV_FALL_SHORT:
    DEBUGNAME("EV_FALL_SHORT");
  trap_S_StartSound(NULL, es->number, CHAN_AUTO, CG_CustomSound(es->number, "*fall1.ogg"));/* 0xA5EA, ci->footsteps*/
    if (clientNum == cg.predictedPlayerState.clientNum)
    {
      // smooth landing z changes
      cg.landChange = -8;
      cg.landTime   = cg.time;
    }
    break;
  case EV_FALL_MEDIUM:
    DEBUGNAME("EV_FALL_MEDIUM");
    //FIXME(0xA5EA): pain here ?
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, CG_CustomSound(es->number, "*fall2.ogg"));
    if (clientNum == cg.predictedPlayerState.clientNum)
    {
      // smooth landing z changes
      cg.landChange = -16;
      cg.landTime   = cg.time;
    }
    break;
  case EV_FALL_FAR:
    DEBUGNAME("EV_FALL_FAR");
    //FIXME(0xA5EA): pain here ?
    trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "*pain100_1.ogg"));
    cent->pe.painTime = cg.time;                                                                                                                                                                                                                                                                                // don't play a pain sound right after this
    if (clientNum == cg.predictedPlayerState.clientNum)
    {
      // smooth landing z changes
      cg.landChange = -24;
      cg.landTime   = cg.time;
    }
    break;

  case EV_STEP_4:
  case EV_STEP_8:
  case EV_STEP_12:
  case EV_STEP_16:
  case EV_STEPDN_4:
  case EV_STEPDN_8:
  case EV_STEPDN_12:
  case EV_STEPDN_16:// smooth out step up transitions
    DEBUGNAME("EV_STEP");
    {
      float oldStep;
      int delta;
      int step;

      if (clientNum != cg.predictedPlayerState.clientNum)
        break;

      // if we are interpolating, we don't need to smooth steps
      if (cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
          cg_nopredict.integer || cg.pmoveParams.synchronous)
        break;

      // check for stepping up before a previous step is completed
      delta = cg.time - cg.stepTime;
      if (delta < STEP_TIME)
        oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
      else
        oldStep = 0;

      // add this amount
      if (event >= EV_STEPDN_4)
      {
        step = 4 * (event - EV_STEPDN_4 + 1);
        cg.stepChange = oldStep - step;
      }
      else
      {
        step  = 4 * (event - EV_STEP_4 + 1);
        cg.stepChange = oldStep + step;
      }

      if (cg.stepChange > MAX_STEP_CHANGE)
        cg.stepChange = MAX_STEP_CHANGE;
      else if (cg.stepChange < -MAX_STEP_CHANGE)
        cg.stepChange = -MAX_STEP_CHANGE;

      cg.stepTime = cg.time;
      break;
    }

  case EV_JUMP_PAD:
    DEBUGNAME("EV_JUMP_PAD");
    {
      localEntity_t *smoke;
      vec3_t up = {0, 0, 1};
      smoke = CG_SmokePuff(cent->lerpOrigin, up, 32, 1, 1, 1, 0.33f, 1000, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.smokePuffShader);
      (void)smoke; // shutup compiler
    }

    // boing sound at origin, jump sound on player
    trap_S_StartSound(cent->lerpOrigin, -1, CHAN_VOICE, cgs.media.jumpPadSound);
    trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "*jump1.ogg"));
    break;

  case EV_JUMP:
    DEBUGNAME("EV_JUMP");
  trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, va("*jump%i.ogg", rand() % 3 + 1))); // event - EV_DEATH1 + 1
    break;
  case EV_TAUNT:
    DEBUGNAME("EV_TAUNT");
    {
  //hypov8 should be fixed now, per model sounds
#if 0 //oldtaunts //note hypov8 need to match taunts to player models. girls dont fart :)
      shortbytes_t index;
      index.s = es->eventParm;
      if (index.b[0] == 1)
      {
        if (index.b[1] > numCustomTaunts2)
        {
          CG_Printf("error: indexoverflow arrayidx:%i  tauntidx:%i ", index.b[0], index.b[1]);
          break;
        }
        trap_S_StartSound(NULL, es->number, CHAN_VOICE, trap_S_RegisterSound(bg_customTauntNames2[index.b[1]], qfalse));
      }
      else
      {
        if (index.b[1] > numCustomTaunts1)
        {
          CG_Printf("error: indexoverflow arrayidx:%i  tauntidx:%i ", index.b[0], index.b[1]);
          break;
        }
        trap_S_StartSound(NULL, es->number, CHAN_VOICE, trap_S_RegisterSound(bg_customTauntNames1[index.b[1]], qfalse));
      }
#else //hypo 12 taunts per model
    trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, va("*taunt%i.ogg", rand() % 12 + 1)));
#endif
    }
    break;
  case EV_TAUNT_YES:
    DEBUGNAME("EV_TAUNT_YES");
    CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_YES);
    break;
  case EV_TAUNT_NO:
    DEBUGNAME("EV_TAUNT_NO");
    CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_NO);
    break;
  case EV_TAUNT_FOLLOWME:
    DEBUGNAME("EV_TAUNT_FOLLOWME");
    CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_FOLLOWME);
    break;
  case EV_TAUNT_GETFLAG:
    DEBUGNAME("EV_TAUNT_GETFLAG");
    CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONGETFLAG);
    break;
  case EV_TAUNT_GUARDBASE:
    DEBUGNAME("EV_TAUNT_GUARDBASE");
    CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONDEFENSE);
    break;
  case EV_TAUNT_PATROL:
    DEBUGNAME("EV_TAUNT_PATROL");
    CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONPATROL);
    break;
  case EV_WATER_TOUCH:
    DEBUGNAME("EV_WATER_TOUCH");
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.watrInSound);
    break;
  case EV_WATER_LEAVE:
    DEBUGNAME("EV_WATER_LEAVE");
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound);
    break;
  case EV_WATER_UNDER:
    DEBUGNAME("EV_WATER_UNDER");
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound);
    break;
  case EV_WATER_CLEAR:
    DEBUGNAME("EV_WATER_CLEAR");
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, CG_CustomSound(es->number, "*gasp.ogg"));
    break;

  case EV_ITEM_PICKUP:
    DEBUGNAME("EV_ITEM_PICKUP");
    {
      gitem_t *item;
      int index;

      index = es->eventParm;      // player predicted

      if (index < 1 || index >= bg_numItems)
        break;

      item = &bg_itemlist[index];

      // powerups and team items will have a separate global sound, this one
      // will be played at prediction time
      if (item->giType == IT_POWERUP || item->giType == IT_TEAM)
      {
        trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.n_healthSound);
      }
      else if (item->giType == IT_PERSISTANT_POWERUP)
      {
        /* 0xA5EA FIXME:
     //   #ifdef MISSIONPACK
            switch (item->giTag )
      {
                case PW_SCOUT:
                    trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.scoutSound );
                break;
                case PW_GUARD:
                    trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.guardSound );
                break;
                case PW_DOUBLER:
                    trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.doublerSound );
                break;
                case PW_AMMOREGEN:
                    trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.ammoregenSound );
                break;
            }
        #endif
        */
      }
    else if (item->giType == IT_WEAPON)	//hypov8 todo: differnt sound?
    {
      if (es->number == cg.snap->ps.clientNum)
        CG_AutoSwitch(index);
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, trap_S_RegisterSound(item->pickup_sound, qfalse));
    }
    else //all other sounds
    {
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, trap_S_RegisterSound(item->pickup_sound, qfalse));
    }

      // show icon and name on status bar. &cg_autoswitch
      if (es->number == cg.snap->ps.clientNum)
      CG_ItemPickup(index);
    }
    break;

  case EV_GLOBAL_ITEM_PICKUP:
    DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
    {
      gitem_t *item;
      int index;

      index = es->eventParm;      // player predicted

      if (index < 1 || index >= bg_numItems)
        break;

      item = &bg_itemlist[index];

      // powerup pickups are global
      if (item->pickup_sound)
        trap_S_StartSound(NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound(item->pickup_sound, qfalse));


      // show icon and name on status bar
    if ( es->number == cg.snap->ps.clientNum )
    {
      if (item->giType == IT_WEAPON)
        CG_AutoSwitch(index);
      CG_ItemPickup(index);
    }
    }
    break;

  // weapon events
  case EV_NOAMMO:
    DEBUGNAME("EV_NOAMMO");
    if (clientNum == cg.predictedPlayerState.clientNum) //local client only
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound);
    break;

  case EV_NOROUNDS:
    DEBUGNAME("EV_ROUNDS");
    if (clientNum == cg.predictedPlayerState.clientNum) //local client only
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound);
    break;

  case EV_HITMEN_INFOMSG:
  DEBUGNAME("EV_HITMEN_INFOMSG");
  CG_HMInfoMsg(cent, es->eventParm);
  break;
  case EV_CHANGE_WEAPON_DROP:
    DEBUGNAME("EV_CHANGE_WEAPON_DROP");
  if ( clientNum == cg.predictedPlayerState.clientNum ){ //local client only
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.useNothingSound); //stop old sounds
    CG_ResetWeaponSwitch(cent, es->eventParm);
  }
    break;
  case EV_CHANGE_WEAPON_RAISE:
  DEBUGNAME("EV_CHANGE_WEAPON_RAISE");
  if (clientNum == cg.predictedPlayerState.clientNum) //local client only
    CG_ResetWeaponSwitch(cent, es->eventParm);
  break;
  case EV_FIRE_WEAPON:
    DEBUGNAME("EV_FIRE_WEAPON");
  CG_FireWeapon(cent, qfalse);
    break;
  case EV_FIRE_SHOTGUN:
  DEBUGNAME("EV_FIRE_SHOTGUN");
  CG_FireWeapon(cent, qfalse);
  break;
  case EV_FIRE_SPISTOL:
  DEBUGNAME("EV_FIRE_SPISTOL");
  CG_FireWeapon(cent, qtrue);
  break;
  /*case EV_FIRE_HMG_SHOT: //hypov8 todo: cleanup. not used anymore
  DEBUGNAME("EV_FIRE_HMG_SHOT");
  CG_FireWeapon(cent, qfalse);
  break;*/
  /*case EV_FIRE_COOLDOWN:
    //todo
    break;*/
  case EV_RELOAD_WEAPON:
  DEBUGNAME("EV_RELOAD_WEAPON");
  CG_ReloadWeapon(cent);
  break;
  case EV_RELOAD_SHOTGUN:
  DEBUGNAME("EV_RELOAD_SHOTGUN");
    CG_ReloadWeapon(cent);
    break;
  case EV_USE_ITEM0:
    DEBUGNAME("EV_USE_ITEM0");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM1:
    DEBUGNAME("EV_USE_ITEM1");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM2:
    DEBUGNAME("EV_USE_ITEM2");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM3:
    DEBUGNAME("EV_USE_ITEM3");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM4:
    DEBUGNAME("EV_USE_ITEM4");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM5:
    DEBUGNAME("EV_USE_ITEM5");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM6:
    DEBUGNAME("EV_USE_ITEM6");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM7:
    DEBUGNAME("EV_USE_ITEM7");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM8:
    DEBUGNAME("EV_USE_ITEM8");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM9:
    DEBUGNAME("EV_USE_ITEM9");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM10:
    DEBUGNAME("EV_USE_ITEM10");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM11:
    DEBUGNAME("EV_USE_ITEM11");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM12:
    DEBUGNAME("EV_USE_ITEM12");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM13:
    DEBUGNAME("EV_USE_ITEM13");
    CG_UseItem(cent);
    break;
  case EV_USE_ITEM14:
    DEBUGNAME("EV_USE_ITEM14");
    CG_UseItem(cent);
    break;

  case EV_PLAYER_TELEPORT_IN:
    DEBUGNAME("EV_PLAYER_TELEPORT_IN");
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.teleInSound);
    CG_SpawnEffect(position);
    break;

  case EV_PLAYER_TELEPORT_OUT:
    DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound);
    CG_SpawnEffect(position);
    break;

  case EV_ITEM_POP:
    DEBUGNAME("EV_ITEM_POP");
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.respawnSound);
    break;

  case EV_ITEM_RESPAWN:
    DEBUGNAME("EV_ITEM_RESPAWN");
    cent->miscTime = cg.time;                                                            // scale up from this
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.respawnSound);
    break;

  case EV_GRENADE_BOUNCE:
    DEBUGNAME("EV_GRENADE_BOUNCE");
    switch (rand() % 3)
    {
    case 0:
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.hgrenb1aSound);
      break;
    case 1:
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.hgrenb2aSound);
      break;
    case 2:
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.hgrenb3aSound);
      break;
    }
    break;

  case EV_GRENADE_BOUNCE_METAL:
    DEBUGNAME("EV_GRENADE_BOUNCE_METAL");
    switch (rand() % 2)
    {
    case 0:
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.hgrenbmetal1Sound);
      break;
    case 1:
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.hgrenbmetal2Sound);
      break;
    }
    break;

  case EV_GRENADE_BOUNCE_WOOD:
    DEBUGNAME("EV_GRENADE_BOUNCE_WOOD");
    switch (rand() % 2)
    {
    case 0:
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.hgrenbwood1Sound);
      break;
    case 1:
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.hgrenbwood2Sound);
      break;
    }
    break;

#ifdef GT_USE_TA_TYPES
  case EV_PROXIMITY_MINE_STICK:
    DEBUGNAME("EV_PROXIMITY_MINE_STICK");
    if (es->eventParm & SURF_FLESH)
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.wstbimplSound);
    else if (es->eventParm & SURF_METALSTEPS)
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.wstbimpmSound);
    else
      trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.wstbimpdSound);
    break;

  case EV_PROXIMITY_MINE_TRIGGER:
    DEBUGNAME("EV_PROXIMITY_MINE_TRIGGER");
    trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.wstbactvSound);
    break;
  case EV_JUICED:
    DEBUGNAME("EV_JUICED");
    //CG_InvulnerabilityJuiced(cent->lerpOrigin);
    break;
  case EV_OBELISKEXPLODE:
    DEBUGNAME("EV_OBELISKEXPLODE");
    CG_ObeliskExplode(cent->lerpOrigin, es->eventParm);
    break;
  case EV_OBELISKPAIN:
    DEBUGNAME("EV_OBELISKPAIN");
    CG_ObeliskPain(cent->lerpOrigin);
    break;
  case EV_INVUL_IMPACT:
    DEBUGNAME("EV_INVUL_IMPACT");
    CG_InvulnerabilityImpact(cent->lerpOrigin, cent->currentState.angles);
    break;
  case EV_LIGHTNINGBOLT:
    DEBUGNAME("EV_LIGHTNINGBOLT");
    CG_LightningBoltBeam(es->origin2, es->pos.trBase);
    break;
#endif
#ifdef USE_KAMIKAZE
  case EV_KAMIKAZE:
    DEBUGNAME("EV_KAMIKAZE");
    CG_KamikazeEffect(cent->lerpOrigin);
    break;
#endif
  case EV_SCOREPLUM: //hypov8 todo: unused. cleanup
    DEBUGNAME("EV_SCOREPLUM");
    CG_ScorePlum(cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time);
    break;

  case EV_MISSILE_HIT:
    DEBUGNAME("EV_MISSILE_HIT");
    ByteToDir(es->eventParm, dir);
    CG_MissileHitPlayer(es->weapon, position, dir, es->otherEntityNum);
    break;

  case EV_MISSILE_MISS:
    DEBUGNAME("EV_MISSILE_MISS");
    ByteToDir(es->eventParm, dir);
    CG_MissileHitWall(es->weapon, 0, position, dir, IMPACTSOUND_DEFAULT);
    break;

  case EV_MISSILE_MISS_METAL:
    DEBUGNAME("EV_MISSILE_MISS_METAL");
    ByteToDir(es->eventParm, dir);
    CG_MissileHitWall(es->weapon, 0, position, dir, IMPACTSOUND_METAL);
    break;

  case EV_BULLET_HIT_WALL:
    DEBUGNAME("EV_BULLET_HIT_WALL");
    ByteToDir(es->eventParm, dir);
  CG_Bullet(es->weapon, es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD);
  break;

  case EV_BULLET_HIT_METAL:
    DEBUGNAME("EV_BULLET_HIT_METAL");
    ByteToDir(es->eventParm, dir);
    CG_Bullet(es->weapon, es->pos.trBase, es->otherEntityNum, dir, IMPACTSOUND_METAL, ENTITYNUM_WORLD);
    break;

  case EV_BULLET_HIT_WOOD:
    DEBUGNAME("EV_BULLET_HIT_WOOD");
    ByteToDir(es->eventParm, dir);
    CG_Bullet(es->weapon, es->pos.trBase, es->otherEntityNum, dir, IMPACTSOUND_WOOD, es->eventParm);
    break;

  case EV_BULLET_HIT_EARTH:
    DEBUGNAME("EV_BULLET_HIT_EARTH");
    ByteToDir(es->eventParm, dir);
    CG_Bullet(es->weapon, es->pos.trBase, es->otherEntityNum, dir, IMPACTSOUND_EARTH, es->eventParm);
    break;

  case EV_BULLET_HIT_SNOW:
    DEBUGNAME("EV_BULLET_HIT_SNOW");
    ByteToDir(es->eventParm, dir);
    CG_Bullet(es->weapon, es->pos.trBase, es->otherEntityNum, dir, IMPACTSOUND_SNOW, es->eventParm);
    break;

  case EV_BULLET_HIT_FLESH:
    DEBUGNAME("EV_BULLET_HIT_FLESH");
    ByteToDir(es->eventParm, dir);
  CG_Bullet(es->weapon, es->pos.trBase, es->otherEntityNum, dir, IMPACTSOUND_FLESH, es->eventParm);
    break;

  case EV_BULLET_HIT_GLASS:
    DEBUGNAME("EV_BULLET_HIT_GLASS");
    ByteToDir(es->eventParm, dir);
    CG_Bullet(es->weapon, es->pos.trBase, es->otherEntityNum, dir, IMPACTSOUND_GLASS, es->eventParm);
    break;

  case EV_GENERAL_SOUND:
    DEBUGNAME("EV_GENERAL_SOUND");
    if (cgs.gameSounds[es->eventParm])
      trap_S_StartSound(NULL, es->number, CHAN_VOICE, cgs.gameSounds[es->eventParm]);
    else
    {
      s = CG_ConfigString(CS_SOUNDS + es->eventParm);
      trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, s));
    }
    break;
  /*
  //hypov8 add sound volume?
  case EV_GENERAL_SOUND_VOLUME:
    int             volume = es->onFireStart;
    DEBUGNAME("EV_GENERAL_SOUND_VOLUME");

    if (cgs.gameSounds[es->eventParm])
      trap_S_StartSound(NULL, es->number, CHAN_VOICE, cgs.gameSounds[es->eventParm]);
    else
    {
      s = CG_ConfigString(CS_SOUNDS + es->eventParm);
      trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, s));
    }
    break;
    */
  case EV_GLOBAL_SOUND:            // play from the player's head so it never diminishes
    DEBUGNAME("EV_GLOBAL_SOUND");
    if (cgs.gameSounds[es->eventParm])
      trap_S_StartSound(NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[es->eventParm]);
    else
    {
      s = CG_ConfigString(CS_SOUNDS + es->eventParm);
      trap_S_StartSound(NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound(es->number, s));
    }
    break;

  case EV_GLOBAL_TEAM_SOUND:       // play from the player's head so it never diminishes
  {
    DEBUGNAME("EV_GLOBAL_TEAM_SOUND");
    switch (es->eventParm)
    {
    case GTS_DRAGON_CAPTURE: // CTF: Dragon team captured the Nikki flag, 1FCTF: Dragon team captured the neutral flag
    if (cgs.clientinfo[cg.clientNum].team == TEAM_DRAGONS)
      CG_AddBufferedSound(cgs.media.captureYourTeamSound);
    else
      CG_AddBufferedSound(cgs.media.captureOpponentSound);
    break;

    case GTS_NIKKI_CAPTURE:  // CTF: Nikki team captured the Dragon flag, 1FCTF: Nikki team captured the neutral flag
    if (cgs.clientinfo[cg.clientNum].team == TEAM_NIKKIS)
      CG_AddBufferedSound(cgs.media.captureYourTeamSound);
    else
      CG_AddBufferedSound(cgs.media.captureOpponentSound);
    break;

    case GTS_DRAGON_RETURN:  // CTF: Nikki flag returned, 1FCTF: never used
    if (cgs.clientinfo[cg.clientNum].team == TEAM_DRAGONS)
      CG_AddBufferedSound(cgs.media.returnYourTeamSound);
    else
      CG_AddBufferedSound(cgs.media.returnOpponentSound);
    //
    CG_AddBufferedSound(cgs.media.blueFlagReturnedSound);
    break;

    case GTS_NIKKI_RETURN:   // CTF Dragon flag returned, 1FCTF: neutral flag returned
    if (cgs.clientinfo[cg.clientNum].team == TEAM_NIKKIS)
      CG_AddBufferedSound(cgs.media.returnYourTeamSound);
    else
      CG_AddBufferedSound(cgs.media.returnOpponentSound);
    CG_AddBufferedSound(cgs.media.redFlagReturnedSound);
    break;

    case GTS_DRAGON_TAKEN:   // CTF: Dragon team took Nikki flag, 1FCTF: Nikki team took the neutral flag
    // if this player picked up the flag then a sound is played in CG_CheckLocalSounds
    if (cg.snap->ps.powerups[PW_NIKKIFLAG] || cg.snap->ps.powerups[PW_NEUTRALFLAG])
    {
      ;//
    }
    else
    {
      if (cgs.clientinfo[cg.clientNum].team == TEAM_NIKKIS)
      {
        if (cgs.gametype == GT_1FCTF)
          CG_AddBufferedSound(cgs.media.yourTeamTookTheFlagSound);
        else
          CG_AddBufferedSound(cgs.media.enemyTookYourFlagSound);
      }
      else if (cgs.clientinfo[cg.clientNum].team == TEAM_DRAGONS)
      {

        if (cgs.gametype == GT_1FCTF)
          CG_AddBufferedSound(cgs.media.enemyTookTheFlagSound);
        //else if (cgs.gametype == GT_BAGMAN)
        //	trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.yourTeamTookEnemyBagSound); /hypov8 todo: why was this here?
          //CG_AddBufferedSound(cgs.media.yourTeamTookEnemyBagSound);
        else
        CG_AddBufferedSound(cgs.media.yourTeamTookEnemyFlagSound);
      }
    }
    break;
    case GTS_NIKKI_TAKEN: // CTF: Nikki team took the Dragon flag, 1FCTF Dragon team took the neutral flag
    // if this player picked up the flag then a sound is played in CG_CheckLocalSounds
    if (cg.snap->ps.powerups[PW_DRAGONFLAG] || cg.snap->ps.powerups[PW_NEUTRALFLAG])
    {
      ;//
    }
    else
    {
      if (cgs.clientinfo[cg.clientNum].team == TEAM_DRAGONS)
      {
        if (cgs.gametype == GT_1FCTF)
          CG_AddBufferedSound(cgs.media.yourTeamTookTheFlagSound);
        else
          CG_AddBufferedSound(cgs.media.enemyTookYourFlagSound);
      }
      else if (cgs.clientinfo[cg.clientNum].team == TEAM_NIKKIS)
      {

        if (cgs.gametype == GT_1FCTF)
          CG_AddBufferedSound(cgs.media.enemyTookTheFlagSound);
        //else if (cgs.gametype == GT_BAGMAN)
        //	trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.yourTeamTookEnemyBagSound); //hypov8 todo: why was this here??
          //CG_AddBufferedSound(cgs.media.yourTeamTookEnemyBagSound);
        else
          CG_AddBufferedSound(cgs.media.yourTeamTookEnemyFlagSound);
      }
    }
    break;
    case GTS_DRAGONOBELISK_ATTACKED: // Overload: Dragon obelisk is being attacked
    if (cgs.clientinfo[cg.clientNum].team == TEAM_DRAGONS)
    {
      CG_AddBufferedSound(cgs.media.yourBaseIsUnderAttackSound);
    }
    break;
    case GTS_NIKKIOBELISK_ATTACKED:  // Overload: Nikki Flag obelisk is being attacked
    if (cgs.clientinfo[cg.clientNum].team == TEAM_NIKKIS)
    {
      CG_AddBufferedSound(cgs.media.yourBaseIsUnderAttackSound);
    }
    break;
    case GTS_DRAGONTEAM_SCORED:
    CG_AddBufferedSound(cgs.media.redScoredSound);
    break;
    case GTS_NIKKITEAM_SCORED:
    CG_AddBufferedSound(cgs.media.blueScoredSound);
    break;
    case GTS_DRAGONTEAM_TOOK_LEAD:
    CG_AddBufferedSound(cgs.media.redLeadsSound);
    break;
    case GTS_NIKKITEAM_TOOK_LEAD:
    CG_AddBufferedSound(cgs.media.blueLeadsSound);
    break;
    case GTS_TEAMS_ARE_TIED:
    CG_AddBufferedSound(cgs.media.teamsTiedSound);
    break;

#ifdef WITH_BAGMAN_MOD
  case GTS_DRAGON_DEPOSIT:
  case GTS_NIKKI_DEPOSIT:
    CG_AddBufferedSound(cgs.media.bmDepositSound);	//hypov8 add
    break;
  case GTS_DRAGON_STOLEN:
  case GTS_NIKKI_STOLEN:
    CG_AddBufferedSound(cgs.media.bmStolenSound);	//hypov8 add
    break;
#endif

#ifdef USE_KAMIKAZE
    case GTS_KAMIKAZE:
      trap_S_StartLocalSound(cgs.media.kamikazeFarSound, CHAN_ANNOUNCER);
      break;
#endif
  default:
    break;
    }
    break;
  }

  case EV_PAIN:
    // local player sounds are triggered in CG_CheckLocalSounds,
    // so ignore events on the player
    DEBUGNAME("EV_PAIN");
    if (cent->currentState.number != cg.snap->ps.clientNum)
      CG_PainEvent(cent, es->eventParm);
  else
      CG_PainEventSelf();
    break;

  case EV_DEATH1:
    DEBUGNAME("EV_DEATH1");
    trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "*death1.ogg")); //hypov8 match sounds to death animations
    break;
  case EV_DEATH2:
    DEBUGNAME("EV_DEATH2");
    trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "*death2.ogg"));
    break;
  case EV_DEATH3:
    DEBUGNAME("EV_DEATH3");
    trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "*death3.ogg"));
    break;

  case EV_OBITUARY:
    DEBUGNAME("EV_OBITUARY");
    CG_Obituary(es);
    break;

    // powerup events
#ifdef GT_USE_TA_TYPES
  case EV_POWERUP_QUAD:
    DEBUGNAME("EV_POWERUP_QUAD");
    if (es->number == cg.snap->ps.clientNum)
    {
      cg.powerupActive = PW_QUAD;
      cg.powerupTime   = cg.time;
    }
    trap_S_StartSound(NULL, es->number, CHAN_ITEM, cgs.media.quadSound);
    break;
  case EV_POWERUP_BATTLESUIT:
    DEBUGNAME("EV_POWERUP_BATTLESUIT");
    if (es->number == cg.snap->ps.clientNum)
    {
      cg.powerupActive = PW_BATTLESUIT;
      cg.powerupTime   = cg.time;
    }
    trap_S_StartSound(NULL, es->number, CHAN_ITEM, cgs.media.protectSound);
    break;
  case EV_POWERUP_REGEN:
    DEBUGNAME("EV_POWERUP_REGEN");
    if (es->number == cg.snap->ps.clientNum)
    {
      cg.powerupActive = PW_REGEN;
      cg.powerupTime   = cg.time;
    }
    trap_S_StartSound(NULL, es->number, CHAN_ITEM, cgs.media.regenSound);
    break;
#endif
  case EV_GIB_PLAYER:
    DEBUGNAME("EV_GIB_PLAYER");
    // don't play gib sound when using the kamikaze because it interferes
    // with the kamikaze sound, downside is that the gib sound will also
    // not be played when someone is gibbed while just carrying the kamikaze
#ifdef USE_KAMIKAZE
    if (!(es->eFlags & EF_KAMIKAZE))
      trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.gibSound);
#else
    trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.gibSound);
#endif
    CG_GibPlayer(cent->lerpOrigin);
    break;

  case EV_STOPLOOPINGSOUND:
    DEBUGNAME("EV_STOPLOOPINGSOUND");
    trap_S_StopLoopingSound(es->number);
    es->loopSound = 0;
    break;

  case EV_DEBUG_LINE:
    DEBUGNAME("EV_DEBUG_LINE");
    CG_Beam(cent);
    break;

  case EV_PLAYER_HIT_INFO:
    CG_HitInfo(es);
    break;

  case EV_FLAMEGUN:
    CG_FireFlameChunks(cent, position, cent->currentState.apos.trBase, 0.6f, 2);
    break;

  default:
    DEBUGNAME("UNKNOWN");
    CG_Error("Unknown event: %i", event);
    break;
  }
}

/*
==============
CG_CheckEvents
==============
*/
void CG_CheckEvents(centity_t *cent)
{
  // check for event-only entities
  if (cent->currentState.eType > ET_EVENTS)
  {
    if (cent->previousEvent)
      return; // already fired

    // if this is a player event set the entity number of the client entity number
    if (cent->currentState.eFlags & EF_PLAYER_EVENT)
      cent->currentState.number = cent->currentState.otherEntityNum;

    cent->previousEvent = 1;
    cent->currentState.event = cent->currentState.eType - ET_EVENTS;
  }
  else
  {
    // check for events riding with another entity
    if (cent->currentState.event == cent->previousEvent)
      return;

    cent->previousEvent = cent->currentState.event;
    if ((cent->currentState.event & ~EV_EVENT_BITS) == 0)
      return;
  }

  // calculate the position at exactly the frame time
  BG_EvaluateTrajectory(&cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin);
  CG_SetEntitySoundPosition(cent);

  CG_EntityEvent(cent, cent->lerpOrigin);
}

void CG_HitInfo(entityState_t *ent)
{
  int hitflags;
  int target, attacker;
//	char *message;
  const char *targetInfo;
  const char *attackerInfo;
  char targetName[32];
  char attackerName[32];
  //gender_t gender;
  clientInfo_t *ci;

  target   = ent->otherEntityNum;
  attacker = ent->otherEntityNum2;
  hitflags = ent->eventParm;

  if (target == attacker)
    return;

  if (target < 0 || target >= MAX_CLIENTS)
    CG_Error("CG_HitInfo: target out of range");

  ci = &cgs.clientinfo[target];
  (void)ci; // shutup compiler
  targetInfo = CG_ConfigString(CS_PLAYERS + target);
  if (!targetInfo)
    return;

  Q_strncpyz(targetName, Info_ValueForKey(targetInfo, "n"), sizeof(targetName) - 2);
  qstrcat(targetName, S_COLOR_WHITE);

  if (attacker < 0 || attacker >= MAX_CLIENTS)
  {
    attacker = ENTITYNUM_WORLD;
    attackerInfo = NULL;
  }
  else
    attackerInfo = CG_ConfigString(CS_PLAYERS + attacker);

  if (!attackerInfo)
  {
    attacker = ENTITYNUM_WORLD;
    qstrcpy(attackerName, "noname");
  }
  else
  {
    Q_strncpyz(attackerName, Info_ValueForKey(attackerInfo, "n"), sizeof(attackerName) - 2);
    qstrcat(attackerName, S_COLOR_WHITE);
    // check for kill messages about the current clientNum
//		if ( target == cg.snap->ps.clientNum ) {
//			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
//		}
  }

  //FIXME(0xA5EA): this is unimplemented
// Check the location ignoring the rotation info
  switch (hitflags & ~(LOCATION_BACK | LOCATION_LEFT | LOCATION_RIGHT | LOCATION_FRONT))
  {
  case LOCATION_HEAD:
    //Com_Printf("%s has been hit in the head by  %s\n", targetName,attackerName);
    CG_Printf("%s has been hit in the head by  %s\n", targetName, attackerName);
    break;

  case LOCATION_FACE:
    CG_Printf("%s has been hit in the face by  %s\n", targetName, attackerName);
    //Com_Printf("%s has been hit in the face by  %s\n", targetName,attackerName);

    break;
  case LOCATION_SHOULDER:
    CG_Printf("%s has been hit in the shoulder by  %s\n", targetName, attackerName);
    //Com_Printf("%s has been hit in the shoulder by  %s\n", targetName,attackerName);

    break;
  case LOCATION_CHEST:
    CG_Printf("%s has been hit in the chest by  %s\n", targetName, attackerName);
    //Com_Printf("%s has been hit in the chest by  %s\n", targetName,attackerName);

    break;
  case LOCATION_STOMACH:
    CG_Printf("%s has been hit in the stomach by  %s\n", targetName, attackerName);
    //Com_Printf("%s has been hit in the stomach by  %s\n", targetName,attackerName);

    break;
  case LOCATION_GROIN:
    CG_Printf("%s has been hit in the groin by  %s\n", targetName, attackerName);
    //Com_Printf("%s has been hit in the stomach by  %s\n", targetName,attackerName);

    break;
  case LOCATION_LEG:
    CG_Printf("%s has been hit in the leg by  %s\n", targetName, attackerName);
    //Com_Printf("%s has been hit in the leg by  %s\n", targetName,attackerName);

    break;
  case LOCATION_FOOT:
    CG_Printf("%s has been hit in the foot by  %s\n", targetName, attackerName);
    //Com_Printf("%s has been hit in the leg by  %s\n", targetName,attackerName);

    break;
    // we don't know what it was
  default:
    CG_Printf("%s was hit.\n", targetName);

  }
}
